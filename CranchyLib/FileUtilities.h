#pragma once

#include <string>
#include <stdexcept>
#include <vector>
#include <windows.h>

using namespace std;






class FileUtilities
{
public:
	static string ReadFileContents(const string& filePath);
	static vector<string> ReadFileLines(const string& filePath);

	static wstring ReadFileWContents(const wstring& filePath);
	static vector<wstring> ReadFileWLines(const wstring& filePath);


	static void WriteFileContents(const string& filePath, const string& content);

	static void WriteFileWContents(const wstring& filePath, const wstring& content);






    template<typename Container>
    static void WriteFileLines(const string& filePath, const Container& lines)
    {
        string content;
        bool first = true;


        for (const auto& line : lines)
        {
            if (!first)
                content.append("\n");
            content.append(line);
            first = false;
        }


        WriteFileContents(filePath, content);
    }


    template<typename Container>
    static void WriteFileAllWLines(const wstring& filePath, const Container& lines)
    {
        wstring content;
        bool first = true;


        for (const auto& line : lines)
        {
            if (!first)
                content.append(L"\n");
            content.append(line);
            first = false;
        }


        WriteFileWContents(filePath, content);
    }
};

