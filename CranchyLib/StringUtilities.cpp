#include "StringUtilities.h"






bool StringUtilities::String_IsASCII(const std::string& str)
{
    for (unsigned char ch : str)
    {
        if (ch > 127)
        {
            return false;
        }
    }


    return true;
}


bool StringUtilities::String_Contains(const std::string& str, const std::string& substr)
{
    return str.find(substr) != std::string::npos;
}


std::string StringUtilities::String_ReplaceFirst(std::string str, const std::string& find, const std::string& replace)
{
    if (find.empty())
    {
        return str;
    }


    // Find the position of the first occurrence of 'find' in the string.
    // If the substring is found (i.e., position is not npos), perform the replacement.
    size_t pos = str.find(find);
    if (pos != std::string::npos)
    {
        str.replace(pos, find.length(), replace);
    }


    return str;
}

std::string StringUtilities::String_ReplaceLast(std::string str, const std::string& find, const std::string& replace)
{
    if (find.empty())
    {
        return str;
    }


    // Find the position of the last occurrence of 'find' in the string.
    // If the substring is found (i.e., position is not npos), perform the replacement.
    size_t pos = str.rfind(find);
    if (pos != std::string::npos)
    {
        str.replace(pos, find.length(), replace);
    }


    return str;
}

std::string StringUtilities::String_ReplaceAll(std::string str, const std::string& find, const std::string& replace)
{
    if (find.empty())
    {
        return str;
    }


    // Start searching from the beginning of the string.
    // Continue replacing until no more occurrences of 'find' are found.
    size_t pos = 0;
    while ((pos = str.find(find, pos)) != std::string::npos)
    {
        str.replace(pos, find.length(), replace);
        // Advance the position pointer by the length of the replacement string.
        // This avoids repeatedly replacing parts of the newly inserted text.
        pos += replace.length();
    }


    return str;
}


std::string StringUtilities::String_Reverse(const std::string& str)
{
    std::string reversedString(str);


    reverse(reversedString.begin(), reversedString.end());
    return reversedString;
}


std::wstring StringUtilities::String_ToWString(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.from_bytes(str);
}




bool StringUtilities::WString_Contains(const std::wstring& wstr, const std::wstring& subWStr)
{
    return wstr.find(subWStr) != std::wstring::npos;
}


std::wstring StringUtilities::WString_ReplaceFirst(std::wstring wstr, const std::wstring& find, const std::wstring& replace)
{
    if (find.empty())
    {
        return wstr;
    }


    size_t pos = wstr.find(find);
    if (pos != std::wstring::npos)
    {
        wstr.replace(pos, find.length(), replace);
    }


    return wstr;
}

std::wstring StringUtilities::WString_ReplaceLast(std::wstring wstr, const std::wstring& find, const std::wstring& replace)
{
    if (find.empty())
    {
        return wstr;
    }


    size_t pos = wstr.rfind(find);
    if (pos != std::wstring::npos)
    {
        wstr.replace(pos, find.length(), replace);
    }


    return wstr;
}

std::wstring StringUtilities::WString_ReplaceAll(std::wstring wstr, const std::wstring& find, const std::wstring& replace)
{
    if (find.empty())
    {
        return wstr;
    }


    size_t pos = 0;
    while ((pos = wstr.find(find, pos)) != std::wstring::npos)
    {
        wstr.replace(pos, find.length(), replace);
        pos += replace.length();
    }


    return wstr;
}


std::wstring StringUtilities::WString_Reverse(const std::wstring& wstr)
{
    std::wstring reversedString(wstr);


    reverse(reversedString.begin(), reversedString.end());
    return reversedString;
}


std::string StringUtilities::WString_ToString(const std::wstring& wstr)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.to_bytes(wstr);
}
