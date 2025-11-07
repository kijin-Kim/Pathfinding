#include "MainLayer.h"

#include "Renderer/Renderer.h"

#include <iostream>

MainLayer::MainLayer()
{
	grid_.resize(GRID_ROWS, std::vector<Node>(GRID_COLUMNS));
	for (int row = 0; row < GRID_ROWS; ++row)
	{
		for (int col = 0; col < GRID_COLUMNS; ++col)
		{
			Node& node = grid_[row][col];
			node.bIsWalkable = false;
		}
	}

	grid_[0][0].bIsWalkable = true;
}
void MainLayer::OnUpdate(float deltaTime) {}
void MainLayer::OnRender(Renderer& renderer)
{
	renderer.BeginScene();
	const int gridWidth = GRID_ROWS * CELL_SIZE;
	const int gridHeight = GRID_COLUMNS * CELL_SIZE;
	for (int col = 0; col <= GRID_COLUMNS; ++col)
	{
		const glm::vec2 start = glm::vec2(-(gridWidth) * 0.5f, -(gridHeight) * 0.5f + col * CELL_SIZE);
		const glm::vec2 end = glm::vec2((gridWidth) * 0.5f, -(gridHeight) * 0.5f + col * CELL_SIZE);
		renderer.DrawLine(start, end, glm::vec4(0.5f, 0.5f, 0.5f, 0.5f), 1.0f);
	}
	for (int row = 0; row <= GRID_ROWS; ++row)
	{
		const glm::vec2 start = glm::vec2(-(gridWidth) * 0.5f + row * CELL_SIZE, -(gridHeight) * 0.5f);
		const glm::vec2 end = glm::vec2(-(gridWidth) * 0.5f + row * CELL_SIZE, (gridHeight) * 0.5f);
		renderer.DrawLine(start, end, glm::vec4(0.5f, 0.5f, 0.5f, 0.5f), 1.0f);
	}

	for (int row = 0; row < GRID_ROWS; ++row)
	{
		for (int col = 0; col < GRID_COLUMNS; ++col)
		{
			const Node& node = grid_[row][col];
			if (node.bIsWalkable)
			{
				continue;
			}
			glm::vec4 color = glm::vec4(0.0f, 1.0f, 0.0f, 0.3f);
			const glm::vec2 position = glm::vec2(-(gridWidth) * 0.5f + col * CELL_SIZE + CELL_SIZE * 0.5f,
												 -(gridHeight) * 0.5f + row * CELL_SIZE + CELL_SIZE * 0.5f);
			renderer.DrawRectangle(position, 0.0f, glm::vec2(CELL_SIZE, CELL_SIZE), color, false);
		}
	}

	renderer.EndScene();
}
