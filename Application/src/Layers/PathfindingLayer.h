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
	void RebuildGridAndOpenSet();
	void StepPathfinding();
	void DrawGrid(Renderer& renderer);
	void DrawPath(Renderer& renderer);
	void DrawClosed(Renderer& renderer);
	void DrawOpen(Renderer& renderer);
	virtual void OnUpdate(float deltaTime) override;
	virtual void OnRender(Renderer& renderer) override;

	std::vector<Node*> GetNeighbors(int row, int col, bool allowDiagonals);

	glm::vec2 IndexToCenterPosition(int row, int col);
	glm::vec4 GetTileColor(ETileType type);
	float GetTileCost(ETileType type);
	float HeuristicCost(int rowA, int colA, int rowB, int colB, EHeuristicMethod::Type method);

	void OnMapRefChanged(const std::weak_ptr<MapData>& weak);
	void OnStartEvent();
	void OnPauseEvent();
	void OnResetEvent();
	void OnStepEvent();
	void OnRebuildEvent();

private:
	std::vector<std::vector<Node>> grid_;

	decltype([](const Node* a, const Node* b)
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

	std::weak_ptr<MapData> mapDataWeak_;

	bool bIsPaused_ = true;
};