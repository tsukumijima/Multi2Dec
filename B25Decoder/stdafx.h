// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#ifdef _WIN32

// 下で指定された定義の前に対象プラットフォームを指定しなければならない場合、以下の定義を変更してください。
// 異なるプラットフォームに対応する値に関する最新情報については、MSDN を参照してください。
#ifndef WINVER				// Windows XP 以降のバージョンに固有の機能の使用を許可します。
#define WINVER 0x0501		// これを Windows の他のバージョン向けに適切な値に変更してください。
#endif

#ifndef _WIN32_WINNT		// Windows XP 以降のバージョンに固有の機能の使用を許可します。                   
#define _WIN32_WINNT 0x0501	// これを Windows の他のバージョン向けに適切な値に変更してください。
#endif						

#ifndef _WIN32_WINDOWS		// Windows 98 以降のバージョンに固有の機能の使用を許可します。
#define _WIN32_WINDOWS 0x0410 // これを Windows Me またはそれ以降のバージョン向けに適切な値に変更してください。
#endif

#ifndef _WIN32_IE			// IE 6.0 以降のバージョンに固有の機能の使用を許可します。
#define _WIN32_IE 0x0600	// これを IE. の他のバージョン向けに適切な値に変更してください。
#endif

#define WIN32_LEAN_AND_MEAN		// Windows ヘッダーから使用されていない部分を除外します。
// Windows ヘッダー ファイル:
#include <Windows.h>
#include <crtdbg.h>
#include <tchar.h>


// TODO: プログラムに必要な追加ヘッダーをここで参照してください。

// 警告の無効設定
#pragma warning(disable: 4355) // warning C4355: "'this' : ベース メンバ初期化リストで使用されました。"
#pragma warning(disable: 4996) // warning C4996: "This function or variable may be unsafe. Consider using _wsplitpath_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details"

#else

#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef char * LPSTR;
typedef const char * LPCSTR;
typedef char TCHAR;
typedef TCHAR * LPTSTR;
typedef const TCHAR * LPCTSTR;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned long long ULONGLONG;
typedef void * PVOID;
typedef int BOOL;

// intが32ビットであること
static_assert(sizeof(int) == 4);

#define FALSE 0
#define TRUE 1
#define _T(x) x
#define TEXT(x) x
#define _tcscpy strcpy
#define _tcslen strlen
#define _tcscmp strcmp
#define _vstprintf vsprintf

struct SYSTEMTIME
{
	WORD wYear;
	WORD wMonth;
	WORD wDayOfWeek;
	WORD wDay;
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wMilliseconds;
};

inline void CopyMemory(void *dest, const void *src, size_t size)
{
	::memcpy(dest, src, size);
}

inline void MoveMemory(void *dest, const void *src, size_t size)
{
	::memmove(dest, src, size);
}

inline void FillMemory(void *dest, size_t size, BYTE filler)
{
	::memset(dest, filler, size);
}

inline void ZeroMemory(void *dest, size_t size)
{
	::memset(dest, 0, size);
}

#endif
