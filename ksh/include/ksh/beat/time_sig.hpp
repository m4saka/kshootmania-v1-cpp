#pragma once
#include "ksh/common/common.hpp"

namespace ksh
{
    struct TimeSig
    {
        int64_t numerator = 4;
        int64_t denominator = 4;
    };

	inline void to_json(nlohmann::json& j, const TimeSig& timeSig)
	{
		j = {
			{ "n", timeSig.numerator },
			{ "d", timeSig.denominator },
		};
	}

	inline void from_json(const nlohmann::json& j, TimeSig& timeSig)
	{
		j.at("n").get_to(timeSig.numerator);
		j.at("d").get_to(timeSig.denominator);
	}
}