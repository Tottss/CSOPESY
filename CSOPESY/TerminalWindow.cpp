#include "TerminalWindow.h"
#include "Scheduler.h"
#include "imgui.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

static void PushLines(std::vector<std::string>& h, const std::string& text)
{
    std::istringstream ss(text);
    std::string line;
    while (std::getline(ss, line)) h.push_back(line);
}

void RenderTerminalWindow(AppState& state)
{
    if (!state.showTerminal) return;

    static std::vector<std::string> history = {
        "CSOPESY OS Emulator terminal.",
        "Commands: screen -ls | exit",
        ""
    };
    static char input[256] = "";
    static bool scrollToBottom = false;

    ImGui::SetNextWindowSize(ImVec2(640.0f, 420.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(60.0f, 60.0f), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Terminal", &state.showTerminal))
    {
        const float footer = ImGui::GetFrameHeightWithSpacing();
        ImGui::BeginChild("TermOutput", ImVec2(0.0f, -footer), true, ImGuiWindowFlags_HorizontalScrollbar);
        for (const auto& line : history) ImGui::TextUnformatted(line.c_str());
        if (scrollToBottom) { ImGui::SetScrollHereY(1.0f); scrollToBottom = false; }
        ImGui::EndChild();

        ImGui::SetNextItemWidth(-1.0f);
        if (ImGui::InputText("##TermInput", input, sizeof(input), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            std::string cmd = input;
            input[0] = '\0';

            
            cmd.erase(0, cmd.find_first_not_of(" \t"));
            cmd.erase(cmd.find_last_not_of(" \t") + 1);

            history.push_back("> " + cmd);

            if (cmd == "screen -ls")
                PushLines(history, Sched::Scheduler::Instance().StatusReport());
            else if (cmd == "exit")
                state.isRunning = false;         
            else if (!cmd.empty())
                history.push_back("Unknown command: " + cmd);

            scrollToBottom = true;
            ImGui::SetKeyboardFocusHere(-1);     
        }
    }
    ImGui::End();
}
