#include "games/texun/main.hpp"
#include "diRW/syscallRW.hpp"
#include <tool.h>

namespace texun {

TexunGame::TexunGame() : Game("枪战特训", "com.ShuiSha.FPS2"), draw_(*this) {
    Attach(&draw_);
}

diRW::baseRW *TexunGame::CreateReader() {
    const int pid = getPID(packageName);
    if (pid <= 0)
        return nullptr;
    return new diRW::syscallRW(diRW::baseRW::PidMode::Private, pid);
}

bool TexunGame::Init(diRW::baseRW *rw) {
    actors.SetUpdateFunc(
        [this](ActorData &actor, bool full) { UpdateActor(actor, full); });
    if (!rw || rw->getProcessPid() <= 0 || !rw->isConnected())
        return false;
    worldData_.moduleBase = rw->get_module_base("libUE4.so");
    return worldData_.moduleBase != 0;
}

void TexunGame::handleBatch(const FrameScanner::Batch &batch) {
    if (batch.count <= 0)
        return;
    if (!rw_->readv(batch.address, actorBuffer_.data(),
                    static_cast<size_t>(batch.count) * sizeof(uintptr_t)))
        return;
    for (int i = 0; i < batch.count; ++i) {
        const uintptr_t object = actorBuffer_[static_cast<size_t>(i)];
        if (object < 0xFFFFFF)
            continue;
        if (rw_->getFloat(object + offsets_.fillter) == 64.0f)
            actors.Add(object);
    }
}

void TexunGame::Update() {
    worldData_.worldAddress = rw_->getPtr64(worldData_.moduleBase + offsets_.gWorld);
    worldData_.ulevelAddress = rw_->getPtr64(worldData_.worldAddress + offsets_.ulevel);
    worldData_.matrixAddress =
        rw_->jumpPoint(worldData_.moduleBase, offsets_.matrix[0], offsets_.matrix[1]) +
        offsets_.matrix[2];
    worldData_.gnameAddress = rw_->getPtr64(worldData_.moduleBase + offsets_.gName);
    worldData_.arrayAddress = rw_->getPtr64(worldData_.ulevelAddress + offsets_.actors);
    worldData_.selfAddress =
        rw_->jumpPoint(worldData_.moduleBase, offsets_.self[0], offsets_.self[1],
                       +offsets_.self[2], offsets_.self[3]) + offsets_.self[4];
    worldData_.actorCount =
        rw_->getDword(worldData_.ulevelAddress + offsets_.actors + 0x8);

    if (worldData_.matrixAddress > 0xFFFFFF)
        rw_->readv(worldData_.matrixAddress, matrix_, sizeof(matrix_));

    const FrameScanner::Batch batch =
        scanner_.Next(worldData_.arrayAddress, worldData_.actorCount);
    if (batch.address >= 0xFFFFFF)
        handleBatch(batch);

    if (scanner_.IsRoundComplete(worldData_.actorCount)) {
        actors.Sweep();
        actors.UpdateAll(true);
        scanner_.Reset();
    } else {
        actors.UpdateAll(false);
    }
}

void TexunGame::UpdateActor(ActorData &actor, bool full) {
    rw_->readv(rw_->getPtr64(actor.address + offsets_.rootComponnent) +
                   offsets_.location,
               &actor.location, sizeof(actor.location));
    (void)full;
}

void TexunGame::Shutdown() {
    actors.Clear();
    worldData_ = {};
    scanner_.Reset();
}

TexunGame game;

} // namespace texun
