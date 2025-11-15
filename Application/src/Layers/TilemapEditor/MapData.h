#pragma once

#include "Layers/LayerCommon.h"

#include <filesystem>
#include <string>
#include <vector>

struct TileSetReference
{
	unsigned int BaseId = 0;
	std::filesystem::path TileSetFileName;
};

struct MapData
{
	MapData() = default;
	explicit MapData(int rows, int columns, int cellSize);
	// File path
	std::filesystem::path FilePath;

	// Grid dimensions
	int Rows;
	int Columns;
	int CellSize;

	// Pathfinding settings
	int StartRow;
	int StartColumn;
	int EndRow;
	int EndColumn;
	EHeuristicMethod::Type HeuristicMethod;
	float SimulationSpeed;

	// TileSet management
	unsigned int NextBaseId;
	std::vector<TileSetReference> TileSetReferences;

	// Map content
	std::vector<std::string> GridData;


	// File I/O
	void LoadFromFile(const std::filesystem::path& path);
	void SaveToFile();
	void SaveToFile(const std::filesystem::path& path);

	// TileSet management
	void AddTileSetReference(int tileCount, const std::string& tileSetFileName);

	bool HasUnsavedChanges() const;

	void MarkDirty() { bDirty_ = true; }

private:
	bool bDirty_ = false;
};