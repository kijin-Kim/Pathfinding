#include "ImGuiLayer.h"
#include "Core/IniParser.h"
#include "GLFW/glfw3.h"
#include "Renderer/TextureManager.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

void ImGuiLayer::OnInit()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(glfwGetCurrentContext(), true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	std::filesystem::path configPath = CONFIG_PATH;
	configPath /= "config.ini";
	IniFile configFile = ParseIniFile(configPath);

	TextureManager& textureManager = TextureManager::GetInstance();
	std::filesystem::path assetsPath = APP_ASSET_PATH;
	assetsPath /= "textures/Sprout Lands - Sprites - Basic pack/Objects/Basic Furniture.png";
	std::shared_ptr<ImageTexture> testTexture = textureManager.LoadTextureFromFile(assetsPath);
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

	ImGui::Begin("Hello from ImGuiLayer");

	ImGui::SeparatorText("Options");
	ImGui::Text("FrameRate: %d (%.0f ms)", static_cast<int>(ImGui::GetIO().Framerate),
				1000.0f / ImGui::GetIO().Framerate);

	if (ImGui::BeginCombo("Mode", bIsPlayMode_ ? "Play Mode" : "Edit Mode"))
	{
		if (ImGui::Selectable("Play Mode"))
		{
			SetIsPlayMode(true);
		}
		if (ImGui::Selectable("Edit Mode"))
		{
			SetIsPlayMode(false);
		}

		ImGui::EndCombo();
	}

	TextureManager& textureManager = TextureManager::GetInstance();

	std::filesystem::path assetsPath = APP_ASSET_PATH;
	assetsPath /= "textures/Sprout Lands - Sprites - Basic pack/Objects/Basic Furniture.png";
	std::shared_ptr<ImageTexture> testTexture = textureManager.LoadTextureFromFile(assetsPath);
	if (testTexture)
	{
		ImGui::Begin("Texture Test");
		ImGui::Text("Test Texture:");
		ImGui::Image(testTexture->GetID(), ImVec2(testTexture->GetWidth(), testTexture->GetHeight()));
		ImGui::End();
	}

	ImGui::ShowDemoWindow();

	ImGui::End();

	ImGui::Render();

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
void ImGuiLayer::SetIsPlayMode(bool bIsPlayMode)
{
	if (bIsPlayMode_ == bIsPlayMode)
	{
		return;
	}
	bIsPlayMode_ = bIsPlayMode;
	OnModeChanged.Execute(bIsPlayMode_);
}