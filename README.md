# CSOPESY Semi-Major Output 2 — Desktop-Style OS Mock-up

## Group Developers
- **Abdulrahman, Saoud**
- **Fabricante, Jeruel**
- **Galvez, Anousheh Monick**

## Entry File & Modular Architecture
- [`CSOPESY/main.cpp`](./CSOPESY/main.cpp) — Entry point (`int main(int, char**)`), GLFW + OpenGL 3 + Dear ImGui initialization, 18px Segoe UI font loading, and main compositor loop.
- [`CSOPESY/Config.h`](./CSOPESY/Config.h) / [`CSOPESY/Config.cpp`](./CSOPESY/Config.cpp) — Runtime parameter loader and validator for [`CSOPESY/config.txt`](./CSOPESY/config.txt) with `std::isfinite` checks and safe built-in defaults so a missing or malformed file never crashes the app.
- [`CSOPESY/AppState.h`](./CSOPESY/AppState.h) — Shared state across the desktop compositor and application windows.
- [`CSOPESY/Desktop.h`](./CSOPESY/Desktop.h) / [`CSOPESY/Desktop.cpp`](./CSOPESY/Desktop.cpp) — Base layer wallpaper (multi-color gradient + geometric grid) and top-right real-time clock rendered via `ImGui::GetForegroundDrawList()` so windows never obscure it.
- [`CSOPESY/Taskbar.h`](./CSOPESY/Taskbar.h) / [`CSOPESY/Taskbar.cpp`](./CSOPESY/Taskbar.cpp) — Fixed taskbar panel with 3 clickable icon buttons, responsive layout for narrow window widths, right-aligned `PWR` shutdown button, and `ImGui::BringWindowToDisplayFront` so windows never cover the taskbar.
- [`CSOPESY/TaskManager.h`](./CSOPESY/TaskManager.h) / [`CSOPESY/TaskManager.cpp`](./CSOPESY/TaskManager.cpp) — Windows-style Task Manager with CPU/Memory summary bars, case-insensitive `"idle"` process detection, Processes table, and optional time-based fluctuation (`task_manager_wobble`).
- [`CSOPESY/SystemInfoWindow.h`](./CSOPESY/SystemInfoWindow.h) / [`CSOPESY/SystemInfoWindow.cpp`](./CSOPESY/SystemInfoWindow.cpp) — Unique UI Screen #1 (system specifications, live uptime timer, developer credits).
- [`CSOPESY/FileExplorerWindow.h`](./CSOPESY/FileExplorerWindow.h) / [`CSOPESY/FileExplorerWindow.cpp`](./CSOPESY/FileExplorerWindow.cpp) — Unique UI Screen #2 (virtual file browser and content preview pane).

## Modifying Parameters Without Recompiling (`config.txt`)
At startup, the executable loads [`CSOPESY/config.txt`](./CSOPESY/config.txt) (checking the executable directory, working directory, and project directories, automatically picking whichever copy was modified more recently) and logs the active file path to `stderr`:
```text
[Config] Loaded 'D:\source\repos\CSOPESY\CSOPESY\config.txt'
```
Any malformed or skipped lines are also logged with their line number. Supported keys:
- `window_title`, `window_width`, `window_height`
- `taskbar_height`, `taskbar_at_bottom` (`true`/`false` or `bottom`/`top`)
- `wallpaper_top_left`, `wallpaper_top_right`, `wallpaper_bottom_right`, `wallpaper_bottom_left` (`R, G, B, A`)
- `clock_format` (`std::strftime` format string — invalid specifiers like `%Q` or `%-d` are safely caught without crashing)
- `total_system_memory_mb`, `task_manager_wobble` (`true`/`false`)
- `process = <Name> | <PID> | <Status> | <CPU%> | <MemoryMB>`

## How to Run in Visual Studio
1. Open **`CSOPESY.slnx`** in Visual Studio (**File → Open → Project/Solution...**).
   > **Note**: The vendored `third_party/glfw/lib-vc2010-*` static libraries are built for Visual Studio (MSVC). Building with MinGW or on Linux via `CMakeLists.txt` requires a system/MSYS2 `glfw3` package.
2. Ensure the configuration dropdown is set to **`Debug`** (or **`Release`**) and **`x64`**.
3. Press **`F5`** (*Local Windows Debugger*) to launch the Desktop OS Emulator.
   - Use the red **`PWR`** button on the taskbar to shut down cleanly (the window close callback blocks the titlebar `X` button and `Alt+F4` per spec; use **`Shift+F5`** in Visual Studio if you ever need to force-stop debugging).

## How to test
1. Launch the emulator. The 10 processes start immediately.
2. Type "screen -ls" every 1 to 2 seconds. With 4 cores and 100 ms per command, it takes about 25 seconds for everything to finish.
3. Once all 10 are under “Finished processes”, type exit.
4. Zip the logs/ folder and submit it.