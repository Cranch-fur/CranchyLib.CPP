#pragma once

#include <string>
#include <locale>






class StringUtilities
{
public:
	static bool String_IsASCII(const std::string& str);

	static bool String_Contains(const std::string& str, const std::string& substr);

	static std::string String_ReplaceFirst(std::string str, const std::string& find, const std::string& replace);
	static std::string String_ReplaceLast(std::string str, const std::string& find, const std::string& replace);
	static std::string String_ReplaceAll(std::string str, const std::string& find, const std::string& replace);

	static std::string String_Reverse(const std::string& str);

	static std::wstring String_ToWString(const std::string& str);




	static bool WString_Contains(const std::wstring& wstr, const std::wstring& subWStr);

	static std::wstring WString_ReplaceFirst(std::wstring wstr, const std::wstring& find, const std::wstring& replace);
	static std::wstring WString_ReplaceLast(std::wstring wstr, const std::wstring& find, const std::wstring& replace);
	static std::wstring WString_ReplaceAll(std::wstring wstr, const std::wstring& find, const std::wstring& replace);

	static std::wstring WString_Reverse(const std::wstring& wstr);

	static std::string WString_ToString(const std::wstring& wstr);
};
