#include "Desktop.h"
#include "Config.h"

#include <chrono>
#include <ctime>
#include <string>

static std::string GetFormattedCurrentTime()
{
    std::time_t nowTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm localTm{};
#ifdef _WIN32
    localtime_s(&localTm, &nowTime);
#else
    localtime_r(&nowTime, &localTm);
#endif
    char buf[128] = { 0 };
    if (std::strftime(buf, sizeof(buf), Config::CLOCK_FORMAT.c_str(), &localTm) == 0)
        std::strftime(buf, sizeof(buf), "%A, %b %d, %Y  |  %I:%M:%S %p", &localTm);
    return buf;
}

void RenderDesktop(const ImGuiIO& io)
{
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(io.DisplaySize);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if (ImGui::Begin("##DesktopBaseLayer", nullptr, flags))
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 pos     = ImGui::GetWindowPos();
        ImVec2 size    = ImGui::GetWindowSize();
        ImVec2 end     = ImVec2(pos.x + size.x, pos.y + size.y);
        float  tbH     = Config::GetEffectiveTaskbarHeight(io.DisplaySize.y);

        // Multi-color gradient wallpaper filling the entire window
        dl->AddRectFilledMultiColor(
            pos, end,
            Config::WALLPAPER_TOP_LEFT, Config::WALLPAPER_TOP_RIGHT,
            Config::WALLPAPER_BOTTOM_RIGHT, Config::WALLPAPER_BOTTOM_LEFT
        );

        // Subtle decorative grid and concentric circles drawn via ImGui draw commands
        const ImU32 gridCol = IM_COL32(255, 255, 255, 12);
        for (float x = 0.0f; x < size.x; x += 64.0f)
            dl->AddLine(ImVec2(pos.x + x, pos.y), ImVec2(pos.x + x, end.y), gridCol, 1.0f);
        for (float y = 0.0f; y < size.y; y += 64.0f)
            dl->AddLine(ImVec2(pos.x, pos.y + y), ImVec2(end.x, pos.y + y), gridCol, 1.0f);

        float centerY = Config::TASKBAR_AT_BOTTOM
            ? (pos.y + (size.y - tbH) * 0.5f)
            : (pos.y + tbH + (size.y - tbH) * 0.5f);
        ImVec2 center(pos.x + size.x * 0.5f, centerY);
        dl->AddCircle(center, 150.0f, IM_COL32(120, 210, 255, 30), 64, 2.0f);
        dl->AddCircle(center, 220.0f, IM_COL32(120, 255, 190, 20), 64, 1.5f);
        dl->AddText(ImVec2(pos.x + 24.0f, center.y), IM_COL32(80, 255, 120, 200), "CSOPESY OS v1.0  -  System Online");

        // Real-time clock in fixed top-right corner (foreground draw list stays above windows)
        ImDrawList* fg  = ImGui::GetForegroundDrawList();
        std::string clk = GetFormattedCurrentTime();
        ImVec2 txtSize  = ImGui::CalcTextSize(clk.c_str());
        float topY      = pos.y + (Config::TASKBAR_AT_BOTTOM ? 12.0f : (tbH + 12.0f));
        ImVec2 boxMin(end.x - txtSize.x - 40.0f, topY);
        ImVec2 boxMax(end.x - 12.0f, topY + txtSize.y + 16.0f);

        fg->AddRectFilled(boxMin, boxMax, IM_COL32(15, 18, 24, 215), 6.0f);
        fg->AddRect(boxMin, boxMax, IM_COL32(90, 140, 200, 120), 6.0f, 0, 1.0f);
        fg->AddText(ImVec2(boxMin.x + 14.0f, boxMin.y + 8.0f), IM_COL32(235, 242, 255, 255), clk.c_str());
    }
    ImGui::End();
    ImGui::PopStyleVar(2);
}
