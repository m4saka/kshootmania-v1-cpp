#include "key_config.hpp"

namespace
{
	// Note: s3d::InputGroup is not used in order to make sure the array size fixed.
	using ConfigSet = std::array<Input, KeyConfig::kButtonEnumCount>;
	std::array<ConfigSet, KeyConfig::kConfigSetEnumCount> s_configSetArray;

	constexpr std::array<InputDeviceType, KeyConfig::kConfigSetEnumCount> kConfigSetDeviceTypes = {
		InputDeviceType::Keyboard,
		InputDeviceType::Keyboard,
		InputDeviceType::Gamepad,
		InputDeviceType::Gamepad,
	};

	constexpr std::array<StringView, KeyConfig::kConfigSetEnumCount> kConfigSetNames = {
		U"Keyboard 1",
		U"Keyboard 2",
		U"Gamepad 1",
		U"Gamepad 2",
	};
}

void KeyConfig::SetConfigValue(ConfigSet targetConfigSet, StringView configValue)
{
	Array<String> values = String(configValue).split(U',');

	if (targetConfigSet < 0 || kConfigSetEnumCount <= targetConfigSet)
	{
		Print << U"Warning: Invalid key config target '{}'!"_fmt(targetConfigSet);
		return;
	}

	if (values.size() != kConfigurableButtonEnumCount)
	{
		Print << U"Warning: Key configuration ({}) is ignored because value count does not match! (expected:{}, actual:{})"_fmt(kConfigSetNames[targetConfigSet], kConfigurableButtonEnumCount, values.size());
		values = String(kDefaultConfigValues[targetConfigSet]).split(U',');
	}

	Array<int32> intValues;
	try
	{
		intValues = values.map([](const String& str) { return Parse<int32>(str); });
	}
	catch (const ParseError&)
	{
		Print << U"Warning: Key configuration ({}) is ignored due to parse error!"_fmt(kConfigSetNames[targetConfigSet]);
		try
		{
			intValues = String(kDefaultConfigValues[targetConfigSet]).split(U',').map([](const String& str) { return Parse<int32>(str); });
		}
		catch (const ParseError&)
		{
			throw Error(U"KeyConfig::SetConfigValue(): Could not parse KeyConfig::kDefaultConfigValues!");
		}
	}

	for (int32 i = 0; i < kConfigurableButtonEnumCount; ++i)
	{
		if (0 <= intValues[i] && intValues[i] < 0x100)
		{
			s_configSetArray[targetConfigSet][i] = Input(kConfigSetDeviceTypes[targetConfigSet], static_cast<uint8>(intValues[i]));
		}
		else
		{
			s_configSetArray[targetConfigSet][i] = Input();
		}
	}

	// Keyboard 1 has keys that cannot be configured by user
	if (targetConfigSet == kKeyboard1)
	{
		s_configSetArray[kKeyboard1][kStart] = KeyEnter;
		s_configSetArray[kKeyboard1][kBack] = KeyEscape;
		s_configSetArray[kKeyboard1][kAutoPlay] = KeyF11;
		s_configSetArray[kKeyboard1][kUp] = KeyUp;
		s_configSetArray[kKeyboard1][kDown] = KeyDown;
		s_configSetArray[kKeyboard1][kLeft] = KeyLeft;
		s_configSetArray[kKeyboard1][kRight] = KeyRight;
	}
}

bool KeyConfig::Pressed(Button button)
{
	if (button == KeyConfig::kUnspecifiedButton)
	{
		return false;
	}

	for (const auto& configSet : s_configSetArray)
	{
		if (configSet[button].pressed())
		{
			return true;
		}
	}
	return false;
}

bool KeyConfig::Down(Button button)
{
	if (button == KeyConfig::kUnspecifiedButton)
	{
		return false;
	}

	for (const auto& configSet : s_configSetArray)
	{
		if (configSet[button].down())
		{
			return true;
		}
	}
	return false;
}

bool KeyConfig::Up(Button button)
{
	if (button == KeyConfig::kUnspecifiedButton)
	{
		return false;
	}

	// All keys need to be released for key up
	if (Pressed(button))
	{
		return false;
	}

	for (const auto& configSet : s_configSetArray)
	{
		if (configSet[button].up())
		{
			return true;
		}
	}
	return false;
}
