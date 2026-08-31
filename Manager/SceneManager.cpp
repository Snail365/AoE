#include <DxLib.h>
#include <algorithm>

#include "Application.h"
#include "SceneManager.h"
#include "FileManager.h"

#include "SceneResult.h"
#include "SceneGame.h"
#include "SceneTitle.h"

SceneManager::SceneManager(FileManager& fileMng) : fileMng_(fileMng)
{
	transition_.state = TransitionState::SwitchScene;
	transition_.timer = 0.0f;
	transition_.duration = 45.0f;
	transition_.nextSceneID = SceneSuper::SceneID::TITLE;
	ChangeScene(transition_.nextSceneID);
}

SceneManager::~SceneManager()
{
	scenes.clear();
}

void SceneManager::Update()
{

	if (_isPopRequested)
	{
		_isPopRequested = false;
		if (scenes.size() > 1)
		{
			scenes.pop_back();
		}
	}

	if (scenes.empty())
	{
		isExit = true;
		return;
	}

	currentScene = scenes.back().get();

	if (IsTransitioning())
	{
		UpdateTransition();

		currentScene->UpdateDuringTransition();
	}
	else
	{
		currentScene->Update();

		if (currentScene->IsEnd())
		{
			currentScene->SetIsTransition(true);

			transition_.state = TransitionState::FadeOutCurrent;
			transition_.timer = 0.0f;

			transition_.nextSceneID = currentScene->GetNextScene();
		}
	}
}

void SceneManager::UpdateTransition()
{
	transition_.timer += 1.0f; // フレームごとにタイマーを進める
	bool isHost = false;
	switch (transition_.state)
	{
		case TransitionState::FadeOutCurrent:
			currentScene->TransitionOut(transition_.timer / transition_.duration);
			if (transition_.timer >= transition_.duration)
			{
				transition_.state = TransitionState::SwitchScene;
			}
			break;
		case TransitionState::SwitchScene:
			ChangeScene(transition_.nextSceneID);

			currentScene = scenes.back().get();
			currentScene->SetIsTransition(true);

			transition_.state = TransitionState::FadeInNext;
			transition_.timer = 0.0f; // フェードインのタイマーをリセット
			break;
		case TransitionState::FadeInNext:
			currentScene->TransitionIn(transition_.timer / transition_.duration);
			if (transition_.timer >= transition_.duration)
			{
				currentScene->SetIsTransition(false);
				transition_.state = TransitionState::None; // トランジション終了
				transition_.nextSceneID = SceneSuper::SceneID::NONE; // 次のシーンIDをリセット
			}
			break;
		default:
			break;
	}
}

void SceneManager::Draw()
{
	if (scenes.empty()) return;



	for (const auto& scene : scenes)
	{
		scene->Draw();
	}
}

std::unique_ptr<SceneSuper> SceneManager::CreateScene(SceneSuper::SceneID sceneID)
{
	switch (sceneID)
	{
		case SceneSuper::SceneID::TITLE:
			return std::make_unique<SceneTitle>(fileMng_, *this);
		//case SceneSuper::SceneID::STAGE_SELECT:
		//	return std::make_unique<SceneStageSelect>(fileMng_, *this, _remoteIp);
		case SceneSuper::SceneID::GAME:
			return std::make_unique<SceneGame>(fileMng_, *this);
		case SceneSuper::SceneID::RESULT:
			return std::make_unique<SceneResult>(fileMng_, *this);
		//case SceneSuper::SceneID::PAUSE:
		//	return std::make_unique<ScenePause>(fileMng_, *this, _remoteIp);
		default:
			return nullptr;
	}
}

void SceneManager::ChangeScene(SceneSuper::SceneID nextSceneID)
{
	if (nextSceneID == SceneSuper::SceneID::EXIT)
	{
		isExit = true;
		return;
	}

	scenes.clear();

	auto newScene = CreateScene(nextSceneID);

	if (newScene != nullptr)
	{
		if (scenes.empty())
		{
			scenes.push_back(std::move(newScene));
		}
		else
		{
			scenes.back() = std::move(newScene);
		}
	}
	else
	{
#ifdef _DEBUG
		OutputDebugStringA("シーンの生成に失敗しました。\n");
#endif
	}
}

void SceneManager::PushScene(SceneSuper::SceneID sceneID)
{
	auto newScene = CreateScene(sceneID);
	if (newScene)
	{
		scenes.push_back(std::move(newScene));
	}
}

void SceneManager::PopScene()
{
	if (scenes.size() > 1)
	{
		_isPopRequested = true;
	}
}

bool SceneManager::GetExit() const
{
	return isExit;
}

void SceneManager::SetClearResult(const ClearResult& result)
{
	_clearResult = result;
}

void SceneManager::SetTransitionDuration(float duration)
{
	transition_.duration = duration;
}
