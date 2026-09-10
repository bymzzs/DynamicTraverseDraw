#include "games/example/main.hpp"
#include "core/draw/DrawPrimitives.hpp"
#include <imgui.h>

namespace example {

ExampleDraw::ExampleDraw(ExampleGame &game)
    : ImGuiDrawModule("示例绘制"), game_(game) {
    AddPage("绘制设置", [this] { DrawSettings(); });
}

void ExampleDraw::DrawObject() {
    ImDrawList *foreground = ImGui::GetForegroundDrawList();
    for (const ActorData &actor : game_.actors.All()) {
        if (actor.findCount < 0)
            continue;
        const Vec4 box = WorldToBox(game_.viewMatrix_, actor.worldPosition,
                                    ScreenCenterX(), ScreenCenterY(), 0.0f,
                                    1.8f, 0.5f);
        if (box.w <= 0.0f)
            continue;
        const ImU32 color = IM_COL32(255, 255, 255, 140);
        if (game_.drawBox)
            DrawPrimitives::DrawBox(foreground, box, color);
        if (game_.drawRay)
            DrawPrimitives::DrawRay(foreground, box, ScreenCenterX(), 150.0f,
                                    50.0f, color);
    }
    if (game_.showDebug)
        DrawDebugWindow();
}

void ExampleDraw::DrawSettings() {
    ImGui::Checkbox("绘制方框", &game_.drawBox);
    ImGui::Checkbox("绘制射线", &game_.drawRay);
    ImGui::Checkbox("显示调试窗口", &game_.showDebug);
}

void ExampleDraw::DrawDebugWindow() {
    if (!ImGui::Begin("示例接入调试")) {
        ImGui::End();
        return;
    }
    ImGui::Text("帧数: %d", game_.frame);
    ImGui::Text("对象数: %zu", game_.actors.Size());
    ImGui::Text("模块基址: 0x%llX",
                static_cast<unsigned long long>(game_.moduleBase));
    ImGui::Text("GName: 0x%llX",
                static_cast<unsigned long long>(game_.gname));
    ImGui::Text("GWorld: 0x%llX",
                static_cast<unsigned long long>(game_.gworld));
    ImGui::Text("Actors 数量: %d", game_.actorCount);
    ImGui::End();
}

} // namespace example
