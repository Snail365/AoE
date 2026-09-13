#pragma once
#include <memory>

class SceneManager;
class FileManager;

// -デバッグ------------------
class ParticleManager;
// --------------------------
class Application
{
public:
	static constexpr int SCREEN_WID = 1920; // ゲーム画面の横サイズ
	static constexpr int SCREEN_HIG = 1080; // ゲーム画面の縦サイズ

	Application();
	~Application();

	void Run();
private:

	// Applicationのみ所有するように
	std::unique_ptr<SceneManager> sceneMng;
	std::unique_ptr<FileManager> fileMng;
	
	void Update();
	void Draw();
};
