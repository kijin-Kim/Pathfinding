#pragma once
#include "Core/Delegate.h"
#include "Core/Layers/Layer.h"


class ImGuiLayer : public ILayer
{
public:
	virtual void OnInit() override;
	virtual void OnDestroy() override;
	virtual void OnRender(Renderer& renderer) override;


public:
	Delegate<bool> OnModeChanged; // true: Play Mode, false: Edit Mode

private:
	void SetIsPlayMode(bool bIsPlayMode);

private:
	bool bIsPlayMode_ = true;
};
