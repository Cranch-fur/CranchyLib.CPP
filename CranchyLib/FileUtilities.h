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
	static void WriteFileContents(const string& filePath, const string& content);
};

