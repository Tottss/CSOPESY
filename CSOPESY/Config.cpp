#include "Config.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace Config
{
    std::string WINDOW_TITLE;
    int         WINDOW_WIDTH;
    int         WINDOW_HEIGHT;
    float       TASKBAR_HEIGHT;
    bool        TASKBAR_AT_BOTTOM;
    ImU32       WALLPAPER_TOP_LEFT;
    ImU32       WALLPAPER_TOP_RIGHT;
    ImU32       WALLPAPER_BOTTOM_RIGHT;
    ImU32       WALLPAPER_BOTTOM_LEFT;
    std::vector<ProcessEntry> DUMMY_PROCESSES;
    float       TOTAL_SYSTEM_MEMORY_MB;
    std::string CLOCK_FORMAT;
    bool        TASK_MANAGER_WOBBLE;

    static std::vector<ProcessEntry> GetDefaultProcesses()
    {
        return {
            { "csopesy_compositor.exe", 104,  "Running",    4.2f, 148.5f },
            { "window_manager.exe",     218,  "Running",    2.1f,  84.2f },
            { "taskmgr.exe",            412,  "Running",    1.5f,  52.8f },
            { "file_explorer.exe",      520,  "Running",    0.8f,  64.0f },
            { "sys_info_service.exe",   608,  "Running",    0.4f,  31.4f },
            { "desktop_clock.exe",      715,  "Running",    0.2f,  18.6f },
            { "audio_endpoint.exe",     884,  "Background", 0.3f,  27.9f },
            { "network_daemon.exe",     932,  "Background", 0.6f,  41.2f },
            { "security_health.exe",    1104, "Background", 0.5f,  58.0f },
            { "system_idle.exe",        4,    "Running",   89.4f,   0.4f }
        };
    }

    float GetEffectiveTaskbarHeight(float displayHeight)
    {
        float safeHeight = std::isfinite(TASKBAR_HEIGHT) ? TASKBAR_HEIGHT : 52.0f;
        return std::clamp(safeHeight, 28.0f, std::max(28.0f, displayHeight * 0.4f));
    }

    void ResetToDefaults()
    {
        WINDOW_TITLE           = "CSOPESY Desktop OS Emulator";
        WINDOW_WIDTH           = 1280;
        WINDOW_HEIGHT          = 720;
        TASKBAR_HEIGHT         = 52.0f;
        TASKBAR_AT_BOTTOM      = true;
        WALLPAPER_TOP_LEFT     = IM_COL32(18, 42, 86, 255);
        WALLPAPER_TOP_RIGHT    = IM_COL32(34, 82, 142, 255);
        WALLPAPER_BOTTOM_RIGHT = IM_COL32(28, 110, 96, 255);
        WALLPAPER_BOTTOM_LEFT  = IM_COL32(16, 68, 78, 255);
        DUMMY_PROCESSES        = GetDefaultProcesses();
        TOTAL_SYSTEM_MEMORY_MB = 4096.0f;
        CLOCK_FORMAT           = "%A, %b %d, %Y  |  %I:%M:%S %p";
        TASK_MANAGER_WOBBLE    = false;
    }

    static std::string Trim(const std::string& s)
    {
        size_t a = 0, b = s.size();
        while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
        while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
        return s.substr(a, b - a);
    }

    static std::string ToLower(std::string s)
    {
        for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return s;
    }

    static std::vector<std::string> Split(const std::string& s, char delim)
    {
        std::vector<std::string> tokens;
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim))
            tokens.push_back(Trim(item));
        return tokens;
    }

    static bool ParseFloat(const std::string& s, float& out)
    {
        try {
            float v = std::stof(s);
            if (!std::isfinite(v)) return false;
            out = v;
            return true;
        } catch (...) { return false; }
    }

    static bool ParseBool(const std::string& s, bool& out)
    {
        std::string v = ToLower(Trim(s));
        if (v == "true" || v == "1" || v == "yes" || v == "bottom") { out = true; return true; }
        if (v == "false" || v == "0" || v == "no" || v == "top")    { out = false; return true; }
        return false;
    }

    static bool ParseColor(const std::string& s, ImU32& out)
    {
        auto parts = Split(s, (s.find('|') != std::string::npos) ? '|' : ',');
        if (parts.size() < 3) return false;
        try {
            int r = std::clamp(std::stoi(parts[0]), 0, 255);
            int g = std::clamp(std::stoi(parts[1]), 0, 255);
            int b = std::clamp(std::stoi(parts[2]), 0, 255);
            int a = (parts.size() >= 4) ? std::clamp(std::stoi(parts[3]), 0, 255) : 255;
            out = IM_COL32(r, g, b, a);
            return true;
        } catch (...) { return false; }
    }

    static bool ParseProcess(const std::string& s, ProcessEntry& out)
    {
        auto p = Split(s, (s.find('|') != std::string::npos) ? '|' : ',');
        if (p.size() < 5) return false;
        try {
            int pid = std::stoi(p[1]);
            float cpu = 0.0f, mem = 0.0f;
            if (!ParseFloat(p[3], cpu) || !ParseFloat(p[4], mem)) return false;
            if (p[0].empty() || p[2].empty() || pid < 0 || cpu < 0.0f || mem < 0.0f) return false;
            out = { p[0], pid, p[2], std::clamp(cpu, 0.0f, 100.0f), mem };
            return true;
        } catch (...) { return false; }
    }

    static std::filesystem::path ResolveConfigPath(const std::string& filename)
    {
        namespace fs = std::filesystem;
        std::error_code ec;
        fs::path best = filename;
        auto bestTime = (fs::file_time_type::min)();
        bool found = false;

        for (const char* dir : { ".", "..", "../..", "CSOPESY", "../CSOPESY", "../../CSOPESY", "../../../CSOPESY", "x64/Debug", "x64/Release", "Debug", "Release" })
        {
            fs::path candidate = fs::path(dir) / filename;
            if (fs::exists(candidate, ec) && !ec)
            {
                auto wt = fs::last_write_time(candidate, ec);
                if (!ec && (!found || wt > bestTime))
                {
                    bestTime = wt;
                    best = candidate;
                    found = true;
                }
            }
        }
        return best;
    }

    static void ValidateConfig()
    {
        if (WINDOW_TITLE.empty()) WINDOW_TITLE = "CSOPESY Desktop OS Emulator";
        if (WINDOW_WIDTH < 320 || WINDOW_WIDTH > 7680)   WINDOW_WIDTH = 1280;
        if (WINDOW_HEIGHT < 240 || WINDOW_HEIGHT > 4320) WINDOW_HEIGHT = 720;

        float maxTb = static_cast<float>(WINDOW_HEIGHT) * 0.4f;
        if (!std::isfinite(TASKBAR_HEIGHT) || TASKBAR_HEIGHT < 28.0f || TASKBAR_HEIGHT > maxTb)
            TASKBAR_HEIGHT = std::clamp(52.0f, 28.0f, maxTb);

        if (!std::isfinite(TOTAL_SYSTEM_MEMORY_MB) || TOTAL_SYSTEM_MEMORY_MB <= 0.0f)
            TOTAL_SYSTEM_MEMORY_MB = 4096.0f;
        if (CLOCK_FORMAT.empty())
            CLOCK_FORMAT = "%A, %b %d, %Y  |  %I:%M:%S %p";
        if (DUMMY_PROCESSES.empty())
            DUMMY_PROCESSES = GetDefaultProcesses();
    }

    bool LoadFromFile(const std::string& filename)
    {
        ResetToDefaults();
        std::filesystem::path path = ResolveConfigPath(filename);
        std::ifstream file(path);
        if (!file.is_open())
        {
            std::fprintf(stderr, "[Config] '%s' not found; using defaults.\n", filename.c_str());
            return false;
        }

        std::fprintf(stderr, "[Config] Loaded '%s'\n", std::filesystem::absolute(path).string().c_str());
        std::vector<ProcessEntry> processes;
        std::string line;
        int lineNum = 0;

        while (std::getline(file, line))
        {
            ++lineNum;
            std::string trimmed = Trim(line);
            if (trimmed.empty() || trimmed[0] == '#' || trimmed[0] == ';') continue;

            size_t eq = trimmed.find('=');
            if (eq == std::string::npos) continue;

            std::string key = ToLower(Trim(trimmed.substr(0, eq)));
            std::string val = Trim(trimmed.substr(eq + 1));
            if (val.empty()) continue;

            bool ok = true;
            try {
                if      (key == "window_title")           WINDOW_TITLE = val;
                else if (key == "window_width")           WINDOW_WIDTH = std::stoi(val);
                else if (key == "window_height")          WINDOW_HEIGHT = std::stoi(val);
                else if (key == "taskbar_height")         ok = ParseFloat(val, TASKBAR_HEIGHT);
                else if (key == "taskbar_at_bottom" ||
                         key == "taskbar_position")       ok = ParseBool(val, TASKBAR_AT_BOTTOM);
                else if (key == "wallpaper_top_left")     ok = ParseColor(val, WALLPAPER_TOP_LEFT);
                else if (key == "wallpaper_top_right")    ok = ParseColor(val, WALLPAPER_TOP_RIGHT);
                else if (key == "wallpaper_bottom_right") ok = ParseColor(val, WALLPAPER_BOTTOM_RIGHT);
                else if (key == "wallpaper_bottom_left")  ok = ParseColor(val, WALLPAPER_BOTTOM_LEFT);
                else if (key == "total_system_memory_mb") ok = ParseFloat(val, TOTAL_SYSTEM_MEMORY_MB);
                else if (key == "clock_format")           CLOCK_FORMAT = val;
                else if (key == "task_manager_wobble")    ok = ParseBool(val, TASK_MANAGER_WOBBLE);
                else if (key == "process") {
                    ProcessEntry entry;
                    if ((ok = ParseProcess(val, entry))) processes.push_back(entry);
                }
            } catch (...) { ok = false; }

            if (!ok)
                std::fprintf(stderr, "[Config] Line %d: invalid value for '%s'\n", lineNum, key.c_str());
        }

        if (!processes.empty())
            DUMMY_PROCESSES = std::move(processes);

        ValidateConfig();
        return true;
    }
}
