#pragma once
#include "Core/Layers/Layer.h"
#include "ImGuiLayer.h"
#include "PathfindingLayer.h"

#include <iostream>
#include <memory>

class ControllerLayer : public ILayer
{
public:
	ControllerLayer(const std::weak_ptr<PathfindingLayer>& pathfindingLayer,
					const std::weak_ptr<ImGuiLayer>& imGuiLayer)
		: pathfindingLayer_(pathfindingLayer)
		, imGuiLayer_(imGuiLayer)
	{
	}

	virtual void OnInit() override
	{
		std::shared_ptr<ImGuiLayer> imGuiLayer = imGuiLayer_.lock();
		std::shared_ptr<PathfindingLayer> pathfindingLayer = pathfindingLayer_.lock();
		if (imGuiLayer && pathfindingLayer)
		{
			imGuiLayer->OnViewportCursorPositionChanged.Bind(
				[this](float x, float y)
				{
					std::shared_ptr<PathfindingLayer> pfLayer = pathfindingLayer_.lock();
					if (pfLayer)
					{
						pfLayer->OnViewportCursorPositionChanged(x, y);
					}
				});
			imGuiLayer->OnMapRefChanged.Bind(
				[this](const std::weak_ptr<MapData>& mapDataWeak)
				{
					std::shared_ptr<PathfindingLayer> pfLayer = pathfindingLayer_.lock();
					if (pfLayer)
					{
						pfLayer->OnMapDataRefChanged(mapDataWeak);
					}
				});
			imGuiLayer->OnTileSelected.Bind(
				[this](const std::shared_ptr<ImageTexture> imageTexture, const Renderer::TextureRegion& textureRegion)
				{
					std::shared_ptr<PathfindingLayer> pfLayer = pathfindingLayer_.lock();
					if (pfLayer)
					{
						pfLayer->OnTileSelected(imageTexture, textureRegion);
					}
				});
			imGuiLayer->OnStartButtonClicked.Bind(
				[this]()
				{
					std::shared_ptr<PathfindingLayer> pfLayer = pathfindingLayer_.lock();
					if (pfLayer)
					{
						pfLayer->OnStartButtonClicked();
					}
				});
			imGuiLayer->OnStopButtonClicked.Bind(
				[this]()
				{
					std::shared_ptr<PathfindingLayer> pfLayer = pathfindingLayer_.lock();
					if (pfLayer)
					{
						pfLayer->OnStopButtonClicked();
					}
				});
			imGuiLayer->OnResetButtonClicked.Bind(
				[this]()
				{
					std::shared_ptr<PathfindingLayer> pfLayer = pathfindingLayer_.lock();
					if (pfLayer)
					{
						pfLayer->OnResetButtonClicked();
					}
				});
			imGuiLayer->OnStepButtonClicked.Bind(
				[this]()
				{
					std::shared_ptr<PathfindingLayer> pfLayer = pathfindingLayer_.lock();
					if (pfLayer)
					{
						pfLayer->OnStepButtonClicked();
					}
				});
		}
	}

private:
	std::weak_ptr<PathfindingLayer> pathfindingLayer_;
	std::weak_ptr<ImGuiLayer> imGuiLayer_;
};