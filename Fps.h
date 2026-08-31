#pragma once

class Fps
{
public:
	static const int FPS = 60;	//設定したFPS

	Fps();
	~Fps();
	bool Update();
	void Draw();
	void Wait();

private:
	int mStartTime;         //測定開始時刻
	int mCount;             //カウンタ
	float mFps;             //fps
	static const int N = 60;//平均を取るサンプル数
	
};
