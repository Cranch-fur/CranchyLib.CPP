#pragma once

#include <string>
#include <regex>
#include <filesystem>
#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include <tlhelp32.h>
#include <processthreadsapi.h>

#include "Psapi.h"
#include "StringUtilities.h"






class WindowsUtilities
{
public:
    /**
    * @brief Basic information about a target process.
    * @param handle - Process handle.
    * @param processId - Process ID.
    * @param imageBase - Base address of the main module GetModuleHandle(nullptr).
    * @param entryPoint - Entry point address [VA] (imageBase + AddressOfEntryPoint [RVA]).
    */
    struct ProcessInformation 
    {
        HANDLE  handle     = nullptr;
        DWORD   processId  = 0;
        HMODULE imageBase  = nullptr;
        LPVOID  entryPoint = nullptr;
    };






    enum class E_ClipboardFormat
    {
        None,
        TextAnsi,          /// CF_TEXT.
        TextUnicode,       /// CF_UNICODETEXT.
        Bitmap,            /// CF_BITMAP.
        Dib,               /// CF_DIB.
        DibV5,             /// CF_DIBV5.
        MetafilePict,      /// CF_METAFILEPICT.
        EnhancedMetafile,  /// CF_ENHMETAFILE.
        Palette,           /// CF_PALETTE.
        FileList,          /// CF_HDROP.
        Locale,            /// CF_LOCALE.
        Custom             /// Any other format (ID > CF_GDIOBJLAST).
    };


    enum class E_MessageBoxResult
    {
        Ok,
        Cancel,
        Abort,
        Retry,
        Ignore,
        Yes,
        No,
        Timeout,
        Unknown
    };






    static std::string GetExecutablePath();
    static std::string GetExecutableName(bool includeExtension = true);
    static std::string GetExecutableDirectory();



    static std::string GetEnvironmentValue(const std::string& varName);



    static std::string GetSystemDirectory();

    static std::string GetProgramFilesDirectory();
    static std::string GetProgramFilesX86Directory();
    static std::string GetUserDirectory();
    static std::string GetDesktopDirectory();
    static std::string GetDownloadsDirectory();
    static std::string GetDocumentsDirectory();
    static std::string GetPicturesDirectory();
    static std::string GetVideosDirectory();
    static std::string GetMusicDirectory();
    static std::string GetAppDataLocalDirectory();
    static std::string GetAppDataLocalLowDirectory();
    static std::string GetAppDataRoamingDirectory();

    static std::string GetSystemDrive();




    static void CreateConsole(bool setTitle = true, bool redirectStreams = false);
    static void SetConsoleBufferSize(short newBufferSize = 9999);
    static void ClearConsole();

    static void ConsoleUTF8(); // Universal
    static void Console1252(); // Windows EN
    static void Console1251(); // Windows RU
    static void Console866();  // DOS
    static void Console437();  // IBM PC




    static E_ClipboardFormat GetClipboardFormat();


    static std::string GetClipboardString();
    static std::wstring GetClipboardUnicodeString();
    static HBITMAP GetClipboardImage();


    static bool SetClipboard(const std::string& text);
    static bool SetClipboard(const std::wstring& text);
    static bool SetClipboard(HBITMAP hBitmap);


    static bool ClipboardContains(const std::string& substring);
    static bool ClipboardContains(const std::wstring& substring);


    static bool ClipboardContainsRegex(const std::string& pattern);
    static bool ClipboardContainsRegex(const std::wstring& pattern);




    static E_MessageBoxResult ShowMessageBox(HWND hwndOwner, const std::string& title, const std::string& message, UINT type);
    static E_MessageBoxResult ShowMessageBox(const std::string& title, const std::string& message, UINT type);
    static E_MessageBoxResult ShowMessageBox(const std::string& title, const std::string& message);
    static E_MessageBoxResult ShowMessageBox(const std::string& message, UINT type);
    static E_MessageBoxResult ShowMessageBox(const std::string& message);




    static std::string ShowFileOpenDialog(HWND hwndOwner, const std::string& initialDirectory, DWORD flags);
    static std::string ShowFileOpenDialog(HWND hwndOwner, const std::string& initialDirectory);
    static std::string ShowFileOpenDialog(HWND hwndOwner, DWORD flags);
    static std::string ShowFileOpenDialog(const std::string& initialDirectory);
    static std::string ShowFileOpenDialog(DWORD flags);
    static std::string ShowFileOpenDialog();




    static bool FileExist(const std::string& filePath);
    static bool DirectoryExist(const std::string& directoryPath);




    static bool StartProcess(const std::string& executablePath, const std::string& startupArguments = "");


    static HMODULE GetMainModuleBase(DWORD processId);
    static LPVOID GetRemoteEntryPoint(HANDLE hProcess, HMODULE imageBase);
    static ProcessInformation GetProcessByName(const std::string& exeName, const DWORD& desiredAccess = PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ);
    static ProcessInformation GetProcessByName(const std::wstring& exeName, const DWORD& desiredAccess = PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ);
    static ProcessInformation GetProcessByPID(const DWORD& processId, const DWORD& desiredAccess);


    static bool CloseProcess(HANDLE hProcess);
    static bool CloseProcess(DWORD processId);
    static bool CloseProcess(const std::wstring& exeName);






    template<typename T>
    static void WriteLineConsole(const T& value)
    {
        std::cout << value << std::endl;
    }

    template<typename Container>
    static void WriteAllLinesConsole(const Container& lines)
    {
        for (const auto& line : lines)
        {
            std::cout << line << std::endl;
        }
    }


    template<typename T>
    static void WriteWLineConsole(const T& value)
    {
        std::wcout << value << std::endl;
    }

    template<typename Container>
    static void WriteAllWLinesConsole(const Container& lines)
    {
        for (const auto& line : lines)
        {
            std::wcout << line << std::endl;
        }
    }
};