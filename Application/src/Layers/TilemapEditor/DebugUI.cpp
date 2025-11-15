#include "DebugUI.h"

#include "Layers/LayerCommon.h"
#include "MapData.h"
#include "imgui.h"

namespace DebugUI
{
	void DrawMapDataDebugWindow(const MapData& mapData)
	{
		ImGui::Begin("MapData Debug");

		ImGui::Text("Rows: %d, Columns: %d", mapData.Rows, mapData.Columns);
		ImGui::Text("Cell Size: %d", mapData.CellSize);
		ImGui::Text("Start RowCol: (%d, %d)", mapData.StartRow, mapData.StartColumn);
		ImGui::Text("End RowCol: (%d, %d)", mapData.EndRow, mapData.EndColumn);
		ImGui::Text("Heuristic Method: %s", EHeuristicMethod::to_string(mapData.HeuristicMethod));
		ImGui::Text("Simulation Speed: %.2f", mapData.SimulationSpeed);
		ImGui::Text("Next BaseId: %u", mapData.NextBaseId);

		ImGui::Separator();
		ImGui::Text("TileSet References:");
		for (const auto& ref : mapData.TileSetReferences)
		{
			ImGui::Text("  BaseId: %u, FileName: %s", ref.BaseId, ref.TileSetFileName.string().c_str());
		}

		ImGui::Separator();
		if (ImGui::TreeNode("Grid Data"))
		{
			for (size_t row = 0; row < mapData.GridData.size(); ++row)
			{
				ImGui::Text("%s", mapData.GridData[row].c_str());
			}
			ImGui::TreePop();
		}

		ImGui::End();
	}
} // namespace DebugUI