#pragma once
#include "SoundFile.h"
#include <memory>

class BGMManager
{
public:
	// シングルトン
	static BGMManager& GetInstance()
	{
		static BGMManager instance;
		return instance;
	}

	void PlayBGM(std::shared_ptr<SoundFile> bgm, int volume = 255)
	{
		if (IsPlaying(bgm)) return; // 同じBGMなら再生しない
		StopBGM();
		currentBGM_ = bgm;
		if (currentBGM_)
		{
			currentBGM_->SetVolume(volume);
			currentBGM_->PlayLoop();
		}
	}

	// 後々フェード処理とか入れるかも(未定)(いいや)
	void StopBGM()
	{
		if (currentBGM_)
		{
			currentBGM_->Stop();
			currentBGM_ = nullptr;
		}
	}

	void SetVolume(int volume)
	{
		if (currentBGM_)
			currentBGM_->SetVolume(volume);
	}

	bool IsPlaying(std::shared_ptr<SoundFile> bgm) const
	{
		if (!currentBGM_ || !bgm) return false;
		return currentBGM_->GetPath() == bgm->GetPath();
	}

	std::shared_ptr<SoundFile> GetCurrentBGM() const
	{
		return currentBGM_;
	}

private:
	BGMManager() = default;
	~BGMManager() { StopBGM(); }

	std::shared_ptr<SoundFile> currentBGM_ = nullptr;
};
