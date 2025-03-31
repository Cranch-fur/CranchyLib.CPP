#pragma once

#include <string>
#include <locale>
#include <codecvt>

using namespace std;






class StringUtilities
{
public:
	static bool String_IsASCII(const string& str);

	static bool String_Contains(const string& str, const string& substr);

	static string String_ReplaceFirst(string str, const string& find, const string& replace);
	static string String_ReplaceLast(string str, const string& find, const string& replace);
	static string String_ReplaceAll(string str, const string& find, const string& replace);

	static wstring String_ToWString(const string& str);




	static bool WString_Contains(const wstring& wstr, const wstring& subWStr);

	static wstring WString_ReplaceFirst(wstring wstr, const wstring& find, const wstring& replace);
	static wstring WString_ReplaceLast(wstring wstr, const wstring& find, const wstring& replace);
	static wstring WString_ReplaceAll(wstring wstr, const wstring& find, const wstring& replace);

	static string WString_ToString(const wstring& wstr);
};
