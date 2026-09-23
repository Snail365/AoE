#pragma once
#include <DxLib.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

class ImGuiManager
{
public:
    static ImGuiManager& GetInstance()
    {
        static ImGuiManager instance;
        return instance;
    }

    bool Init();
    void NewFrame();
    void Render();
    void Shutdown();

private:
    ImGuiManager() = default;
    ~ImGuiManager() = default;

    static LRESULT CALLBACK WndProcHook(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};