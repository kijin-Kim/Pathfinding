#pragma once
#include "Core/Layers/Layer.h"
#include "ImGuiLayer.h"
#include "PathfindingLayer.h"

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
			imGuiLayer->OnMapRefChanged.Bind([pathfindingLayer](const std::weak_ptr<MapData>& mapDataWeak)
											 { pathfindingLayer->OnMapRefChanged(mapDataWeak); });
			pathfindingLayer->OnMapRefChanged(imGuiLayer->GetCurrentMap());

			imGuiLayer->OnStartEvent.Bind(
				[this]()
				{
					std::shared_ptr<PathfindingLayer> pfLayer = pathfindingLayer_.lock();
					if (pfLayer)
					{
						pfLayer->OnStartEvent();
					}
				});
			imGuiLayer->OnPauseEvent.Bind(
				[this]()
				{
					std::shared_ptr<PathfindingLayer> pfLayer = pathfindingLayer_.lock();
					if (pfLayer)
					{
						pfLayer->OnPauseEvent();
					}
				});
			imGuiLayer->OnResetEvent.Bind(
				[this]()
				{
					std::shared_ptr<PathfindingLayer> pfLayer = pathfindingLayer_.lock();
					if (pfLayer)
					{
						pfLayer->OnResetEvent();
					}
				});
			imGuiLayer->OnStepEvent.Bind(
				[this]()
				{
					std::shared_ptr<PathfindingLayer> pfLayer = pathfindingLayer_.lock();
					if (pfLayer)
					{
						pfLayer->OnStepEvent();
					}
				});
			imGuiLayer->OnRebuildEvent.Bind(
				[this]()
				{
					std::shared_ptr<PathfindingLayer> pfLayer = pathfindingLayer_.lock();
					if (pfLayer)
					{
						pfLayer->OnRebuildEvent();
					}
				});
			pathfindingLayer->OnRebuildEvent();
		}
	}

private:
	std::weak_ptr<PathfindingLayer> pathfindingLayer_;
	std::weak_ptr<ImGuiLayer> imGuiLayer_;
};