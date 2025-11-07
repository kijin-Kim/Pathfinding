#pragma once
#include "Core/Layers/Layer.h"
#include "glm/vec2.hpp"
#include "glm/vec4.hpp"

enum class ETileType
{
	Path,
	Wall,
	Water,
	Sand,
	Forest
};

struct Node
{
	int Row = 0;
	int Column = 0;
	ETileType Type = ETileType::Path;

	bool IsWalkable() const { return Type != ETileType::Wall; }
};

class MainLayer : public ILayer
{
public:
	MainLayer();
	virtual void OnUpdate(float deltaTime) override;
	void DrawGrid(Renderer& renderer);
	virtual void OnRender(Renderer& renderer) override;

	std::vector<Node*> GetNeighbors(int row, int col);

	glm::ivec2 IndexToCenterPosition(const glm::ivec2& index);
	glm::vec4 GetTileColor(ETileType type);
	int GetTileCost(ETileType type);

private:
	std::vector<std::vector<Node>> grid_;
	std::vector<std::pair<glm::ivec2, glm::ivec2>> cameFromLines_;
	constexpr static int GRID_ROWS_ = 13;
	constexpr static int GRID_COLUMNS_ = 13;
	constexpr static float CELL_SIZE_ = 50.0f;
	constexpr static float HALF_CELL_SIZE_ = CELL_SIZE_ * 0.5f;
	constexpr static int GRID_HALF_WIDTH_ = GRID_ROWS_ * CELL_SIZE_ * 0.5f;
	constexpr static int GRID_HALF_HEIGHT_ = GRID_COLUMNS_ * CELL_SIZE_ * 0.5f;
	constexpr static glm::ivec2 GOAL_ = {GRID_ROWS_ - 1, GRID_COLUMNS_ - 1};
};