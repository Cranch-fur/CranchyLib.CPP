#include "WindowsUtilities.h"






std::string WindowsUtilities::GetExecutablePath()
{
    char executablePath[MAX_PATH] = { 0 };
    if (GetModuleFileNameA(NULL, executablePath, MAX_PATH))
    {
        return std::string(executablePath);
    }


    return std::string();
}


std::string WindowsUtilities::GetExecutableName(bool includeExtension)
{
    std::string executablePath = GetExecutablePath();
    if (executablePath.empty())
    {
        return std::string();
    }


    std::filesystem::path fsPath(executablePath);
    return includeExtension ? fsPath.filename().string() : fsPath.stem().string();
}


std::string WindowsUtilities::GetExecutableDirectory()
{
    std::string executablePath = GetExecutablePath();
    if (executablePath.empty())
    {
        return std::string();
    }


    std::filesystem::path fsPath(executablePath);
    return fsPath.parent_path().string();
}




std::string WindowsUtilities::GetEnvironmentValue(const std::string& varName)
{
    char* directoryPath = nullptr;
    size_t len = 0;


    if (_dupenv_s(&directoryPath, &len, varName.c_str()) == 0 && directoryPath != nullptr)
    {
        std::string outPath(directoryPath);
        free(directoryPath);


        return outPath;
    }


    return std::string();
}




std::string WindowsUtilities::GetSystemDirectory()
{
    char buffer[MAX_PATH];
    UINT len = GetWindowsDirectoryA(buffer, MAX_PATH);
    if (len > 0)
    {
        return std::string(buffer);
    }


    return std::string();
}


std::string WindowsUtilities::GetProgramFilesDirectory()
{
    std::string outPath = GetEnvironmentValue("ProgramFiles");
    if (!outPath.empty())
    {
        return outPath;
    }


    return std::string();
}

std::string WindowsUtilities::GetProgramFilesX86Directory()
{
    std::string outPath = GetEnvironmentValue("ProgramFiles(x86)");
    if (!outPath.empty())
    {
        return outPath;
    }


    return std::string();
}

std::string WindowsUtilities::GetUserDirectory()
{
    std::string outPath = GetEnvironmentValue("USERPROFILE");
    if (!outPath.empty())
    {
        return outPath;
    }


    return std::string();
}

std::string WindowsUtilities::GetDesktopDirectory()
{
    std::string outPath = GetEnvironmentValue("USERPROFILE");
    if (!outPath.empty())
    {
        outPath += "\\Desktop";
        return outPath;
    }


    return std::string();
}

std::string WindowsUtilities::GetDownloadsDirectory()
{
    std::string outPath = GetEnvironmentValue("USERPROFILE");
    if (!outPath.empty())
    {
        outPath += "\\Downloads";
        return outPath;
    }


    return std::string();
}

std::string WindowsUtilities::GetDocumentsDirectory()
{
    std::string outPath = GetEnvironmentValue("USERPROFILE");
    if (!outPath.empty())
    {
        outPath += "\\Documents";
        return outPath;
    }


    return std::string();
}

std::string WindowsUtilities::GetPicturesDirectory()
{
    std::string outPath = GetEnvironmentValue("USERPROFILE");
    if (!outPath.empty())
    {
        outPath += "\\Pictures";
        return outPath;
    }


    return std::string();
}

std::string WindowsUtilities::GetVideosDirectory()
{
    std::string outPath = GetEnvironmentValue("USERPROFILE");
    if (!outPath.empty())
    {
        outPath += "\\Videos";
        return outPath;
    }


    return std::string();
}

std::string WindowsUtilities::GetMusicDirectory()
{
    std::string outPath = GetEnvironmentValue("USERPROFILE");
    if (!outPath.empty())
    {
        outPath += "\\Music";
        return outPath;
    }


    return std::string();
}

std::string WindowsUtilities::GetAppDataLocalDirectory()
{
    std::string outPath = GetEnvironmentValue("LOCALAPPDATA");
    if (!outPath.empty())
    {
        return outPath;
    }


    return std::string();
}

std::string WindowsUtilities::GetAppDataLocalLowDirectory()
{
    std::string outPath = GetEnvironmentValue("USERPROFILE");
    if (!outPath.empty())
    {
        outPath += "\\AppData\\LocalLow";
        return outPath;
    }


    return std::string();
}

std::string WindowsUtilities::GetAppDataRoamingDirectory()
{
    std::string outPath = GetEnvironmentValue("APPDATA");
    if (!outPath.empty())
    {
        return outPath;
    }


    return std::string();
}


std::string WindowsUtilities::GetSystemDrive()
{
    std::string systemPath = GetSystemDirectory();
    if (systemPath.empty() == false && systemPath.size() >= 3)
    {
        return systemPath.substr(0, 3);
    }


    return std::string();
}






void WindowsUtilities::CreateConsole(bool setTitle, bool redirectStreams)
{
    AllocConsole();


    if (setTitle)
    {
        std::string executableName = GetExecutableName(false);
        if (executableName.empty())
        {
            SetConsoleTitleA("DEBUG CONSOLE");
        }
        else
        {
            SetConsoleTitleA(executableName.c_str());
        }
    }


    if (redirectStreams)
    {
        FILE* fConsole;
        freopen_s(&fConsole, "CONIN$", "r", stdin);
        freopen_s(&fConsole, "CONOUT$", "w", stdout);
        freopen_s(&fConsole, "CONOUT$", "w", stderr);
    }
}


void WindowsUtilities::SetConsoleBufferSize(short newBufferSize)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole)
    {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(hConsole, &csbi))
        {
            short minHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
            short maxHeight = 32767;


            newBufferSize = std::clamp(newBufferSize, minHeight, maxHeight);


            COORD newSize;
            newSize.X = csbi.srWindow.Right - csbi.srWindow.Left + 1;
            newSize.Y = newBufferSize;
            SetConsoleScreenBufferSize(hConsole, newSize);
        }
    }
}


void WindowsUtilities::ClearConsole()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole)
    {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(hConsole, &csbi))
        {
            DWORD count;
            DWORD cellCount = csbi.dwSize.X * csbi.dwSize.Y;
            COORD homeCoords = { 0, 0 };


            if (!FillConsoleOutputCharacter(
                hConsole,        // Handle to console screen buffer
                (TCHAR)' ',      // Character to write to the buffer
                cellCount,       // Number of cells to write
                homeCoords,      // Coordinates of first cell
                &count           // Receive number of characters written
            )) return;

            if (!FillConsoleOutputAttribute(
                hConsole,        // Handle to console screen buffer
                csbi.wAttributes,// Character attributes to use
                cellCount,       // Number of cells to set attribute
                homeCoords,      // Coordinates of first cell
                &count           // Receive number of attributes written
            )) return;


            SetConsoleCursorPosition(hConsole, homeCoords);
        }
    }
}




void WindowsUtilities::ConsoleUTF8()
{
    SetConsoleOutputCP(CP_UTF8); // CP_UTF8 = 65001
    SetConsoleCP(CP_UTF8);
}

void WindowsUtilities::Console1252()
{
    SetConsoleOutputCP(1252);
    SetConsoleCP(1252);
}

void WindowsUtilities::Console1251()
{
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
}

void WindowsUtilities::Console866()
{
    SetConsoleOutputCP(866);
    SetConsoleCP(866);
}

void WindowsUtilities::Console437()
{
    SetConsoleOutputCP(437);
    SetConsoleCP(437);
}




std::string WindowsUtilities::FileOpenDialog(HWND hwndOwner, std::string filesFilter, bool startingPoint)
{
    char filePath[MAX_PATH] = { 0 };


    OPENFILENAMEA ofn = { 0 };
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = hwndOwner; // OPTIONAL: if main window is present, you can pass its handle here
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = sizeof(filePath);
    ofn.lpstrFilter = filesFilter.c_str();
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;


    if (startingPoint == true)
    {
        std::string executableDirectory = GetExecutableDirectory();
        if (executableDirectory.empty())
        {

        }
        WCHAR executablePath[MAX_PATH] = { 0 };
        if (GetModuleFileNameEx(GetCurrentProcess(), nullptr, executablePath, MAX_PATH))
        {
            std::filesystem::path fsPath(executablePath);


            std::string executableDirectory = fsPath.parent_path().string();
            ofn.lpstrInitialDir = executableDirectory.c_str();
        }
    }


    if (GetOpenFileNameA(&ofn))
    {
        return std::string(filePath);
    }
}
