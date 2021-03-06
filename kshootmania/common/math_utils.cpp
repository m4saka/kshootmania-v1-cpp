#include "math_utils.hpp"

int32 MathUtils::NumDigits(int32 number)
{
	return static_cast<int32>(Log10(number) + 1.0);
}
