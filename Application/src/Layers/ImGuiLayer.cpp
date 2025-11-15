#include "ImGuiLayer.h"

#include "Core/Application.h"
#include "Core/Assert.h"
#include "Core/IniParser.h"
#include "GLFW/glfw3.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/ImageTexture.h"
#include "Renderer/ResourceManager.h"
#include "TilemapEditor/DebugUI.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"
#include <commdlg.h>
#include <windows.h>

namespace // cpp 파일 내부용 익명 네임스페이스
{
	constexpr const char* MAP_FILE_FILTER = "Map Files\0*.map.json\0";
	constexpr const char* MAP_FILE_EXTENSION = "map.json";
	constexpr const char* TILESET_FILE_EXTENSION = "tileset.json";
	constexpr const char* IMAGE_FILE_FILTER = "Image Files\0*.png;*.jpg;*.jpeg;*.bmp;*.tga;*.gif\0";

	std::filesystem::path OpenFileDialog(const char* filter)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = {0};
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = glfwGetWin32Window(glfwGetCurrentContext());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		if (GetOpenFileNameA(&ofn))
		{
			return ofn.lpstrFile;
		}
		return {};
	}

	std::filesystem::path SaveFileDialog(const char* filter, const char* defaultExt)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = {0};
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = glfwGetWin32Window(glfwGetCurrentContext());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.lpstrDefExt = defaultExt;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

		if (GetSaveFileNameA(&ofn))
		{
			return ofn.lpstrFile;
		}
		return {};
	}
} // namespace

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

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
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

	if (currentMap_)
	{
		DebugUI::DrawMapDataDebugWindow(*currentMap_);
	}

	RenderMainMenuBar();
	RenderViewport(renderer);
	RenderDetailPanel();

	// Handle popups
	if (bShouldOpenNewMapPopup_)
	{
		bShouldOpenNewMapPopup_ = false;
		if (currentMap_ && currentMap_->HasUnsavedChanges())
		{
			ImGui::OpenPopup("Lost Changes");
		}
		else
		{
			ImGui::OpenPopup("New Map");
		}
	}

	if (bShouldOpenOpenMapPopup_)
	{
		bShouldOpenOpenMapPopup_ = false;
		if (currentMap_ && currentMap_->HasUnsavedChanges())
		{
			ImGui::OpenPopup("Lost Changes");
		}
		else
		{
			OpenMap();
		}
	}

	if (bShouldOpenNewTileSetPopup_)
	{
		bShouldOpenNewTileSetPopup_ = false;
		ImGui::OpenPopup("New TileSet");
	}

	// TODO: Clean TileSet IDs that are no longer referenced by the map
	std::erase_if(tileSetViews_, [](const TileSetView& view) { return !view.bOpened; });

	RenderNewMapPopup();
	RenderLostChangesPopup();
	RenderNewTileSetPopup();

	ImGui::ShowDemoWindow();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiLayer::RenderMainMenuBar()
{
	if (!ImGui::BeginMainMenuBar())
	{
		return;
	}

	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("New", "CTRL+N"))
		{
			bShouldOpenNewMapPopup_ = true;
		}

		if (ImGui::MenuItem("Open", "CTRL+O"))
		{
			bShouldOpenOpenMapPopup_ = true;
		}

		if (ImGui::MenuItem("Save", "CTRL+S", false, currentMap_ != nullptr))
		{
			SaveMap();
		}

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Edit"))
	{
		ImGui::MenuItem("Undo", "CTRL+Z");
		ImGui::MenuItem("Redo", "CTRL+Y", false, false);
		ImGui::Separator();
		ImGui::MenuItem("Cut", "CTRL+X");
		ImGui::MenuItem("Copy", "CTRL+C");
		ImGui::MenuItem("Paste", "CTRL+V");
		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();
}

void ImGuiLayer::RenderViewport(Renderer& renderer)
{
	ImGui::Begin("Viewport");

	auto& framebufferManager = ResourceManager<Framebuffer>::GetInstance();
	auto framebuffer = framebufferManager.Get("Viewport");

	ImVec2 imageSize = ImGui::GetContentRegionAvail();
	ImVec2 imagePos = ImGui::GetCursorScreenPos();

	framebuffer->Resize(static_cast<int>(imageSize.x), static_cast<int>(imageSize.y));
	ImGui::Image(framebuffer->GetColorAttachmentId(), imageSize, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

	ImVec2 mouse = ImGui::GetMousePos();
	ImVec2 localPos = ImVec2(mouse.x - imagePos.x, mouse.y - imagePos.y);
	OnViewportCursorPositionChanged.Execute(localPos.x, localPos.y);

	glm::mat4 projection = renderer.GetProjectionMatrix();
	float mouseX = localPos.x / imageSize.x * 2.0f - 1.0f;
	float mouseY = 1.0f - localPos.y / imageSize.y * 2.0f;

	glm::vec4 mousePosNDC = glm::vec4(mouseX, mouseY, 0.0f, 1.0f);
	glm::vec4 cursorPos = glm::inverse(projection) * mousePosNDC;

	ImGui::End();
}

void ImGuiLayer::RenderDetailPanel()
{
	ImGui::Begin("Detail Panel");

	if (!currentMap_)
	{
		ImGui::Text("No map loaded. File -> New or Open to create/load a map.");
		ImGui::End();
		return;
	}

	ImGui::SeparatorText("Options");
	ImGui::Text("FrameRate: %d (%.0f ms)", static_cast<int>(ImGui::GetIO().Framerate),
				1000.0f / ImGui::GetIO().Framerate);

	if (ImGui::Button("Start"))
	{
		OnStartButtonClicked.Execute();
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop"))
	{
		OnStopButtonClicked.Execute();
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset"))
	{
		OnResetButtonClicked.Execute();
	}
	ImGui::SameLine();
	if (ImGui::Button("Step"))
	{
		OnStepButtonClicked.Execute();
	}

	if (ImGui::SliderInt("Cell Size", &currentMap_->CellSize, 4, 64))
	{
		currentMap_->MarkDirty();
	}

	ImGui::Columns(2, "Start Position");
	if (ImGui::SliderInt("Start Row", &currentMap_->StartRow, 0, currentMap_->Rows - 1))
	{
		currentMap_->MarkDirty();
	}
	ImGui::NextColumn();
	if (ImGui::SliderInt("Start Col", &currentMap_->StartColumn, 0, currentMap_->Columns - 1))
	{
		currentMap_->MarkDirty();
	}
	ImGui::Columns(1);

	ImGui::Columns(2, "End Position");
	if (ImGui::SliderInt("End Row", &currentMap_->EndRow, 0, currentMap_->Rows - 1))
	{
		currentMap_->MarkDirty();
	}
	ImGui::NextColumn();
	if (ImGui::SliderInt("End Col", &currentMap_->EndColumn, 0, currentMap_->Columns - 1))
	{
		currentMap_->MarkDirty();
	}
	ImGui::Columns(1);

	if (ImGui::DragFloat("Simulation Speed", &currentMap_->SimulationSpeed))
	{
		currentMap_->MarkDirty();
	}

	const char* heuristicMethodNames[EHeuristicMethod::NUM_TYPES];
	for (int i = 0; i < EHeuristicMethod::NUM_TYPES; ++i)
	{
		heuristicMethodNames[i] = EHeuristicMethod::to_string(static_cast<EHeuristicMethod::Type>(i));
	}

	int selectedIndex = currentMap_->HeuristicMethod;
	if (ImGui::Combo("Heuristic Method", &selectedIndex, heuristicMethodNames, EHeuristicMethod::NUM_TYPES))
	{
		currentMap_->HeuristicMethod = static_cast<EHeuristicMethod::Type>(selectedIndex);
		currentMap_->MarkDirty();
	}

	RenderTileSetTabs();

	ImGui::End();
}

void ImGuiLayer::RenderTileSetTabs()
{
	ImGuiTabBarFlags tabBarFlags = ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs;

	if (!ImGui::BeginTabBar("TileSetTabBar", tabBarFlags))
	{
		return;
	}

	for (size_t i = 0; i < tileSetViews_.size(); ++i)
	{
		std::string tabName = "TileSet " + std::to_string(i);

		if (!ImGui::BeginTabItem(tabName.c_str(), &tileSetViews_[i].bOpened))
		{
			continue;
		}

		if (ImGui::BeginChild("TileSetChild", ImVec2(-FLT_MIN, 0.0f), ImGuiChildFlags_Borders,
							  ImGuiWindowFlags_HorizontalScrollbar))
		{
			TileSetView& view = tileSetViews_[i];

			if (view.SourceImage)
			{
				int imageWidth = view.SourceImage->GetWidth();
				int imageHeight = view.SourceImage->GetHeight();

				for (int y = 0; y < imageHeight; y += view.TileHeight)
				{
					for (int x = 0; x < imageWidth; x += view.TileWidth)
					{
						ImGui::PushID(static_cast<int>((i << 16) | (y << 8) | x)); // i 16 bits, y 8 bits, x 8 bits

						ImVec2 uv0(static_cast<float>(x) / imageWidth, 1.0f - static_cast<float>(y) / imageHeight);
						ImVec2 uv1(static_cast<float>(x + view.TileWidth) / imageWidth,
								   1.0f - static_cast<float>(y + view.TileHeight) / imageHeight);
						if (ImGui::ImageButton("", view.SourceImage->GetID(), ImVec2(16, 16), uv0, uv1))
						{
							Renderer::TextureRegion region;
							region.UVStart = glm::vec2(uv0.x, uv0.y);
							region.UVEnd = glm::vec2(uv1.x, uv1.y);
							OnTileSelected.Execute(view.SourceImage, region);
						}

						ImGui::PopID();
						ImGui::SameLine();
					}
					ImGui::NewLine();
				}
			}

			ImGui::EndChild();
		}

		ImGui::EndTabItem();
	}

	if (ImGui::TabItemButton("+", ImGuiTabItemFlags_Trailing))
	{
		bShouldOpenNewTileSetPopup_ = true;
	}

	ImGui::EndTabBar();
}

void ImGuiLayer::RenderNewMapPopup()
{
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (!ImGui::BeginPopupModal("New Map"))
	{
		return;
	}

	ImGui::Text("Create a New Map");
	ImGui::Separator();

	static int rows = 30;
	static int columns = 30;
	static int cellSize = 16;

	ImGui::Columns(2);
	ImGui::InputInt("Rows", &rows);
	ImGui::NextColumn();
	ImGui::InputInt("Columns", &columns);
	ImGui::Columns(1);
	ImGui::InputInt("Cell Size", &cellSize);

	if (ImGui::Button("Create", ImVec2(120, 0)))
	{
		CreateNewMap(rows, columns, cellSize);
		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();

	if (ImGui::Button("Cancel", ImVec2(120, 0)))
	{
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}

void ImGuiLayer::RenderNewTileSetPopup()
{
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (!ImGui::BeginPopupModal("New TileSet", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		return;
	}

	ImGui::Text("Create a New TileSet");
	ImGui::Separator();

	static std::string sourceImagePath;
	ImGui::InputText("##path", &sourceImagePath, ImGuiInputTextFlags_ReadOnly);
	ImGui::SameLine();

	if (ImGui::Button("Browse", ImVec2(65, 0)))
	{
		std::filesystem::path result = OpenFileDialog(IMAGE_FILE_FILTER);
		if (!result.empty())
		{
			sourceImagePath = result.string();
		}
	}

	ImGui::Separator();

	static int tileWidth = 16;
	static int tileHeight = 16;
	ImGui::InputInt("Tile Width", &tileWidth);
	ImGui::InputInt("Tile Height", &tileHeight);

	if (ImGui::Button("Create", ImVec2(120, 0)))
	{
		CreateNewTileSet(sourceImagePath, tileWidth, tileHeight);
		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();

	if (ImGui::Button("Cancel", ImVec2(120, 0)))
	{
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}
void ImGuiLayer::RenderLostChangesPopup()
{
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (!ImGui::BeginPopupModal("Lost Changes", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		return;
	}

	ImGui::Text("You have unsaved changes in the current map. Creating a new map will discard these changes.");
	ImGui::Separator();

	if (ImGui::Button("Save", ImVec2(150, 0)))
	{
		// TODO : UI Stack으로 New/Open으로 복귀
		if (SaveMap())
		{
			currentMap_.reset();
			ImGui::CloseCurrentPopup();
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Discard Changes", ImVec2(150, 0)))
	{
		// TODO : UI Stack으로 New/Open으로 복귀
		currentMap_.reset();
		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();

	if (ImGui::Button("Cancel", ImVec2(150, 0)))
	{
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}

void ImGuiLayer::CreateNewMap(int rows, int columns, int cellSize)
{
	currentMap_ = std::make_shared<MapData>(rows, columns, cellSize);
	tileSetDataList_.clear();
	tileSetViews_.clear();
	OnMapRefChanged.Execute(currentMap_);
}

void ImGuiLayer::OpenMap()
{
	std::filesystem::path result = OpenFileDialog(MAP_FILE_FILTER);
	if (result.empty())
	{
		return;
	}

	currentMap_ = std::make_shared<MapData>();
	currentMap_->LoadFromFile(result);
	OnMapRefChanged.Execute(currentMap_);
}

bool ImGuiLayer::SaveMap()
{
	CC_ASSERT(currentMap_);

	if (currentMap_->FilePath.empty())
	{
		currentMap_->FilePath = SaveFileDialog(MAP_FILE_FILTER, MAP_FILE_EXTENSION);
		if (currentMap_->FilePath.empty())
		{
			return false;
		}
	}

	currentMap_->SaveToFile();
	for (const auto& tileSetData : tileSetDataList_)
	{
		std::filesystem::path tileSetJsonPath = currentMap_->FilePath.parent_path();
		tileSetJsonPath /= tileSetData.SourceImagePath.filename().string() + "." + TILESET_FILE_EXTENSION;
		tileSetData.SaveToFile(tileSetJsonPath);
	}
	return true;
}

void ImGuiLayer::CreateNewTileSet(const std::filesystem::path& sourceImagePath, int tileWidth, int tileHeight)
{
	CC_ASSERT(currentMap_);

	TileSetData tileSetData;
	tileSetData.SourceImagePath = sourceImagePath;
	tileSetData.TileWidth = tileWidth;
	tileSetData.TileHeight = tileHeight;
	tileSetDataList_.push_back(tileSetData);

	auto& imageTextureManager = ResourceManager<ImageTexture>::GetInstance();
	auto sourceImage = imageTextureManager.GetOrCreate(sourceImagePath.string(), sourceImagePath);

	int tileCount = tileSetData.GetTotalTileCount(sourceImage->GetWidth(), sourceImage->GetHeight());
	currentMap_->AddTileSetReference(tileCount, sourceImagePath.filename().string() + "." + TILESET_FILE_EXTENSION);

	TileSetView view;
	view.SourceImage = sourceImage;
	view.TileWidth = tileWidth;
	view.TileHeight = tileHeight;
	tileSetViews_.push_back(view);
}