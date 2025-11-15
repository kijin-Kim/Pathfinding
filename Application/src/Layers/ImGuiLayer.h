#pragma once

#include "Core/Delegate.h"
#include "Core/Layers/Layer.h"
#include "Renderer/Renderer.h"
#include "TilemapEditor/MapData.h"
#include "TilemapEditor/TileSetData.h"

#include <filesystem>
#include <memory>
#include <vector>

class ImageTexture;

struct TileSetView
{
	std::shared_ptr<ImageTexture> SourceImage;
	int TileWidth = 32;
	int TileHeight = 32;
	bool bOpened = true;
};

class ImGuiLayer : public ILayer
{
public:
	virtual void OnInit() override;
	virtual void OnDestroy() override;
	virtual void OnRender(Renderer& renderer) override;

public:
	Delegate<const std::shared_ptr<ImageTexture>&, const Renderer::TextureRegion&> OnTileSelected;
	Delegate<float, float> OnViewportCursorPositionChanged;
	Delegate<const std::weak_ptr<MapData>&> OnMapRefChanged;
	Delegate<> OnStartButtonClicked;
	Delegate<> OnStopButtonClicked;
	Delegate<> OnResetButtonClicked;
	Delegate<> OnStepButtonClicked;

private:
	// Rendering methods
	void RenderMainMenuBar();
	void RenderViewport(Renderer& renderer);
	void RenderDetailPanel();
	void RenderTileSetTabs();

	// Popup methods
	void RenderNewMapPopup();
	void RenderNewTileSetPopup();
	void RenderLostChangesPopup();

	// Actions
	void CreateNewMap(int rows, int columns, int cellSize);
	void OpenMap();
	bool SaveMap();
	void CreateNewTileSet(const std::filesystem::path& sourceImagePath, int tileWidth, int tileHeight);

private:
	std::shared_ptr<MapData> currentMap_;
	std::vector<TileSetData> tileSetDataList_;
	std::vector<TileSetView> tileSetViews_;

	// Popup states
	bool bShouldOpenNewMapPopup_ = false;
	bool bShouldOpenNewTileSetPopup_ = false;
	bool bShouldOpenOpenMapPopup_ = false;
};