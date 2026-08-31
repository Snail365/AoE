#pragma once

#include <memory>
#include <list>
#include <string>
#include "SceneSuper.h"

class FileManager;

struct ClearResult
{
	float time;
	int stageIndex;
};

class SceneManager
{
public:
	enum class TransitionState
	{
		None,
		FadeOutCurrent,
		SwitchScene,
		FadeInNext,
	};

	struct Transition
	{
		TransitionState state = TransitionState::None;
		float timer = 0.0f;
		float duration = 45.0f; // フェードの総時間（フレーム数）
		SceneSuper::SceneID nextSceneID = SceneSuper::SceneID::NONE;
	};
	SceneManager(FileManager& fileMng);
	~SceneManager();

	void Update();
	void Draw();

	bool GetExit() const;
	void SetGameResult(bool isClear) { _isClear = isClear; }
	void SetClearResult(const ClearResult& result);
	void SetTransitionDuration(float duration);


	void PushScene(SceneSuper::SceneID sceneID);
	void PopScene();

	bool IsTransitioning() const { return transition_.state != TransitionState::None; }

private:

	std::unique_ptr<SceneSuper> CreateScene(SceneSuper::SceneID sceneID);
	void ChangeScene(SceneSuper::SceneID nextSceneID);

	void UpdateTransition();

	std::list<std::unique_ptr<SceneSuper>> scenes; // シーンリストをスタックで管理
	FileManager& fileMng_;
	SceneSuper* currentScene;

	bool isExit;
	bool _isClear;
	bool _isPopRequested = false;

	Transition transition_;

	ClearResult _clearResult;
};
