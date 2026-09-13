#include "KeyConfig.h"
#include <fstream>
#include <sstream>
#include <DxLib.h>
#include "InputManager.h"

KeyConfig::KeyConfig()
{
	// デフォルトのキー設定をここで定義
	//keyConfigMap_ = {
	//	{ ActionID::MoveH,    { 
	//		{InputType::Keyboard, KEY_INPUT_D, 1.0f},
	//		{InputType::Keyboard, KEY_INPUT_A, -1.0f},
	//		{InputType::GamepadAxis, (int)PadAxis::Pad_L_X, 1.0f}
	//	}	},
	//	{ ActionID::MoveV,    { 
	//		{InputType::Keyboard, KEY_INPUT_W, -1.0f},
	//		{InputType::Keyboard, KEY_INPUT_S, 1.0f},
	//		{InputType::GamepadAxis, (int)PadAxis::Pad_L_Y, 1.0f}
	//	}	},
	//	{ ActionID::Jump,		{ 
	//		{InputType::Keyboard, KEY_INPUT_SPACE, 1.0f},
	//		{InputType::GamepadButton, (int)PadButton::Pad_Face_Down, 1.0f}
	//	}	} };
	LoadFromFile(L"KeyConfig/KeyConfig.txt"); // デフォルトのキー設定ファイルを読み込む
}


KeyConfig::~KeyConfig()
{
}

bool KeyConfig::StringToActionID(const std::wstring& str, ActionID& actionId) const
{
	if (str == L"MoveH") { actionId = ActionID::MoveH;  return true; } // X軸移動
	if (str == L"MoveHAxis") { actionId = ActionID::MoveHAxis;  return true; } // アナログ
	if (str == L"MoveV") { actionId = ActionID::MoveV;  return true; } // Y軸移動
	if (str == L"MoveVAxis") { actionId = ActionID::MoveVAxis;  return true; } // アナログ
	if (str == L"Jump") { actionId = ActionID::Jump;   return true; } // 通常ジャンプ
	if (str == L"SJump") { actionId = ActionID::SJump;   return true; } // 炭酸の発射によるジャンプ
	if (str == L"Shake") { actionId = ActionID::Shake;   return true; } // 炭酸のシェイク動作
	if (str == L"Rotate") { actionId = ActionID::Rotate; return true; }
	if (str == L"Shake") { actionId = ActionID::Shake; return true; }
	if (str == L"Decide") { actionId = ActionID::Decide; return true; }
	if (str == L"Cancel") { actionId = ActionID::Cancel; return true; }
	if (str == L"Pause") { actionId = ActionID::Pause; return true; }
	if (str == L"Exit") { actionId = ActionID::Exit;   return true; }
	else return false; // 未知のアクション名は無視
}

int KeyConfig::StringToKeyCode(const std::wstring& str) const
{
	// 文字列とDxLib定数の対応表
	// staticにすることで、関数を呼ぶたびにテーブルが生成されるのを防ぐ
	static const std::unordered_map<std::wstring, int> keyMap = {
		{ L"KEY_INPUT_ESCAPE", KEY_INPUT_ESCAPE },
		{ L"KEY_INPUT_TAB", KEY_INPUT_TAB },
		{ L"KEY_INPUT_SPACE",  KEY_INPUT_SPACE },
		{ L"KEY_INPUT_RETURN", KEY_INPUT_RETURN },
		{ L"KEY_INPUT_LEFT",   KEY_INPUT_LEFT },
		{ L"KEY_INPUT_RIGHT",  KEY_INPUT_RIGHT },
		{ L"KEY_INPUT_UP",     KEY_INPUT_UP },
		{ L"KEY_INPUT_DOWN",   KEY_INPUT_DOWN },
		{ L"KEY_INPUT_A",      KEY_INPUT_A },
		{ L"KEY_INPUT_D",      KEY_INPUT_D },
		{ L"KEY_INPUT_W",      KEY_INPUT_W },
		{ L"KEY_INPUT_S",      KEY_INPUT_S },
		{ L"KEY_INPUT_LSHIFT", KEY_INPUT_LSHIFT },
		{ L"Mouse_Left", (int)MouseButton::Mouse_Left},
		{ L"Mouse_Right", (int)MouseButton::Mouse_Right},
		{ L"Mouse_Middle", (int)MouseButton::Mouse_Middle},
		{ L"Mouse_X", (int)MouseAxis::Mouse_X},
		{ L"Mouse_Y", (int)MouseAxis::Mouse_Y},
		{ L"Mouse_Dist", (int)MouseAxis::Mouse_Dist},
		{ L"Pad_L_X", (int)PadAxis::Pad_L_X},
		{ L"Pad_L_Y", (int)PadAxis::Pad_L_Y},
		{ L"Pad_L_Dist", (int)PadAxis::Pad_L_Dist},
		{ L"Pad_R_X", (int)PadAxis::Pad_R_X},
		{ L"Pad_R_Y", (int)PadAxis::Pad_R_Y},
		{ L"Pad_R_Dist", (int)PadAxis::Pad_R_Dist},
		{ L"Pad_Face_Left", (int)PadButton::Pad_Face_Left},
		{ L"Pad_Face_Right", (int)PadButton::Pad_Face_Right},
		{ L"Pad_Face_Top", (int)PadButton::Pad_Face_Top},
		{ L"Pad_Face_Down", (int)PadButton::Pad_Face_Down},
		{ L"Pad_Left", (int)PadButton::Pad_Left},
		{ L"Pad_Right", (int)PadButton::Pad_Right},
		{ L"Pad_Up", (int)PadButton::Pad_Up},
		{ L"Pad_Down", (int)PadButton::Pad_Down},
		{ L"Pad_L_Shoulder", (int)PadButton::Pad_L_Shoulder},
		{ L"Pad_R_Shoulder", (int)PadButton::Pad_R_Shoulder},
		{ L"Pad_L_Trigger", (int)PadButton::Pad_L_Trigger},
		{ L"Pad_R_Trigger", (int)PadButton::Pad_R_Trigger},
		{ L"Pad_L_Thumb", (int)PadButton::Pad_L_Thumb},
		{ L"Pad_R_Thumb", (int)PadButton::Pad_R_Thumb},
		{ L"Xbox_Start", (int)PadButton::Xbox_Start},
		{ L"Xbox_Back", (int)PadButton::Xbox_Back},
		// 必要なキーをここに追加
	};

	auto it = keyMap.find(str);
	if (it != keyMap.end())
	{
		return it->second;
	}
	printfDx("Unknown Key: %S\n", str.c_str());
	// もしテーブルになければ、従来の数値読み込みを試みる
	try
	{
		return std::stoi(str);
	}
	catch (...)
	{
		return -1; // 変換不能な場合は-1を返す
	}
}

bool KeyConfig::LoadFromFile(const std::wstring& filename)
{
	std::wifstream file(filename);
	if (!file.is_open())
	{
		printfDx("キー設定ファイルの読み込みに失敗: %ls\n", filename.c_str());
		return false;
	}
	keyConfigMap_.clear();
	std::wstring line;
	while (std::getline(file, line))
	{
		if (line.empty() || line[0] == L'#') continue; // 空行やコメントをスキップ

		std::wstringstream ss(line);
		std::wstring actionStr, typeStr, codeStr, scaleStr;

		// 形式例:Jump,Keyboard,KEY_INPUT_SPACE
		if (std::getline(ss, actionStr, L',') &&
			std::getline(ss, typeStr, L',') && 
			std::getline(ss, codeStr, L','))
		{
			if (!std::getline(ss, scaleStr, L','))
			{
				scaleStr = L"1.0";
			}
			// 読み込んだ各変数の空白をトリム
			auto Trim = [](std::wstring& s)
				{
					s.erase(0, s.find_first_not_of(L" \t"));
					s.erase(s.find_last_not_of(L" \t") + 1);
				};
			Trim(actionStr);
			Trim(typeStr);
			Trim(codeStr);
			Trim(scaleStr);

			ActionID actionId;
			if (StringToActionID(actionStr, actionId))
			{
				KeyMapping mapping;
				mapping.code = StringToKeyCode(codeStr); // KEY＿INPUT_SPACEなどのキーコードを数値に変更
				try
				{
					mapping.scale = std::stof(scaleStr);
				}
				catch (...)
				{
					mapping.scale = 1.0f;
				}
	
				if (typeStr == L"Keyboard") mapping.inputType = InputType::Keyboard;
				else if (typeStr == L"GamepadButton") mapping.inputType = InputType::GamepadButton;
				else if (typeStr == L"GamepadAxis") mapping.inputType = InputType::GamepadAxis;
				else if (typeStr == L"Mouse") mapping.inputType = InputType::Mouse;
				else if (typeStr == L"MouseAxis") mapping.inputType = InputType::MouseAxis;

				keyConfigMap_[actionId].push_back(mapping);
			}
			// else: 未知のアクション名は無視
		}

	}
	return true;
}

const std::vector<KeyMapping>& KeyConfig::GetMappings(ActionID action) const
{
	static const std::vector<KeyMapping> empty; // アクションが見つからない場合の空のベクター
	auto it = keyConfigMap_.find(action);
	return (it != keyConfigMap_.end()) ? it->second : empty;
}