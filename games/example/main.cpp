#include "games/example/main.hpp"
#include "core/draw/DrawPrimitives.hpp"
#include <imgui.h>

namespace example {
namespace {
constexpr const char *kModuleName = "libUE4.so";
constexpr int kGName = 0x01234567;
constexpr int kGWorld = 0x02345678;
constexpr int kPersistentLevel = 0x30;
constexpr int kActors = 0x98;
constexpr int kViewMatrix = 0x03456789;
constexpr int kActorState = 0x120;
constexpr int kActorNameId = 0x18;
constexpr uintptr_t kMinValidAddress = 0xFFFFFF;

struct RemoteActorArray {
    uintptr_t data = 0;
    int count = 0;
    int capacity = 0;
};
} // namespace

ExampleGame::ExampleGame()
    : Game("示例接入模板", "com.example.game"), draw_(*this) {
    Attach(&draw_);
}

diRW::baseRW *ExampleGame::CreateReader() { return nullptr; }

bool ExampleGame::Init(diRW::baseRW *rw) {
    if (!rw)
        return false;

    moduleBase = rw->get_module_base(kModuleName);
    if (!moduleBase)
        return false;
    gname = rw->getPtr64(moduleBase + kGName);
    gworld = rw->getPtr64(moduleBase + kGWorld);
    if (!gname || !gworld)
        return false;

    actors.SetUpdateFunc(
        [this](ActorData &actor, bool full) { UpdateActor(actor, full); });
    scanner_.Reset();
    frame = 0;
    return true;
}

void ExampleGame::Update() {
    ++frame;
    const uintptr_t level = rw_->getPtr64(gworld + kPersistentLevel);
    if (!level) {
        FinishActorRound();
        return;
    }

    RemoteActorArray remoteActors{};
    if (!rw_->readv(level + kActors, &remoteActors, sizeof(remoteActors))) {
        scanner_.Reset();
        return;
    }
    actorCount = remoteActors.count;
    if (actorCount <= 0 || !remoteActors.data) {
        FinishActorRound();
        return;
    }
    if (!rw_->readv(moduleBase + kViewMatrix, viewMatrix_, sizeof(viewMatrix_))) {
        scanner_.Reset();
        return;
    }

    const auto batch = scanner_.Next(remoteActors.data, actorCount);
    if (batch.count <= 0 ||
        !rw_->readv(batch.address, actorBuffer_.data(),
                    static_cast<size_t>(batch.count) * sizeof(uintptr_t))) {
        scanner_.Reset();
        return;
    }
    for (int i = 0; i < batch.count; ++i) {
        const uintptr_t actor = actorBuffer_[static_cast<size_t>(i)];
        if (actor >= kMinValidAddress)
            actors.Add(actor);
    }
    if (scanner_.IsRoundComplete(actorCount))
        FinishActorRound();
    actors.UpdateAll(false);
}

void ExampleGame::Shutdown() {
    actors.Clear();
    scanner_.Reset();
    actorCount = 0;
    moduleBase = 0;
    gname = 0;
    gworld = 0;
}

void ExampleGame::FinishActorRound() {
    actors.Sweep();
    actors.UpdateAll(true);
    scanner_.Reset();
}

void ExampleGame::UpdateActor(ActorData &actor, bool full) {
    if (full)
        rw_->readv(actor.address + kActorNameId, &actor.nameId,
                   sizeof(actor.nameId));

    struct ActorState {
        Vec3 position;
        float health;
        float healthMax;
    } state{};
    if (!rw_->readv(actor.address + kActorState, &state, sizeof(state)))
        return;
    actor.worldPosition = state.position;
    actor.health = state.health;
    actor.healthMax = state.healthMax;
}

ExampleGame game;

} // namespace example
