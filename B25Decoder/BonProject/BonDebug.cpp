// BonDebug.cpp: デバッグ補助ライブラリ
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include <StdLib.h>


/////////////////////////////////////////////////////////////////////////////
// 汎用例外クラス
/////////////////////////////////////////////////////////////////////////////

CBonException::CBonException(LPCSTR lpszSourceFile, const DWORD dwSourceLine, const BONGUID ClassId)
	: m_ClassId(ClassId)
	, m_lpszSourceFile(lpszSourceFile)
	, m_dwSourceLine(dwSourceLine)
{
	// 説明を初期化
	::_tcscpy(m_szDescription, TEXT("例外に関する説明はありません"));
}

CBonException::~CBonException(void)
{
	// 何もしない
}

CBonException & CBonException::operator( )(LPCTSTR lpszFormat, ...)
{
	// 説明を書式化
	va_list Args;
	va_start(Args ,lpszFormat);
	::_vstprintf(m_szDescription, lpszFormat, Args);
	va_end(Args);
	
	return *this;
}

void CBonException::Notify(const bool bEnable)
{
	// 例外を通知する
	TCHAR szOutput[1024] = TEXT("");
	TCHAR szSource[1024] = TEXT("不明なソースファイル");
	
	// ソースファイル名を取得
	GetSourceFile(szSource);
	
	if(m_ClassId != ::GetGuid()){
		TCHAR szModuleName[BON_MAX_MODULE_NAME] = TEXT("不明な");
		TCHAR szClassName[BON_MAX_CLASS_NAME]	= TEXT("不明な");
		
		// クラス名を取得
		::GET_BON_CLASS_NAME(m_ClassId, szClassName);
		
		// モジュール名を取得
		::GET_BON_MODULE_NAME(::GET_BON_CLASS_MODULE(m_ClassId), szModuleName);
		
		// デバッグ出力文字列生成
		::_stprintf(szOutput, TEXT("[例外イベント]\r\n　・場所\t： %sモジュール　%sクラス\r\n　・ソース\t： %s　%lu 行目\r\n　・説明\t： %s\r\n"), szModuleName, szClassName, szSource, m_dwSourceLine, m_szDescription);
		}
	else{
		// デバッグ出力文字列生成
		::_stprintf(szOutput, TEXT("[例外イベント]\r\n　・場所\t： グローバル又は非Bonクラス\r\n　・ソース\t： %s　%lu 行目\r\n　・説明\t： %s\r\n"), szSource, m_dwSourceLine, m_szDescription);
		}

	// デバッグ出力
	::OutputDebugString(szOutput);

	// イベントログに出力
		// まだ未実装

	// ユーザに通知
	if(bEnable){
		::_tcscat(szOutput, TEXT("\r\n\r\n例外を無視してプログラムの実行を継続しますか？"));

		if(::MessageBox(NULL, szOutput, TEXT("BonSDK 例外通知"), MB_YESNO | MB_ICONERROR | MB_TASKMODAL) == IDYES){
			// 実行を継続
			return;
			}

		// デバッグブレーク
		::DebugBreak();
		}
}

const BONGUID CBonException::GetGuid(void)
{
	// 例外発生時のBONGUIDを返す
	return m_ClassId;
}

const DWORD CBonException::GetDescription(LPTSTR lpszDescription)
{
	// 例外発生時の説明を返す
	if(lpszDescription)::_tcscpy(lpszDescription, m_szDescription);

	// 文字数を返す
	return ::_tcslen(m_szDescription);
}

const DWORD CBonException::GetSourceFile(LPTSTR lpszSourceFile)
{
	// 例外発生時のソースファイル名を返す
	TCHAR szPathName[_MAX_PATH] = TEXT("");
	TCHAR szFileName[_MAX_FNAME] = TEXT("");
	TCHAR szExtName[_MAX_EXT] = TEXT("");

#ifdef _UNICODE
	// UNICODEに変換
	::MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, m_lpszSourceFile, -1, szPathName, sizeof(szPathName));
#else
	// そのままコピー
	::_tcscpy(szPathName, m_lpszSourceFile);
#endif

	// ファイル名を抽出
	::_tsplitpath(szPathName, NULL, NULL, szFileName, szExtName);
	::_tmakepath(szPathName, NULL, NULL, szFileName, szExtName);	

	if(lpszSourceFile)::_tcscpy(lpszSourceFile, szPathName);

	// 文字数を返す
	return ::_tcslen(szPathName);
}

const DWORD CBonException::GetSourceLine(void)
{
	// 例外発生時のソースコード行を返す
	return m_dwSourceLine;
}


/////////////////////////////////////////////////////////////////////////////
// 汎用照合クラス
/////////////////////////////////////////////////////////////////////////////

CBonAssert::CBonAssert(LPCSTR lpszSourceFile, const DWORD dwSourceLine, const BONGUID ClassId)
	: m_lpszSourceFile(lpszSourceFile)
	, m_dwSourceLine(dwSourceLine)
	, m_ClassId(ClassId)
{
	// 何もしない
}

void CBonAssert::operator( )(const bool bSuccess, LPCTSTR lpszFormat, ...)
{
	// 条件式版
	if(!bSuccess){
		// 例外オブジェクト生成
		CBonException Exception(m_lpszSourceFile, m_dwSourceLine, m_ClassId);
		
		if(lpszFormat){
			TCHAR szDescription[1024] = TEXT("");

			// フォーマットを書式化
			va_list Args;
			va_start(Args , lpszFormat);
			::_vstprintf(szDescription, lpszFormat, Args);
			va_end(Args);
			Exception(szDescription);
			}

		// 例外を発生させる
		Exception.Notify();
		}
}

void CBonAssert::operator( )(const void *pPointer, LPCTSTR lpszFormat, ...)
{
	// ポインタ版
	if(!pPointer){
		// 例外オブジェクト生成
		CBonException Exception(m_lpszSourceFile, m_dwSourceLine, m_ClassId);
		
		if(lpszFormat){
			TCHAR szDescription[1024] = TEXT("");

			// フォーマットを書式化
			va_list Args;
			va_start(Args , lpszFormat);
			::_vstprintf(szDescription, lpszFormat, Args);
			va_end(Args);
			Exception(szDescription);
			}

		// 例外を発生させる
		Exception.Notify();
		}
}


/////////////////////////////////////////////////////////////////////////////
// デバッグ補助関数
/////////////////////////////////////////////////////////////////////////////

const BONGUID GetGuid(void)
{
	// グローバルスコープの場合はBCID_NULLを返す
	return BCID_NULL;
}

void BON_TRACE(LPCTSTR lpszFormat, ...)
{
	TCHAR szOutput[1024] = TEXT("");

	// メッセージを書式化
	va_list Args;
	va_start(Args , lpszFormat);
	::_vstprintf(szOutput, lpszFormat, Args);
	va_end(Args);

	// デバッグ出力に書き込み
	::OutputDebugString(szOutput);
}
