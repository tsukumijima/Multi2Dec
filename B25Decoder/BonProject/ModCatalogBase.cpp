// ModCatalogBase.cpp: モジュールカタログ基底クラス
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "ModCatalogBase.h"
#include <StdLib.h>
#include <TChar.h>


#pragma comment(lib, "Version.lib")


/////////////////////////////////////////////////////////////////////////////
// モジュールカタログ基底クラス
/////////////////////////////////////////////////////////////////////////////

const WORD CModCatalogBase::GetVersion(void)
{
	if(!m_hModule){
		// バージョンリソースから取得しない場合
		return m_wVersion;
		}

	WORD wVersion = 0x0000U;

	try{
		// モジュールのファイル名を取得
		TCHAR szFileName[_MAX_PATH + 1];
		if(!::GetModuleFileName(m_hModule, szFileName, sizeof(szFileName) - 1))throw ::BON_EXPECTION();

		// バージョン情報リソースのサイズを取得
		DWORD dwHandle = 0;
		DWORD dwLength = ::GetFileVersionInfoSize(szFileName, &dwHandle);
		if(!dwLength)throw ::BON_EXPECTION();

		// バージョン情報リソースロード
		CMediaData Buffer((const BYTE)0x00U, dwLength + 1UL);
		if(!::GetFileVersionInfo(szFileName, 0, dwLength, Buffer.GetData()))throw ::BON_EXPECTION();

		// LangCodepageを取得
		DWORD *pdwLang;
		TCHAR szSubBlock[50] = TEXT("\\VarFileInfo\\Translation");
		if(!::VerQueryValue(Buffer.GetData(), szSubBlock, (void **)&pdwLang, (PUINT)&dwLength))throw ::BON_EXPECTION();

		// 情報を取得する
		LPTSTR lpszBuffer;
		::_stprintf(szSubBlock, TEXT("\\StringFileInfo\\%04x%04x\\ProductVersion"), LOWORD(*pdwLang), HIWORD(*pdwLang));
		if(!::VerQueryValue(Buffer.GetData(), szSubBlock, (void **)&lpszBuffer, (PUINT)&dwLength))throw ::BON_EXPECTION();
		
		// 整形する
		DWORD adwVers[4] = {0UL, 0UL, 0UL, 0UL};
		::_stscanf(lpszBuffer, TEXT("%lu,%lu,%lu,%lu"), &adwVers[0], &adwVers[1], &adwVers[2], &adwVers[3]);
		
		wVersion = (WORD)(((adwVers[0] & 0xFUL) << 12) | ((adwVers[1] & 0xFUL) << 8) | ((adwVers[2] & 0xFUL) << 4) | ((adwVers[3] & 0xFUL) << 0));
		}
	catch(CBonException &Exception){
		// エラー発生
		Exception.Notify();
		return 0x0000U;
		}

	return wVersion;
}

const DWORD CModCatalogBase::GetComment(LPTSTR lpszComment)
{
	// コメントを返す
	if(lpszComment)::_tcscpy(lpszComment, m_lpszComment);
	return ::_tcslen(m_lpszComment);
}

const DWORD CModCatalogBase::GetAuthor(LPTSTR lpszAuthor)
{
	// 作者名を返す
	if(lpszAuthor)::_tcscpy(lpszAuthor, m_lpszAuthor);
	return ::_tcslen(m_lpszAuthor);
}

const DWORD CModCatalogBase::GetWebsite(LPTSTR lpszWebsite)
{
	// 連絡先を返す
	if(lpszWebsite)::_tcscpy(lpszWebsite, m_lpszWebsite);
	return ::_tcslen(m_lpszWebsite);
}

const BONGUID CModCatalogBase::EnumClass(LPCTSTR lpszBIId, const DWORD dwIndex)
{
	// 指定されたインタフェースに対応するクラスを列挙する
	for(DWORD dwInfoPos = 0UL, dwClassPos = 0UL ; dwInfoPos < m_dwClassNum ; dwInfoPos++){
		if(!::_tcscmp(m_pClassInfo[dwInfoPos].szBIId, lpszBIId)){
			if(dwClassPos++ == dwIndex){
				return ::BON_NAME_TO_GUID(m_pClassInfo[dwInfoPos].szBCId);
				}
			}	
		}

	return BCID_NULL;
}

const DWORD CModCatalogBase::GetClassDesc(const BONGUID ClassId, LPTSTR lpszDescription)
{
	// 指定されたGUIDのクラスを検索する
	for(DWORD dwInfoPos = 0UL ; dwInfoPos < m_dwClassNum ; dwInfoPos++){
		if(::BON_NAME_TO_GUID(m_pClassInfo[dwInfoPos].szBCId) == ClassId){
			// GUIDが一致、説明を返す
			if(lpszDescription)::_tcscpy(lpszDescription, m_pClassInfo[dwInfoPos].szDesc);
			return ::_tcslen(m_pClassInfo[dwInfoPos].szDesc);
			}
		}

	return 0UL;
}

CModCatalogBase::CModCatalogBase(IBonObject *pOwner)
	: CBonObject(pOwner)
	, m_pClassInfo(NULL)
	, m_dwClassNum(0UL)
	, m_hModule(NULL)
	, m_wVersion(0U)
	, m_lpszComment(NULL)
	, m_lpszAuthor(NULL)
	, m_lpszWebsite(NULL)
{
	// 使用してはならない
	::BON_ASSERT(false);
}

CModCatalogBase::CModCatalogBase(IBonObject *pOwner, const MOD_CLASS_INFO *pClassInfo, const DWORD dwClassNum, const HMODULE hModule, const LPCTSTR lpszComment, const LPCTSTR lpszAuthor, const LPCTSTR lpszWebsite)
	: CBonObject(pOwner)
	, m_pClassInfo(pClassInfo)
	, m_dwClassNum(dwClassNum)
	, m_hModule(hModule)
	, m_wVersion(0U)
	, m_lpszComment(lpszComment)
	, m_lpszAuthor(lpszAuthor)
	, m_lpszWebsite(lpszWebsite)
{
	// 何もしない
}

CModCatalogBase::CModCatalogBase(IBonObject *pOwner, const MOD_CLASS_INFO *pClassInfo, const DWORD dwClassNum, const WORD wVersion, const LPCTSTR lpszComment, const LPCTSTR lpszAuthor, const LPCTSTR lpszWebsite)
	: CBonObject(pOwner)
	, m_pClassInfo(pClassInfo)
	, m_dwClassNum(dwClassNum)
	, m_hModule(NULL)
	, m_wVersion(wVersion)
	, m_lpszComment(lpszComment)
	, m_lpszAuthor(lpszAuthor)
	, m_lpszWebsite(lpszWebsite)
{
	// 何もしない
}

CModCatalogBase::~CModCatalogBase(void)
{
	// 何もしない
}
