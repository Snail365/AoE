#pragma once
#include "SceneSuper.h"
#include "Camera3D.h"
#include "GameSystemManager.h"
#include "RenderManager.h"

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

private:
    Camera3D camera_;
    GameSystemManager systemMng_;
    RenderManager renderMng_;
};

