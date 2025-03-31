#pragma once

#include <string>
#include <filesystem>
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
    static string GetDesktopDirectory();
    static string GetDownloadsDirectory();
    static string GetDocumentsDirectory();
    static string GetPicturesDirectory();
    static string GetVideosDirectory();
    static string GetMusicDirectory();




public:
    static void CreateConsole(bool setTitle = true, bool redirectStreams = false, bool increaseBufferSize = false);
    static void ClearConsole();

    static void ConsoleUTF8(); // Universal
    static void Console1252(); // Windows EN
    static void Console1251(); // Windows RU
    static void Console866();  // DOS
    static void Console437();  // IBM PC




public:
    static string FileOpenDialog(HWND hwndOwner = nullptr, string filesFilter = "All Files\0*.*\0", bool startingPoint = true);
};

