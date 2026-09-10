#include "games/texun/main.hpp"
#include "core/draw/DrawPrimitives.hpp"
#include <imgui.h>

namespace texun {

TexunDraw::TexunDraw(TexunGame &game)
    : ImGuiDrawModule("绘图模块"), game_(game) {
    AddPage("绘制设置", [this] { DrawEspSettings(); });
}

void TexunDraw::DrawObject() {
    ImDrawList *foreground = ImGui::GetForegroundDrawList();
    for (const ActorData &actor : game_.actors.All()) {
        if (actor.findCount < 0)
            continue;
        const Vec4 box = WorldToBox(game_.matrix_, actor.location,
                                    ScreenCenterX(), ScreenCenterY(), -100.0f,
                                    100.0f, 0.5f);
        if (box.w <= 0.0f)
            continue;
        const ImU32 color = IM_COL32(255, 255, 255, 140);
        if (drawBox)
            DrawPrimitives::DrawBox(foreground, box, color);
        if (drawRay)
            DrawPrimitives::DrawRay(foreground, box, ScreenCenterX(), 150.0f,
                                    50.0f, color);
    }
    if (showDebug)
        DrawDebugWindow();
}

void TexunDraw::DrawEspSettings() {
    ImGui::Checkbox("绘制方框", &drawBox);
    ImGui::Checkbox("绘制射线", &drawRay);
    ImGui::Checkbox("显示调试窗口", &showDebug);
    ImGui::Separator();
}

void TexunDraw::DrawDebugWindow() {
    if (!ImGui::Begin("枪战特训调试")) {
        ImGui::End();
        return;
    }
    if (ImGui::CollapsingHeader("运行状态")) {
        ImGui::Text("游戏: %s", game_.name ? game_.name : "<null>");
        ImGui::Text("包名: %s", game_.packageName ? game_.packageName : "<null>");
        ImGui::Text("初始化: %s", game_.Inited() ? "成功" : "未初始化");
        ImGui::Text("读写后端: %s", game_.Inited() ? "可用" : "不可用");
    }
    if (ImGui::CollapsingHeader("世界地址")) {
        ImGui::Text("模块基址: 0x%llX", static_cast<unsigned long long>(game_.worldData_.moduleBase));
        ImGui::Text("GWorld: 0x%llX", static_cast<unsigned long long>(game_.worldData_.worldAddress));
        ImGui::Text("ULevel: 0x%llX", static_cast<unsigned long long>(game_.worldData_.ulevelAddress));
        ImGui::Text("GName: 0x%llX", static_cast<unsigned long long>(game_.worldData_.gnameAddress));
        ImGui::Text("矩阵地址: 0x%llX", static_cast<unsigned long long>(game_.worldData_.matrixAddress));
        ImGui::Text("Actors 数组: 0x%llX", static_cast<unsigned long long>(game_.worldData_.arrayAddress));
        ImGui::Text("Actors 数量: %d", game_.worldData_.actorCount);
        ImGui::Text("selfAddr: 0x%llX", static_cast<unsigned long long>(game_.worldData_.selfAddress));
    }
    if (ImGui::CollapsingHeader("Actors 数组")) {
        ImGui::Text("数组地址: 0x%llX", static_cast<unsigned long long>(game_.worldData_.arrayAddress));
        ImGui::Text("对象槽位数: %d", game_.worldData_.actorCount);
        ImGui::Text("扫描器游标: %d", game_.scanner_.Cursor());
        ImGui::Text("每帧最大批量: %d", game_.scanner_.maxPerFrame);
        ImGui::Text("本轮完成: %s", game_.scanner_.IsRoundComplete(game_.worldData_.actorCount) ? "是" : "否");
    }
    if (ImGui::CollapsingHeader("对象池")) {
        size_t activeCount = 0;
        size_t tombstoneCount = 0;
        for (const ActorData &actor : game_.actors.All()) {
            if (actor.findCount < 0)
                ++tombstoneCount;
            else
                ++activeCount;
        }
        ImGui::Text("对象总数: %zu", game_.actors.Size());
        ImGui::Text("存活对象: %zu", activeCount);
        ImGui::Text("墓碑对象: %zu", tombstoneCount);
        if (ImGui::TreeNode("对象列表")) {
            int index = 0;
            for (const ActorData &actor : game_.actors.All()) {
                ImGui::Text("[%d] 地址: 0x%llX | findCount: %d | 坐标: (%.2f, %.2f, %.2f)",
                            index++, static_cast<unsigned long long>(actor.address),
                            actor.findCount, actor.location.x, actor.location.y,
                            actor.location.z);
            }
            ImGui::TreePop();
        }
    }
    if (ImGui::CollapsingHeader("屏幕状态")) {
        ImGui::Text("屏幕初始化: %s", IsScreenInitialized() ? "成功" : "未初始化");
        ImGui::Text("屏幕大小: %.1f x %.1f", ScreenX(), ScreenY());
        ImGui::Text("屏幕中心: %.1f, %.1f", ScreenCenterX(), ScreenCenterY());
    }
    ImGui::End();
}

} // namespace texun
