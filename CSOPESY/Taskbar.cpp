#include "Taskbar.h"
#include "Config.h"
#include "imgui_internal.h"

#include <algorithm>
#include <cstdio>

enum class IconType { SystemInfo, FileExplorer, TaskManager };

static bool DrawTaskbarButton(
    const char* id, const char* label, IconType icon,
    bool active, ImVec2 size, bool showLabel)
{
    ImVec2 pos = ImGui::GetCursorScreenPos();

    ImGui::PushStyleColor(ImGuiCol_Button, active ? ImVec4(0.22f, 0.36f, 0.56f, 1.0f) : ImVec4(0.14f, 0.16f, 0.22f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.44f, 0.68f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.18f, 0.30f, 0.48f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

    bool clicked = ImGui::Button(id, size);

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 ic(showLabel ? (pos.x + 22.0f) : (pos.x + size.x * 0.5f), pos.y + size.y * 0.5f);

    if (icon == IconType::SystemInfo)
    {
        dl->AddCircleFilled(ic, 10.0f, IM_COL32(70, 175, 255, 255), 24);
        ImVec2 isz = ImGui::CalcTextSize("i");
        dl->AddText(ImVec2(ic.x - isz.x * 0.5f, ic.y - isz.y * 0.5f), IM_COL32(15, 20, 30, 255), "i");
    }
    else if (icon == IconType::FileExplorer)
    {
        dl->AddRectFilled(ImVec2(ic.x - 10.0f, ic.y - 7.0f), ImVec2(ic.x - 2.0f, ic.y - 4.0f), IM_COL32(245, 190, 60, 255), 2.0f);
        dl->AddRectFilled(ImVec2(ic.x - 10.0f, ic.y - 4.0f), ImVec2(ic.x + 10.0f, ic.y + 8.0f), IM_COL32(255, 210, 80, 255), 2.0f);
    }
    else
    {
        dl->AddRectFilled(ImVec2(ic.x - 10.0f, ic.y - 8.0f), ImVec2(ic.x + 10.0f, ic.y + 8.0f), IM_COL32(45, 130, 200, 255), 3.0f);
        ImVec2 pts[5] = {
            ImVec2(ic.x - 8.0f, ic.y + 3.0f), ImVec2(ic.x - 3.0f, ic.y - 4.0f),
            ImVec2(ic.x + 1.0f, ic.y + 4.0f), ImVec2(ic.x + 5.0f, ic.y - 5.0f),
            ImVec2(ic.x + 8.0f, ic.y + 1.0f)
        };
        dl->AddPolyline(pts, 5, IM_COL32(180, 255, 210, 255), ImDrawFlags_None, 1.8f);
    }

    if (showLabel)
    {
        ImVec2 tsz = ImGui::CalcTextSize(label);
        dl->AddText(ImVec2(pos.x + 38.0f, pos.y + (size.y - tsz.y) * 0.5f), IM_COL32(240, 244, 255, 255), label);
    }

    if (active)
    {
        dl->AddRectFilled(
            ImVec2(pos.x + 10.0f, pos.y + size.y - 3.0f),
            ImVec2(pos.x + size.x - 10.0f, pos.y + size.y),
            IM_COL32(90, 200, 255, 255), 2.0f
        );
    }

    return clicked;
}

void RenderTaskbar(const ImGuiIO& io, AppState& state)
{
    const float tbH  = Config::GetEffectiveTaskbarHeight(io.DisplaySize.y);
    const float yPos = Config::TASKBAR_AT_BOTTOM ? (io.DisplaySize.y - tbH) : 0.0f;

    ImGui::SetNextWindowPos(ImVec2(0.0f, yPos));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, tbH));

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

    const float padX = 12.0f, padY = 7.0f;
    const float btnH = std::max(20.0f, tbH - padY * 2.0f);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.09f, 0.13f, 0.96f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padX, padY));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if (ImGui::Begin("##TaskbarPanel", nullptr, flags))
    {
        const float winW    = ImGui::GetWindowWidth();
        const float pwrW    = std::max(76.0f, ImGui::CalcTextSize("PWR").x + 34.0f);
        const float pwrPosX = winW - pwrW - padX;

        float idealW = std::max({ 148.0f, ImGui::CalcTextSize("File Explorer").x + 50.0f, ImGui::CalcTextSize("Task Manager").x + 50.0f });
        float availW = pwrPosX - padX - 24.0f;
        bool  labels = (availW >= idealW * 3.0f);
        ImVec2 btnSz(labels ? idealW : std::clamp(availW / 3.0f, 44.0f, idealW), btnH);

        if (DrawTaskbarButton("##BtnSysInfo", "System Info", IconType::SystemInfo, state.showSystemInfo, btnSz, labels))
            state.showSystemInfo = !state.showSystemInfo;

        ImGui::SameLine(0.0f, 8.0f);
        if (DrawTaskbarButton("##BtnFiles", "File Explorer", IconType::FileExplorer, state.showFileExplorer, btnSz, labels))
            state.showFileExplorer = !state.showFileExplorer;

        ImGui::SameLine(0.0f, 8.0f);
        if (DrawTaskbarButton("##BtnTaskMgr", "Task Manager", IconType::TaskManager, state.showTaskManager, btnSz, labels))
            state.showTaskManager = !state.showTaskManager;

        // Running applications indicator
        int openCount = (state.showSystemInfo ? 1 : 0) + (state.showFileExplorer ? 1 : 0) + (state.showTaskManager ? 1 : 0);
        char activeBuf[64];
        std::snprintf(activeBuf, sizeof(activeBuf), "|   Active Windows: %d", openCount);
        if (padX + btnSz.x * 3.0f + 52.0f + ImGui::CalcTextSize(activeBuf).x < pwrPosX)
        {
            ImGui::SameLine(0.0f, 20.0f);
            ImGui::SetCursorPosY((tbH - ImGui::GetTextLineHeight()) * 0.5f);
            ImGui::TextDisabled("%s", activeBuf);
        }

        // Right-aligned PWR shutdown button
        ImGui::SameLine();
        ImGui::SetCursorPos(ImVec2(pwrPosX, padY));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.72f, 0.16f, 0.18f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.88f, 0.22f, 0.24f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.58f, 0.12f, 0.14f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

        if (ImGui::Button("PWR", ImVec2(pwrW, btnH)))
            state.isRunning = false;

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
    }
    ImGui::End();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}
