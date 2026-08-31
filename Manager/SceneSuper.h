#pragma once
#include <DxLib.h>
#include "FileManager.h"
class SceneManager;

class SceneSuper
{
public:
	enum class SceneID
	{
		NONE = -1,
		TITLE,
		STAGE_SELECT,
		GAME,
		RESULT,
		PAUSE,
		EXIT,
	};

	// SceneからFileManagerを消さないようにポインタ参照
	SceneSuper(FileManager& fileMng, SceneManager& sceneMng) : fileMng_(fileMng), sceneMng_(sceneMng) {};
	virtual ~SceneSuper() = default;
	virtual void Update() = 0;
	virtual void Draw() = 0;

	virtual void UpdateDuringTransition() {} // トランジション中の更新処理（必要に応じてオーバーライド）

	bool IsEnd() const { return isEnd; }
	SceneID GetNextScene() const { return nextSceneID; }
	void SetNextScene(SceneID next) { nextSceneID = next; }
	float EaseInOut(float t) { return t * t * (3.0f - 2.0f * t); }
	// 3次イージング
	float EaseInCubic(float t)
	{
		return t * t * t;
	}
	float EaseOutCubic(float t)
	{
		float f = t - 1.0f;
		return (f * f * f) + 1.0f;
	}

	virtual void TransitionIn(float t) {}
	virtual void TransitionOut(float t) {}
	void SetIsTransition(bool b) { isTransition = b; }
	bool GetIsTransition() { return isTransition; }
protected:
	FileManager& fileMng_;
	SceneManager& sceneMng_;
	bool isEnd = false;
	bool isTransition = false;
	SceneID nextSceneID = SceneID::NONE;
};
