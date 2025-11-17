#pragma once
#include <limits>

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
	void Reset()
	{
		GCost = std::numeric_limits<float>::max();
		HCost = 0.0f;
		bClosed = false;
		Parent = nullptr;
	}
};