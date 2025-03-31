#include "WindowsUtilities.h"






string WindowsUtilities::GetExecutablePath()
{
    char executablePath[MAX_PATH] = { 0 };
    if (GetModuleFileNameA(NULL, executablePath, MAX_PATH))
    {
        return string(executablePath);
    }


    return string();
}


string WindowsUtilities::GetExecutableName(bool includeExtension)
{
    string executablePath = GetExecutablePath();
    if (executablePath.empty())
    {
        return string();
    }


    filesystem::path fsPath(executablePath);
    return includeExtension ? fsPath.filename().string() : fsPath.stem().string();
}


string WindowsUtilities::GetExecutableDirectory()
{
    string executablePath = GetExecutablePath();
    if (executablePath.empty())
    {
        return string();
    }


    filesystem::path fsPath(executablePath);
    return fsPath.parent_path().string();
}




string WindowsUtilities::GetKnownDirectory(REFKNOWNFOLDERID folderId)
{
    PWSTR folderPath = nullptr;


    HRESULT hr = SHGetKnownFolderPath(folderId, 0, NULL, &folderPath);
    if (SUCCEEDED(hr))
    {
        wstring wFolderPath(folderPath);
        CoTaskMemFree(folderPath);


        return StringUtilities::WString_ToString(wFolderPath);
    }


    return string();
}


string WindowsUtilities::GetSystemDirectory()
{
    GetKnownDirectory(FOLDERID_Windows);
}

string WindowsUtilities::GetProgramFilesDirectory()
{
    return GetKnownDirectory(FOLDERID_ProgramFiles);
}

string WindowsUtilities::GetProgramFilesX86Directory()
{
    return GetKnownDirectory(FOLDERID_ProgramFilesX86);
}

string WindowsUtilities::GetUserDirectory()
{
    return GetKnownDirectory(FOLDERID_Profile);
}

string WindowsUtilities::GetDesktopDirectory()
{
    return GetKnownDirectory(FOLDERID_Desktop);
}

string WindowsUtilities::GetDownloadsDirectory()
{
    return GetKnownDirectory(FOLDERID_Downloads);
}

string WindowsUtilities::GetDocumentsDirectory()
{
    return GetKnownDirectory(FOLDERID_Documents);
}

string WindowsUtilities::GetPicturesDirectory()
{
    return GetKnownDirectory(FOLDERID_Pictures);
}

string WindowsUtilities::GetVideosDirectory()
{
    return GetKnownDirectory(FOLDERID_Videos);
}

string WindowsUtilities::GetMusicDirectory()
{
    return GetKnownDirectory(FOLDERID_Music);
}

string WindowsUtilities::GetAppDataLocalDirectory()
{
    return GetKnownDirectory(FOLDERID_LocalAppData);
}

string WindowsUtilities::GetAppDataLocalLowDirectory()
{
    return GetKnownDirectory(FOLDERID_LocalAppDataLow);
}

string WindowsUtilities::GetAppDataRoamingDirectory()
{
    return GetKnownDirectory(FOLDERID_RoamingAppData);
}


string WindowsUtilities::GetSystemDrive()
{
    string systemPath = GetSystemDirectory();
    if (systemPath.empty() == false && systemPath.size() >= 3)
    {
        return systemPath.substr(0, 3);
    }


    return string();
}






void WindowsUtilities::CreateConsole(bool setTitle, bool redirectStreams, bool increaseBufferSize)
{
    AllocConsole();


    if (setTitle)
    {
        string executableName = GetExecutableName(false);
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


    if (increaseBufferSize)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole)
        {
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            if (GetConsoleScreenBufferInfo(hConsole, &csbi))
            {
                COORD newSize;
                newSize.X = csbi.srWindow.Right - csbi.srWindow.Left + 1;
                newSize.Y = 9999;
                SetConsoleScreenBufferSize(hConsole, newSize);
            }
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




string WindowsUtilities::FileOpenDialog(HWND hwndOwner, string filesFilter, bool startingPoint)
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
        string executableDirectory = GetExecutableDirectory();
        if (executableDirectory.empty())
        {

        }
        WCHAR executablePath[MAX_PATH] = { 0 };
        if (GetModuleFileNameEx(GetCurrentProcess(), nullptr, executablePath, MAX_PATH))
        {
            filesystem::path fsPath(executablePath);


            string executableDirectory = fsPath.parent_path().string();
            ofn.lpstrInitialDir = executableDirectory.c_str();
        }
    }


    if (GetOpenFileNameA(&ofn))
    {
        return string(filePath);
    }
}
