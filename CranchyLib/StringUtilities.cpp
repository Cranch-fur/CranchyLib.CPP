#include "StringUtilities.h"






bool StringUtilities::String_IsASCII(const string& str)
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


bool StringUtilities::String_Contains(const string& str, const string& substr)
{
    return str.find(substr) != string::npos;
}


string StringUtilities::String_ReplaceFirst(string str, const string& find, const string& replace)
{
    if (find.empty())
    {
        return str;
    }


    // Find the position of the first occurrence of 'find' in the string.
    // If the substring is found (i.e., position is not npos), perform the replacement.
    size_t pos = str.find(find);
    if (pos != string::npos)
    {
        str.replace(pos, find.length(), replace);
    }


    return str;
}

string StringUtilities::String_ReplaceLast(string str, const string& find, const string& replace)
{
    if (find.empty())
    {
        return str;
    }


    // Find the position of the last occurrence of 'find' in the string.
    // If the substring is found (i.e., position is not npos), perform the replacement.
    size_t pos = str.rfind(find);
    if (pos != string::npos)
    {
        str.replace(pos, find.length(), replace);
    }


    return str;
}

string StringUtilities::String_ReplaceAll(string str, const string& find, const string& replace)
{
    if (find.empty())
    {
        return str;
    }


    // Start searching from the beginning of the string.
    // Continue replacing until no more occurrences of 'find' are found.
    size_t pos = 0;
    while ((pos = str.find(find, pos)) != string::npos)
    {
        str.replace(pos, find.length(), replace);
        // Advance the position pointer by the length of the replacement string.
        // This avoids repeatedly replacing parts of the newly inserted text.
        pos += replace.length();
    }


    return str;
}


wstring StringUtilities::String_ToWString(const string& str)
{
    wstring_convert<codecvt_utf8<wchar_t>> conv;
    return conv.from_bytes(str);
}




bool StringUtilities::WString_Contains(const wstring& wstr, const wstring& subWStr)
{
    return wstr.find(subWStr) != wstring::npos;
}


wstring StringUtilities::WString_ReplaceFirst(wstring wstr, const wstring& find, const wstring& replace)
{
    if (find.empty())
    {
        return wstr;
    }


    size_t pos = wstr.find(find);
    if (pos != wstring::npos)
    {
        wstr.replace(pos, find.length(), replace);
    }


    return wstr;
}

wstring StringUtilities::WString_ReplaceLast(wstring wstr, const wstring& find, const wstring& replace)
{
    if (find.empty())
    {
        return wstr;
    }


    size_t pos = wstr.rfind(find);
    if (pos != wstring::npos)
    {
        wstr.replace(pos, find.length(), replace);
    }


    return wstr;
}

wstring StringUtilities::WString_ReplaceAll(wstring wstr, const wstring& find, const wstring& replace)
{
    if (find.empty())
    {
        return wstr;
    }


    size_t pos = 0;
    while ((pos = wstr.find(find, pos)) != wstring::npos)
    {
        wstr.replace(pos, find.length(), replace);
        pos += replace.length();
    }


    return wstr;
}


string StringUtilities::WString_ToString(const wstring& wstr)
{
    wstring_convert<codecvt_utf8<wchar_t>> conv;
    return conv.to_bytes(wstr);
}
