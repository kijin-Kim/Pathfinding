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
		case EHeuristicMethod::Type::None:
			return "None";
		case EHeuristicMethod::Type::Manhattan:
			return "Manhattan";
		case EHeuristicMethod::Type::Euclidean:
			return "Euclidean";
		case EHeuristicMethod::Type::Octile:
			return "Octile";
		default:
			return "Unknown";
		}
	}
	inline Type from_string(const std::string& str)
	{
		if (str == "None")
			return EHeuristicMethod::Type::None;
		else if (str == "Manhattan")
			return EHeuristicMethod::Type::Manhattan;
		else if (str == "Euclidean")
			return EHeuristicMethod::Type::Euclidean;
		else if (str == "Octile")
			return EHeuristicMethod::Type::Octile;
		return EHeuristicMethod::Type::None;
	}

} // namespace EHeuristicMethod
