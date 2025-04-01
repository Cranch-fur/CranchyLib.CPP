#pragma once

#include <string>
#include <filesystem>
#include <iostream>
#include <stdio.h>
#include <shlobj.h>
#include <Windows.h>
#include <processthreadsapi.h>

#include "Psapi.h";
#include "StringUtilities.h";

using namespace std;






class WindowsUtilities
{
public:
    static string GetExecutablePath();
    static string GetExecutableName(bool includeExtension = true);
    static string GetExecutableDirectory();




    static string GetKnownDirectory(REFKNOWNFOLDERID folderId);
    static string GetSystemDirectory();
    static string GetProgramFilesDirectory();
    static string GetProgramFilesX86Directory();
    static string GetUserDirectory();
    static string GetDesktopDirectory();
    static string GetDownloadsDirectory();
    static string GetDocumentsDirectory();
    static string GetPicturesDirectory();
    static string GetVideosDirectory();
    static string GetMusicDirectory();
    static string GetAppDataLocalDirectory();
    static string GetAppDataLocalLowDirectory();
    static string GetAppDataRoamingDirectory();

    static string GetSystemDrive();




    static void CreateConsole(bool setTitle = true, bool redirectStreams = false);
    static void SetConsoleBufferSize(short newBufferSize = 9999);
    static void ClearConsole();

    static void ConsoleUTF8(); // Universal
    static void Console1252(); // Windows EN
    static void Console1251(); // Windows RU
    static void Console866();  // DOS
    static void Console437();  // IBM PC




    static string FileOpenDialog(HWND hwndOwner = nullptr, string filesFilter = "All Files\0*.*\0", bool startingPoint = true);






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