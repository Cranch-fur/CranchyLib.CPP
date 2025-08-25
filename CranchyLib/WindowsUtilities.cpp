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




bool WindowsUtilities::SetClipboard(const std::string& newText)
{
    if (OpenClipboard(nullptr)) 
    {
        EmptyClipboard();
        size_t newTextSize = newText.size() + 1;


        HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, newTextSize);
        if (hGlobal)
        {
            void* pMem = GlobalLock(hGlobal);
            if (pMem)
            {
                std::memcpy(pMem, newText.c_str(), newTextSize);
                GlobalUnlock(hGlobal);

                SetClipboardData(CF_TEXT, hGlobal);

                CloseClipboard();
                return true;
            }


            GlobalUnlock(hGlobal);
        }


        CloseClipboard();
    }


    return true;
}




WindowsUtilities::E_ClipboardFormat WindowsUtilities::GetClipboardFormat()
{
    if (OpenClipboard(nullptr))
    {
        UINT clipboardFormat = EnumClipboardFormats(0);
        CloseClipboard();


        switch (clipboardFormat)
        {
            case CF_TEXT:           return E_ClipboardFormat::TextAnsi;
            case CF_BITMAP:         return E_ClipboardFormat::Bitmap;
            case CF_METAFILEPICT:   return E_ClipboardFormat::MetafilePict;
            case CF_DIB:            return E_ClipboardFormat::Dib;
            case CF_PALETTE:        return E_ClipboardFormat::Palette;
            case CF_UNICODETEXT:    return E_ClipboardFormat::TextUnicode;
            case CF_ENHMETAFILE:    return E_ClipboardFormat::EnhancedMetafile;
            case CF_HDROP:          return E_ClipboardFormat::FileList;
            case CF_LOCALE:         return E_ClipboardFormat::Locale;
            case CF_DIBV5:          return E_ClipboardFormat::DibV5;
            default:
                if (clipboardFormat > CF_GDIOBJLAST)
                    return E_ClipboardFormat::Custom;
                else
                    return E_ClipboardFormat::None;
        }
    }


    return E_ClipboardFormat::None;
}




std::string WindowsUtilities::GetClipboardString()
{
    if (OpenClipboard(nullptr))
    {
        HANDLE hData = GetClipboardData(CF_TEXT);
        if (hData)
        {
            char* pData = static_cast<char*>(GlobalLock(hData));
            if (pData)
            {
                std::string text(pData);
                GlobalUnlock(hData);

                CloseClipboard();
                return text;
            }


            GlobalUnlock(hData);
        }


        CloseClipboard();
    }


    return std::string();
}


std::wstring WindowsUtilities::GetClipboardUnicodeString()
{
    if (OpenClipboard(nullptr))
    {
        HANDLE hData = GetClipboardData(CF_UNICODETEXT);
        if (hData)
        {
            wchar_t* pData = static_cast<wchar_t*>(GlobalLock(hData));
            if (pData)
            {
                std::wstring text(pData);
                GlobalUnlock(hData);

                CloseClipboard();
                return text;
            }


            GlobalUnlock(hData);
        }


        CloseClipboard();
    }


    return std::wstring();
}


HBITMAP WindowsUtilities::GetClipboardImage()
{
    if (OpenClipboard(nullptr))
    {
        HBITMAP hBitmap = static_cast<HBITMAP>(GetClipboardData(CF_BITMAP));

        CloseClipboard();
        return hBitmap;
    }


    return nullptr;
}




bool WindowsUtilities::SetClipboard(const std::wstring& newText)
{
    if (OpenClipboard(nullptr))
    {
        EmptyClipboard();

        size_t newTextSize = (newText.size() + 1) * sizeof(wchar_t);
        HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, newTextSize);
        if (hGlobal)
        {
            void* pMem = GlobalLock(hGlobal);
            if (pMem)
            {
                std::memcpy(pMem, newText.c_str(), newTextSize);
                GlobalUnlock(hGlobal);

                SetClipboardData(CF_UNICODETEXT, hGlobal);

                CloseClipboard();
                return true;
            }


            GlobalUnlock(hGlobal);
        }


        CloseClipboard();
    }


    return false;
}


bool WindowsUtilities::SetClipboard(HBITMAP hBitmap)
{
    if (OpenClipboard(nullptr))
    {
        EmptyClipboard();
        SetClipboardData(CF_BITMAP, hBitmap);
        CloseClipboard();


        return true;
    }


    return false;
}




bool WindowsUtilities::ClipboardContains(const std::string& substring)
{
    std::string text = GetClipboardString();
    return (text.find(substring) != std::string::npos);
}


bool WindowsUtilities::ClipboardContains(const std::wstring& substring)
{
    std::wstring text = GetClipboardUnicodeString();
    return (text.find(substring) != std::wstring::npos);
}




bool WindowsUtilities::ClipboardContainsRegex(const std::string& pattern)
{
    std::string text = GetClipboardString();
    try
    {
        std::regex re(pattern);
        return std::regex_search(text, re);
    }
    catch (std::regex_error&)
    {
        return false;
    }
}


bool WindowsUtilities::ClipboardContainsRegex(const std::wstring& pattern)
{
    std::wstring text = GetClipboardUnicodeString();
    try
    {
        std::wregex re(pattern);
        return std::regex_search(text, re);
    }
    catch (std::regex_error&)
    {
        return false;
    }
}




WindowsUtilities::E_MessageBoxResult WindowsUtilities::ShowMessageBox(HWND hwndOwner, const std::string& title, const std::string& message, UINT type)
{
    int winResult = MessageBoxA(hwndOwner, message.c_str(), title.c_str(), type);


    switch (winResult)
    {
        case IDOK:      return E_MessageBoxResult::Ok;
        case IDCANCEL:  return E_MessageBoxResult::Cancel;
        case IDABORT:   return E_MessageBoxResult::Abort;
        case IDRETRY:   return E_MessageBoxResult::Retry;
        case IDIGNORE:  return E_MessageBoxResult::Ignore;
        case IDYES:     return E_MessageBoxResult::Yes;
        case IDNO:      return E_MessageBoxResult::No;
        case 0:         return E_MessageBoxResult::Timeout;  // if MB_SERVICE_NOTIFICATION or timeout
        default:        return E_MessageBoxResult::Unknown;
    }

}


WindowsUtilities::E_MessageBoxResult WindowsUtilities::ShowMessageBox(const std::string& title, const std::string& message, UINT type)
{
    return ShowMessageBox(nullptr, title, message, type);
}


WindowsUtilities::E_MessageBoxResult WindowsUtilities::ShowMessageBox(const std::string& title, const std::string& message)
{
    return ShowMessageBox(nullptr, title, message, MB_OK);
}


WindowsUtilities::E_MessageBoxResult WindowsUtilities::ShowMessageBox(const std::string& message, UINT type)
{
    return ShowMessageBox(nullptr, GetExecutableName(false), message, type);
}


WindowsUtilities::E_MessageBoxResult WindowsUtilities::ShowMessageBox(const std::string& message)
{
    return ShowMessageBox(nullptr, GetExecutableName(false), message, MB_OK);
}




std::string WindowsUtilities::ShowFileOpenDialog(HWND hwndOwner, const std::string& initialDirectory, DWORD flags)
{
    char filePath[MAX_PATH] = { 0 };


    OPENFILENAMEA ofn = { 0 };
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = hwndOwner;
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = sizeof(filePath);
    ofn.lpstrFilter = NULL;
    ofn.Flags = flags;


    if (initialDirectory.empty() == false && DirectoryExist(initialDirectory))
    {
        ofn.lpstrInitialDir = initialDirectory.c_str();
    }
    else 
    {
        std::string executableDirectory = GetExecutableDirectory();
        if (executableDirectory.empty() == false && DirectoryExist(executableDirectory))
        {
            ofn.lpstrInitialDir = executableDirectory.c_str();
        }
    }


    if (GetOpenFileNameA(&ofn))
    {
        return std::string(filePath);
    }
}


std::string WindowsUtilities::ShowFileOpenDialog(HWND hwndOwner, const std::string& initialDirectory)
{
    return ShowFileOpenDialog(hwndOwner, initialDirectory, OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER);
}


std::string WindowsUtilities::ShowFileOpenDialog(HWND hwndOwner, DWORD flags)
{
    return ShowFileOpenDialog(hwndOwner, std::string(), flags);
}


std::string WindowsUtilities::ShowFileOpenDialog(const std::string& initialDirectory)
{
    return ShowFileOpenDialog(nullptr, initialDirectory, OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER);
}


std::string WindowsUtilities::ShowFileOpenDialog(DWORD flags)
{
    return ShowFileOpenDialog(nullptr, std::string(), flags);
}


std::string WindowsUtilities::ShowFileOpenDialog()
{
    return ShowFileOpenDialog(nullptr, std::string(), OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER);
}






bool WindowsUtilities::FileExist(const std::string& filePath)
{
    bool fileExist = false;


    std::filesystem::path p(filePath);
    if (std::filesystem::exists(p))
    {
        fileExist = std::filesystem::is_regular_file(p);
    }
    else 
    {
        std::filesystem::path pV2(GetExecutableDirectory() + "\\" + filePath);
        fileExist = (std::filesystem::exists(pV2) && std::filesystem::is_regular_file(pV2));
    }


    return fileExist;
}


bool WindowsUtilities::DirectoryExist(const std::string& directoryPath)
{
    std::filesystem::path p(directoryPath);
    return std::filesystem::exists(p) && std::filesystem::is_directory(p);
}




bool WindowsUtilities::StartProcess(const std::string& executablePath, const std::string& startupArguments)
{
    if (executablePath.empty()) 
    {
        return false;
    }


    std::string commandLine;
    const std::regex driveRegex("^[A-Za-z]:\\\\");
    

    if (std::regex_search(executablePath, driveRegex)) 
    {
        commandLine = "\"" + executablePath + "\"";
    }
    else 
    {
        commandLine = "\"" + GetExecutableDirectory() + "\\" + executablePath + "\"";
    }


    if (startupArguments.empty() == false) 
    {
        commandLine += " " + startupArguments;
    }


    // Prepare the structures for process creation
    STARTUPINFOA si = { 0 };
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi = { 0 };


    bool createProcessResult = CreateProcessA(NULL,                           // Module name (NULL when using commandLine)
                                              &commandLine[0],                // Command line (modifiable array of characters)
                                              NULL,                           // Process security attributes
                                              NULL,                           // Thread security attributes
                                              FALSE,                          // Inherit handles
                                              0,                              // Creation flags
                                              NULL,                           // Environment variables
                                              NULL,                           // Current directory
                                              &si,                            // STARTUPINFO structure
                                              &pi);


    // Close the process and thread handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);


    return createProcessResult;
}




HMODULE WindowsUtilities::GetMainModuleBase(DWORD processId)
{
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
    if (snap == INVALID_HANDLE_VALUE) 
        return nullptr;

    MODULEENTRY32W me{};
    me.dwSize = sizeof(me);
    HMODULE base = nullptr;

    if (Module32FirstW(snap, &me)) 
        base = me.hModule;

    CloseHandle(snap);
    return base;
}

LPVOID WindowsUtilities::GetRemoteEntryPoint(HANDLE hProcess, HMODULE imageBase)
{
    if (!hProcess || !imageBase) 
        return nullptr;

    IMAGE_DOS_HEADER dos{};
    SIZE_T br = 0;
    if (!ReadProcessMemory(hProcess, imageBase, &dos, sizeof(dos), &br) 
        || dos.e_magic != IMAGE_DOS_SIGNATURE)
        return nullptr;

    // Read NT headers (buffer sized for 64-bit header; fine for 32-bit too).
    BYTE ntBuf[sizeof(IMAGE_NT_HEADERS64)]{};
    auto ntAddr = reinterpret_cast<const BYTE*>(imageBase) + dos.e_lfanew;

    if (!ReadProcessMemory(hProcess, ntAddr, ntBuf, sizeof(ntBuf), &br))
        return nullptr;

    const IMAGE_NT_HEADERS64* nt64 = reinterpret_cast<const IMAGE_NT_HEADERS64*>(ntBuf);
    if (nt64->Signature != IMAGE_NT_SIGNATURE) return nullptr;

    // Check optional header type and take AddressOfEntryPoint.
    if (nt64->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) 
    {
        DWORD epRva = nt64->OptionalHeader.AddressOfEntryPoint;
        return const_cast<BYTE*>(reinterpret_cast<const BYTE*>(imageBase)) + epRva;
    }
    else 
    {
        const auto* nt32 = reinterpret_cast<const IMAGE_NT_HEADERS32*>(ntBuf);
        if (nt32->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC) return nullptr;
        DWORD epRva = nt32->OptionalHeader.AddressOfEntryPoint;
        return const_cast<BYTE*>(reinterpret_cast<const BYTE*>(imageBase)) + epRva;
    }
}

WindowsUtilities::ProcessInformation WindowsUtilities::GetProcessByName(const std::string& exeName, const DWORD& desiredAccess)
{
    return GetProcessByName(std::wstring(exeName.begin(), exeName.end()), desiredAccess);
}

WindowsUtilities::ProcessInformation WindowsUtilities::GetProcessByName(const std::wstring& exeName, const DWORD& desiredAccess)
{
    ProcessInformation result{};

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
        return result;

    PROCESSENTRY32W pe{};
    pe.dwSize = sizeof(pe);

    if (Process32FirstW(snapshot, &pe)) 
    {
        do 
        {
            if (_wcsicmp(pe.szExeFile, exeName.c_str()) == 0) 
            {
                HANDLE hProcess = OpenProcess(desiredAccess, FALSE, pe.th32ProcessID);
                if (!hProcess) 
                    break;

                HMODULE base = GetMainModuleBase(pe.th32ProcessID);
                LPVOID entry = GetRemoteEntryPoint(hProcess, base);

                result.handle = hProcess;
                result.processId = pe.th32ProcessID;
                result.imageBase = base;
                result.entryPoint = entry;
                break;
            }
        } while (Process32NextW(snapshot, &pe));
    }

    CloseHandle(snapshot);
    return result;
}

WindowsUtilities::ProcessInformation WindowsUtilities::GetProcessByPID(const DWORD& processId, const DWORD& desiredAccess)
{
    ProcessInformation result{};

    if (processId == 0)
        return result;

    HANDLE hProcess = OpenProcess(desiredAccess, FALSE, processId);
    if (!hProcess)
        return result;

    HMODULE base = GetMainModuleBase(processId);
    LPVOID entry = base ? GetRemoteEntryPoint(hProcess, base) : nullptr;

    result.handle = hProcess;
    result.processId = processId;
    result.imageBase = base;
    result.entryPoint = entry;

    return result;
}




bool WindowsUtilities::CloseProcess(HANDLE hProcess)
{
    if (hProcess == nullptr || hProcess == INVALID_HANDLE_VALUE)
        return false;


    if (!TerminateProcess(hProcess, 0))
        return false;


    DWORD wait = WaitForSingleObject(hProcess, INFINITE);
    return (wait == WAIT_OBJECT_0);
}


bool WindowsUtilities::CloseProcess(DWORD processId)
{
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION, FALSE, processId);
    if (!hProcess)
        return false;


    bool result = CloseProcess(hProcess);
    CloseHandle(hProcess);
    return result;
}


bool WindowsUtilities::CloseProcess(const std::wstring& exeName)
{
    bool anyClosed = false;


    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return false;


    PROCESSENTRY32W pe{};
    pe.dwSize = sizeof(pe);


    if (Process32FirstW(hSnapshot, &pe)) 
    {
        do {
            if (_wcsicmp(pe.szExeFile, exeName.c_str()) == 0) 
            {
                if (CloseProcess(pe.th32ProcessID))
                    anyClosed = true;
            }
        } while (Process32NextW(hSnapshot, &pe));
    }


    CloseHandle(hSnapshot);
    return anyClosed;
}