#include "Core/Application.h"
#include "Layers/ControllerLayer.h"
#include "Layers/ImGuiLayer.h"
#include "Layers/PathfindingLayer.h"

#include "common/TracySystem.hpp"

int main()
{
	tracy::SetThreadName("Main Thread");
	Application& app = Application::GetInstance();
	Application::ApplicationSettings settings;
	settings.Width = 1920;
	settings.Height = 1080;
	app.Initialize(settings);
	std::weak_ptr<PathfindingLayer> pathfindingLayer = app.AddLayer<PathfindingLayer>();
	std::weak_ptr<ImGuiLayer> imGuiLayer = app.AddLayer<ImGuiLayer>();
	app.AddLayer<ControllerLayer>(pathfindingLayer, imGuiLayer);
	app.Run();
}