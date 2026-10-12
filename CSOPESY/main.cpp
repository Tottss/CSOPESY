#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include "AppState.h"
#include "Config.h"
#include "Desktop.h"
#include "FileExplorerWindow.h"
#include "SystemInfoWindow.h"
#include "TaskManager.h"
#include "Taskbar.h"

#include <cstdio>
#include <filesystem>

#ifdef _WIN32
#include <crtdbg.h>
#include <cstdlib>
static void IgnoreInvalidParam(const wchar_t*, const wchar_t*, const wchar_t*, unsigned int, uintptr_t) {}
#endif

static void glfw_error_callback(int error, const char* description)
{
    std::fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Block OS window 'X' button and Alt+F4 so shutdown only happens via the in-OS PWR button
static void glfw_window_close_callback(GLFWwindow* window)
{
    glfwSetWindowShouldClose(window, GLFW_FALSE);
}

int main(int, char**)
{
#ifdef _WIN32
    _set_invalid_parameter_handler(IgnoreInvalidParam);
    _CrtSetReportMode(_CRT_ASSERT, 0);
#endif

    Config::LoadFromFile("config.txt");

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(
        Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT, Config::WINDOW_TITLE.c_str(), nullptr, nullptr
    );
    if (!window)
    {
        glfwTerminate();
        return 1;
    }

    glfwSetWindowCloseCallback(window, glfw_window_close_callback);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;

    std::error_code ec;
    const char* fontPath = "C:/Windows/Fonts/segoeui.ttf";
    if (std::filesystem::exists(fontPath, ec) && !ec)
        io.Fonts->AddFontFromFileTTF(fontPath, 18.0f);

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 6.0f;
    style.FrameRounding  = 4.0f;
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.0f);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    AppState appState;

    // Main Compositor Loop
    while (appState.isRunning)
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Base Layer Desktop (Wallpaper + Foreground Real-Time Clock)
        RenderDesktop(io);

        // Application Windows (2 Unique UI Screens + Task Manager)
        RenderSystemInfoWindow(appState);
        RenderFileExplorerWindow(appState);
        RenderTaskManagerWindow(appState);

        // Fixed Taskbar Panel
        RenderTaskbar(io, appState);

        ImGui::Render();
        int displayW = 0, displayH = 0;
        glfwGetFramebufferSize(window, &displayW, &displayH);
        glViewport(0, 0, displayW, displayH);
        glClearColor(0.05f, 0.07f, 0.10f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
