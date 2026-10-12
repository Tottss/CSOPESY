#include "TaskManager.h"
#include "Config.h"
#include "imgui.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <string>

static bool IsIdleProcess(std::string name)
{
    for (char& c : name) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return name.find("idle") != std::string::npos;
}

static float GetProcessCpu(const Config::ProcessEntry& p, size_t idx, double t)
{
    if (!Config::TASK_MANAGER_WOBBLE || IsIdleProcess(p.name)) return p.cpuPercent;
    return std::clamp(p.cpuPercent + 0.35f * std::sin(static_cast<float>(t * 2.2 + idx * 1.37)), 0.0f, 100.0f);
}

static float GetProcessMem(const Config::ProcessEntry& p, size_t idx, double t)
{
    if (!Config::TASK_MANAGER_WOBBLE || IsIdleProcess(p.name)) return p.memoryMB;
    return std::max(0.1f, p.memoryMB + 1.2f * std::sin(static_cast<float>(t * 1.1 + idx * 0.91)));
}

void RenderTaskManagerWindow(AppState& state)
{
    if (!state.showTaskManager) return;

    ImGui::SetNextWindowSize(ImVec2(640.0f, 420.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(540.0f, 100.0f), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Task Manager", &state.showTaskManager))
    {
        const double t = ImGui::GetTime();
        float activeCpu = 0.0f, usedMem = 0.0f;

        for (size_t i = 0; i < Config::DUMMY_PROCESSES.size(); ++i)
        {
            const auto& p = Config::DUMMY_PROCESSES[i];
            usedMem += GetProcessMem(p, i, t);
            if (!IsIdleProcess(p.name))
                activeCpu += GetProcessCpu(p, i, t);
        }
        activeCpu = std::clamp(activeCpu, 0.0f, 100.0f);
        float totalMem = std::max(1.0f, Config::TOTAL_SYSTEM_MEMORY_MB);
        float memFrac  = std::clamp(usedMem / totalMem, 0.0f, 1.0f);

        if (ImGui::BeginTable("PerfSummary", 2, ImGuiTableFlags_SizingStretchSame))
        {
            char cpuBuf[64], memBuf[64];
            std::snprintf(cpuBuf, sizeof(cpuBuf), "CPU Usage: %.1f%%", activeCpu);
            std::snprintf(memBuf, sizeof(memBuf), "Memory: %.1f / %.0f MB (%.1f%%)", usedMem, totalMem, memFrac * 100.0f);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::ProgressBar(activeCpu / 100.0f, ImVec2(-1.0f, 22.0f), cpuBuf);
            ImGui::TableSetColumnIndex(1);
            ImGui::ProgressBar(memFrac, ImVec2(-1.0f, 22.0f), memBuf);
            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::SeparatorText("Processes");

        ImGuiTableFlags tableFlags =
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
            ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp;

        if (ImGui::BeginTable("ProcessesTable", 5, tableFlags))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Processes",   ImGuiTableColumnFlags_WidthStretch, 2.2f);
            ImGui::TableSetupColumn("PID",         ImGuiTableColumnFlags_WidthStretch, 0.7f);
            ImGui::TableSetupColumn("Status",      ImGuiTableColumnFlags_WidthStretch, 1.1f);
            ImGui::TableSetupColumn("CPU (%)",     ImGuiTableColumnFlags_WidthStretch, 1.0f);
            ImGui::TableSetupColumn("Memory (MB)", ImGuiTableColumnFlags_WidthStretch, 1.1f);
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < Config::DUMMY_PROCESSES.size(); ++i)
            {
                const auto& p = Config::DUMMY_PROCESSES[i];
                float cpu = (IsIdleProcess(p.name) && Config::TASK_MANAGER_WOBBLE)
                    ? std::clamp(100.0f - activeCpu, 0.0f, 100.0f)
                    : GetProcessCpu(p, i, t);
                float mem = GetProcessMem(p, i, t);

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(p.name.c_str());
                ImGui::TableSetColumnIndex(1); ImGui::Text("%d", p.pid);
                ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(p.status.c_str());
                ImGui::TableSetColumnIndex(3); ImGui::Text("%.1f%%", cpu);
                ImGui::TableSetColumnIndex(4); ImGui::Text("%.1f MB", mem);
            }
            ImGui::EndTable();
        }
    }
    ImGui::End();
}
