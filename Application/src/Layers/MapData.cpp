#include "MapData.h"
#include "Layers/LayerCommon.h"

MapData::MapData(int rows, int columns, int cellSize)
	: Rows(rows)
	, Columns(columns)
	, CellSize(cellSize)
	, StartRow(0)
	, StartColumn(0)
	, EndRow(rows - 1)
	, EndColumn(columns - 1)
	, HeuristicMethod(EHeuristicMethod::None)
	, SimulationSpeed(1.0f)
{
}