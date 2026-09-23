#include "ImGuiManager.h"

// Windowsのメッセージ（マウスやキーボード入力）をImGuiに流し込むための設定
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// DxLibのウィンドウプロシージャをフックする関数
LRESULT CALLBACK ImGuiManager::WndProcHook(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // ImGuiがメッセージを消費したらDxLib側には流さない
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    return 0; // 消費されなければDXライブラリ側の通常の処理を継続させる
}

bool ImGuiManager::Init()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark(); // 何となくダークテーマ適用

    // DXライブラリのウィンドウハンドルを取得
    HWND hWnd = GetMainWindowHandle();

    // DXライブラリからDirectX 11デバイスとコンテキストを取得
    ID3D11Device* pDevice = (ID3D11Device*)GetUseDirect3D11Device();
    ID3D11DeviceContext* pContext = (ID3D11DeviceContext*)GetUseDirect3D11DeviceContext();

    if (!pDevice || !pContext) return false;

    // Win32 / DX11 バックエンドの初期化
    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(pDevice, pContext);

    // DXライブラリのウィンドウプロシージャをフック（マウス・キーボード入力をImGuiへ渡す）
    SetHookWinProc(WndProcHook);

    return true;
}

void ImGuiManager::NewFrame()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImGuiManager::Render()
{
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiManager::Shutdown()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}