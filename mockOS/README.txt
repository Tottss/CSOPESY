CSOPESY Semi-Major Output 2 - Desktop-Style OS Mock-up (Linux edition)
Group members: <your names here>

Entry file: main.cpp  (main function)

Dependencies: Dear ImGui (with imgui_impl_glfw + imgui_impl_opengl3 backends), GLFW 3, OpenGL

Build (Linux, g++), with imgui cloned to ./imgui:
  g++ main.cpp imgui/imgui*.cpp imgui/backends/imgui_impl_glfw.cpp \
      imgui/backends/imgui_impl_opengl3.cpp -Iimgui -Iimgui/backends \
      -lglfw -lGL -o csopesy_os
  ./csopesy_os

Build (Windows/Visual Studio): add the same ImGui .cpp files to the project, include GLFW,
link glfw3.lib + opengl32.lib.

Usage:
- A systemd-style boot log plays, then the desktop loads.
- Top panel: Activities (app launcher), real-time clock (top right), VOL / NET / PWR.
- PWR -> "Shut Down..." is the only way to close the app (shutdown sequence, then exit).
- Bottom dock: Files, Terminal, System Monitor (Task Manager), About/Settings.
- Terminal commands: help, neofetch, ls, pwd, whoami, date, uname, clear.

How to run (Using MSYS2 UCRT64)
Install msys2 from "https://www.msys2.org/"
1.Open MSYS2 UCRT64
2.Run "pacman -Syu"
3.RUn "pacman -S --needed git mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-glfw"
4.cd to folder where main.cpp exists
5.run "g++ main.cpp imgui/imgui*.cpp imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp -Iimgui -Iimgui/backends -static -lglfw3 -lopengl32 -lgdi32 -limm32 -o csopesy_os.exe"
6.run the generated .exe file