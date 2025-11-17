#pragma once

#include "Core/Delegate.h"
#include "Core/Layers/Layer.h"
#include "MapData.h"
#include <memory>

class ImGuiLayer : public ILayer
{
public:
	virtual void OnInit() override;
	virtual void OnDestroy() override;
	virtual void OnRender(Renderer& renderer) override;
	std::weak_ptr<MapData> GetCurrentMap() const { return currentMap_; }

public:
	Delegate<const std::weak_ptr<MapData>&> OnMapRefChanged;
	Delegate<> OnStartEvent;
	Delegate<> OnPauseEvent;
	Delegate<> OnResetEvent;
	Delegate<> OnStepEvent;
	Delegate<> OnRebuildEvent;

private:
	void RenderViewport();
	void RenderDetailPanel();

private:
	std::shared_ptr<MapData> currentMap_;
	bool bIsRefreshed = true;
};