#pragma once
#include "Core/Delegate.h"
#include "Core/Layers/Layer.h"
#include "LayerCommon.h"
#include "Renderer/ImageTexture.h"
#include "Renderer/Renderer.h"
#include "TilemapEditor/MapData.h"
#include "glm/vec2.hpp"
#include "glm/vec4.hpp"

#include <queue>

enum class ETileType
{
	Path,
	Wall
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
	virtual void OnInit() override;
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
	float HeuristicCost(int rowA, int colA, int rowB, int colB, EHeuristicMethod::Type method);

	void OnViewportCursorPositionChanged(float x, float y);
	void OnMapDataRefChanged(const std::weak_ptr<MapData>& mapDataWeak);
	void OnTileSelected(const std::shared_ptr<ImageTexture>& imageTexture,
						const Renderer::TextureRegion& textureRegion);
	void OnStartButtonClicked();
	void OnStopButtonClicked();
	void OnResetButtonClicked();
	void OnStepButtonClicked();

private:
	std::vector<std::vector<Node>> grid_;

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


	float viewportCursorX_ = 0.0f;
	float viewportCursorY_ = 0.0f;

	glm::vec2 cursorPos_;

	std::weak_ptr<MapData> mapDataWeak_;

	std::shared_ptr<ImageTexture> selectedTileSetImage_;
	Renderer::TextureRegion selectedTileRegion_;

	bool bIsPaused_ = true;
};