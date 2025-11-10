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
			imGuiLayer->OnModeChanged.Bind(
				[this](bool bIsPlayMode)
				{
					std::cout << "Mode changed to: " << (bIsPlayMode ? "Play Mode" : "Edit Mode") << std::endl;
					std::shared_ptr<PathfindingLayer> pathfindingLayer = pathfindingLayer_.lock();
					if (pathfindingLayer)
					{
						pathfindingLayer->SetIsPlayMode(bIsPlayMode);
					}

				});
		}
	}

private:
	std::weak_ptr<PathfindingLayer> pathfindingLayer_;
	std::weak_ptr<ImGuiLayer> imGuiLayer_;
};