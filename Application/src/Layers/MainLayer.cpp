#include "MainLayer.h"

#include "Renderer/Renderer.h"

#include <queue>

void MainLayer::RebuildGridAndOpenSet()
{
	grid_.clear();
	grid_.resize(GRID_ROWS_, std::vector<Node>(GRID_COLUMNS_));
	for (int row = 0; row < GRID_ROWS_; ++row)
	{
		for (int col = 0; col < GRID_COLUMNS_; ++col)
		{
			Node& node = grid_[row][col];
			node.Row = row;
			node.Column = col;
		}
	}

	// grid_[START_ROW_ - 2][START_COLUMN_ + 3].Type = ETileType::Wall;
	// grid_[START_ROW_ - 1][START_COLUMN_ + 3].Type = ETileType::Wall;
	// grid_[START_ROW_][START_COLUMN_ + 3].Type = ETileType::Wall;
	// grid_[START_ROW_ + 1][START_COLUMN_ + 3].Type = ETileType::Wall;
	// grid_[START_ROW_ + 2][START_COLUMN_ + 3].Type = ETileType::Wall;
	for (int i = 0; i < GRID_ROWS_ * GRID_COLUMNS_ * 0.3f; ++i)
	{
		int randRow = rand() % GRID_ROWS_;
		int randCol = rand() % GRID_COLUMNS_;
		grid_[randRow][randCol].Type = ETileType::Wall;
	}

	grid_[START_ROW_][START_COLUMN_].Type = ETileType::Path;
	grid_[GOAL_ROW_][GOAL_COLUMN_].Type = ETileType::Path;

	Node& start = grid_[START_ROW_][START_COLUMN_];
	start.GCost = 0;
	start.HCost = HeuristicCost(START_ROW_, START_COLUMN_, GOAL_ROW_, GOAL_COLUMN_, heuristicMethod_);
	openSet_ = std::priority_queue<Node*, std::vector<Node*>, decltype(comp_)>(comp_);
	openSet_.push(&start);
}
void MainLayer::StepPathfinding()
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
		if (current->Row == GOAL_ROW_ && current->Column == GOAL_COLUMN_)
		{
			bPathFound_ = true;
			return;
		}

		const bool bAllowDiagonals = heuristicMethod_ != EHeuristicMethod::Manhattan;
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
				neighbor->HCost
					= HeuristicCost(neighbor->Row, neighbor->Column, GOAL_ROW_, GOAL_COLUMN_, heuristicMethod_);
				neighbor->Parent = current;
				openSet_.push(neighbor);
			}
		}
	}
	//
	// for (int row = 0; row < GRID_ROWS_; ++row)
	// {
	// 	for (int col = 0; col < GRID_COLUMNS_; ++col)
	// 	{
	// 		const Node& node = grid_[row][col];
	//
	// 		if (node.GCost != std::numeric_limits<float>::max())
	// 		{
	// 			std::cout << node.GCost << "\t";
	// 		}
	// 		else
	// 		{
	// 			std::cout << "X\t";
	// 		}
	// 	}
	// 	std::cout << std::endl;
	// }
}
MainLayer::MainLayer()
{
	RebuildGridAndOpenSet();
}

void MainLayer::DrawGrid(Renderer& renderer)
{
	constexpr glm::vec4 GRID_COLOR = glm::vec4(0.5f, 0.5f, 0.5f, 0.3f);
	for (int col = 0; col <= GRID_COLUMNS_; ++col)
	{
		const glm::vec2 start = glm::vec2(-GRID_HALF_WIDTH_, -GRID_HALF_HEIGHT_ + col * CELL_SIZE_);
		const glm::vec2 end = glm::vec2(GRID_HALF_WIDTH_, -GRID_HALF_HEIGHT_ + col * CELL_SIZE_);
		renderer.DrawLine(start, end, GRID_COLOR, 1.0f);
	}
	for (int row = 0; row <= GRID_ROWS_; ++row)
	{
		const glm::vec2 start = glm::vec2(-GRID_HALF_WIDTH_ + row * CELL_SIZE_, -GRID_HALF_HEIGHT_);
		const glm::vec2 end = glm::vec2(-GRID_HALF_WIDTH_ + row * CELL_SIZE_, GRID_HALF_HEIGHT_);
		renderer.DrawLine(start, end, GRID_COLOR, 1.0f);
	}

	for (int row = 0; row < GRID_ROWS_; ++row)
	{
		for (int col = 0; col < GRID_COLUMNS_; ++col)
		{
			glm::vec2 center = IndexToCenterPosition(row, col);
			glm::vec2 leftTop = glm::vec2(center.x - HALF_CELL_SIZE_, center.y + HALF_CELL_SIZE_);
			glm::vec2 rightBottom = leftTop + glm::vec2(CELL_SIZE_, -CELL_SIZE_);
			renderer.DrawLine(leftTop, rightBottom, GRID_COLOR, 1.0f);
			glm::vec2 rightTop = glm::vec2(center.x + HALF_CELL_SIZE_, center.y + HALF_CELL_SIZE_);
			glm::vec2 leftBottom = rightTop + glm::vec2(-CELL_SIZE_, -CELL_SIZE_);
			renderer.DrawLine(rightTop, leftBottom, GRID_COLOR, 1.0f);
		}
	}
}
void MainLayer::DrawPath(Renderer& renderer)
{
	static constexpr glm::vec4 PATH_COLOR = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	Node* current = openSet_.empty() ? &grid_[GOAL_ROW_][GOAL_COLUMN_] : openSet_.top();
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
void MainLayer::DrawClosed(Renderer& renderer)
{
	constexpr glm::vec4 CLOSED_COLOR = glm::vec4(0.27f, 0.68f, 0.73f, 0.3f);
	for (int row = 0; row < GRID_ROWS_; ++row)
	{
		for (int col = 0; col < GRID_COLUMNS_; ++col)
		{
			if (!grid_[row][col].bClosed)
			{
				continue;
			}

			glm::ivec2 position = IndexToCenterPosition(row, col);
			renderer.DrawRectangle(position, 0.0f, glm::vec2(CELL_SIZE_, CELL_SIZE_), CLOSED_COLOR, false);
		}
	}
}

void MainLayer::DrawOpen(Renderer& renderer)
{
	constexpr glm::vec4 OPEN_COLOR = glm::vec4(0.0f, 1.0f, 1.0f, 0.3f);
	std::priority_queue<Node*, std::vector<Node*>, decltype(comp_)> tempOpenSet = openSet_;
	while (!tempOpenSet.empty())
	{
		Node* node = tempOpenSet.top();
		tempOpenSet.pop();

		glm::ivec2 position = IndexToCenterPosition(node->Row, node->Column);
		renderer.DrawRectangle(position, 0.0f, glm::vec2(CELL_SIZE_, CELL_SIZE_), OPEN_COLOR, false);
	}
}

void MainLayer::OnUpdate(float deltaTime)
{
	static float interval = 0.01f;
	accumulatedTime_ += deltaTime;
	if (accumulatedTime_ >= interval)
	{
		StepPathfinding();
		accumulatedTime_ = 0.0f;
	}
}
void MainLayer::OnRender(Renderer& renderer)
{
	renderer.BeginScene();
	DrawGrid(renderer);

	for (int row = 0; row < GRID_ROWS_; ++row)
	{
		for (int col = 0; col < GRID_COLUMNS_; ++col)
		{
			const Node& node = grid_[row][col];
			glm::ivec2 position = IndexToCenterPosition(row, col);
			renderer.DrawRectangle(position, 0.0f, glm::vec2(CELL_SIZE_, CELL_SIZE_), GetTileColor(node.Type), false);
		}
	}

	renderer.DrawRectangle(IndexToCenterPosition(START_ROW_, START_COLUMN_), 0.0f, glm::vec2(CELL_SIZE_, CELL_SIZE_),
						   glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), true);
	renderer.DrawRectangle(IndexToCenterPosition(GOAL_ROW_, GOAL_COLUMN_), 0.0f, glm::vec2(CELL_SIZE_, CELL_SIZE_),
						   glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), true);

	DrawClosed(renderer);
	DrawPath(renderer);
	DrawOpen(renderer);

	renderer.EndScene();
}
std::vector<Node*> MainLayer::GetNeighbors(int row, int col, bool allowDiagonals)
{
	std::vector<Node*> neighbors;
	constexpr static int directions[8][2] = {
		{-1, 0},  // Up
		{1, 0},	  // Down
		{0, -1},  // Left
		{0, 1},	  // Right
		{-1, -1}, // Up-Left
		{-1, 1},  // Up-Right
		{1, -1},  // Down-Left
		{1, 1}	  // Down-Right
	};

	int numDirections = allowDiagonals ? 8 : 4;

	for (int i = 0; i < numDirections; ++i)
	{
		int newRow = row + directions[i][0];
		int newCol = col + directions[i][1];

		if (newRow >= 0 && newRow < GRID_ROWS_ && newCol >= 0 && newCol < GRID_COLUMNS_)
		{
			neighbors.push_back(&grid_[newRow][newCol]);
		}
	}

	return neighbors;
}

glm::vec2 MainLayer::IndexToCenterPosition(int row, int col)
{
	return glm::vec2(-GRID_HALF_WIDTH_ + col * CELL_SIZE_ + HALF_CELL_SIZE_,
					 GRID_HALF_HEIGHT_ - row * CELL_SIZE_ - HALF_CELL_SIZE_);
}

glm::vec4 MainLayer::GetTileColor(ETileType type)
{
	switch (type)
	{
	case ETileType::Path:
		return glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
	case ETileType::Wall:
		return glm::vec4(1.0f, 0.0f, 0.0f, 0.3f);
	case ETileType::Water:
		return glm::vec4(0.0f, 0.0f, 1.0f, 0.3f);
	case ETileType::Sand:
		return glm::vec4(1.0f, 1.0f, 0.0f, 0.3f);
	case ETileType::Forest:
		return glm::vec4(0.0f, 0.5f, 0.0f, 0.3f);
	default:
		return glm::vec4(1.0f, 1.0f, 1.0f, 0.3f);
	}
}
float MainLayer::GetTileCost(ETileType type)
{
	switch (type)
	{
	case ETileType::Path:
		return 1.0f;
	case ETileType::Wall:
		return std::numeric_limits<float>::max();
	case ETileType::Water:
		return 5.0f;
	case ETileType::Sand:
		return 3.0f;
	case ETileType::Forest:
		return 4.0f;
	default:
		return 1.0f;
	}
}
float MainLayer::HeuristicCost(int rowA, int colA, int rowB, int colB, EHeuristicMethod method)
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
