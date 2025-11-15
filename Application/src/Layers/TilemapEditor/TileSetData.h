#pragma once

#include <filesystem>

struct TileSetData
{
	std::filesystem::path SourceImagePath;
	int TileWidth = 32;
	int TileHeight = 32;

	void LoadFromFile(const std::filesystem::path& path);
	void SaveToFile(const std::filesystem::path& path) const;

	int GetTileCountX(int imageWidth) const;
	int GetTileCountY(int imageHeight) const;
	int GetTotalTileCount(int imageWidth, int imageHeight) const;
};