#pragma once
// 空白游戏接入模板。复制此目录后，在 main.cpp/draw.cpp 中实现具体逻辑。

#include "core/Game.h"
#include "core/ImGuiDrawModule.h"
#include "core/math/VecMath.h"
#include "core/traverse/FrameScanner.h"
#include "core/traverse/TrackedList.h"
#include <array>
#include <cstdint>

namespace game_template {

struct ActorData : TrackedObject {
    Vec3 location{};
};

class TemplateGame;

class TemplateDraw : public ImGuiDrawModule {
  public:
    explicit TemplateDraw(TemplateGame &game);
    void DrawObject() override;

  private:
    TemplateGame &game_;
};

class TemplateGame : public Game {
  public:
    TemplateGame();
    TrackedList<ActorData> actors;

  protected:
    diRW::baseRW *CreateReader() override;
    bool Init(diRW::baseRW *rw) override;
    void Update() override;
    void Shutdown() override;

  private:
    void UpdateActor(ActorData &actor, bool full);

    TemplateDraw draw_;
    FrameScanner scanner_{30};
    std::array<uintptr_t, 30> actorBuffer_{};
};

// 注册到 games/Games.h 前取消注释并修改名称。
// extern TemplateGame game;

} // namespace game_template
