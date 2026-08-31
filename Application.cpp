#include <DxLib.h>
#include "Application.h"
#include "SceneManager.h"
#include "FileManager.h"
#include "Fps.h"

//constexpr auto DEBUG_PARTICLE_PATH = "Resource/ParticleJsonData/parametera.json";

// デバッグ---------------------
//#include "ParticleManager.h"
// ----------------------------

Application::Application()
{
	SetUseCharCodeFormat(DX_CHARCODEFORMAT_UTF8);
	SetWindowText("カーレース");
	//SetWindowSizeChangeEnableFlag(TRUE, TRUE);
	//SetDoubleStartValidFlag(TRUE);
	//SetAlwaysRunFlag(TRUE);

	SetGraphMode(SCREEN_WID, SCREEN_HIG, 32);
	ChangeWindowMode(true);

//#ifdef _DEBUG
//	SetGraphMode(SCREEN_WID - 100, SCREEN_HIG - 100, 32);
//	ChangeWindowMode(true);
//#endif // DEBUG

//#ifdef _WIN64
//	Live2D_SetCubism4CoreDLLPath(TEXT("CubismSdkForNative-5-r.4.1/Core/dll/windows/x86_64/Live2DCubismCore.dll"));
//#else
//	Live2D_SetCubism4CoreDLLPath(TEXT("CubismSdkForNative-5-r.4.1/Core/dll/windows/x86/Live2DCubismCore.dll"));
//#endif

	DxLib_Init();
	fileMng = std::make_unique<FileManager>();
	sceneMng = std::make_unique<SceneManager>(*fileMng);
	// デバッグ----------------------------------------------------
	//pMng = std::make_unique<ParticleManager>(*fileMng);
	//pMng->RegisterConfig(DEBUG_PARTICLE_PATH);
	//------------------------------------------------------------
}

Application::~Application()
{
	// デストラクト明示
	sceneMng.reset();
	fileMng.reset();
	DxLib_End();
}

void Application::Run()
{
	Fps fps;
	// エンターキーとエスケープキーの同時押しで強制終了
	// どちらか片方だけだと誤操作で終了してしまう可能性あり
	while (ProcessMessage() == 0 && !sceneMng->GetExit() && (!CheckHitKey(KEY_INPUT_RETURN) || !CheckHitKey(KEY_INPUT_ESCAPE)))
	{
		SetDrawScreen(DX_SCREEN_BACK);
		ClearDrawScreen();

		fps.Update();
		Update();

		Draw();

		ScreenFlip();

		fps.Wait();
	}
}

void Application::Update()
{
	sceneMng->Update();
}

void Application::Draw()
{
	sceneMng->Draw();
}