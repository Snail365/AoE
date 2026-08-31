#pragma once
#include "FileSuper.h"
#include <DxLib.h>

class SoundFile final : public FileSuper
{
public:
    SoundFile(const std::string& path, int handle)
        : FileSuper(path, handle)
    {
    }

    ~SoundFile() override
    {
        DeleteSoundMem(handle_);
    }

	void PlayLoop(int playType = DX_PLAYTYPE_LOOP) const
	{
		// もし再生中でなければ再生する
		if (!CheckSoundMem(handle_))
		{
			PlaySoundMem(handle_, playType);
		}
	}

	void PlayOneShot(int playType = DX_PLAYTYPE_BACK) const
	{
		int frame = GetNowCount();
		if (frame != lastPlayFrame)
		{
			lastPlayFrame = frame;
			PlaySoundMem(handle_, playType);
		}
	}

	// 再生中でなければワンショット再生する
	void PlayOneShotIfNotPlaying(int playType = DX_PLAYTYPE_BACK) const
	{
		if (!CheckSoundMem(handle_))
		{
			PlaySoundMem(handle_, playType);
		}
	}

	void Stop() const
	{
		StopSoundMem(handle_);
	}

	void SetVolume(int Volume) const
	{
		ChangeVolumeSoundMem(Volume, handle_);
	}

private:
	// constメソッド内で扱えるようにmutableにする
	mutable int lastPlayFrame = -1;
};
