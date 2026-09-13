#pragma once
#include <map>
#include <string>
#include <vector>
#include <unordered_map>
#include "ActionID.h"
enum class InputType
{
	Keyboard,
	GamepadButton,
	GamepadAxis,
	Mouse,
	MouseAxis
};

struct KeyMapping
{
	InputType inputType;
	int code; // キーコード、ゲームパッドのボタン番号、マウスボタン番号など
	float scale = 1.0f; // ゲームパッドの軸など、値にスケールをかけたい場合に使用
};

using KeyConfigMap = std::map<ActionID, std::vector<KeyMapping>>;

class KeyConfig
{
public:
	// シングルトン
	static KeyConfig& GetInstance()
	{
		static KeyConfig instance;
		return instance;
	}
	bool LoadFromFile(const std::wstring& filename);
	const std::vector<KeyMapping>& GetMappings(ActionID action) const;

private:
	KeyConfig();
	~KeyConfig();
	bool StringToActionID(const std::wstring& str, ActionID& actionId) const;
	KeyConfigMap keyConfigMap_;
	int StringToKeyCode(const std::wstring& str) const;
	std::unordered_map<std::wstring, int> keyCodeMap_;
};

