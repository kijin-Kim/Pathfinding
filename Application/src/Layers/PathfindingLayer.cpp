#include "PathfindingLayer.h"

#include "Core/Application.h"
#include "GLFW/glfw3.h"
#include "Renderer/Renderer.h"
#include "Renderer/ResourceManager.h"
#include "glm/ext/matrix_clip_space.hpp"

#include <iostream>
#include <queue>

PathfindingLayer::PathfindingLayer()
{
	RebuildGridAndOpenSet();
}
void PathfindingLayer::OnInit()
{

	const Application::Settings& settings = Application::GetInstance().GetSettings();
	ResourceManager<Framebuffer>& framebufferManager = ResourceManager<Framebuffer>::GetInstance();
	std::shared_ptr<Framebuffer> framebuffer
		= framebufferManager.GetOrCreate("Viewport", settings.Width, settings.Height);
}
void PathfindingLayer::RebuildGridAndOpenSet()
{
	std::shared_ptr<MapData> mapData = mapDataWeak_.lock();
	if (!mapData)
	{
		return;
	}

	grid_.clear();
	grid_.resize(mapData->Rows, std::vector<Node>(mapData->Columns));
	for (int row = 0; row < mapData->Rows; ++row)
	{
		for (int col = 0; col < mapData->Columns; ++col)
		{
			Node& node = grid_[row][col];
			node.Row = row;
			node.Column = col;
		}
	}

	for (int i = 0; i < mapData->Rows * mapData->Columns * 0.3f; ++i)
	{
		int randRow = rand() % mapData->Rows;
		int randCol = rand() % mapData->Columns;
		grid_[randRow][randCol].Type = ETileType::Wall;
	}

	grid_[mapData->StartRow][mapData->StartColumn].Type = ETileType::Path;
	grid_[mapData->EndRow][mapData->EndColumn].Type = ETileType::Path;

	Node& start = grid_[mapData->StartRow][mapData->StartColumn];
	start.GCost = 0;
	start.HCost = HeuristicCost(mapData->StartRow, mapData->StartColumn, mapData->EndRow, mapData->EndColumn,
								mapData->HeuristicMethod);
	openSet_ = std::priority_queue<Node*, std::vector<Node*>, decltype(comp_)>(comp_);
	openSet_.push(&start);
}
void PathfindingLayer::StepPathfinding()
{
	std::shared_ptr<MapData> mapData = mapDataWeak_.lock();
	if (!mapData)
	{
		return;
	}

	if (!openSet_.empty() && !bPathFound_)
	{
		Node* current = openSet_.top();
		openSet_.pop();
		if (current->bClosed)
		{
			return;
		}
		current->bClosed = true;
		if (current->Row == mapData->EndRow && current->Column == mapData->EndColumn)
		{
			bPathFound_ = true;
			return;
		}

		const bool bAllowDiagonals = mapData->HeuristicMethod != EHeuristicMethod::Manhattan;
		std::vector<Node*> neighbors = GetNeighbors(current->Row, current->Column, bAllowDiagonals);
		for (Node* neighbor : neighbors)
		{
			if (!neighbor->IsWalkable() || neighbor->bClosed)
			{
				continue;
			}
			int walkCost = GetTileCost(neighbor->Type);
			float newCost = current->GCost + walkCost;
			if (newCost < neighbor->GCost)
			{
				neighbor->GCost = newCost;
				neighbor->HCost = HeuristicCost(neighbor->Row, neighbor->Column, mapData->EndRow, mapData->EndColumn,
												mapData->HeuristicMethod);
				neighbor->Parent = current;
				openSet_.push(neighbor);
			}
		}
	}
}

void PathfindingLayer::DrawGrid(Renderer& renderer)
{
	std::shared_ptr<MapData> mapData = mapDataWeak_.lock();
	if (!mapData)
	{
		return;
	}

	const float gridHalfWidth = (mapData->Columns * mapData->CellSize) / 2.0f;
	const float gridHalfHeight = (mapData->Rows * mapData->CellSize) / 2.0f;

	constexpr glm::vec4 GRID_COLOR = glm::vec4(0.5f, 0.5f, 0.5f, 0.3f);
	for (int col = 0; col <= mapData->Columns; ++col)
	{
		const glm::vec2 start = glm::vec2(-gridHalfWidth, -gridHalfHeight + col * mapData->CellSize);
		const glm::vec2 end = glm::vec2(gridHalfWidth, -gridHalfHeight + col * mapData->CellSize);
		renderer.DrawLine(start, end, GRID_COLOR, 1.0f);
	}
	for (int row = 0; row <= mapData->Rows; ++row)
	{
		const glm::vec2 start = glm::vec2(-gridHalfWidth + row * mapData->CellSize, -gridHalfHeight);
		const glm::vec2 end = glm::vec2(-gridHalfWidth + row * mapData->CellSize, gridHalfHeight);
		renderer.DrawLine(start, end, GRID_COLOR, 1.0f);
	}

	for (int row = 0; row < mapData->Rows; ++row)
	{
		for (int col = 0; col < mapData->Columns; ++col)
		{
			glm::vec2 center = IndexToCenterPosition(row, col);
			glm::vec2 leftTop = glm::vec2(center.x - mapData->CellSize * 0.5f, center.y + mapData->CellSize * 0.5f);
			glm::vec2 rightBottom = leftTop + glm::vec2(mapData->CellSize, -mapData->CellSize);
			renderer.DrawLine(leftTop, rightBottom, GRID_COLOR, 1.0f);
			glm::vec2 rightTop = glm::vec2(center.x + mapData->CellSize * 0.5f, center.y + mapData->CellSize * 0.5f);
			glm::vec2 leftBottom = rightTop + glm::vec2(-mapData->CellSize, -mapData->CellSize);
			renderer.DrawLine(rightTop, leftBottom, GRID_COLOR, 1.0f);
		}
	}
}
void PathfindingLayer::DrawPath(Renderer& renderer)
{
	std::shared_ptr<MapData> mapData = mapDataWeak_.lock();
	if (!mapData)
	{
		return;
	}

	static constexpr glm::vec4 PATH_COLOR = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	Node* current = nullptr;
	if (bPathFound_)
	{
		current = &grid_[mapData->EndRow][mapData->EndColumn];
	}
	else
	{
		if (openSet_.empty())
		{
			return;
		}
		current = openSet_.top();
	}

	int currentRow = current->Row;
	int currentCol = current->Column;
	while (Node* parent = grid_[currentRow][currentCol].Parent)
	{
		renderer.DrawLine(IndexToCenterPosition(currentRow, currentCol),
						  IndexToCenterPosition(parent->Row, parent->Column), PATH_COLOR, 3.0f);
		currentRow = parent->Row;
		currentCol = parent->Column;
	}
}
void PathfindingLayer::DrawClosed(Renderer& renderer)
{
	std::shared_ptr<MapData> mapData = mapDataWeak_.lock();
	if (!mapData)
	{
		return;
	}

	constexpr glm::vec4 CLOSED_COLOR = glm::vec4(0.27f, 0.68f, 0.73f, 0.3f);
	for (int row = 0; row < mapData->Rows; ++row)
	{
		for (int col = 0; col < mapData->Columns; ++col)
		{
			if (!grid_[row][col].bClosed)
			{
				continue;
			}

			glm::ivec2 position = IndexToCenterPosition(row, col);
			renderer.DrawRectangle(position, 0.0f, glm::vec2(mapData->CellSize, mapData->CellSize), CLOSED_COLOR,
								   false);
		}
	}
}

void PathfindingLayer::DrawOpen(Renderer& renderer)
{
	std::shared_ptr<MapData> mapData = mapDataWeak_.lock();
	if (!mapData)
	{
		return;
	}

	constexpr glm::vec4 OPEN_COLOR = glm::vec4(0.0f, 1.0f, 1.0f, 0.3f);
	std::priority_queue<Node*, std::vector<Node*>, decltype(comp_)> tempOpenSet = openSet_;
	while (!tempOpenSet.empty())
	{
		Node* node = tempOpenSet.top();
		tempOpenSet.pop();

		glm::ivec2 position = IndexToCenterPosition(node->Row, node->Column);
		renderer.DrawRectangle(position, 0.0f, glm::vec2(mapData->CellSize, mapData->CellSize), OPEN_COLOR, false);
	}
}

void PathfindingLayer::OnUpdate(float deltaTime)
{
	static float interval = 0.01f;
	accumulatedTime_ += deltaTime;
	if (accumulatedTime_ >= interval && !bIsPaused_)
	{
		StepPathfinding();
		accumulatedTime_ = 0.0f;
	}
}
void PathfindingLayer::OnRender(Renderer& renderer)
{
	ResourceManager<Framebuffer>& framebufferManager = ResourceManager<Framebuffer>::GetInstance();
	std::shared_ptr<Framebuffer> framebuffer = framebufferManager.Get("Viewport");
	renderer.BeginScene(framebuffer);
	glViewport(0, 0, framebuffer->GetWidth(), framebuffer->GetHeight());
	renderer.Clear(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
	std::shared_ptr<MapData> mapData = mapDataWeak_.lock();
	if (!mapData)
	{
		renderer.EndScene();
		return;
	}

	DrawGrid(renderer);
	for (int row = 0; row < mapData->Rows; ++row)
	{
		for (int col = 0; col < mapData->Columns; ++col)
		{
			const Node& node = grid_[row][col];
			glm::ivec2 position = IndexToCenterPosition(row, col);
			renderer.DrawRectangle(position, 0.0f, glm::vec2(mapData->CellSize, mapData->CellSize),
								   GetTileColor(node.Type), false);
		}
	}

	renderer.DrawRectangle(IndexToCenterPosition(mapData->StartRow, mapData->StartColumn), 0.0f,
						   glm::vec2(mapData->CellSize, mapData->CellSize), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), true);
	renderer.DrawRectangle(IndexToCenterPosition(mapData->EndRow, mapData->EndColumn), 0.0f,
						   glm::vec2(mapData->CellSize, mapData->CellSize), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), true);

	DrawClosed(renderer);
	DrawPath(renderer);
	DrawOpen(renderer);

	double mouseX = viewportCursorX_;
	double mouseY = viewportCursorY_;

	glm::mat4 projection = renderer.GetProjectionMatrix();
	mouseX = mouseX / framebuffer->GetWidth() * 2.0f - 1.0f;
	mouseY = 1.0f - mouseY / framebuffer->GetHeight() * 2.0f;

	glm::vec4 mousePosNDC = glm::vec4(static_cast<float>(mouseX), static_cast<float>(mouseY), 0.0f, 1.0f);
	glm::vec4 worldPos = glm::inverse(projection) * mousePosNDC;

	const float gridHalfWidth = (mapData->Columns * mapData->CellSize) / 2.0f;
	const float gridHalfHeight = (mapData->Rows * mapData->CellSize) / 2.0f;

	cursorPos_ = glm::vec2(worldPos.x, worldPos.y);
	int col = static_cast<int>((cursorPos_.x + gridHalfWidth) / mapData->CellSize);
	int row = static_cast<int>((gridHalfHeight - cursorPos_.y) / mapData->CellSize);
	if (col >= 0 && col < mapData->Columns && row >= 0 && row < mapData->Rows)
	{

		if (selectedTileSetImage_)
		{
			renderer.DrawTexturedRectangle(IndexToCenterPosition(row, col), 0.0f,
										   glm::vec2(mapData->CellSize, mapData->CellSize), selectedTileSetImage_,
										   selectedTileRegion_);
		}
	}

	renderer.EndScene();
}
bool PathfindingLayer::OnMouseButtonEvent(int button, int action, int mods)
{
	std::shared_ptr<MapData> mapData = mapDataWeak_.lock();
	if (!mapData)
	{
		return false;
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		ResourceManager<Framebuffer>& framebufferManager = ResourceManager<Framebuffer>::GetInstance();
		std::shared_ptr<Framebuffer> framebuffer = framebufferManager.Get("Viewport");

		const float gridHalfWidth = (mapData->Columns * mapData->CellSize) / 2.0f;
		const float gridHalfHeight = (mapData->Rows * mapData->CellSize) / 2.0f;
		int col = static_cast<int>((cursorPos_.x + gridHalfWidth) / mapData->CellSize);
		int row = static_cast<int>((gridHalfHeight - cursorPos_.y) / mapData->CellSize);
		if (row >= 0 && row < mapData->Rows && col >= 0 && col < mapData->Columns)
		{
			Node& node = grid_[row][col];
			if (node.Type == ETileType::Wall)
			{
				node.Type = ETileType::Path;
			}
			else if (node.Type == ETileType::Path)
			{
				node.Type = ETileType::Wall;
			}
		}
	}

	return true;
}
std::vector<Node*> PathfindingLayer::GetNeighbors(int row, int col, bool allowDiagonals)
{
	std::shared_ptr<MapData> mapData = mapDataWeak_.lock();
	if (!mapData)
	{
		return {};
	}

	std::vector<Node*> neighbors;

	// 직교 방향 먼저 확인
	bool canUp = (row - 1 >= 0) && grid_[row - 1][col].IsWalkable();
	bool canDown = (row + 1 < mapData->Rows) && grid_[row + 1][col].IsWalkable();
	bool canLeft = (col - 1 >= 0) && grid_[row][col - 1].IsWalkable();
	bool canRight = (col + 1 < mapData->Columns) && grid_[row][col + 1].IsWalkable();

	// 직교 이웃 추가
	if (row - 1 >= 0)
	{
		neighbors.push_back(&grid_[row - 1][col]);
	}
	if (row + 1 < mapData->Rows)
	{
		neighbors.push_back(&grid_[row + 1][col]);
	}
	if (col - 1 >= 0)
	{
		neighbors.push_back(&grid_[row][col - 1]);
	}
	if (col + 1 < mapData->Columns)
	{
		neighbors.push_back(&grid_[row][col + 1]);
	}

	if (allowDiagonals)
	{
		// 대각선은 인접한 두 직교 방향이 모두 통과 가능할 때만
		if (canUp && canLeft && row - 1 >= 0 && col - 1 >= 0)
		{
			neighbors.push_back(&grid_[row - 1][col - 1]);
		}

		if (canUp && canRight && row - 1 >= 0 && col + 1 < mapData->Columns)
		{
			neighbors.push_back(&grid_[row - 1][col + 1]);
		}

		if (canDown && canLeft && row + 1 < mapData->Rows && col - 1 >= 0)
		{
			neighbors.push_back(&grid_[row + 1][col - 1]);
		}

		if (canDown && canRight && row + 1 < mapData->Rows && col + 1 < mapData->Columns)
		{
			neighbors.push_back(&grid_[row + 1][col + 1]);
		}
	}

	return neighbors;
}

glm::vec2 PathfindingLayer::IndexToCenterPosition(int row, int col)
{
	std::shared_ptr<MapData> mapData = mapDataWeak_.lock();
	if (!mapData)
	{
		return glm::vec2(0.0f, 0.0f);
	}
	const float gridHalfWidth = (mapData->Columns * mapData->CellSize) / 2.0f;
	const float gridHalfHeight = (mapData->Rows * mapData->CellSize) / 2.0f;

	return glm::vec2(-gridHalfWidth + col * mapData->CellSize + mapData->CellSize * 0.5f,
					 gridHalfHeight - row * mapData->CellSize - mapData->CellSize * 0.5f);
}

glm::vec4 PathfindingLayer::GetTileColor(ETileType type)
{
	switch (type)
	{
	case ETileType::Path:
		return glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
	case ETileType::Wall:
		return glm::vec4(1.0f, 0.0f, 0.0f, 0.3f);
	default:
		return glm::vec4(1.0f, 1.0f, 1.0f, 0.3f);
	}
}
float PathfindingLayer::GetTileCost(ETileType type)
{
	switch (type)
	{
	case ETileType::Path:
		return 1.0f;
	case ETileType::Wall:
		return std::numeric_limits<float>::max();
	default:
		return 1.0f;
	}
}
float PathfindingLayer::HeuristicCost(int rowA, int colA, int rowB, int colB, EHeuristicMethod::Type method)
{
	const int deltaRow = abs(rowA - rowB);
	const int deltaCol = abs(colA - colB);
	switch (method)
	{
	case EHeuristicMethod::None:
		return 0.0f;
	case EHeuristicMethod::Manhattan:
		return static_cast<float>(deltaRow + deltaCol);
	case EHeuristicMethod::Euclidean:
		return sqrtf(static_cast<float>(deltaRow * deltaRow + deltaCol * deltaCol));
	case EHeuristicMethod::Octile:
		return static_cast<float>(std::min(deltaRow, deltaCol)) * 1.4142f
			   + std::abs(static_cast<float>(deltaRow - deltaCol));
	default:
		return 0.0f;
	}
}
void PathfindingLayer::OnViewportCursorPositionChanged(float x, float y)
{
	viewportCursorX_ = x;
	viewportCursorY_ = y;
	//	std::cout << "Viewport Cursor Position: (" << x << ", " << y << ")" << std::endl;
}
void PathfindingLayer::OnMapDataRefChanged(const std::weak_ptr<MapData>& mapDataWeak)
{
	mapDataWeak_ = mapDataWeak;
	RebuildGridAndOpenSet();
}
void PathfindingLayer::OnTileSelected(const std::shared_ptr<ImageTexture>& imageTexture,
									  const Renderer::TextureRegion& textureRegion)
{
	selectedTileSetImage_ = imageTexture;
	selectedTileRegion_ = textureRegion;
}
void PathfindingLayer::OnStartButtonClicked()
{
	bIsPaused_ = false;
}
void PathfindingLayer::OnStopButtonClicked()
{
	bIsPaused_ = true;
}
void PathfindingLayer::OnResetButtonClicked()
{
	bIsPaused_ = true;
	bPathFound_ = false;

	std::shared_ptr<MapData> mapData = mapDataWeak_.lock();
	if (!mapData)
	{
		return;
	}

	for (int row = 0; row < mapData->Rows; ++row)
	{
		for (int col = 0; col < mapData->Columns; ++col)
		{
			Node& node = grid_[row][col];
			node.GCost = std::numeric_limits<float>::max();
			node.HCost = 0.0f;
			node.Parent = nullptr;
			node.bClosed = false;
		}
	}

	Node& start = grid_[mapData->StartRow][mapData->StartColumn];
	start.GCost = 0;
	start.HCost = HeuristicCost(mapData->StartRow, mapData->StartColumn, mapData->EndRow, mapData->EndColumn,
								mapData->HeuristicMethod);
	openSet_ = std::priority_queue<Node*, std::vector<Node*>, decltype(comp_)>(comp_);
	openSet_.push(&start);
}
void PathfindingLayer::OnStepButtonClicked()
{
	StepPathfinding();
}
