#pragma once

#include <iostream>
#include <string>
#include <regex>
#include <Windows.h>

using namespace std;
const std::string WHITESPACE = " \n\r\t\f\v";

//std::string을 std::printf처럼 조합하여 반환해주는 함수
template<typename ... Args>
std::string string_format(const std::string& format, Args ... args)
{
    size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
    if (size <= 0) { throw std::runtime_error("string_format Error during formatting."); }
    std::unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

//공백(' ') N개를 1개로 줄여주는 함수
std::string String_ReplaceNSpaceTo1Space(std::string str)
{
    std::regex reg(R"(\s+)");
    str = std::regex_replace(str, reg, " ");
    return str;
}

std::string String_ltrim(const std::string& s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

std::string String_rtrim(const std::string& s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

std::string String_trim(const std::string& s)
{
    return String_rtrim(String_ltrim(s));
}

std::string String_MultiByteToUtf8(const std::string& str)
{
    int nLen = str.size();
    wchar_t warr[256];
    MultiByteToWideChar(CP_ACP, 0, (LPCSTR)str.c_str(), -1, warr, 256);
    char carr[256];
    memset(carr, '\0', sizeof(carr));
    WideCharToMultiByte(CP_UTF8, 0, warr, -1, carr, 256, NULL, NULL);
    return carr;
}

std::string String_Utf8ToMultiByte(const std::string& str)
{
    wchar_t warr[256];
    int nLen = str.size();
    memset(warr, '\0', sizeof(warr));
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, warr, 256);
    char carr[256];
    memset(carr, '\0', sizeof(carr));
    WideCharToMultiByte(CP_ACP, 0, warr, -1, carr, 256, NULL, NULL);
    return carr;
}

size_t String_UTF8ByteLength(const std::string& str)
{
    size_t uint8_byte_count = 0;
    for (int i = 0; i < str.length();)
    {
        // 4바이트 문자인지 확인
        // 0xF0 = 1111 0000
        if (0xF0 == (0xF0 & str[i]))
        {
            // 나머지 3바이트 확인
            // 0x80 = 1000 0000
            if (0x80 != (0x80 & str[i + 1]) || 0x80 != (0x80 & str[i + 2]) || 0x80 != (0x80 & str[i + 3]))
            {
                throw std::exception("not utf-8 encoded string");
            }
            i += 4;
            uint8_byte_count += 4;
            continue;
        }
        // 3바이트 문자인지 확인
        // 0xE0 = 1110 0000
        else if (0xE0 == (0xE0 & str[i]))
        {
            // 나머지 2바이트 확인
            // 0x80 = 1000 0000
            if (0x80 != (0x80 & str[i + 1]) || 0x80 != (0x80 & str[i + 2]))
            {
                throw std::exception("not utf-8 encoded string");
            }
            i += 3;
            uint8_byte_count += 3;
            continue;
        }
        // 2바이트 문자인지 확인
        // 0xC0 = 1100 0000
        else if (0xC0 == (0xC0 & str[i]))
        {
            // 나머지 1바이트 확인
            // 0x80 = 1000 0000
            if (0x80 != (0x80 & str[i + 1]))
            {
                throw std::exception("not utf-8 encoded string");
            }
            i += 2;
            uint8_byte_count += 2;
            continue;
        }
        // 최상위 비트가 0인지 확인
        else if (0 == (str[i] >> 7))
        {
            i += 1;
            uint8_byte_count += 1;
        }
        else
        {
            throw std::exception("not utf-8 encoded string");
        }
    }
    return uint8_byte_count;
}

size_t String_UTF8Length(const std::string& str)
{
    size_t utf8_char_count = 0;
    for (int i = 0; i < str.length();)
    {
        // 4바이트 문자인지 확인
        // 0xF0 = 1111 0000
        if (0xF0 == (0xF0 & str[i]))
        {
            // 나머지 3바이트 확인
            // 0x80 = 1000 0000
            if (0x80 != (0x80 & str[i + 1]) || 0x80 != (0x80 & str[i + 2]) || 0x80 != (0x80 & str[i + 3]))
            {
                throw std::exception("not utf-8 encoded string");
            }
            i += 4;
            utf8_char_count++;
            continue;
        }
        // 3바이트 문자인지 확인
        // 0xE0 = 1110 0000
        else if (0xE0 == (0xE0 & str[i]))
        {
            // 나머지 2바이트 확인
            // 0x80 = 1000 0000
            if (0x80 != (0x80 & str[i + 1]) || 0x80 != (0x80 & str[i + 2]))
            {
                throw std::exception("not utf-8 encoded string");
            }
            i += 3;
            utf8_char_count++;
            continue;
        }
        // 2바이트 문자인지 확인
        // 0xC0 = 1100 0000
        else if (0xC0 == (0xC0 & str[i]))
        {
            // 나머지 1바이트 확인
            // 0x80 = 1000 0000
            if (0x80 != (0x80 & str[i + 1]))
            {
                throw std::exception("not utf-8 encoded string");
            }
            i += 2;
            utf8_char_count++;
            continue;
        }
        // 최상위 비트가 0인지 확인
        else if (0 == (str[i] >> 7))
        {
            i += 1;
            utf8_char_count++;
        }
        else
        {
            throw std::exception("not utf-8 encoded string");
        }
    }
    return utf8_char_count;
}