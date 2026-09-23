#pragma once
#include "SceneSuper.h"
#include "Camera3D.h"
#include "GameSystemManager.h"
#include "RenderManager.h"
#include "NavGrid.h"

#include <memory>
#include <vector>

class SceneGame :
    public SceneSuper
{
public:
    SceneGame(FileManager& fileMng, SceneManager& sceneMng);
    ~SceneGame() override;
    void Update() override;
    void Draw() override;
    void TransitionIn(float t) override;
	void TransitionOut(float t) override;

    void DebugUIDraw();

private:
    Camera3D camera_;
    std::unique_ptr<GameSystemManager> systemMng_;
    std::unique_ptr<RenderManager> renderMng_;

	bool isShopOpen_ = false;

    bool showNavMesh_ = false; // ImGuiと連動する描画フラグ
};

