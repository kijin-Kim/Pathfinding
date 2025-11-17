#pragma once
#include "Core/Layers/Layer.h"
#include "LayerCommon.h"
#include "MapData.h"
#include "Pathfinding/Node.h"
#include "Renderer/Renderer.h"
#include "glm/vec2.hpp"
#include "glm/vec4.hpp"

#include <queue>

class PathfindingLayer : public ILayer
{
public:
	virtual void OnInit() override;
	void GenerateRandomWalls(int rowCount, int columnCount);
	void RebuildGrid(int rowCount, int columnCount, int startRow, int startColumn, int endRow, int endColumn,
					 EHeuristicMethod::Type method);
	void StepPathfinding(int rowCount, int columnCount, int endRow, int endColumn, EHeuristicMethod::Type method);
	void DrawGridLines(Renderer& renderer, int rowCount, int columnCount, int cellSize);
	void DrawCurrentPath(Renderer& renderer, int rowCount, int columnCount, int cellSize, int endRow, int endColumn);
	void DrawClosedNodes(Renderer& renderer, int rowCount, int columnCount, int cellSize);
	void DrawOpenNodes(Renderer& renderer, int rowCount, int columnCount, int cellSize);
	virtual void OnUpdate(float deltaTime) override;
	void DrawTiles(Renderer& renderer, int rowCount, int columnCount, int cellSize);
	void DrawStartAndEnd(Renderer& renderer, int startRow, int startColumn, int endRow, int endColumn, int rowCount,
						 int columnCount, int cellSize);
	virtual void OnRender(Renderer& renderer) override;

	std::vector<Node*> GetNeighbors(int row, int column, int rowCount, int columnCount, bool bAllowDiagonals);

	glm::vec2 GridToWorldPosition(int row, int column, int rowCount, int columnCount, int cellSize);
	glm::vec4 GetTileColor(ETileType type) const;
	float GetWalkCost(ETileType type) const;
	float CalculateHeuristicCost(int rowStart, int columnStart, int rowEnd, int columnEnd,
								 EHeuristicMethod::Type method) const;

	void OnMapRefChanged(const std::weak_ptr<MapData>& weak);
	void OnStartEvent();
	void OnPauseEvent();
	void ResetPathfinding(int startRow, int startColumn, int endRow, int endColumn, EHeuristicMethod::Type method);
	void OnResetEvent();
	void OnStepEvent();
	void OnRebuildEvent();

private:
	std::vector<std::vector<Node>> grid_;

	struct NodeComparator
	{
		bool operator()(const Node* a, const Node* b) const
		{
			if (a->FCost() == b->FCost())
			{
				return a->HCost > b->HCost;
			}
			return a->FCost() > b->FCost();
		}
	};
	std::priority_queue<Node*, std::vector<Node*>, NodeComparator> openSet_;

	bool bPathFound_ = false;
	float accumulatedTime_ = 0.0f;

	std::weak_ptr<MapData> mapDataWeak_;

	bool bIsPaused_ = true;
};