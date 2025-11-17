#pragma once

#include "Layers/LayerCommon.h"

struct MapData
{
	MapData() = default;
	explicit MapData(int rows, int columns, int cellSize);

	// Grid dimensions
	int RowCount;
	int ColumnCount;
	int CellSize;

	// Pathfinding settings
	int StartRow;
	int StartColumn;
	int EndRow;
	int EndColumn;
	EHeuristicMethod::Type HeuristicMethod;
	float SimulationSpeed;
};