#include "kson/io/kson_io.hpp"
#include <fstream>

using json = nlohmann::json;

bool kson::SaveKSONChartData(std::ostream& stream, const ChartData& chartData)
{
	const json j = chartData;
	stream << j.dump();
	// TODO: Error handling
	return true;
}

bool kson::SaveKSONChartData(const std::string& filePath, const ChartData& chartData)
{
	std::ofstream ofs(filePath);
	if (ofs.good())
	{
		return SaveKSONChartData(ofs, chartData);
	}
	else
	{
		return false;
	}
}