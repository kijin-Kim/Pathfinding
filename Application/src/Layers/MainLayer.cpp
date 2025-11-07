#include "MainLayer.h"

#include "Renderer/Renderer.h"

#include <iostream>
#include <queue>

MainLayer::MainLayer()
{
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

	for (int i = 0; i < GRID_ROWS_ * GRID_COLUMNS_ * 0.2f; ++i)
	{
		int row = rand() % GRID_ROWS_;
		int col = rand() % GRID_COLUMNS_;
		grid_[row][col].Type = ETileType::Wall;
	}

	grid_[0][0].Type = ETileType::Path;
	grid_[GOAL_.x][GOAL_.y].Type = ETileType::Path;

	std::vector<std::vector<int>> costMap(GRID_ROWS_, std::vector<int>(GRID_COLUMNS_, std::numeric_limits<int>::max()));
	std::vector<std::vector<bool>> closedSet(GRID_ROWS_, std::vector<bool>(GRID_COLUMNS_, false));
	std::vector<std::vector<Node*>> cameFrom(GRID_ROWS_, std::vector<Node*>(GRID_COLUMNS_, nullptr));
	costMap[0][0] = 0;
	auto comp = [&costMap](const glm::ivec2& a, const glm::ivec2& b) { return costMap[a.x][a.y] > costMap[b.x][b.y]; };
	std::priority_queue<glm::ivec2, std::vector<glm::ivec2>, decltype(comp)> openSet(comp);

	openSet.push({0, 0});
	while (!openSet.empty())
	{
		glm::ivec2 pair = openSet.top();
		openSet.pop();
		int currentRow = pair.x;
		int currentCol = pair.y;
		if (closedSet[currentRow][currentCol])
		{
			continue;
		}
		closedSet[currentRow][currentCol] = true;
		if (currentRow == GOAL_.x && currentCol == GOAL_.y)
		{
			break;
		}

		std::vector<Node*> neighbors = GetNeighbors(currentRow, currentCol);
		for (Node* neighbor : neighbors)
		{
			if (!neighbor->IsWalkable())
			{
				continue;
			}
			int walkCost = GetTileCost(neighbor->Type);
			int newCost = costMap[currentRow][currentCol] + walkCost;
			if (newCost < costMap[neighbor->Row][neighbor->Column])
			{
				costMap[neighbor->Row][neighbor->Column] = newCost;
				cameFrom[neighbor->Row][neighbor->Column] = &grid_[currentRow][currentCol];
				openSet.push({neighbor->Row, neighbor->Column});
			}
		}
	}

	int currentRow = GOAL_.x;
	int currentCol = GOAL_.y;
	while (cameFrom[currentRow][currentCol] != nullptr)
	{
		Node* fromNode = cameFrom[currentRow][currentCol];
		glm::ivec2 fromPos = {fromNode->Row, fromNode->Column};
		glm::ivec2 toPos = {currentRow, currentCol};
		cameFromLines_.push_back({fromPos, toPos});
		currentRow = fromNode->Row;
		currentCol = fromNode->Column;
	}

	for (int row = 0; row < GRID_ROWS_; ++row)
	{
		for (int col = 0; col < GRID_COLUMNS_; ++col)
		{
			if (costMap[row][col] == std::numeric_limits<int>::max())
			{
				std::cout << "X ";
				continue;
			}
			std::cout << costMap[row][col] << " ";
		}
		std::cout << std::endl;
	}
}
void MainLayer::OnUpdate(float deltaTime) {}
void MainLayer::DrawGrid(Renderer& renderer)
{
	for (int col = 0; col <= GRID_COLUMNS_; ++col)
	{
		const glm::vec2 start = glm::vec2(-GRID_HALF_WIDTH_, -GRID_HALF_HEIGHT_ + col * CELL_SIZE_);
		const glm::vec2 end = glm::vec2(GRID_HALF_WIDTH_, -GRID_HALF_HEIGHT_ + col * CELL_SIZE_);
		renderer.DrawLine(start, end, glm::vec4(0.5f, 0.5f, 0.5f, 0.5f), 1.0f);
	}
	for (int row = 0; row <= GRID_ROWS_; ++row)
	{
		const glm::vec2 start = glm::vec2(-GRID_HALF_WIDTH_ + row * CELL_SIZE_, -GRID_HALF_HEIGHT_);
		const glm::vec2 end = glm::vec2(-GRID_HALF_WIDTH_ + row * CELL_SIZE_, GRID_HALF_HEIGHT_);
		renderer.DrawLine(start, end, glm::vec4(0.5f, 0.5f, 0.5f, 0.5f), 1.0f);
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
			glm::ivec2 position = IndexToCenterPosition({row, col});
			renderer.DrawRectangle(position, 0.0f, glm::vec2(CELL_SIZE_, CELL_SIZE_), GetTileColor(node.Type), false);
		}
	}

	for (const std::pair<glm::ivec2, glm::ivec2>& edge : cameFromLines_)
	{
		const glm::ivec2 from = edge.first;
		const glm::ivec2 to = edge.second;
		renderer.DrawLine(IndexToCenterPosition(from), IndexToCenterPosition(to), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
						  3.0f);
	}

	renderer.EndScene();
}
std::vector<Node*> MainLayer::GetNeighbors(int row, int col)
{
	std::vector<Node*> neighbors;
	constexpr glm::ivec2 directions[] = {
		{-1, 0}, // Up
		{1, 0},	 // Down
		{0, -1}, // Left
		{0, 1}	 // Right
	};
	for (const glm::ivec2& dir : directions)
	{
		int newRow = row + dir.x;
		int newCol = col + dir.y;
		if (newRow >= 0 && newRow < GRID_ROWS_ && newCol >= 0 && newCol < GRID_COLUMNS_)
		{
			neighbors.push_back(&grid_[newRow][newCol]);
		}
	}
	return neighbors;
}
glm::ivec2 MainLayer::IndexToCenterPosition(const glm::ivec2& index)
{
	return glm::ivec2(-GRID_HALF_WIDTH_ + index.y * CELL_SIZE_ + HALF_CELL_SIZE_,
					  GRID_HALF_HEIGHT_ - index.x * CELL_SIZE_ - HALF_CELL_SIZE_);
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
int MainLayer::GetTileCost(ETileType type)
{
	switch (type)
	{
	case ETileType::Path:
		return 1;
	case ETileType::Wall:
		return std::numeric_limits<int>::max();
	case ETileType::Water:
		return 5;
	case ETileType::Sand:
		return 3;
	case ETileType::Forest:
		return 4;
	default:
		return 1;
	}
}
