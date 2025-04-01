#pragma once

#include <string>
#include <filesystem>
#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include <processthreadsapi.h>

#include "Psapi.h"
#include "StringUtilities.h"






class WindowsUtilities
{
public:
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




    static std::string FileOpenDialog(HWND hwndOwner = nullptr, std::string filesFilter = "All Files\0*.*\0", bool startingPoint = true);






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