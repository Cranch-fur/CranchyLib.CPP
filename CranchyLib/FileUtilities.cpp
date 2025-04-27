#include "FileUtilities.h"






std::string FileUtilities::ReadFileContents(const std::string& filePath)
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
        return std::string();
    }


    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) // Get file size & write it in to variable.
    {
        CloseHandle(hFile);
        return std::string();
    }


    std::vector<char> buffer(fileSize.QuadPart); // Allocate memory for file contents.
    DWORD bytesRead;
    if (!ReadFile(hFile, buffer.data(), static_cast<DWORD>(fileSize.QuadPart), &bytesRead, nullptr)) {
        CloseHandle(hFile);
        return std::string();
    }

    // Close file handle and return its contents from the buffer.
    CloseHandle(hFile);
    std::string fileContents = std::string(buffer.begin(), buffer.end());


    return fileContents;
}

std::vector<std::string> FileUtilities::ReadFileLines(const std::string& filePath)
{
    std::string fileContents = ReadFileContents(filePath);
    std::vector<std::string> lines;
    size_t start = 0;
    size_t pos = 0;


    while ((pos = fileContents.find('\n', start)) != std::string::npos)
    {
        std::string line = fileContents.substr(start, pos - start);


        // If line ends with an '\r', remove it.
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }


        lines.push_back(line);
        start = pos + 1;
    }


    // Add last line if it isn't empty.
    if (start < fileContents.size())
    {
        std::string line = fileContents.substr(start);


        if (!line.empty() && line.back() == '\r') 
        {
            line.pop_back();
        }


        lines.push_back(line);
    }


    return lines;
}


std::wstring FileUtilities::ReadFileWContents(const std::wstring& filePath)
{
    HANDLE hFile = CreateFileW(
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
        return std::wstring();
    }


    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) // Get file size & write it in to variable.
    {
        CloseHandle(hFile);
        return std::wstring();
    }

    
    if (fileSize.QuadPart % sizeof(wchar_t) != 0) // File size must be multiple of sizeof(wchar_t).
    {
        CloseHandle(hFile);
        return std::wstring();
    }


    size_t wCharCount = fileSize.QuadPart / sizeof(wchar_t);


    std::vector<wchar_t> buffer(wCharCount); // Allocate memory for file contents.
    DWORD bytesRead;
    if (!ReadFile(hFile, buffer.data(), static_cast<DWORD>(fileSize.QuadPart), &bytesRead, nullptr))
    {
        CloseHandle(hFile);
        return std::wstring();
    }


    // Close file handle and return its contents from the buffer.
    CloseHandle(hFile);
    return std::wstring(buffer.begin(), buffer.end());
}

std::vector<std::wstring> FileUtilities::ReadFileWLines(const std::wstring& filePath)
{
    std::wstring fileContents = ReadFileWContents(filePath);
    std::vector<std::wstring> lines;
    size_t start = 0;
    size_t pos = 0;


    while ((pos = fileContents.find(L'\n', start)) != std::wstring::npos)
    {
        std::wstring line = fileContents.substr(start, pos - start);


        if (!line.empty() && line.back() == L'\r')
        {
            line.pop_back();
        }


        lines.push_back(line);
        start = pos + 1;
    }


    if (start < fileContents.size())
    {
        std::wstring line = fileContents.substr(start);


        if (!line.empty() && line.back() == L'\r')
        {
            line.pop_back();
        }


        lines.push_back(line);
    }


    return lines;
}




bool FileUtilities::WriteFileContents(const std::string& filePath, const std::string& content)
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
        return false;
    }


    DWORD bytesWritten;
    if (!WriteFile(hFile, content.c_str(), static_cast<DWORD>(content.size()), &bytesWritten, nullptr)) 
    {
        CloseHandle(hFile);
        return false;
    }


    CloseHandle(hFile);
    return true;
}


bool FileUtilities::WriteFileWContents(const std::wstring& filePath, const std::wstring& content)
{
    HANDLE hFile = CreateFileW(
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
        return false;
    }


    DWORD bytesWritten;
    if (!WriteFile(hFile, content.c_str(), static_cast<DWORD>(content.size() * sizeof(wchar_t)), &bytesWritten, nullptr))
    {
        CloseHandle(hFile);
        return true;
    }


    CloseHandle(hFile);
}
