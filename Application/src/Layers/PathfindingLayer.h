#pragma once
#include "Core/Delegate.h"
#include "Core/Layers/Layer.h"
#include "glm/vec2.hpp"
#include "glm/vec4.hpp"

#include <queue>

enum class EHeuristicMethod
{
	None,
	Manhattan,
	Euclidean,
	Octile,
};

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

	float GCost = std::numeric_limits<float>::max();
	float HCost = 0.0f;
	bool bClosed = false;
	Node* Parent = nullptr;

	float FCost() const { return GCost + HCost; }
	bool IsWalkable() const { return Type != ETileType::Wall; }
};

class PathfindingLayer : public ILayer
{
public:
	PathfindingLayer();
	void RebuildGridAndOpenSet();
	void StepPathfinding();
	void DrawGrid(Renderer& renderer);
	void DrawPath(Renderer& renderer);
	void DrawClosed(Renderer& renderer);
	void DrawOpen(Renderer& renderer);
	virtual void OnUpdate(float deltaTime) override;
	virtual void OnRender(Renderer& renderer) override;
	virtual bool OnMouseButtonEvent(int button, int action, int mods) override;

	std::vector<Node*> GetNeighbors(int row, int col, bool allowDiagonals);

	glm::vec2 IndexToCenterPosition(int row, int col);
	glm::vec4 GetTileColor(ETileType type);
	float GetTileCost(ETileType type);
	float HeuristicCost(int rowA, int colA, int rowB, int colB, EHeuristicMethod method);
	void SetIsPlayMode(bool bIsPlayMode) { bIsPlayMode_ = bIsPlayMode; }


private:
	std::vector<std::vector<Node>> grid_;
	constexpr static int GRID_ROWS_ = 27;
	constexpr static int GRID_COLUMNS_ = 27;
	constexpr static float CELL_SIZE_ = 30.0f;
	constexpr static float HALF_CELL_SIZE_ = CELL_SIZE_ * 0.5f;
	constexpr static int GRID_HALF_WIDTH_ = GRID_ROWS_ * CELL_SIZE_ * 0.5f;
	constexpr static int GRID_HALF_HEIGHT_ = GRID_COLUMNS_ * CELL_SIZE_ * 0.5f;

	// constexpr static int START_ROW_ = GRID_ROWS_ / 2;
	// constexpr static int START_COLUMN_ = GRID_COLUMNS_ / 2;
	// constexpr static int GOAL_ROW_ = GRID_ROWS_ / 2;
	// constexpr static int GOAL_COLUMN_ = GRID_COLUMNS_ - 4;

	constexpr static int START_ROW_ = 0;
	constexpr static int START_COLUMN_ = 0;
	constexpr static int GOAL_ROW_= GRID_ROWS_ - 1;
	constexpr static int GOAL_COLUMN_ = GRID_COLUMNS_ - 1;
	EHeuristicMethod heuristicMethod_ = EHeuristicMethod::None;

	decltype([](Node* a, Node* b)
	{
		if (a->FCost() == b->FCost())
		{
			return a->HCost > b->HCost;
		}
		return a->FCost() > b->FCost();
	}) comp_;
	std::priority_queue<Node*, std::vector<Node*>, decltype(comp_)> openSet_;
	bool bPathFound_ = false;
	float accumulatedTime_ = 0.0f;
	bool bIsPlayMode_ = true;
};