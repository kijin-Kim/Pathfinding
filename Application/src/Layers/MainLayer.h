#pragma once
#include "Core/Layers/Layer.h"

struct Node
{
	bool bIsWalkable = true;
};

class MainLayer : public ILayer
{
public:
	MainLayer();
	virtual void OnUpdate(float deltaTime) override;
	virtual void OnRender(Renderer& renderer) override;


private:
	std::vector<std::vector<Node>> grid_;
	constexpr static int GRID_ROWS = 3;
	constexpr static int GRID_COLUMNS = 3;
	constexpr static float CELL_SIZE = 50.0f;
};