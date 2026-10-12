#include "SystemInfoWindow.h"
#include "Config.h"
#include "imgui.h"

#include <chrono>
#include <cstdio>

void RenderSystemInfoWindow(AppState& state)
{
    if (!state.showSystemInfo) return;

    ImGui::SetNextWindowSize(ImVec2(460.0f, 320.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(80.0f, 80.0f), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("System Information", &state.showSystemInfo))
    {
        ImGui::TextColored(ImVec4(0.45f, 0.82f, 1.0f, 1.0f), "CSOPESY Desktop OS Emulator v1.0");
        ImGui::Separator();
        ImGui::Spacing();

        auto sec = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - state.startTime
        ).count();

        if (ImGui::BeginTable("SysSpecsTable", 2, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 150.0f);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            auto AddRow = [](const char* k, const char* v) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(k);
                ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(v);
            };

            char ramBuf[64], upBuf[64];
            std::snprintf(ramBuf, sizeof(ramBuf), "%.0f MB (%.1f GB)", Config::TOTAL_SYSTEM_MEMORY_MB, Config::TOTAL_SYSTEM_MEMORY_MB / 1024.0f);
            std::snprintf(upBuf, sizeof(upBuf), "%02d:%02d:%02d", static_cast<int>(sec / 3600), static_cast<int>((sec % 3600) / 60), static_cast<int>(sec % 60));

            AddRow("OS Edition", "CSOPESY Megatrends Desktop Edition");
            AddRow("Compositor Stack", "GLFW 3 + OpenGL 3 + Dear ImGui");
            AddRow("Processor", "Pentium III Virtual Core @ 1.00 GHz");
            AddRow("Installed RAM", ramBuf);
            AddRow("System Type", "64-bit Operating System Mock-up");
            AddRow("System Uptime", upBuf);
            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::SeparatorText("Group Developers");
        ImGui::BulletText("Abdulrahman, Saoud");
        ImGui::BulletText("Fabricante, Jeruel");
        ImGui::BulletText("Galvez, Anousheh Monick");
    }
    ImGui::End();
}
