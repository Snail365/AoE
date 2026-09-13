#pragma once
#include <string>
#include <functional>
#include <vector>
#include <map>
#include "ActionID.h"

enum class PadAxis { Pad_L_X, Pad_L_Y, Pad_R_X, Pad_R_Y, Pad_L_Dist, Pad_R_Dist };
enum class MouseAxis { Mouse_X, Mouse_Y, Mouse_Dist };
// ゲームコントローラーの認識番
// DxLib定数、DX_INPUT_PAD1等に対応
enum class JOYPAD_NO
{
	KEY_PAD1 = 0,			// キー入力とパッド１入力
	PAD1 = 1,				// パッド１入力
	PAD2 = 2,				// パッド２入力
	PAD3 = 3,				// パッド３入力
	PAD4 = 4,				// パッド４入力
	INPUT_KEY = 4096	// キー入力
};

// ゲームコントローラータイプ
// DxLib定数、DX_OTHER等に対応
enum class JOYPAD_TYPE
{
	OTHER = 0,
	XBOX_360,
	XBOX_ONE,
	DUAL_SHOCK_4,
	DUAL_SENSE,
	SWITCH_JOY_CON_L,
	SWITCH_JOY_CON_R,
	SWITCH_PRO_CTRL,
	MAX
};

// マウスボタンの種類
enum class MouseButton
{
	Mouse_Left = 0,
	Mouse_Right,
	Mouse_Middle,
};

// コントローラーのボタンの種類
enum class PadButton
{
	Pad_Face_Left = 0,
	Pad_Face_Right,
	Pad_Face_Top,
	Pad_Face_Down,
	Pad_Left,
	Pad_Right,
	Pad_Up,
	Pad_Down,
	Pad_L_Shoulder,
	Pad_R_Shoulder,
	Pad_L_Trigger,
	Pad_R_Trigger,
	Pad_L_Thumb,
	Pad_R_Thumb,
	Xbox_Start,// Xbox:Start
	Xbox_Back, // Xbox:Back
	Max
};

// アクションIDに対する入力状態を取得する関数
using ActionCallback = std::function<void()>;

class InputManager
{
private:
	InputManager();
	~InputManager();
	static constexpr int KEY_COUNT = 256;
	static constexpr int MOUSE_BUTTON_COUNT = 3;
	static constexpr int MAX_JOYPADS = 4;
	static constexpr float DEADZONE = 200.0f; // スティックのデッドゾーン
	static constexpr float STICK_MAX = 32767.0f; // スティックの最大値
	static constexpr float AXIS_THRESHOLD = 0.5f; // 軸入力を「押された」とみなす閾値

public:
	// シングルトン
	static InputManager& GetInstance()
	{
		static InputManager instance;
		return instance;
	}

	void Update();
	void DrawDebug(int x, int y) const;

	// 新しいレイヤーを追加
	void PushLayer() { _layerStack.push_back(InputLayer()); }
	// 最前のレイヤーを削除して、前のレイヤーに戻る
	void PopLayer() { if (_layerStack.size() > 1) _layerStack.pop_back(); }

	// キー入力フレーム数取得
	int GetKeyFrameCount(int keyCode) const { return key[keyCode]; }

	// マウス現在座標取得
	int GetMouseX() const { return mouseX; }
	int GetMouseY() const { return mouseY; }

	// アクションに対する入力状態を取得する関数
	float GetActionValue(ActionID action, int padNo = 0) const; // ボタン
	float GetActionAxis(ActionID action, int padNo = 0) const; // 軸

	// アナログ値取得
	float GetMouseAxisValue(MouseAxis axis) const; // 変化量を取得(正規化はしていない)
	float GetPadAxisValue(PadAxis axis, int padNo = 0) const;// -1.0～1.0で取得

	// アクションに対するコールバックの登録
	void SetAxisCallback(ActionID action, ActionCallback callback);
	void SetTriggerCallback(ActionID action, ActionCallback callback);
	void SetPressCallback(ActionID action, ActionCallback callback);
	void SetReleaseCallback(ActionID action, ActionCallback callback);
	// 登録されたコールバックのクリア
	void ClearAxisCallbacks();
	void ClearTriggerCallbacks();
	void ClearPressCallbacks();
	void ClearReleaseCallbacks();
	void ClearAllCallbacks();

	// アクションに対する入力状態を取得する関数
	bool IsActionPressed(ActionID action, int padNo = 0) const;// 押されている間
	bool IsActionTriggered(ActionID action, int padNo = 0) const;// 押された瞬間
	bool IsActionReleased(ActionID action, int padNo = 0) const;// 離された瞬間

	// 振動機能
	void StartVibration(int Power, int Time = -1, int EffectIndex = -1, int padNo = 0);
	void StopVibration(int padNo = 0);

	// キー入力
	bool IsKeyPressed(int keyCode) const;
	bool IsKeyTriggered(int keyCode) const;
	bool IsKeyReleased(int keyCode) const;

	// マウス入力
	bool IsMousePressed(int button) const;
	bool IsMouseTriggered(int button) const;
	bool IsMouseReleased(int button) const;
	// パッド入力
	bool IsPadPressed(int no, PadButton btn) const;
	bool IsPadTriggered(int no, PadButton btn) const;
	bool IsPadReleased(int no, PadButton btn) const;
	// パッドが接続されているかどうか
	bool IsPadConnect() const;
private:
	struct InputLayer
	{
		std::map<ActionID, ActionCallback> axisCallbacks;
		std::map<ActionID, ActionCallback> triggerCallbacks;
		std::map<ActionID, ActionCallback> pressCallbacks;
		std::map<ActionID, ActionCallback> releaseCallbacks;
	};

	std::vector<InputLayer> _layerStack = {InputLayer()};

	// 最前のレイヤーを取得するヘルパー関数
	InputLayer& GetCurrentLayer() { return _layerStack.back(); }
	const InputLayer& GetCurrentLayer() const { return _layerStack.back(); }

	// キー入力
	int key[KEY_COUNT] = {};
	int prevKey[KEY_COUNT] = {};

	// マウスボタンの入力状態
	int mouseButton[MOUSE_BUTTON_COUNT] = {}; // 0:左 1:右 2:中 
	int prevMouseButton[MOUSE_BUTTON_COUNT] = {};

	int mouseX = 0;
	int mouseY = 0;
	int prevMouseX = 0;
	int prevMouseY = 0;

	// パッド入力状態
	int padButton[MAX_JOYPADS][(int)PadButton::Max] = {};
	int prevPadButton[MAX_JOYPADS][(int)PadButton::Max] = {};
	int padLX[MAX_JOYPADS] = {};
	int prevPadLX[MAX_JOYPADS] = {};
	int padLY[MAX_JOYPADS] = {};
	int prevPadLY[MAX_JOYPADS] = {};
	int padRX[MAX_JOYPADS] = {};
	int prevPadRX[MAX_JOYPADS] = {};
	int padRY[MAX_JOYPADS] = {};
	int prevPadRY[MAX_JOYPADS] = {};

	// コントローラーごとのボタンマッピングを解決するための関数
	bool GetRawState(int padNo, PadButton btn, const DINPUT_JOYSTATE& dState, const XINPUT_STATE& xState);
	// コントローラーの種類取得
	const char* GetPadName(int type) const;

	// 入力状態の判定を行う共通関数
	bool IsInputTriggered(int current, int previous) const { return current == 1; }
	bool IsInputPressed(int current) const { return current > 0; }
	bool IsInputReleased(int current, int previous) const { return current == 0 && previous > 0; }


	// 登録されたコールバックを呼び出す関数
	void DispatchCallbacks();
};
