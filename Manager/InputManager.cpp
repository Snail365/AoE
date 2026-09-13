#include <DxLib.h>

#include "InputManager.h"
#include "KeyConfig.h"

// デバッグ表示用の関数
void InputManager::DrawDebug(int x, int y) const
{
	int curY = y;
	int color = GetColor(255, 255, 255);
	int activeColor = GetColor(255, 255, 0); // 押されている時は黄色

	// 1. キーボード情報 (押されているキーのみ表示)
	DrawString(x, curY, "--- Keyboard (Active Keys) ---", color);
	curY += 20;
	for (int i = 0; i < KEY_COUNT; i++)
	{
		if (key[i] > 0)
		{
			DrawFormatString(x, curY, activeColor, "Key[%d]: %d frames", i, key[i]);
			curY += 20;
		}
	}

	// 2. マウス情報
	DrawFormatString(x, curY, color, "--- Mouse: (%d, %d) ---", mouseX, mouseY);
	curY += 20;
	for (int i = 0; i < MOUSE_BUTTON_COUNT; i++)
	{
		if (mouseButton[i] > 0)
		{
			DrawFormatString(x, curY, activeColor, "MB[%d]: %d frames", i, mouseButton[i]);
			curY += 20;
		}
	}

	// 3. ジョイパッド情報
	for (int no = 0; no < MAX_JOYPADS; no++)
	{
		int dxNo = no + 1;
		// パッドが接続されているか確認
		char deviceName[260];
		char productName[260];
		if (GetJoypadName(dxNo, &deviceName[0], &productName[0]) == -1) continue;

		int type = GetJoypadType(dxNo);
		DrawFormatString(x, curY, color, "--- Pad %d [%s] ---", no, GetPadName(type));
		curY += 20;

		// スティック値
		DrawFormatString(x, curY, color, " L-Stick: %d, %d", padLX[no], padLY[no]);
		curY += 18;
		DrawFormatString(x, curY, color, " R-Stick: %d, %d", padRX[no], padRY[no]);
		curY += 20;

		// ボタン状態 (押されているボタンのみ表示)
		const char* btnNames[] = { "F_Left", "F_Right", "F_Top", "F_Down", "Up", "Down", "Left", "Right", "L_S", "R_S", "L_T", "R_T", "L_StickClick", "R_StickClick", "Start", "Back"};
		for (int b = 0; b < (int)PadButton::Max; b++)
		{
			if (padButton[no][b] > 0)
			{
				DrawFormatString(x + 10, curY, activeColor, "%s: %d frames", btnNames[b], padButton[no][b]);
				curY += 18;
			}
		}
		curY += 10;
	}
}

// ジョイパッドのタイプに応じた名前を返す関数
const char* InputManager::GetPadName(int type) const
{
	switch (type)
	{
		case (int)JOYPAD_TYPE::OTHER: return "Other";
		case (int)JOYPAD_TYPE::XBOX_360: return "Xbox 360";
		case (int)JOYPAD_TYPE::XBOX_ONE: return "Xbox One";
		case (int)JOYPAD_TYPE::DUAL_SHOCK_4: return "DualShock 4";
		case (int)JOYPAD_TYPE::DUAL_SENSE: return "DualSense";
		case (int)JOYPAD_TYPE::SWITCH_JOY_CON_L: return "Switch Joy-Con L";
		case (int)JOYPAD_TYPE::SWITCH_JOY_CON_R: return "Switch Joy-Con R";
		case (int)JOYPAD_TYPE::SWITCH_PRO_CTRL: return "Switch Pro Controller";
		default: return "Unknown";
	}
}

// コントローラーのボタンマッピングを解決する関数
InputManager::InputManager()
{
	// 配列の初期化を明示
	memset(key, 0, sizeof(key));
	memset(prevKey, 0, sizeof(prevKey));
	memset(padButton, 0, sizeof(padButton));
	memset(prevPadButton, 0, sizeof(prevPadButton));
	memset(padLX, 0, sizeof(padLX));
	memset(padLY, 0, sizeof(padLY));
	memset(padRX, 0, sizeof(padRX));
	memset(padRY, 0, sizeof(padRY));
	memset(mouseButton, 0, sizeof(mouseButton));
	memset(prevMouseButton, 0, sizeof(prevMouseButton));
	mouseX = mouseY = prevMouseX = prevMouseY = 0;

	_layerStack.push_back(InputLayer());
}

InputManager::~InputManager()
{
	ClearAllCallbacks();
}

void InputManager::Update()
{
	// キー入力
	for (int i = 0; i < KEY_COUNT; i++)
	{
		prevKey[i] = key[i];
	}
	char tmpKey[KEY_COUNT];
	GetHitKeyStateAll(tmpKey);
	for (int i = 0; i < KEY_COUNT; i++)
	{
		key[i] = (tmpKey[i] != 0) ? key[i] + 1 : 0;
	}

	// マウス座標
	prevMouseX = mouseX;
	prevMouseY = mouseY;
	GetMousePoint(&mouseX, &mouseY);

	// マウスボタン
	int mouseInput = GetMouseInput();
	for (int i = 0; i < MOUSE_BUTTON_COUNT; i++)
	{
		prevMouseButton[i] = mouseButton[i];
	}

	mouseButton[0] = (mouseInput & MOUSE_INPUT_LEFT) ? mouseButton[0] + 1 : 0;
	mouseButton[1] = (mouseInput & MOUSE_INPUT_RIGHT) ? mouseButton[1] + 1 : 0;
	mouseButton[2] = (mouseInput & MOUSE_INPUT_MIDDLE) ? mouseButton[2] + 1 : 0;

	// コントローラー
	for (int no = 0; no < MAX_JOYPADS; no++)
	{
		prevPadLX[no] = padLX[no];
		prevPadLY[no] = padLY[no];
		prevPadRX[no] = padRX[no];
		prevPadRY[no] = padRY[no];
		int dxNo = no + 1; // DxLibのジョイパッド番号は1から始まる
		int type = GetJoypadType(dxNo);
		DINPUT_JOYSTATE dState;
		XINPUT_STATE xState;
		int dResult = GetJoypadDirectInputState(dxNo, &dState);
		int xResult = GetJoypadXInputState(dxNo, &xState);

		if (dResult == -1 && xResult == -1)
		{
			// パッドが接続されていないか、両方の入力取得に失敗した場合はスキップ
			continue;
		}
		// 正常に取得できた場合は、規格に応じてスティック値を更新する
		if (type == DX_PADTYPE_XBOX_360 || type == DX_PADTYPE_XBOX_ONE)
		{
			padLX[no] = xState.ThumbLX;
			padLY[no] = xState.ThumbLY;
			padRX[no] = xState.ThumbRX;
			padRY[no] = xState.ThumbRY;
		}
		else
		{
			padLX[no] = dState.X;
			padLY[no] = dState.Y;
			padRX[no] = dState.Z;
			padRY[no] = dState.Rz;
		}

		for (int btn = 0; btn < (int)PadButton::Max; btn++)
		{
			prevPadButton[no][btn] = padButton[no][btn];
			if (GetRawState(no, (PadButton)btn, dState, xState))
			{
				padButton[no][btn]++;
			}
			else
			{
				padButton[no][btn] = 0;
			}
		}
	}

	DispatchCallbacks();
}

// キーボード入力
bool InputManager::IsKeyPressed(int keyCode) const
{
	return IsInputPressed(key[keyCode]);
}
bool InputManager::IsKeyTriggered(int keyCode) const
{
	return IsInputTriggered(key[keyCode], prevKey[keyCode]);
}
bool InputManager::IsKeyReleased(int keyCode) const
{
	return IsInputReleased(key[keyCode], prevKey[keyCode]);
}

// マウス入力
bool InputManager::IsMousePressed(int button) const
{
	return IsInputPressed(mouseButton[(int)button]);
}
bool InputManager::IsMouseTriggered(int button) const
{
	return IsInputTriggered(mouseButton[(int)button], prevMouseButton[(int)button]);
}
bool InputManager::IsMouseReleased(int button) const
{
	return IsInputReleased(mouseButton[(int)button], prevMouseButton[(int)button]);
}
// アナログ値取得(0.0~1.0ではない)
float InputManager::GetMouseAxisValue(MouseAxis axis) const
{
	if (axis == MouseAxis::Mouse_X)
	{
		return (float)(mouseX - prevMouseX);
	}
	if (axis == MouseAxis::Mouse_Y)
	{
		return (float)(mouseY - prevMouseY);
	}
	if (axis == MouseAxis::Mouse_Dist)
	{
		int dx = mouseX - prevMouseX;
		int dy = mouseY - prevMouseY;
		return sqrtf(dx * dx + dy * dy);
	}
	return 0.0f;
}
// スティックの値を-1.0f〜1.0fの範囲で取得する関数
float InputManager::GetPadAxisValue(PadAxis axis, int padNo) const
{
	int rawValue = 0;
	switch (axis)
	{
		case PadAxis::Pad_L_X: rawValue = padLX[padNo]; break;
		case PadAxis::Pad_L_Y: rawValue = padLY[padNo]; break;
		case PadAxis::Pad_R_X: rawValue = padRX[padNo]; break;
		case PadAxis::Pad_R_Y: rawValue = padRY[padNo]; break;
		case PadAxis::Pad_L_Dist:
			{
				int dx = padLX[padNo] - prevPadLX[padNo];
				int dy = padLY[padNo] - prevPadLY[padNo];
				rawValue = (int)sqrtf(dx * dx + dy * dy);
				break;
			}
		case PadAxis::Pad_R_Dist:
			{
				int dx = padRX[padNo] - prevPadRX[padNo];
				int dy = padRY[padNo] - prevPadRY[padNo];
				rawValue = (int)sqrtf(dx * dx + dy * dy);
				break;
			}
	}

	if (abs(rawValue) < DEADZONE) return 0.0f; // デッドゾーン内は0とみなす

	float normalized = (float)(abs(rawValue) - DEADZONE) / (STICK_MAX - DEADZONE); // デッドゾーンを除いた範囲で正規化
	return (rawValue > 0) ? normalized : -normalized;
}
// コントローラー入力
bool InputManager::IsPadPressed(int no, PadButton btn) const
{
	return IsInputPressed(padButton[no][(int)btn]);
}
bool InputManager::IsPadTriggered(int no, PadButton btn) const
{
	return IsInputTriggered(padButton[no][(int)btn], prevPadButton[no][(int)btn]);
}
bool InputManager::IsPadReleased(int no, PadButton btn) const
{
	return IsInputReleased(padButton[no][(int)btn], prevPadButton[no][(int)btn]);
}

bool InputManager::IsPadConnect() const
{
	return GetJoypadNum() > 0;
}


// アクションに対する入力値を取得する関数
// ボタン
float InputManager::GetActionValue(ActionID action, int padNo) const
{
	const std::vector<KeyMapping> mappings = KeyConfig::GetInstance().GetMappings(action);
	if (mappings.empty()) return 0.0f; // アクションにマッピングがない場合は0を返す

	float total = 0.0f;

	for (const auto& m : mappings)
	{

		float val = 0.0f;
		switch (m.inputType)
		{
			case InputType::Keyboard:
				val = IsKeyPressed(m.code) ? 1.0f : 0.0f;
				break;
			case InputType::GamepadButton:
				val = IsPadPressed(padNo, (PadButton)m.code) ? 1.0f : 0.0f;
				break;
			case InputType::Mouse:
				val = IsMousePressed(m.code) ? 1.0f : 0.0f;
				break;
		}
		total += val * m.scale; // スケールをかけて加算
	}
	// 複数の入力がある場合は合算するが、値が-1.0f〜1.0fの範囲を超えないようにクランプする
	return (total > 1.0f) ? 1.0f : (total < -1.0f) ? -1.0f : total;
}
// Axis
float InputManager::GetActionAxis(ActionID action, int padNo) const
{
	const std::vector<KeyMapping> mappings = KeyConfig::GetInstance().GetMappings(action);
	if (mappings.empty()) return 0.0f; // アクションにマッピングがない場合は0を返す
	float total = 0.0f;

	for (const auto& m : mappings)
	{
		float val = 0.0f;
		switch (m.inputType)
		{
			case InputType::GamepadAxis:
				val = GetPadAxisValue((PadAxis)m.code, padNo);
				break;
			case InputType::MouseAxis:
				val = GetMouseAxisValue((MouseAxis)m.code);
				break;
		}
		total += val * m.scale;
	}
	return total;
}
// アクションに対する入力状態を取得する関数
bool InputManager::IsActionPressed(ActionID action, int padNo) const
{
	const auto& mappings = KeyConfig::GetInstance().GetMappings(action);
	if (mappings.empty()) return false; // アクションにマッピングがない場合はfalseを返す
	for (const auto& m : mappings)
	{
		switch (m.inputType)
		{
			case InputType::Keyboard:
				if (IsKeyPressed(m.code)) return true;
				break;
			case InputType::GamepadButton:
				if (IsPadPressed(padNo, (PadButton)m.code)) return true;
				break;
			case InputType::Mouse:
				if (IsMousePressed(m.code)) return true;
				break;
		}
	}
	return false;
}
bool InputManager::IsActionTriggered(ActionID action, int padNo) const
{
	const auto& mappings = KeyConfig::GetInstance().GetMappings(action);
	if (mappings.empty()) return false; // アクションにマッピングがない場合はfalseを返す
	for (const auto& m : mappings)
	{
		switch (m.inputType)
		{
			case InputType::Keyboard:
				if (IsKeyTriggered(m.code)) return true;
				break;
			case InputType::GamepadButton:
				if (IsPadTriggered(padNo, (PadButton)m.code)) return true;
				break;
			case InputType::Mouse:
				if (IsMouseTriggered(m.code)) return true;
				break;
		}
	}
	return false;
}
bool InputManager::IsActionReleased(ActionID action, int padNo) const
{
	const auto& mappings = KeyConfig::GetInstance().GetMappings(action);
	if (mappings.empty()) return false; // アクションにマッピングがない場合はfalseを返す
	for (const auto& m : mappings)
	{
		switch (m.inputType)
		{
			case InputType::Keyboard:
				if (IsKeyReleased(m.code)) return true;
				break;
			case InputType::GamepadButton:
				if (IsPadReleased(padNo, (PadButton)m.code)) return true;
				break;
			case InputType::Mouse:
				if (IsMouseReleased(m.code)) return true;
				break;
		}
	}
	return false;
}
// アクションに対するコールバックの登録
void InputManager::SetAxisCallback(ActionID action, ActionCallback callback)
{
	GetCurrentLayer().axisCallbacks[action] = callback;
}
void InputManager::SetTriggerCallback(ActionID action, ActionCallback callback)
{
	GetCurrentLayer().triggerCallbacks[action] = callback;
}
void InputManager::SetPressCallback(ActionID action, ActionCallback callback)
{
	GetCurrentLayer().pressCallbacks[action] = callback;
}
void InputManager::SetReleaseCallback(ActionID action, ActionCallback callback)
{
	GetCurrentLayer().releaseCallbacks[action] = callback;
}
// 登録されたコールバックのクリア
void InputManager::ClearAxisCallbacks()
{
	GetCurrentLayer().axisCallbacks.clear();
}
void InputManager::ClearTriggerCallbacks()
{
	GetCurrentLayer().triggerCallbacks.clear();
}
void InputManager::ClearPressCallbacks()
{
	GetCurrentLayer().pressCallbacks.clear();
}
void InputManager::ClearReleaseCallbacks()
{
	GetCurrentLayer().releaseCallbacks.clear();
}

void InputManager::ClearAllCallbacks()
{
	ClearAxisCallbacks();
	ClearTriggerCallbacks();
	ClearPressCallbacks();
	ClearReleaseCallbacks();
}

void InputManager::StartVibration(int Power, int Time, int EffectIndex, int padNo)
{
	int inputNo = padNo + 1; // DxLibのジョイパッド番号は1から始まる
	StopVibration(padNo); // 先に振動を停止してから開始することで、連続して呼び出された場合の不具合を防止
	StartJoypadVibration(inputNo, Power, Time, EffectIndex);
}

void InputManager::StopVibration(int padNo)
{
	int inputNo = padNo + 1; // DxLibのジョイパッド番号は1から始まる
	StopJoypadVibration(inputNo);
}

void InputManager::DispatchCallbacks()
{
	if (_layerStack.empty()) return; // レイヤーがない場合は処理をスキップ

	auto axisSnapshot = GetCurrentLayer().axisCallbacks; // 軸コールバックのスナップショットを作成
	for (const auto& [action, callback] : axisSnapshot)
	{
		if (GetActionAxis(action)) callback();
	}
	auto triggerSnapshot = GetCurrentLayer().triggerCallbacks; // トリガーコールバックのスナップショットを作成
	for (const auto& [action, callback] : triggerSnapshot)
	{
		if (IsActionTriggered(action)) callback();
	}
	auto pressSnapshot = GetCurrentLayer().pressCallbacks; // 押下コールバックのスナップショットを作成
	for (const auto& [action, callback] : pressSnapshot)
	{
		if (IsActionPressed(action)) callback();
	}
	auto releaseSnapshot = GetCurrentLayer().releaseCallbacks; // 解放コールバックのスナップショットを作成
	for (const auto& [action, callback] : releaseSnapshot)
	{
		if (IsActionReleased(action)) callback();
	}
}

bool InputManager::GetRawState(int padNo, PadButton btn, const DINPUT_JOYSTATE& dState, const XINPUT_STATE& xState)
{
	int dxNo = padNo + 1;
	int type = GetJoypadType(dxNo);

	switch (btn)
	{
		case PadButton::Pad_Face_Left: // Xbox:X, PS:□
			if (type == DX_PADTYPE_DUAL_SENSE) return dState.Buttons[0] != 0;
			return (dState.Buttons[2] != 0) || (xState.Buttons[XINPUT_BUTTON_X] != 0);

		case PadButton::Pad_Face_Right: // Xbox:B, PS:〇
			if (type == DX_PADTYPE_DUAL_SENSE) return dState.Buttons[2] != 0;
			return (dState.Buttons[1] != 0) || (xState.Buttons[XINPUT_BUTTON_B] != 0);

		case PadButton::Pad_Face_Top: // Xbox:Y, PS:△
			return (dState.Buttons[3] != 0) || (xState.Buttons[XINPUT_BUTTON_Y] != 0);

		case PadButton::Pad_Face_Down: // Xbox:A, PS:×
			if (type == DX_PADTYPE_DUAL_SENSE) return dState.Buttons[1] != 0;
			return (dState.Buttons[0] != 0) || (xState.Buttons[XINPUT_BUTTON_A] != 0);

		case PadButton::Pad_Left:
			return (dState.POV[0] == 27000) || (xState.Buttons[XINPUT_BUTTON_DPAD_LEFT] != 0);
		case PadButton::Pad_Right:
			return (dState.POV[0] == 9000) || (xState.Buttons[XINPUT_BUTTON_DPAD_RIGHT] != 0);
		case PadButton::Pad_Up:
			return (dState.POV[0] == 0) || (xState.Buttons[XINPUT_BUTTON_DPAD_UP] != 0);
		case PadButton::Pad_Down:
			return (dState.POV[0] == 18000) || (xState.Buttons[XINPUT_BUTTON_DPAD_DOWN] != 0);

		case PadButton::Pad_L_Shoulder: return (dState.Buttons[4] != 0) || (xState.Buttons[XINPUT_BUTTON_LEFT_SHOULDER] != 0);
		case PadButton::Pad_R_Shoulder: return (dState.Buttons[5] != 0) || (xState.Buttons[XINPUT_BUTTON_RIGHT_SHOULDER] != 0);

		case PadButton::Pad_L_Trigger:
			// XInputは0-255の値が来るので、30以上を「押し込み」と判定
			if (type == DX_PADTYPE_XBOX_360 || type == DX_PADTYPE_XBOX_ONE) return xState.LeftTrigger > 30;
			return (dState.Buttons[6] != 0);
		case PadButton::Pad_R_Trigger:
			if (type == DX_PADTYPE_XBOX_360 || type == DX_PADTYPE_XBOX_ONE) return xState.RightTrigger > 30;
			return (dState.Buttons[7] != 0);
		case PadButton::Pad_L_Thumb: return (dState.Buttons[8] != 0) || (xState.Buttons[XINPUT_BUTTON_LEFT_THUMB] != 0);
		case PadButton::Pad_R_Thumb: return (dState.Buttons[9] != 0) || (xState.Buttons[XINPUT_BUTTON_RIGHT_THUMB] != 0);
		case PadButton::Xbox_Start:
			return (dState.Buttons[10] != 0) || (xState.Buttons[XINPUT_BUTTON_START] != 0);
		case PadButton::Xbox_Back:
			return (dState.Buttons[11] != 0) || (xState.Buttons[XINPUT_BUTTON_BACK] != 0);
		default: return false;
	}
}