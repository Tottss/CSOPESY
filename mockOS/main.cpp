// CSOPESY Desktop OS Emulator - Linux based design
// Stack: GLFW + OpenGL3 + Dear ImGui


#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>

static ImU32 C(int r, int g, int b, int a = 255) { return IM_COL32(r, g, b, a); }

//state
enum State { BOOT, DESKTOP, SHUTDOWN };
static bool g_powerRequested = false;
static bool g_showFiles = false, g_showTerm = false, g_showMon = false, g_showAbout = false;

// theme (Yaru-like dark)
static void ApplyTheme()
{
    ImGuiStyle& s = ImGui::GetStyle();
    ImGui::StyleColorsDark();
    s.WindowRounding = 10; s.ChildRounding = 6; s.FrameRounding = 6;
    s.PopupRounding = 8; s.GrabRounding = 6; s.TabRounding = 6;
    s.WindowBorderSize = 1; s.FrameBorderSize = 0;
    s.WindowPadding = ImVec2(12, 10); s.ItemSpacing = ImVec2(8, 7);
    s.WindowTitleAlign = ImVec2(0.5f, 0.5f);

    ImVec4* c = s.Colors;
    ImVec4 accent(0.914f, 0.329f, 0.125f, 1.0f); // Ubuntu orange #E95420
    c[ImGuiCol_WindowBg]         = ImVec4(0.18f, 0.18f, 0.18f, 0.98f);
    c[ImGuiCol_ChildBg]          = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    c[ImGuiCol_TitleBg]          = ImVec4(0.14f, 0.14f, 0.14f, 1.0f);
    c[ImGuiCol_TitleBgActive]    = ImVec4(0.22f, 0.22f, 0.22f, 1.0f);
    c[ImGuiCol_Border]           = ImVec4(1, 1, 1, 0.08f);
    c[ImGuiCol_FrameBg]          = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
    c[ImGuiCol_Button]           = ImVec4(0.27f, 0.27f, 0.27f, 1.0f);
    c[ImGuiCol_ButtonHovered]    = ImVec4(0.34f, 0.34f, 0.34f, 1.0f);
    c[ImGuiCol_ButtonActive]     = accent;
    c[ImGuiCol_Header]           = ImVec4(accent.x, accent.y, accent.z, 0.55f);
    c[ImGuiCol_HeaderHovered]    = ImVec4(accent.x, accent.y, accent.z, 0.35f);
    c[ImGuiCol_HeaderActive]     = accent;
    c[ImGuiCol_Tab]              = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
    c[ImGuiCol_TabHovered]       = accent;
    c[ImGuiCol_TabActive]        = ImVec4(accent.x, accent.y, accent.z, 0.8f);
    c[ImGuiCol_PlotLines]        = ImVec4(0.45f, 0.82f, 0.30f, 1.0f);
    c[ImGuiCol_PlotHistogram]    = accent;
    c[ImGuiCol_TableHeaderBg]    = ImVec4(0.22f, 0.22f, 0.22f, 1.0f);
    c[ImGuiCol_TableRowBgAlt]    = ImVec4(1, 1, 1, 0.03f);
    c[ImGuiCol_CheckMark]        = accent;
}

// boot / shutdown
struct BootLine { float t; bool ok; const char* txt; };
static const BootLine BOOT_LINES[] = {
    {0.00f, false, "CSOPESY GNU/Linux 1.0 (csopesy-pc) - kernel 6.8.0-csopesy"},
    {0.15f, false, "CPU: Pentium III   RAM: 64000K OK   BIOS: BCN SIT 1989-1994 Special CSOPESY"},
    {0.35f, true,  "Reached target Local File Systems."},
    {0.60f, true,  "Started Journal Service."},
    {0.90f, true,  "Started Network Manager."},
    {1.30f, true,  "Started Dispatcher Service."},
    {1.70f, true,  "Mounted /dev/hda1 (420 MB WDCAC2420H)."},
    {2.10f, true,  "Started Dear ImGui Compositor."},
    {2.60f, true,  "Started GLFW Display Manager."},
    {3.10f, true,  "Reached target Graphical Interface."},
    {3.60f, false, "csopesy-pc login: guest (automatic login)"},
};

static void BeginFullscreen(const char* name)
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, C(0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::Begin(name, nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);
}
static void EndFullscreen() { ImGui::End(); ImGui::PopStyleVar(2); ImGui::PopStyleColor(); }

static void DrawBoot(float elapsed)
{
    BeginFullscreen("##boot");
    for (const BootLine& l : BOOT_LINES) {
        if (elapsed < l.t) break;
        if (l.ok) {
            ImGui::TextUnformatted("[  ");
            ImGui::SameLine(0, 0);
            ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1), "OK");
            ImGui::SameLine(0, 0);
            ImGui::Text("  ] %s", l.txt);
        } else {
            ImGui::TextUnformatted(l.txt);
        }
    }
    EndFullscreen();
}

static void DrawShutdown(float elapsed)
{
    BeginFullscreen("##shutdown");
    ImGui::TextUnformatted("Stopping Dear ImGui Compositor...");
    if (elapsed > 0.4f) ImGui::Text("[  OK  ] Stopped GLFW Display Manager.");
    if (elapsed > 0.9f) ImGui::Text("[  OK  ] Unmounted /dev/hda1.");
    if (elapsed > 1.4f) ImGui::Text("[  OK  ] Reached target Power-Off.");
    if (elapsed > 1.7f) ImGui::Text("System halted.");
    EndFullscreen();
}

// wallpaper
static void DrawWallpaper()
{
    ImVec2 sz = ImGui::GetIO().DisplaySize;
    ImDrawList* bg = ImGui::GetBackgroundDrawList();
    // aubergine -> orange gradient (UL, UR, BR, BL)
    bg->AddRectFilledMultiColor(ImVec2(0, 0), sz, C(119, 33, 111), C(44, 0, 30), C(150, 45, 30), C(233, 84, 32));

    float t = (float)glfwGetTime();
    for (int i = 0; i < 6; i++) {
        float fx = 0.5f + 0.35f * sinf(t * 0.07f * (i + 1) + i * 1.7f);
        float fy = 0.5f + 0.30f * cosf(t * 0.05f * (i + 2) + i * 2.3f);
        bg->AddCircleFilled(ImVec2(sz.x * fx, sz.y * fy), sz.y * (0.18f + 0.04f * i), C(255, 255, 255, 10), 64);
    }
    // status text (matches the reference screenshot)
    bg->AddText(ImVec2(24, sz.y * 0.5f), C(80, 255, 80), "CSOPESY OS v1.0 - System Online");
}

// top panel
static void DrawTopBar()
{
    ImVec2 sz = ImGui::GetIO().DisplaySize;
    const float H = 30;
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(sz.x, H));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 3));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, C(0, 0, 0, 190));
    ImGui::PushStyleColor(ImGuiCol_Button, C(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, C(255, 255, 255, 40));
    ImGui::Begin("##topbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                      ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);

    // Activities 
    if (ImGui::Button("Activities")) ImGui::OpenPopup("activities");
    if (ImGui::BeginPopup("activities")) {
        if (ImGui::MenuItem("Files"))          g_showFiles = true;
        if (ImGui::MenuItem("Terminal"))       g_showTerm = true;
        if (ImGui::MenuItem("System Monitor")) g_showMon = true;
        if (ImGui::MenuItem("About / Settings")) g_showAbout = true;
        ImGui::EndPopup();
    }

    // right side: clock | VOL | NET | PWR  
    time_t now = time(nullptr);
    char clock_buf[64];
    strftime(clock_buf, sizeof clock_buf, "%a %b %d  %I:%M:%S %p", localtime(&now));
    float cw = ImGui::CalcTextSize(clock_buf).x;
    float bw = 44, total = cw + 16 + bw * 3 + 12;
    ImGui::SameLine(sz.x - total - 8);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(clock_buf);
    ImGui::SameLine(0, 16);
    ImGui::Button("VOL", ImVec2(bw, 0));
    ImGui::SameLine(0, 4);
    ImGui::Button("NET", ImVec2(bw, 0));
    ImGui::SameLine(0, 4);
    ImGui::PushStyleColor(ImGuiCol_Text, C(255, 90, 80));
    if (ImGui::Button("PWR", ImVec2(bw, 0))) ImGui::OpenPopup("power");
    ImGui::PopStyleColor();
    if (ImGui::BeginPopup("power")) {
        ImGui::TextUnformatted("Power Off");
        ImGui::Separator();
        if (ImGui::MenuItem("Shut Down...")) g_powerRequested = true;
        ImGui::MenuItem("Cancel");
        ImGui::EndPopup();
    }

    ImGui::End();
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(4);
}


enum Icon { ICON_FILES, ICON_TERM, ICON_MON, ICON_SET };

static bool DockIcon(const char* id, Icon icon, bool open, const char* tip)
{
    const float S = 48;
    ImVec2 p = ImGui::GetCursorScreenPos();
    bool clicked = ImGui::InvisibleButton(id, ImVec2(S, S));
    bool hov = ImGui::IsItemHovered();
    ImDrawList* d = ImGui::GetWindowDrawList();
    auto P = [&](float x, float y) { return ImVec2(p.x + x, p.y + y); };

    if (hov) d->AddRectFilled(P(0, 0), P(S, S), C(255, 255, 255, 40), 10);

    switch (icon) {
    case ICON_FILES:  // folder
        d->AddRectFilled(P(6, 10), P(22, 20), C(200, 70, 25), 3);
        d->AddRectFilled(P(6, 16), P(42, 40), C(247, 130, 60), 4);
        d->AddRectFilled(P(6, 22), P(42, 40), C(233, 100, 40), 4);
        break;
    case ICON_TERM:   // terminal
        d->AddRectFilled(P(5, 7), P(43, 41), C(48, 10, 36), 7);
        d->AddRect(P(5, 7), P(43, 41), C(255, 255, 255, 90), 7);
        d->AddText(ImGui::GetFont(), 20, P(11, 13), C(255, 255, 255), ">_");
        break;
    case ICON_MON: {  // system monitor graph
        d->AddRectFilled(P(5, 7), P(43, 41), C(32, 32, 32), 7);
        d->AddRect(P(5, 7), P(43, 41), C(255, 255, 255, 90), 7);
        ImVec2 pts[] = {P(9, 34), P(16, 24), P(22, 30), P(29, 14), P(35, 22), P(40, 16)};
        d->AddPolyline(pts, 6, C(110, 210, 80), 0, 2.5f);
        break;
    }
    case ICON_SET:    // gear
        for (int i = 0; i < 8; i++) {
            float a = i * 3.14159f / 4;
            d->AddCircleFilled(P(24 + cosf(a) * 15, 24 + sinf(a) * 15), 4.5f, C(170, 170, 170));
        }
        d->AddCircleFilled(P(24, 24), 14, C(170, 170, 170), 32);
        d->AddCircleFilled(P(24, 24), 6, C(40, 40, 40), 24);
        break;
    }
    if (open) d->AddCircleFilled(P(S / 2, S + 1), 2.5f, C(233, 84, 32));
    if (hov) ImGui::SetTooltip("%s", tip);
    return clicked;
}

static void DrawDock()
{
    ImVec2 sz = ImGui::GetIO().DisplaySize;
    float w = 4 * 48 + 3 * 8 + 28, h = 48 + 22;
    ImGui::SetNextWindowPos(ImVec2((sz.x - w) * 0.5f, sz.y - h - 8));
    ImGui::SetNextWindowSize(ImVec2(w, h));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 16);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14, 8));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, C(25, 25, 25, 205));
    ImGui::Begin("##dock", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
    if (DockIcon("files", ICON_FILES, g_showFiles, "Files"))                 g_showFiles = !g_showFiles;
    ImGui::SameLine();
    if (DockIcon("term", ICON_TERM, g_showTerm, "Terminal"))                 g_showTerm = !g_showTerm;
    ImGui::SameLine();
    if (DockIcon("mon", ICON_MON, g_showMon, "System Monitor (Task Manager)")) g_showMon = !g_showMon;
    ImGui::SameLine();
    if (DockIcon("set", ICON_SET, g_showAbout, "About / Settings"))          g_showAbout = !g_showAbout;
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}

struct FileEnt { const char* name; const char* type; const char* size; const char* date; };
static const FileEnt HOME[]  = {{"Desktop","Folder","-","Oct 10"},{"Documents","Folder","-","Oct 09"},{"Downloads","Folder","-","Oct 11"},
                                {"Pictures","Folder","-","Sep 30"},{"Music","Folder","-","Sep 18"},{".bashrc","Text","3.7 kB","Sep 18"}};
static const FileEnt DOCS[]  = {{"CSOPESY_Report.pptx","Presentation","4.2 MB","Oct 09"},{"README.txt","Text","1.1 kB","Oct 08"},
                                {"notes.md","Markdown","8.4 kB","Oct 05"},{"architecture.pdf","PDF","312 kB","Oct 02"}};
static const FileEnt DLS[]   = {{"imgui-1.91.zip","Archive","2.8 MB","Oct 11"},{"glfw-3.4.zip","Archive","1.9 MB","Oct 11"},{"wallpaper.png","Image","1.2 MB","Oct 10"}};
static const FileEnt PICS[]  = {{"screenshot_01.png","Image","640 kB","Sep 30"},{"screenshot_02.png","Image","702 kB","Sep 30"}};

static void DrawFiles()
{
    if (!g_showFiles) return;
    ImGui::SetNextWindowPos(ImVec2(110, 80), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(640, 380), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Files", &g_showFiles)) {
        static int place = 0;
        const char* places[] = {"Home", "Documents", "Downloads", "Pictures", "Trash"};
        ImGui::BeginChild("sidebar", ImVec2(150, 0), true);
        ImGui::TextDisabled("Places");
        for (int i = 0; i < 5; i++)
            if (ImGui::Selectable(places[i], place == i)) place = i;
        ImGui::EndChild();
        ImGui::SameLine();

        const FileEnt* list = HOME; int n = 6;
        if (place == 1) { list = DOCS; n = 4; }
        if (place == 2) { list = DLS;  n = 3; }
        if (place == 3) { list = PICS; n = 2; }
        if (place == 4) { list = nullptr; n = 0; }

        ImGui::BeginChild("content", ImVec2(0, 0), true);
        ImGui::Text("/home/guest%s%s", place ? "/" : "", place ? places[place] : "");
        ImGui::Separator();
        if (n == 0) ImGui::TextDisabled("Trash is empty");
        else if (ImGui::BeginTable("files", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
            ImGui::TableSetupColumn("Name"); ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Size"); ImGui::TableSetupColumn("Modified");
            ImGui::TableHeadersRow();
            for (int i = 0; i < n; i++) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Selectable(list[i].name, false, ImGuiSelectableFlags_SpanAllColumns);
                ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(list[i].type);
                ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(list[i].size);
                ImGui::TableSetColumnIndex(3); ImGui::TextUnformatted(list[i].date);
            }
            ImGui::EndTable();
        }
        ImGui::EndChild();
    }
    ImGui::End();
}


struct Term { std::vector<std::string> lines; char buf[256] = ""; bool scroll = true; bool init = false; };
static Term g_term;

static void RunCmd(Term& t, const std::string& c)
{
    t.lines.push_back("guest@csopesy-pc:~$ " + c);
    if (c.empty()) return;
    if (c == "help") {
        t.lines.push_back("Commands: help neofetch ls pwd whoami date uname clear");
    } else if (c == "clear") {
        t.lines.clear();
    } else if (c == "ls") {
        t.lines.push_back("Desktop  Documents  Downloads  Music  Pictures");
    } else if (c == "pwd") {
        t.lines.push_back("/home/guest");
    } else if (c == "whoami") {
        t.lines.push_back("guest");
    } else if (c == "uname" || c == "uname -a") {
        t.lines.push_back("Linux csopesy-pc 6.8.0-csopesy #1 SMP PREEMPT x86_64 GNU/Linux");
    } else if (c == "date") {
        time_t n = time(nullptr); char b[64];
        strftime(b, sizeof b, "%a %b %d %H:%M:%S %Y", localtime(&n));
        t.lines.push_back(b);
    } else if (c == "neofetch") {
        char up[64]; int s = (int)glfwGetTime();
        snprintf(up, sizeof up, "Uptime: %d min, %d sec", s / 60, s % 60);
        t.lines.push_back("guest@csopesy-pc");
        t.lines.push_back("----------------");
        t.lines.push_back("OS: CSOPESY GNU/Linux 1.0 x86_64");
        t.lines.push_back("Kernel: 6.8.0-csopesy");
        t.lines.push_back(up);
        t.lines.push_back("Shell: bash 5.2");
        t.lines.push_back("DE: Dear ImGui Compositor");
        t.lines.push_back("CPU: Pentium III");
        t.lines.push_back("Memory: 31MiB / 62MiB");
    } else {
        t.lines.push_back("bash: " + c + ": command not found");
    }
}

static void DrawTerminal()
{
    if (!g_showTerm) return;
    if (!g_term.init) {
        g_term.lines.push_back("CSOPESY GNU/Linux 1.0  -  type 'help' for commands");
        g_term.init = true;
    }
    ImGui::SetNextWindowPos(ImVec2(200, 130), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(620, 340), ImGuiCond_FirstUseEver);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, C(48, 10, 36, 245));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, C(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, C(0, 0, 0, 60));
    if (ImGui::Begin("guest@csopesy-pc: ~", &g_showTerm)) {
        ImGui::BeginChild("out", ImVec2(0, -34));
        for (const std::string& l : g_term.lines) {
            bool prompt = l.rfind("guest@csopesy-pc:~$", 0) == 0;
            if (prompt) ImGui::PushStyleColor(ImGuiCol_Text, C(138, 226, 52));
            ImGui::TextUnformatted(l.c_str());
            if (prompt) ImGui::PopStyleColor();
        }
        if (g_term.scroll) { ImGui::SetScrollHereY(1.0f); g_term.scroll = false; }
        ImGui::EndChild();

        ImGui::TextColored(ImVec4(0.54f, 0.89f, 0.2f, 1), "guest@csopesy-pc:~$");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputText("##in", g_term.buf, sizeof g_term.buf, ImGuiInputTextFlags_EnterReturnsTrue)) {
            RunCmd(g_term, g_term.buf);
            g_term.buf[0] = 0;
            g_term.scroll = true;
            ImGui::SetKeyboardFocusHere(-1);
        }
    }
    ImGui::End();
    ImGui::PopStyleColor(3);
}

// ------------------------------------------------------------------ app 3: System Monitor (Task Manager)
struct Proc { const char* name; int pid; float cpu; float mem; };
static const Proc PROCS[] = {
    {"systemd", 1, 0.3f, 12.4f},          {"Xorg", 987, 6.2f, 148.7f},
    {"gnome-shell", 1204, 9.8f, 312.5f},  {"NetworkManager", 640, 0.6f, 21.3f},
    {"pulseaudio", 1050, 1.4f, 18.9f},    {"nautilus", 1388, 2.1f, 96.0f},
    {"gnome-terminal", 1502, 1.7f, 74.2f},{"firefox", 2310, 14.5f, 824.6f},
    {"code", 2455, 11.2f, 640.1f},        {"csopesy-emu", 4021, 7.9f, 156.8f},
};

static void DrawMonitor()
{
    if (!g_showMon) return;
    ImGui::SetNextWindowPos(ImVec2(260, 100), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(620, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("System Monitor", &g_showMon)) {
        float t = (float)glfwGetTime();
        static float hist[90] = {0};
        static double last = 0;
        float total = 0;
        float cpu[10];
        for (int i = 0; i < 10; i++) {
            cpu[i] = PROCS[i].cpu * (0.8f + 0.2f * sinf(t * (0.6f + i * 0.13f) + PROCS[i].pid));
            total += cpu[i];
        }
        if (t - last > 0.1) {
            memmove(hist, hist + 1, sizeof(float) * 89);
            hist[89] = total; last = t;
        }

        if (ImGui::BeginTabBar("tabs")) {
            if (ImGui::BeginTabItem("Processes")) {
                if (ImGui::BeginTable("proc", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders |
                                                 ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable,
                                      ImVec2(0, -4))) {
                    ImGui::TableSetupScrollFreeze(0, 1);
                    ImGui::TableSetupColumn("Process Name", ImGuiTableColumnFlags_WidthStretch, 2.0f);
                    ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed, 60);
                    ImGui::TableSetupColumn("% CPU", ImGuiTableColumnFlags_WidthStretch, 1.2f);
                    ImGui::TableSetupColumn("Memory", ImGuiTableColumnFlags_WidthFixed, 90);
                    ImGui::TableHeadersRow();
                    for (int i = 0; i < 10; i++) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(PROCS[i].name);
                        ImGui::TableSetColumnIndex(1); ImGui::Text("%d", PROCS[i].pid);
                        ImGui::TableSetColumnIndex(2);
                        char ov[16]; snprintf(ov, sizeof ov, "%.1f%%", cpu[i]);
                        ImGui::ProgressBar(cpu[i] / 30.0f, ImVec2(-1, 0), ov);
                        ImGui::TableSetColumnIndex(3); ImGui::Text("%.1f MiB", PROCS[i].mem);
                    }
                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Resources")) {
                char ov[32]; snprintf(ov, sizeof ov, "CPU %.0f%%", total);
                ImGui::TextUnformatted("CPU History");
                ImGui::PlotLines("##cpu", hist, 90, 0, ov, 0.0f, 100.0f, ImVec2(-1, 110));
                ImGui::Spacing();
                float mem = 0; for (int i = 0; i < 10; i++) mem += PROCS[i].mem;
                ImGui::TextUnformatted("Memory");
                char mo[48]; snprintf(mo, sizeof mo, "%.0f MiB of 4096 MiB", mem);
                ImGui::ProgressBar(mem / 4096.0f, ImVec2(-1, 22), mo);
                ImGui::TextUnformatted("Swap");
                ImGui::ProgressBar(0.04f, ImVec2(-1, 22), "82 MiB of 2048 MiB");
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

// ------------------------------------------------------------------ app 4: About (extra)
static void DrawAbout()
{
    if (!g_showAbout) return;
    ImGui::SetNextWindowPos(ImVec2(340, 140), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(420, 260), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("About", &g_showAbout)) {
        ImGui::TextColored(ImVec4(0.914f, 0.329f, 0.125f, 1), "CSOPESY GNU/Linux 1.0");
        ImGui::Separator();
        ImGui::Text("Device name:   csopesy-pc");
        ImGui::Text("Processor:     Pentium III");
        ImGui::Text("Memory:        64000K");
        ImGui::Text("Graphics:      OpenGL 3 via GLFW");
        ImGui::Text("Compositor:    Dear ImGui (immediate mode)");
        ImGui::Spacing();
        ImGui::TextWrapped("Part of Project Anito: Support for the PH Gaming Industry "
                           "Towards Embracing In-House Game Engine Development.");
        ImGui::Spacing();
        static bool dark = true; static float bright = 0.8f;
        ImGui::Checkbox("Dark style", &dark);
        ImGui::SliderFloat("Brightness", &bright, 0.0f, 1.0f);
    }
    ImGui::End();
}

// ------------------------------------------------------------------ main
int main()
{
    if (!glfwInit()) return 1;
    const char* glsl = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* win = glfwCreateWindow(1280, 720, "CSOPESY Desktop OS Emulator", nullptr, nullptr);
    if (!win) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(win);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ApplyTheme();
    ImGui_ImplGlfw_InitForOpenGL(win, true);
    ImGui_ImplOpenGL3_Init(glsl);

    State st = BOOT;
    double t0 = glfwGetTime(), shutT = 0;

    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        double now = glfwGetTime();
        if (st == BOOT) {
            DrawBoot((float)(now - t0));
            if (now - t0 > 4.8) st = DESKTOP;
        } else if (st == DESKTOP) {
            DrawWallpaper();          // first thing drawn each frame
            DrawTopBar();
            DrawFiles();
            DrawTerminal();
            DrawMonitor();
            DrawAbout();
            DrawDock();
            if (g_powerRequested) { st = SHUTDOWN; shutT = now; }
        } else {
            DrawShutdown((float)(now - shutT));
            if (now - shutT > 2.2) glfwSetWindowShouldClose(win, 1);   // closed only via PWR
        }

        ImGui::Render();
        int w, h;
        glfwGetFramebufferSize(win, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(win);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
