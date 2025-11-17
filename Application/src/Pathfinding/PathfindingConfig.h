#pragma once

#include "glm/vec4.hpp"

namespace PathfindingConfig
{
	constexpr float WALL_DENSITY = 0.3f;

	constexpr float BASE_STEP_INTERVAL = 0.01f;

	constexpr float DIAGONAL_COST = 1.4142135f;
	constexpr float ORTHOGONAL_COST = 1.0f;
	constexpr float IMPASSABLE_COST = std::numeric_limits<float>::max();

	namespace Colors
	{
		constexpr glm::vec4 GRID_LINE = glm::vec4(0.5f, 0.5f, 0.5f, 0.3f);
		constexpr glm::vec4 PATH_LINE = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		constexpr glm::vec4 CLOSED_NODE = glm::vec4(0.27f, 0.68f, 0.73f, 0.3f);
		constexpr glm::vec4 OPEN_NODE = glm::vec4(0.0f, 1.0f, 1.0f, 0.3f);
		constexpr glm::vec4 START_NODE = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		constexpr glm::vec4 END_NODE = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
		constexpr glm::vec4 WALL_TILE = glm::vec4(1.0f, 0.0f, 0.0f, 0.3f);
		constexpr glm::vec4 PATH_TILE = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
		constexpr glm::vec4 CLEAR_COLOR = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
	}

	constexpr float GRID_LINE_WIDTH = 1.0f;
	constexpr float PATH_LINE_WIDTH = 3.0f;
}
