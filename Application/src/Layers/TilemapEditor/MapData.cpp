#include "MapData.h"

#include "Core/Assert.h"
#include "Layers/LayerCommon.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>

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
	, NextBaseId(1)
{

	TileSetReferences.clear();
	GridData.resize(rows, std::string(columns, '0'));
	bDirty_ = true;
}

void MapData::LoadFromFile(const std::filesystem::path& path)
{
	std::ifstream file(path);
	if (!file.is_open())
	{
		std::cerr << "Failed to open map file: " << path.string() << std::endl;
		return;
	}

	nlohmann::json json;
	file >> json;

	FilePath = path;
	Rows = json.at("rows").get<int>();
	Columns = json.at("columns").get<int>();
	CellSize = json.at("cellSize").get<int>();
	StartRow = json.at("startRow").get<int>();
	StartColumn = json.at("startCol").get<int>();
	EndRow = json.at("endRow").get<int>();
	EndColumn = json.at("endCol").get<int>();
	HeuristicMethod = json.at("heuristicMethod").get<EHeuristicMethod::Type>();
	SimulationSpeed = json.at("simulationSpeed").get<float>();
	NextBaseId = json.at("currentBaseId").get<unsigned int>();

	TileSetReferences.clear();
	for (const auto& entry : json.at("tileSetReferences"))
	{
		TileSetReference ref;
		ref.BaseId = entry.at("baseId").get<unsigned int>();
		ref.TileSetFileName = entry.at("tileSetFilePath").get<std::string>();
		TileSetReferences.push_back(ref);
	}

	GridData = json.at("mapData").get<std::vector<std::string>>();
	bDirty_ = false;
}

void MapData::SaveToFile()
{
	CC_ASSERT(!FilePath.empty());

	nlohmann::json json;
	json["rows"] = Rows;
	json["columns"] = Columns;
	json["cellSize"] = CellSize;
	json["startRow"] = StartRow;
	json["startCol"] = StartColumn;
	json["endRow"] = EndRow;
	json["endCol"] = EndColumn;
	json["heuristicMethod"] = HeuristicMethod;
	json["simulationSpeed"] = SimulationSpeed;
	json["currentBaseId"] = NextBaseId;

	json["tileSetReferences"] = nlohmann::json::array();
	for (const auto& ref : TileSetReferences)
	{
		nlohmann::json entry;
		entry["baseId"] = ref.BaseId;
		entry["tileSetFilePath"] = ref.TileSetFileName.string();
		json["tileSetReferences"].push_back(entry);
	}

	json["mapData"] = GridData;

	std::ofstream file(FilePath);
	if (!file.is_open())
	{
		std::cerr << "Failed to save map file: " << FilePath << std::endl;
		return;
	}

	file << json.dump(4);
	bDirty_ = false;
}

void MapData::SaveToFile(const std::filesystem::path& path)
{
	FilePath = path;
	SaveToFile();
}

void MapData::AddTileSetReference(int tileCount, const std::string& tileSetFileName)
{
	TileSetReference ref;
	ref.BaseId = NextBaseId;
	ref.TileSetFileName = tileSetFileName;
	TileSetReferences.push_back(ref);
	NextBaseId += tileCount;
	bDirty_ = true;
}

bool MapData::HasUnsavedChanges() const
{
	return bDirty_;
}