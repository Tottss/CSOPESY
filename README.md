# CSOPESY Semi-Major Output 1 - Marquee Operator

## Instructions
1. Open Command Prompt or PowerShell in this directory.
2. Compile:
   ```bash
   g++ main.cpp -o csopesy.exe
   ```
3. Run:
   ```bash
   .\main.exe
   ```

## Supported Commands
- `help`: Displays commands and descriptions.
- `clear`: Clears the console screen and redraws the header.
- `start_marquee`: Starts the marquee animation.
- `stop_marquee`: Stops the marquee animation.
- `set_text <str>`: Sets the marquee display text (e.g., `set_text Welcome to CSOPESY!`).
- `set_speed <ms>`: Sets the marquee animation refresh in milliseconds (e.g., `set_speed 100`).
- `set_polling <ms>`: Sets keyboard polling interval in milliseconds (e.g., `set_polling 15`).
- `exit`: Terminates the console.