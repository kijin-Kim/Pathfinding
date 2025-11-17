#include "PathfindingLayer.h"

#include "Core/Application.h"
#include "GLFW/glfw3.h"
#include "Pathfinding/PathfindingConfig.h"
#include "Renderer/Renderer.h"
#include "Renderer/ResourceManager.h"
#include "glm/ext/matrix_clip_space.hpp"

#include <iostream>
#include <queue>

void PathfindingLayer::OnInit()
{
	const Application::Settings& settings = Application::GetInstance().GetSettings();
	auto& framebufferManager = ResourceManager<Framebuffer>::GetInstance();
	framebufferManager.GetOrCreate("Viewport", settings.Width, settings.Height);
}
void PathfindingLayer::GenerateRandomWalls(int rowCount, int columnCount)
{
	for (int i = 0; i < rowCount * columnCount * 0.3f; ++i)
	{
		const int randRow = rand() % rowCount;
		const int randCol = rand() % columnCount;
		grid_[randRow][randCol].Type = ETileType::Wall;
	}
}
void PathfindingLayer::RebuildGrid(int rowCount, int columnCount, int startRow, int startColumn, int endRow,
								   int endColumn, EHeuristicMethod::Type method)
{

	grid_.clear();
	grid_.resize(rowCount, std::vector<Node>(columnCount));
	for (int row = 0; row < rowCount; ++row)
	{
		for (int column = 0; column < columnCount; ++column)
		{
			grid_[row][column].Row = row;
			grid_[row][column].Column = column;
		}
	}

	GenerateRandomWalls(rowCount, columnCount);

	grid_[startRow][startColumn].Type = ETileType::Path;
	grid_[endRow][endColumn].Type = ETileType::Path;
}
void PathfindingLayer::StepPathfinding(int rowCount, int columnCount, int endRow, int endColumn,
									   EHeuristicMethod::Type method)
{
	if (!openSet_.empty() && !bPathFound_)
	{
		Node* current = openSet_.top();
		openSet_.pop();
		if (current->bClosed)
		{
			return;
		}
		current->bClosed = true;
		if (current->Row == endRow && current->Column == endColumn)
		{
			bPathFound_ = true;
			return;
		}

		const bool bAllowDiagonals = method != EHeuristicMethod::Manhattan;
		const std::vector<Node*> neighbors
			= GetNeighbors(current->Row, current->Column, rowCount, columnCount, bAllowDiagonals);
		for (Node* neighbor : neighbors)
		{
			if (!neighbor->IsWalkable() || neighbor->bClosed)
			{
				continue;
			}
			int walkCost = GetWalkCost(neighbor->Type);
			float newCost = current->GCost + walkCost;
			if (newCost < neighbor->GCost)
			{
				neighbor->GCost = newCost;
				neighbor->HCost = CalculateHeuristicCost(neighbor->Row, neighbor->Column, endRow, endColumn, method);
				neighbor->Parent = current;
				openSet_.push(neighbor);
			}
		}
	}
}

void PathfindingLayer::DrawGridLines(Renderer& renderer, int rowCount, int columnCount, int cellSize)
{
	const float gridHalfWidth = columnCount * cellSize / 2.0f;
	const float gridHalfHeight = (rowCount * cellSize) / 2.0f;

	for (int column = 0; column <= columnCount; ++column)
	{
		const glm::vec2 start = glm::vec2(-gridHalfWidth, -gridHalfHeight + column * cellSize);
		const glm::vec2 end = glm::vec2(gridHalfWidth, -gridHalfHeight + column * cellSize);
		renderer.DrawLine(start, end, PathfindingConfig::Colors::GRID_LINE, PathfindingConfig::GRID_LINE_WIDTH);
	}
	for (int row = 0; row <= rowCount; ++row)
	{
		const glm::vec2 start = glm::vec2(-gridHalfWidth + row * cellSize, -gridHalfHeight);
		const glm::vec2 end = glm::vec2(-gridHalfWidth + row * cellSize, gridHalfHeight);
		renderer.DrawLine(start, end, PathfindingConfig::Colors::GRID_LINE, PathfindingConfig::GRID_LINE_WIDTH);
	}

	for (int row = 0; row < rowCount; ++row)
	{
		for (int column = 0; column < columnCount; ++column)
		{
			glm::vec2 center = GridToWorldPosition(row, column, rowCount, columnCount, cellSize);
			glm::vec2 leftTop = glm::vec2(center.x - cellSize * 0.5f, center.y + cellSize * 0.5f);
			glm::vec2 rightBottom = leftTop + glm::vec2(cellSize, -cellSize);
			glm::vec2 rightTop = glm::vec2(center.x + cellSize * 0.5f, center.y + cellSize * 0.5f);
			glm::vec2 leftBottom = rightTop + glm::vec2(-cellSize, -cellSize);

			renderer.DrawLine(leftTop, rightBottom, PathfindingConfig::Colors::GRID_LINE,
							  PathfindingConfig::GRID_LINE_WIDTH);
			renderer.DrawLine(rightTop, leftBottom, PathfindingConfig::Colors::GRID_LINE,
							  PathfindingConfig::GRID_LINE_WIDTH);
		}
	}
}
void PathfindingLayer::DrawCurrentPath(Renderer& renderer, int rowCount, int columnCount, int cellSize, int endRow,
									   int endColumn)
{
	Node* current = nullptr;
	if (bPathFound_)
	{
		current = &grid_[endRow][endColumn];
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

		renderer.DrawLine(GridToWorldPosition(currentRow, currentCol, rowCount, columnCount, cellSize),
						  GridToWorldPosition(parent->Row, parent->Column, rowCount, columnCount, cellSize),
						  PathfindingConfig::Colors::PATH_LINE, PathfindingConfig::PATH_LINE_WIDTH);
		currentRow = parent->Row;
		currentCol = parent->Column;
	}
}
void PathfindingLayer::DrawClosedNodes(Renderer& renderer, int rowCount, int columnCount, int cellSize)
{
	for (int row = 0; row < rowCount; ++row)
	{
		for (int column = 0; column < columnCount; ++column)
		{
			if (!grid_[row][column].bClosed)
			{
				continue;
			}

			glm::ivec2 position = GridToWorldPosition(row, column, rowCount, columnCount, cellSize);
			renderer.DrawRectangle(position, 0.0f, glm::vec2(cellSize, cellSize),
								   PathfindingConfig::Colors::CLOSED_NODE, false);
		}
	}
}

void PathfindingLayer::DrawOpenNodes(Renderer& renderer, int rowCount, int columnCount, int cellSize)
{
	std::priority_queue<Node*, std::vector<Node*>, NodeComparator> tempOpenSet = openSet_;
	while (!tempOpenSet.empty())
	{
		Node* node = tempOpenSet.top();
		tempOpenSet.pop();

		glm::ivec2 position = GridToWorldPosition(node->Row, node->Column, rowCount, columnCount, cellSize);
		renderer.DrawRectangle(position, 0.0f, glm::vec2(cellSize, cellSize), PathfindingConfig::Colors::OPEN_NODE,
							   false);
	}
}

void PathfindingLayer::OnUpdate(float deltaTime)
{
	if (bIsPaused_)
	{
		return;
	}

	std::shared_ptr<MapData> mapData = mapDataWeak_.lock();
	if (!mapData)
	{
		return;
	}

	float interval = PathfindingConfig::BASE_STEP_INTERVAL / mapData->SimulationSpeed;

	accumulatedTime_ += deltaTime;
	if (accumulatedTime_ >= interval)
	{
		StepPathfinding(mapData->RowCount, mapData->ColumnCount, mapData->EndRow, mapData->EndColumn,
						mapData->HeuristicMethod);
		accumulatedTime_ = 0.0f;
	}
}
void PathfindingLayer::DrawTiles(Renderer& renderer, int rowCount, int columnCount, int cellSize)
{

	for (int row = 0; row < rowCount; ++row)
	{

		for (int column = 0; column < columnCount; ++column)
		{
			const Node& node = grid_[row][column];
			const glm::ivec2 position = GridToWorldPosition(row, column, rowCount, columnCount, cellSize);
			renderer.DrawRectangle(position, 0.0f, glm::vec2(cellSize, cellSize), GetTileColor(node.Type), false);
		}
	}
}
void PathfindingLayer::DrawStartAndEnd(Renderer& renderer, int startRow, int startColumn, int endRow, int endColumn,
									   int rowCount, int columnCount, int cellSize)
{
	renderer.DrawRectangle(GridToWorldPosition(startRow, startColumn, rowCount, columnCount, cellSize), 0.0f,
						   glm::vec2(cellSize, cellSize), PathfindingConfig::Colors::START_NODE, true);
	renderer.DrawRectangle(GridToWorldPosition(endRow, endColumn, rowCount, columnCount, cellSize), 0.0f,
						   glm::vec2(cellSize, cellSize), PathfindingConfig::Colors::END_NODE, true);
}
void PathfindingLayer::OnRender(Renderer& renderer)
{
	auto& framebufferManager = ResourceManager<Framebuffer>::GetInstance();
	auto framebuffer = framebufferManager.Get("Viewport");

	renderer.BeginScene(framebuffer);
	glViewport(0, 0, framebuffer->GetWidth(), framebuffer->GetHeight());
	renderer.Clear(PathfindingConfig::Colors::CLEAR_COLOR);

	std::shared_ptr<MapData> mapData = mapDataWeak_.lock();
	if (!mapData)
	{
		renderer.EndScene();
		return;
	}

	const int rowCount = mapData->RowCount;
	const int columnCount = mapData->ColumnCount;
	const int cellSize = mapData->CellSize;

	const int startRow = mapData->StartRow;
	const int startColumn = mapData->StartColumn;
	const int endRow = mapData->EndRow;
	const int endColumn = mapData->EndColumn;

	DrawGridLines(renderer, rowCount, columnCount, cellSize);
	DrawTiles(renderer, rowCount, columnCount, cellSize);
	DrawStartAndEnd(renderer, startRow, startColumn, endRow, endColumn, rowCount, columnCount, cellSize);
	DrawClosedNodes(renderer, rowCount, columnCount, cellSize);
	DrawCurrentPath(renderer, rowCount, columnCount, cellSize, endRow, endColumn);
	DrawOpenNodes(renderer, rowCount, columnCount, cellSize);

	renderer.EndScene();
}

std::vector<Node*> PathfindingLayer::GetNeighbors(int row, int column, int rowCount, int columnCount,
												  bool bAllowDiagonals)
{
	std::vector<Node*> neighbors;
	neighbors.reserve(bAllowDiagonals ? 8 : 4);

	// 직교 방향 먼저 확인
	const bool bCanUp = row - 1 >= 0 && grid_[row - 1][column].IsWalkable();
	const bool bCanDown = row + 1 < rowCount && grid_[row + 1][column].IsWalkable();
	const bool bCanLeft = column - 1 >= 0 && grid_[row][column - 1].IsWalkable();
	const bool bCanRight = column + 1 < columnCount && grid_[row][column + 1].IsWalkable();

	// 직교 이웃 추가
	if (row - 1 >= 0)
	{
		neighbors.push_back(&grid_[row - 1][column]);
	}
	if (row + 1 < rowCount)
	{
		neighbors.push_back(&grid_[row + 1][column]);
	}
	if (column - 1 >= 0)
	{
		neighbors.push_back(&grid_[row][column - 1]);
	}
	if (column + 1 < columnCount)
	{
		neighbors.push_back(&grid_[row][column + 1]);
	}

	if (bAllowDiagonals)
	{
		// 대각선은 인접한 두 직교 방향이 모두 통과 가능할 때만
		if (bCanUp && bCanLeft && row - 1 >= 0 && column - 1 >= 0)
		{
			neighbors.push_back(&grid_[row - 1][column - 1]);
		}

		if (bCanUp && bCanRight && row - 1 >= 0 && column + 1 < columnCount)
		{
			neighbors.push_back(&grid_[row - 1][column + 1]);
		}

		if (bCanDown && bCanLeft && row + 1 < rowCount && column - 1 >= 0)
		{
			neighbors.push_back(&grid_[row + 1][column - 1]);
		}

		if (bCanDown && bCanRight && row + 1 < rowCount && column + 1 < columnCount)
		{
			neighbors.push_back(&grid_[row + 1][column + 1]);
		}
	}

	return neighbors;
}

glm::vec2 PathfindingLayer::GridToWorldPosition(int row, int column, int rowCount, int columnCount, int cellSize)
{
	const float gridHalfWidth = columnCount * cellSize / 2.0f;
	const float gridHalfHeight = rowCount * cellSize / 2.0f;

	return glm::vec2(-gridHalfWidth + column * cellSize + cellSize * 0.5f,
					 gridHalfHeight - row * cellSize - cellSize * 0.5f);
}

glm::vec4 PathfindingLayer::GetTileColor(ETileType type) const
{
	switch (type)
	{
	case ETileType::Path:
		return PathfindingConfig::Colors::PATH_TILE;
	case ETileType::Wall:
		return PathfindingConfig::Colors::WALL_TILE;
	default:
		return PathfindingConfig::Colors::PATH_TILE;
	}
}
float PathfindingLayer::GetWalkCost(ETileType type) const
{
	switch (type)
	{
	case ETileType::Path:
		return PathfindingConfig::ORTHOGONAL_COST;
	case ETileType::Wall:
		return PathfindingConfig::IMPASSABLE_COST;
	default:
		return PathfindingConfig::ORTHOGONAL_COST;
	}
}

float PathfindingLayer::CalculateHeuristicCost(int rowStart, int columnStart, int rowEnd, int columnEnd,
											   EHeuristicMethod::Type method) const
{
	const int deltaRow = abs(rowStart - rowEnd);
	const int deltaCol = abs(columnStart - columnEnd);
	switch (method)
	{
	case EHeuristicMethod::None:
		return 0.0f;
	case EHeuristicMethod::Manhattan:
		return static_cast<float>(deltaRow + deltaCol);
	case EHeuristicMethod::Euclidean:
		return sqrtf(static_cast<float>(deltaRow * deltaRow + deltaCol * deltaCol));
	case EHeuristicMethod::Octile:
		return static_cast<float>(std::min(deltaRow, deltaCol)) * PathfindingConfig::DIAGONAL_COST
			   + std::abs(static_cast<float>(deltaRow - deltaCol));
	default:
		return 0.0f;
	}
}
void PathfindingLayer::OnMapRefChanged(const std::weak_ptr<MapData>& weak)
{
	mapDataWeak_ = weak;
	if (std::shared_ptr<MapData> mapData = mapDataWeak_.lock())
	{
		RebuildGrid(mapData->RowCount, mapData->ColumnCount, mapData->StartRow, mapData->StartColumn, mapData->EndRow,
					mapData->EndColumn, mapData->HeuristicMethod);
		ResetPathfinding(mapData->StartRow, mapData->StartColumn, mapData->EndRow, mapData->EndColumn,
						 mapData->HeuristicMethod);
	}
}

void PathfindingLayer::OnStartEvent()
{
	bIsPaused_ = false;
}
void PathfindingLayer::OnPauseEvent()
{
	bIsPaused_ = true;
}
void PathfindingLayer::ResetPathfinding(int startRow, int startColumn, int endRow, int endColumn,
										EHeuristicMethod::Type method)
{
	bPathFound_ = false;

	for (std::vector<Node>& row : grid_)
	{
		for (Node& node : row)
		{
			node.Reset();
		}
	}

	Node& start = grid_[startRow][startColumn];
	start.GCost = 0;
	start.HCost = CalculateHeuristicCost(startRow, startColumn, endRow, endColumn, method);
	openSet_ = std::priority_queue<Node*, std::vector<Node*>, NodeComparator>();
	openSet_.push(&start);
}
void PathfindingLayer::OnResetEvent()
{
	if (std::shared_ptr<MapData> mapData = mapDataWeak_.lock())
	{
		ResetPathfinding(mapData->StartRow, mapData->StartColumn, mapData->EndRow, mapData->EndColumn,
						 mapData->HeuristicMethod);
	}
}
void PathfindingLayer::OnStepEvent()
{
	if (std::shared_ptr<MapData> mapData = mapDataWeak_.lock())
	{
		StepPathfinding(mapData->RowCount, mapData->ColumnCount, mapData->EndRow, mapData->EndColumn,
						mapData->HeuristicMethod);
	}
}
void PathfindingLayer::OnRebuildEvent()
{
	if (std::shared_ptr<MapData> mapData = mapDataWeak_.lock())
	{
		RebuildGrid(mapData->RowCount, mapData->ColumnCount, mapData->StartRow, mapData->StartColumn, mapData->EndRow,
					mapData->EndColumn, mapData->HeuristicMethod);
		ResetPathfinding(mapData->StartRow, mapData->StartColumn, mapData->EndRow, mapData->EndColumn,
						 mapData->HeuristicMethod);
	}
}
