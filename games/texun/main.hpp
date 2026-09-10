#pragma once

#include "core/Game.h"
#include "core/ImGuiDrawModule.h"
#include "core/math/VecMath.h"
#include "core/traverse/FrameScanner.h"
#include "core/traverse/TrackedList.h"
#include <array>
#include <cstdint>

namespace texun {

struct ActorData : TrackedObject {
    Vec3 location{};
};

class TexunGame;

class TexunDraw : public ImGuiDrawModule {
  public:
    explicit TexunDraw(TexunGame &game);
    void DrawObject() override;

  private:
    void DrawEspSettings();
    void DrawDebugWindow();
    TexunGame &game_;
    bool drawBox = true;
    bool drawRay = true;
    bool showDebug = false;
};

class TexunGame : public Game {
  public:
    struct WorldData {
        uintptr_t moduleBase = 0;
        uintptr_t worldAddress = 0;
        uintptr_t ulevelAddress = 0;
        uintptr_t gnameAddress = 0;
        uintptr_t matrixAddress = 0;
        uintptr_t arrayAddress = 0;
        uintptr_t selfAddress = 0;
        int actorCount = 0;
    };

    struct ActorOffsets {
        int gWorld = 0xAAB2D50;
        int gName = 0xA8F7980;
        int ulevel = 0x30;
        int actors = 0x98;
        int matrix[3] = {0xAA833A8, 0x20, 0x280};
        int self[5] = {0xAA833A8, 0x8, 0x270, 0x110, 0x0};
        int rootComponnent = 0x130;
        int location = 0x11C;
        int rotation = 0x128;
        int fillter = 0x22C;
        int health[2] = {0x4C0, 0xB0};
    };

    TexunGame();
    TrackedList<ActorData> actors;
    WorldData worldData_{};
    ActorOffsets offsets_{};

  protected:
    diRW::baseRW *CreateReader() override;
    bool Init(diRW::baseRW *rw) override;
    void handleBatch(const FrameScanner::Batch &batch);
    void Update() override;
    void Shutdown() override;

  private:
    friend class TexunDraw;
    void UpdateActor(ActorData &actor, bool full);

    TexunDraw draw_;
    FrameScanner scanner_{30};
    std::array<uintptr_t, 30> actorBuffer_{};
    float matrix_[16]{};
};

extern TexunGame game;

} // namespace texun
