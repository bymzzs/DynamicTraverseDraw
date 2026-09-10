#pragma once

#include "core/Game.h"
#include "core/ImGuiDrawModule.h"
#include "core/math/VecMath.h"
#include "core/traverse/FrameScanner.h"
#include "core/traverse/TrackedList.h"
#include <array>
#include <cstdint>

namespace example {

struct ActorData : TrackedObject {
    int nameId = 0;
    float health = 0.0f;
    float healthMax = 0.0f;
    Vec3 worldPosition{};
};

class ExampleGame;

class ExampleDraw : public ImGuiDrawModule {
  public:
    explicit ExampleDraw(ExampleGame &game);
    void DrawObject() override;

  private:
    void DrawSettings();
    void DrawDebugWindow();
    ExampleGame &game_;
};

class ExampleGame : public Game {
  public:
    ExampleGame();

    TrackedList<ActorData> actors;
    bool drawBox = true;
    bool drawRay = true;
    bool showDebug = false;
    uintptr_t moduleBase = 0;
    uintptr_t gname = 0;
    uintptr_t gworld = 0;
    int actorCount = 0;
    int frame = 0;
    float viewMatrix_[16]{};

  protected:
    diRW::baseRW *CreateReader() override;
    bool Init(diRW::baseRW *rw) override;
    void Update() override;
    void Shutdown() override;

  private:
    friend class ExampleDraw;
    void FinishActorRound();
    void UpdateActor(ActorData &actor, bool full);

    ExampleDraw draw_;
    FrameScanner scanner_{30};
    std::array<uintptr_t, 30> actorBuffer_{};
};

extern ExampleGame game;

} // namespace example
