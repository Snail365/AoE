#include <math.h>
#include "DxLib.h"
#include "Fps.h"

Fps::Fps()
{
	mStartTime = 0;
	mCount = 0;
	mFps = 0;
}

Fps::~Fps()
{

}

bool Fps::Update()
{
	if (mCount == 0)
	{
		mStartTime = GetNowCount();
	}
	if (mCount == N)
	{
		int t = GetNowCount();
		mFps = 1000.f / ((t - mStartTime) / (float)N);
		mCount = 0;
		mStartTime = t;
	}
	mCount++;
	return true;
}

void Fps::Draw()
{
#ifdef _DEBUG
	DrawFormatString(0, 0, 0x00ff00, "%.1f", mFps);
#endif
}

void Fps::Wait()
{
	int tookTime = GetNowCount() - mStartTime;
	int waitTime = mCount * 1000 / FPS - tookTime;

	if (waitTime > 0)
	{
		Sleep(waitTime);
	}
}
