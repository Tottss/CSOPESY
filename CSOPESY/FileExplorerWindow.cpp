#include "FileExplorerWindow.h"
#include "imgui.h"

void RenderFileExplorerWindow(AppState& state)
{
    if (!state.showFileExplorer) return;

    ImGui::SetNextWindowSize(ImVec2(580.0f, 360.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(160.0f, 140.0f), ImGuiCond_FirstUseEver);

    struct VirtualFile { const char* name; const char* size; const char* type; const char* content; };
    static const VirtualFile files[] = {
        { "readme.txt", "1.2 KB", "Text Document",
          "Welcome to CSOPESY Desktop OS Emulator!\n\n"
          "Use the Taskbar to launch system applications,\n"
          "inspect active processes in Task Manager, or shut down\n"
          "the system cleanly using the red PWR button." },
        { "config.txt", "1.1 KB", "Configuration File",
          "# Loaded at startup without recompiling:\n"
          "window_title, window_width, window_height\n"
          "taskbar_height, taskbar_at_bottom\n"
          "wallpaper_top_left / right / bottom_right / bottom_left\n"
          "clock_format, total_system_memory_mb, process entries" },
        { "boot_log.txt", "2.4 KB", "System Log",
          "[00:00.01] Loading runtime parameters from config.txt...\n"
          "[00:00.03] Initializing GLFW window and OpenGL 3.0 context...\n"
          "[00:00.06] Starting immediate-mode desktop compositor...\n"
          "[00:00.10] Taskbar and foreground clock services online." },
        { "developers.txt", "0.5 KB", "Text Document",
          "CSOPESY Semi-Major Output 2\n---------------------------\n"
          "1. Abdulrahman, Saoud\n2. Fabricante, Jeruel\n3. Galvez, Anousheh Monick" }
    };

    if (ImGui::Begin("File Explorer", &state.showFileExplorer))
    {
        ImGui::TextDisabled("Path: C:\\CSOPESY\\System\\Documents");
        ImGui::Separator();

        ImGui::BeginChild("FileListPane", ImVec2(210.0f, 0.0f), ImGuiChildFlags_Borders);
        for (int i = 0; i < 4; ++i)
        {
            if (ImGui::Selectable(files[i].name, state.selectedFileIndex == i))
                state.selectedFileIndex = i;
        }
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("FilePreviewPane", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders);
        const auto& sel = files[state.selectedFileIndex];
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.35f, 1.0f), "%s", sel.name);
        ImGui::TextDisabled("Type: %s   |   Size: %s", sel.type, sel.size);
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextWrapped("%s", sel.content);
        ImGui::EndChild();
    }
    ImGui::End();
}
