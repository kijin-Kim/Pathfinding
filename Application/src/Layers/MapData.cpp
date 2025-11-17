#include "MapData.h"
#include "Layers/LayerCommon.h"

MapData::MapData(int rows, int columns, int cellSize)
	: RowCount(rows)
	, ColumnCount(columns)
	, CellSize(cellSize)
	, StartRow(0)
	, StartColumn(0)
	, EndRow(rows - 1)
	, EndColumn(columns - 1)
	, HeuristicMethod(EHeuristicMethod::None)
	, SimulationSpeed(1.0f)
{
}