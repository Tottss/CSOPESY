#pragma once

#include <chrono>

// Shared application state across the desktop compositor and windows
struct AppState
{
    bool isRunning          = true;  // Controlled by the PWR button
    bool showSystemInfo     = false; // Taskbar Icon Button 1: Unique UI Screen #1
    bool showFileExplorer   = false; // Taskbar Icon Button 2: Unique UI Screen #2
    bool showTaskManager    = false; // Taskbar Icon Button 3: Task Manager
    int  selectedFileIndex  = 0;
    std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
};
