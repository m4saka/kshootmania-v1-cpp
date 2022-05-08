#include "ksh/beat/beat_map.hpp"

void ksh::to_json(nlohmann::json& j, const BeatMap& beatMap)
{
	nlohmann::json bpmChanges = {};
	for (const auto [y, bpm] : beatMap.bpmChanges)
	{
		bpmChanges.push_back({
			{ "y", y },
			{ "v", bpm },
		});
	}

	j = {
		{ "bpm", bpmChanges },
		{ "time_sig", beatMap.timeSigChanges },
	};

	if (beatMap.resolution != kDefaultResolution)
	{
		j["resolution"] = beatMap.resolution;
	}
}

void ksh::from_json(const nlohmann::json& j, BeatMap& beatMap)
{
	j.at("bpm").get_to(beatMap.bpmChanges);
	j.at("time_sig").get_to(beatMap.timeSigChanges);
	if (j.contains("resolution"))
	{
		j.at("resolution").get_to(beatMap.resolution);
	}
	else
	{
		beatMap.resolution = kDefaultResolution;
	}
}