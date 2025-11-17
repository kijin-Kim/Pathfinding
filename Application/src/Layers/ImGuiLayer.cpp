#include "ImGuiLayer.h"

#include "Core/Application.h"
#include "GLFW/glfw3.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/ResourceManager.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

void ImGuiLayer::OnInit()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(glfwGetCurrentContext(), true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	currentMap_ = std::make_shared<MapData>(31, 31, 32);
	OnMapRefChanged.Execute(currentMap_);
}

void ImGuiLayer::OnDestroy()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiLayer::OnRender(Renderer& renderer)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport();

	RenderViewport();
	RenderDetailPanel();

#ifndef NDEBUG
	ImGui::ShowDemoWindow();
#endif

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiLayer::RenderViewport()
{
	ImGui::Begin("Viewport");

	auto& framebufferManager = ResourceManager<Framebuffer>::GetInstance();
	auto framebuffer = framebufferManager.Get("Viewport");

	ImVec2 imageSize = ImGui::GetContentRegionAvail();
	framebuffer->Resize(static_cast<int>(imageSize.x), static_cast<int>(imageSize.y));
	ImGui::Image(framebuffer->GetColorAttachmentId(), imageSize, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

	ImGui::End();
}

void ImGuiLayer::RenderDetailPanel()
{
	ImGui::Begin("Detail Panel");

	ImGui::SeparatorText("Simulation Controls");
	ImGui::Text("FrameRate: %d (%.0f ms)", static_cast<int>(ImGui::GetIO().Framerate),
				1000.0f / ImGui::GetIO().Framerate);

	if (ImGui::Button("Start"))
	{
		bIsRefreshed = false;
		OnStartEvent.Execute();
	}
	ImGui::SameLine();
	if (ImGui::Button("Pause"))
	{
		OnPauseEvent.Execute();
	}
	ImGui::SameLine();
	ImGui::PushItemFlag(ImGuiItemFlags_ButtonRepeat, true);
	if (ImGui::Button("Step"))
	{
		bIsRefreshed = false;
		OnPauseEvent.Execute();
		OnStepEvent.Execute();
	}
	ImGui::PopItemFlag();

	ImGui::SliderInt("Cell Size", &currentMap_->CellSize, 4, 64);
	ImGui::DragFloat("Simulation Speed", &currentMap_->SimulationSpeed, 0.1f, 0.1f, 5.0f, "%.1f x");

	ImGui::SeparatorText("Pathfinding Settings");
	if (ImGui::Button("Reset"))
	{
		bIsRefreshed = true;
		OnPauseEvent.Execute();
		OnResetEvent.Execute();
	}
	ImGui::SameLine();
	if (ImGui::Button("Rebuild"))
	{
		bIsRefreshed = true;
		OnPauseEvent.Execute();
		OnRebuildEvent.Execute();
	}

	ImGui::BeginDisabled(!bIsRefreshed);
	ImGui::BeginGroup();
	ImGui::Columns(2, "Start Position");
	ImGui::SliderInt("Start Row", &currentMap_->StartRow, 0, currentMap_->Rows - 1);
	ImGui::NextColumn();
	ImGui::SliderInt("Start Col", &currentMap_->StartColumn, 0, currentMap_->Columns - 1);
	ImGui::Columns(1);

	ImGui::Columns(2, "End Position");
	ImGui::SliderInt("End Row", &currentMap_->EndRow, 0, currentMap_->Rows - 1);
	ImGui::NextColumn();
	ImGui::SliderInt("End Col", &currentMap_->EndColumn, 0, currentMap_->Columns - 1);
	ImGui::Columns(1);

	const char* heuristicMethodNames[EHeuristicMethod::NUM_TYPES];
	for (int i = 0; i < EHeuristicMethod::NUM_TYPES; ++i)
	{
		heuristicMethodNames[i] = EHeuristicMethod::to_string(static_cast<EHeuristicMethod::Type>(i));
	}

	int selectedIndex = currentMap_->HeuristicMethod;
	if (ImGui::Combo("Heuristic Method", &selectedIndex, heuristicMethodNames, EHeuristicMethod::NUM_TYPES))
	{
		currentMap_->HeuristicMethod = static_cast<EHeuristicMethod::Type>(selectedIndex);
	}
	ImGui::EndGroup();
	if (!bIsRefreshed)
	{
		ImGui::SetItemTooltip("Reset or Rebuild the map to change start/end positions or heuristic method.");
	}
	ImGui::EndDisabled();

	ImGui::End();
}