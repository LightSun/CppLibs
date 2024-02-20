#ifndef COPY_DISABLER_H_
#define COPY_DISABLER_H_
 
#define DISABLE_COPY(Type)\
    Type(const Type&) = delete;\
    Type& operator=(const Type&) = delete
 
#define DISABLE_MOVE(Type)\
    Type(Type&&) = delete;\
    Type& operator=(Type&&) = delete
 
#define DISABLE_COPY_MOVE(Type)\
    DISABLE_COPY(Type);\
    DISABLE_MOVE(Type)
 
 
#define DISABLE_COPY_AND_MOVE(Type)\
private:\
    Type(const Type&);\
    Type& operator=(const Type&);\
    Type(Type&&);\
    Type& operator=(Type&&);


#include <string>
#include <locale>
#include <codecvt>
#include <windows.h>

#define _T(x) x

//convert string to wstring
static inline std::wstring to_wide_string(const std::string& input)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(input);
}

//convert wstring to string
inline std::string to_byte_string(const std::wstring& input)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(input);
}

// std::string 转 wstring
static inline std::wstring s2ws(const std::string& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}

// std:wstring 转 LPWSTR
static inline LPWSTR ws2LPWSTR(const std::wstring& ws) {
    return const_cast<LPWSTR>(ws.c_str());
}

static inline std::wstring CharToWchar(const char* c, size_t m_encode = CP_ACP)
{
    std::wstring str;
    int len = MultiByteToWideChar(m_encode, 0, c, strlen(c), NULL, 0);
    wchar_t*	m_wchar = new wchar_t[len + 1];
    MultiByteToWideChar(m_encode, 0, c, strlen(c), m_wchar, len);
    m_wchar[len] = '\0';
    str = m_wchar;
    delete m_wchar;
    return str;
}

static inline std::string WcharToChar(const wchar_t* wp, size_t m_encode = CP_ACP)
{
    std::string str;
    int len = WideCharToMultiByte(m_encode, 0, wp, wcslen(wp), NULL, 0, NULL, NULL);
    char	*m_char = new char[len + 1];
    WideCharToMultiByte(m_encode, 0, wp, wcslen(wp), m_char, len, NULL, NULL);
    m_char[len] = '\0';
    str = m_char;
    delete m_char;
    return str;
}

 
#endif // COPY_DISABLER_H_
