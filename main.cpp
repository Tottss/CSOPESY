#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#endif

using namespace std;

bool marqueeRunning = false;
bool appRunning = true;

int refreshRateMs = 100; // how often the marquee moves, in ms
int pollingRateMs = 15;  // how often we check for a keypress, in ms

string marqueeText = "Welcome to CSOPESY!";
int offset = 0;    // current horizontal position of the text
int direction = 1; // +1 = moving right, -1 = moving left

#ifdef _WIN32
int getConsoleWidth()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    return width < 30 ? 80 : width;
}

COORD getCursorPosition()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.dwCursorPosition;
}

void setCursorPosition(int x, int y)
{
    COORD pos = {(SHORT)x, (SHORT)y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}
#else
int getConsoleWidth() { return 80; }
#endif

void clearScreen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void drawMarquee()
{
#ifdef _WIN32
    int width = getConsoleWidth();
    COORD savedPos = getCursorPosition();

    string line;
    int textLen = (int)marqueeText.length();

    if (textLen < width)
    {
        // Text fits on screen: it bounces left and right.
        line = string(offset, ' ') + marqueeText;
    }
    else
    {
        // Text is wider than the screen: scroll it continuously.
        string banner = marqueeText + "   ";
        int start = offset % (int)banner.length();
        string doubled = banner + banner;
        line = doubled.substr(start, width);
    }

    // Pad or trim so we always overwrite the full line (no leftover characters).
    if ((int)line.length() < width)
        line += string(width - line.length(), ' ');
    else
        line = line.substr(0, width);

    setCursorPosition(0, 0);
    cout << line;
    setCursorPosition(savedPos.X, savedPos.Y);
    cout.flush();
#endif
}

// Advances the marquee by one step, ready for the next frame.
void updateMarqueePosition()
{
    int width = getConsoleWidth();
    int textLen = (int)marqueeText.length();

    if (textLen < width)
    {
        offset += direction;
        if (offset + textLen >= width)
        {
            offset = width - textLen;
            direction = -1;
        }
        else if (offset <= 0)
        {
            offset = 0;
            direction = 1;
        }
    }
    else
    {
        offset++;
    }
}

void printHeader()
{
    cout << "\n\n"; // leaves row 0 free for the marquee, row 1 as a spacer
    cout << "Group developer:\n";
    cout << "Abdulrahman, Saoud\n";
    cout << "Fabricante, Jeruel\n";
    cout << "Galvez, Anousheh Monick\n\n";
    drawMarquee();
}

void printHelp()
{
    cout << "Commands:\n"
         << "  help                 - Displays commands and descriptions\n"
         << "  clear                - Clears the console screen\n"
         << "  start_marquee        - Starts the marquee animation\n"
         << "  stop_marquee         - Stops the marquee animation\n"
         << "  set_text <text>      - Sets the marquee display text\n"
         << "  set_speed <ms>       - Sets the marquee refresh rate (milliseconds)\n"
         << "  set_polling <ms>     - Sets keyboard polling rate (milliseconds)\n"
         << "  exit                 - Terminates the console\n\n";
}

// Parses one line typed by the user and runs the matching command.
void handleCommand(const string &line)
{
    stringstream ss(line);
    string command;
    ss >> command;

    if (command.empty())
        return;

    if (command == "exit")
    {
        appRunning = false;
    }
    else if (command == "clear")
    {
        clearScreen();
        printHeader();
    }
    else if (command == "help")
    {
        printHelp();
    }
    else if (command == "start_marquee")
    {
        marqueeRunning = true;
        cout << "Starting marquee animation...\n";
    }
    else if (command == "stop_marquee")
    {
        marqueeRunning = false;
        cout << "Stopping marquee animation...\n";
    }
    else if (command == "set_text")
    {
        string newText;
        getline(ss >> ws, newText);

        if (newText.empty())
        {
            cout << "Usage: set_text <text>\n";
        }
        else
        {
            marqueeText = newText;
            offset = 0;
            direction = 1;
            drawMarquee();
            cout << "Marquee text set to: \"" << newText << "\"\n";
        }
    }
    else if (command == "set_speed")
    {
        int ms = 0;
        if (ss >> ms && ms > 0)
        {
            refreshRateMs = ms;
            cout << "Marquee refresh rate set to " << ms << " ms.\n";
        }
        else
        {
            cout << "Usage: set_speed <positive integer in ms>\n";
        }
    }
    else if (command == "set_polling")
    {
        int ms = 0;
        if (ss >> ms && ms > 0)
        {
            pollingRateMs = ms;
            cout << "Keyboard polling rate set to " << ms << " ms.\n";
        }
        else
        {
            cout << "Usage: set_polling <positive integer in ms>\n";
        }
    }
    else
    {
        cout << command << " is not a recognized command.\n";
    }
}

int main()
{
    clearScreen();
    printHeader();

    string inputBuffer;
    cout << "Command> " << flush;

    auto lastFrameTime = chrono::steady_clock::now();

    while (appRunning)
    {
#ifdef _WIN32
        if (_kbhit())
        {
            int ch = _getch();

            if (ch == '\r')
            {
                cout << "\n";
                handleCommand(inputBuffer);
                inputBuffer.clear();
                if (appRunning)
                    cout << "Command> " << flush;
            }
            else if (ch == '\b')
            {
                if (!inputBuffer.empty())
                {
                    inputBuffer.pop_back();
                    cout << "\b \b" << flush;
                }
            }
            else if (ch >= 32 && ch <= 126)
            {
                inputBuffer.push_back((char)ch);
                cout << (char)ch << flush;
            }
        }
#endif

        auto now = chrono::steady_clock::now();
        long long elapsed = chrono::duration_cast<chrono::milliseconds>(now - lastFrameTime).count();

        if (marqueeRunning && elapsed >= refreshRateMs)
        {
            updateMarqueePosition();
            drawMarquee();
            lastFrameTime = now;
        }

        this_thread::sleep_for(chrono::milliseconds(pollingRateMs));
    }

    return 0;
}
