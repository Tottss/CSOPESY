#pragma once

#include "imgui.h"
#include <string>
#include <vector>

namespace Config
{
    struct ProcessEntry
    {
        std::string name;
        int         pid;
        std::string status;
        float       cpuPercent;
        float       memoryMB;
    };

    extern std::string WINDOW_TITLE;
    extern int         WINDOW_WIDTH;
    extern int         WINDOW_HEIGHT;

    extern float       TASKBAR_HEIGHT;
    extern bool        TASKBAR_AT_BOTTOM; // true = bottom, false = top

    extern ImU32       WALLPAPER_TOP_LEFT;
    extern ImU32       WALLPAPER_TOP_RIGHT;
    extern ImU32       WALLPAPER_BOTTOM_RIGHT;
    extern ImU32       WALLPAPER_BOTTOM_LEFT;

    extern std::vector<ProcessEntry> DUMMY_PROCESSES;
    extern float       TOTAL_SYSTEM_MEMORY_MB;
    extern std::string CLOCK_FORMAT;
    extern bool        TASK_MANAGER_WOBBLE;

    float GetEffectiveTaskbarHeight(float displayHeight);
    void  ResetToDefaults();
    bool  LoadFromFile(const std::string& filename = "config.txt");
}
