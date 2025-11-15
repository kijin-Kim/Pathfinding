#include "TileSetData.h"

#include "nlohmann/json.hpp"
#include <cmath>
#include <fstream>
#include <iostream>

void TileSetData::LoadFromFile(const std::filesystem::path& path)
{
	std::ifstream file(path);
	if (!file.is_open())
	{
		std::cerr << "Failed to open tileset file: " << path.string() << std::endl;
		return;
	}

	nlohmann::json json;
	file >> json;

	SourceImagePath = json.at("sourceImagePath").get<std::string>();
	TileWidth = json.at("tileWidth").get<int>();
	TileHeight = json.at("tileHeight").get<int>();
}

void TileSetData::SaveToFile(const std::filesystem::path& path) const
{
	nlohmann::json json;
	json["sourceImagePath"] = SourceImagePath.string();
	json["tileWidth"] = TileWidth;
	json["tileHeight"] = TileHeight;

	std::filesystem::create_directories(path.parent_path());

	std::ofstream file(path);
	if (!file.is_open())
	{
		std::cerr << "Failed to create tileset file: " << path.string();
		return;
	}

	file << json.dump(4);
}

int TileSetData::GetTileCountX(int imageWidth) const
{
	return static_cast<int>(std::ceil(static_cast<float>(imageWidth) / TileWidth));
}

int TileSetData::GetTileCountY(int imageHeight) const
{
	return static_cast<int>(std::ceil(static_cast<float>(imageHeight) / TileHeight));
}

int TileSetData::GetTotalTileCount(int imageWidth, int imageHeight) const
{
	return GetTileCountX(imageWidth) * GetTileCountY(imageHeight);
}