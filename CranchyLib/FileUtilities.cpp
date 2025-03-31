#include "FileUtilities.h"






string FileUtilities::ReadFileContents(const string& filePath)
{
    HANDLE hFile = CreateFileA(
        filePath.c_str(),      // File path.
        GENERIC_READ,          // Desired access.
        FILE_SHARE_READ,       // Shared access.
        nullptr,               // Security attributes.
        OPEN_EXISTING,         // Creation disposition.
        FILE_ATTRIBUTE_NORMAL, // Flags and attributes.
        nullptr                // Template file.
    );


    if (hFile == INVALID_HANDLE_VALUE) // Check if handle isn't valid.
    {
        throw runtime_error("Failed to create file handle.");
    }


    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) // Get file size & write it in to variable.
    {
        CloseHandle(hFile);
        throw runtime_error("Failed to retrieve file size.");
    }


    vector<char> buffer(fileSize.QuadPart); // Allocate memory for file contents.
    DWORD bytesRead;
    if (!ReadFile(hFile, buffer.data(), static_cast<DWORD>(fileSize.QuadPart), &bytesRead, nullptr)) {
        CloseHandle(hFile);
        throw runtime_error("Failed to read file contents.");
    }

    // Close file handle and return its contents from the buffer.
    CloseHandle(hFile);
    string fileContents = string(buffer.begin(), buffer.end());


#ifdef _DEBUG_PRINT
    // This specific DebugPrint comes with an #ifdef statement for optimization purposes.
    // Usually we read large chunks of data, and it would have been dumb to move them throughout the memory when there's no need to.
    GeneralUtilities::DebugPrint("Successfully read \"%s\" file contents:\n%s", filePath.c_str(), fileContents.c_str());
#endif
    return fileContents;
}


void FileUtilities::WriteFileContents(const string& filePath, const string& content)
{
    HANDLE hFile = CreateFileA(
        filePath.c_str(),      // File path.
        GENERIC_WRITE,         // Desired access.
        0,                     // Shared access.
        nullptr,               // Security attributes.
        CREATE_ALWAYS,         // Creation disposition.
        FILE_ATTRIBUTE_NORMAL, // Flags and attributes.
        nullptr                // Template file.
    );


    if (hFile == INVALID_HANDLE_VALUE)
    {
        throw runtime_error("Failed to create file handle.");
    }


    DWORD bytesWritten;
    if (!WriteFile(hFile, content.c_str(), static_cast<DWORD>(content.size()), &bytesWritten, nullptr)) {
        CloseHandle(hFile);
        throw runtime_error("Failed to write file contents.");
    }


    CloseHandle(hFile);
}