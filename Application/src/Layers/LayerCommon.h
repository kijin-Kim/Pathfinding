#pragma once
#include "Renderer/ImageTexture.h"

#include <string>

namespace EHeuristicMethod
{
	enum Type
	{
		None = 0,
		Manhattan,
		Euclidean,
		Octile,
		NUM_TYPES
	};

	inline const char* to_string(EHeuristicMethod::Type e)
	{
		switch (e)
		{
		case EHeuristicMethod::None:
			return "None";
		case EHeuristicMethod::Manhattan:
			return "Manhattan";
		case EHeuristicMethod::Euclidean:
			return "Euclidean";
		case EHeuristicMethod::Octile:
			return "Octile";
		default:
			return "Unknown";
		}
	}
	inline EHeuristicMethod::Type from_string(const std::string& str)
	{
		if (str == "None")
			return EHeuristicMethod::None;
		else if (str == "Manhattan")
			return EHeuristicMethod::Manhattan;
		else if (str == "Euclidean")
			return EHeuristicMethod::Euclidean;
		else if (str == "Octile")
			return EHeuristicMethod::Octile;
		return EHeuristicMethod::None;
	}

} // namespace EHeuristicMethod
