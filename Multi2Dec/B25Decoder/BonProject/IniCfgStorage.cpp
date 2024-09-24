// IniCfgStorage.cpp: 汎用コンフィギュレーションストレージクラス(INIファイル版)
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include <StdLib.h>
#include "TChar.h"
#include "IniCfgStorage.h"


/////////////////////////////////////////////////////////////////////////////
// ファイルローカル定数設定
/////////////////////////////////////////////////////////////////////////////

// デフォルトのセクション名
#define ROOT_SECTNAME		TEXT("General")		// [General]

#define MAX_SECTNAME_LEN	256UL				// 最大セクション名長
#define MAX_SECTNAME_NUM	256UL				// 最大セクション数
#define MAX_VALNAME_LEN		256UL				// 最大アイテム名長
#define MAX_VALNAME_NUM		256UL				// 最大アイテム数
#define MAX_TEXTVAL_LEN		65536UL				// 最大テキストアイテム長


/////////////////////////////////////////////////////////////////////////////
// 汎用コンフィギュレーションストレージクラス(INIファイル版)
/////////////////////////////////////////////////////////////////////////////

const bool CIniCfgStorage::OpenStorage(LPCTSTR lpszPathName, const bool bCreate)
{
	// 一旦クローズ
	CloseStorage();

	// 実行ファイルのパス名を取得
	TCHAR szDrive[_MAX_DRIVE + 1] = TEXT("\0");
	TCHAR szDir[_MAX_DIR + 1]	  = TEXT("\0");
	TCHAR szFile[_MAX_FNAME + 1]  = TEXT("\0");

	if(!::GetModuleFileName(NULL, m_szFileName, sizeof(m_szFileName) - 1))return false;
	::_tsplitpath(m_szFileName, szDrive, szDir, szFile, NULL);
	::_tmakepath(m_szFileName, szDrive, szDir, szFile, TEXT("ini"));	

	// セクション名を設定
	::_tcscpy(m_szSectName, (lpszPathName)? lpszPathName : ROOT_SECTNAME);

	// セクションの存在をチェック
	if(!bCreate){
		if(!IsSectionExist(m_szSectName)){
			// セクションが見つからない
			CloseStorage();
			return false;
			}
		}

	return true;
}

const bool CIniCfgStorage::CloseStorage(void)
{
	// ファイル名、セクション名をクリアする
	m_szFileName[0] = TEXT('\0');
	m_szSectName[0] = TEXT('\0');

	return true;
}

IConfigStorage * CIniCfgStorage::OpenSection(LPCTSTR lpszSection, const bool bCreate)
{
	if(!m_szFileName[0] || !lpszSection)return NULL;

	// セクションの存在をチェック
	if(!bCreate){
		// 作成モードでないときはセクションがなければエラー
		if(!IsSectionExist(lpszSection))return NULL;
		}

	// 新しいインスタンス生成
	CIniCfgStorage *pNewInstance = new CIniCfgStorage(NULL);
	if(!pNewInstance)return NULL;
		
	// ファイル名を設定
	::_tcscpy(pNewInstance->m_szFileName, m_szFileName);

	// 新しいインスタンスにセクション名設定
	GetSectionPath(pNewInstance->m_szSectName, lpszSection);
		
	return pNewInstance;
}

const DWORD CIniCfgStorage::GetSectionNum(void)
{
	if(!m_szFileName[0])return false;

	// セクション名を列挙する
	TCHAR szSections[MAX_SECTNAME_LEN * MAX_SECTNAME_NUM] = TEXT("\0");

	const DWORD dwRet = ::GetPrivateProfileSectionNames(szSections, sizeof(szSections) / sizeof(szSections[0]), m_szFileName);
	if(!dwRet || ((dwRet + 2UL) == (sizeof(szSections) / sizeof(szSections[0]))))return false;

	// セクション数をカウントする
	DWORD dwSectNum = 0UL;
	LPCTSTR lpszFindPos = szSections;

	while(*lpszFindPos){
		// セクション数インクリメント
		if(IsSubSectionPath(lpszFindPos))dwSectNum++;
		
		// 検索位置更新
		lpszFindPos += (::_tcslen(lpszFindPos) + 1UL);
		}

	return dwSectNum;
}

const DWORD CIniCfgStorage::EnumSection(const DWORD dwIndex, LPTSTR lpszSection)
{
	if(!m_szFileName[0])return 0UL;

	// セクション名を列挙する
	TCHAR szSections[MAX_SECTNAME_LEN * MAX_SECTNAME_NUM] = TEXT("\0");

	const DWORD dwRet = ::GetPrivateProfileSectionNames(szSections, sizeof(szSections) / sizeof(szSections[0]), m_szFileName);
	if(!dwRet || ((dwRet + 2UL) == (sizeof(szSections) / sizeof(szSections[0]))))return 0UL;

	// セクション数をカウントする
	DWORD dwSectNum = 0UL;
	LPCTSTR lpszFindPos = szSections;

	while(*lpszFindPos){
		// セクション数インクリメント
		if(IsSubSectionPath(lpszFindPos)){
			if(dwSectNum == dwIndex){
				// インデックス一致
				if(lpszSection)::_tcscpy(lpszSection, lpszFindPos);
				return ::_tcslen(lpszFindPos);
				}

			dwSectNum++;
			}
		
		// 検索位置更新
		lpszFindPos += (::_tcslen(lpszFindPos) + 1UL);
		}

	return 0UL;
}

const bool CIniCfgStorage::DeleteSection(LPCTSTR lpszSection)
{
	if(!m_szFileName[0] || !lpszSection)return false;

	// セクションの存在をチェック
	if(!IsSectionExist(lpszSection))return false;

	// セクションパス生成
	TCHAR szPath[MAX_SECTNAME_NUM];
	GetSectionPath(szPath, lpszSection);

	// セクションを削除
	return (::WritePrivateProfileSection(szPath, NULL, m_szFileName))? true : false;
}

const DWORD CIniCfgStorage::GetItemNum(void)
{
	if(!m_szFileName[0])return 0UL;

	// アイテムを列挙する
	TCHAR szItems[MAX_VALNAME_LEN * MAX_VALNAME_NUM] = TEXT("\0");
	const DWORD dwRet = ::GetPrivateProfileString(m_szSectName, NULL, NULL, szItems, sizeof(szItems) / sizeof(szItems[0]), m_szFileName);

	if(!dwRet || ((dwRet + 2UL) == (sizeof(szItems) / sizeof(szItems[0]))))return 0UL;

	// アイテム数をカウントする
	DWORD dwItemNum = 0UL;
	LPCTSTR lpszFindPos = szItems;

	while(*lpszFindPos){
		// アイテム数インクリメント
		dwItemNum++;
		
		// 検索位置更新
		lpszFindPos += (::_tcslen(lpszFindPos) + 1UL);
		}

	// アイテム数を返す
	return dwItemNum;
}

const DWORD CIniCfgStorage::EnumItem(const DWORD dwIndex, LPTSTR lpszItem)
{
	if(!m_szFileName[0])return 0UL;

	// アイテムを列挙する
	TCHAR szItems[MAX_VALNAME_LEN * MAX_VALNAME_NUM] = TEXT("\0");
	const DWORD dwRet = ::GetPrivateProfileString(m_szSectName, NULL, NULL, szItems, sizeof(szItems) / sizeof(szItems[0]), m_szFileName);

	if(!dwRet || ((dwRet + 2UL) == (sizeof(szItems) / sizeof(szItems[0]))))return 0UL;

	// アイテム数をカウントする
	DWORD dwItemNum = 0UL;
	LPTSTR lpszFindPos = szItems;

	while(*lpszFindPos){
		// アイテム数インクリメント
		if(dwItemNum == dwIndex){
			// インデックス一致
			if(lpszItem)::_tcscpy(lpszItem, lpszFindPos);
			return ::_tcslen(lpszFindPos);
			}		
		
		// 検索位置更新
		lpszFindPos += (::_tcslen(lpszFindPos) + 1UL);
		dwItemNum++;
		}

	return 0UL;
}

const DWORD CIniCfgStorage::GetItemType(LPCTSTR lpszItem)
{
	if(!m_szFileName[0] || !lpszItem)return ITEM_TYPE_INVALID;

	// アイテムタイプを返す(INIファイルの場合は常にテキスト)
	return (IsItemExist(lpszItem))? ITEM_TYPE_TEXT : ITEM_TYPE_INVALID;
}

const bool CIniCfgStorage::DeleteItem(LPCTSTR lpszItem)
{
	if(!m_szFileName[0] || !lpszItem)return false;

	// アイテムを削除する
	return (::WritePrivateProfileString(m_szSectName, lpszItem, NULL, m_szFileName))? true : false;
}

const bool CIniCfgStorage::SetBoolItem(LPCTSTR lpszItem, const bool bValue)
{
	if(!m_szFileName[0] || !lpszItem)return false;
	
	// BOOLをセーブする
	return (::WritePrivateProfileString(m_szSectName, lpszItem, (bValue)? TEXT("1") : TEXT("0"), m_szFileName))? true : false;
}

const bool CIniCfgStorage::SetIntItem(LPCTSTR lpszItem, const DWORD dwValue)
{
	if(!m_szFileName[0] || !lpszItem)return false;

	TCHAR szValue[16];
	::_stprintf(szValue, TEXT("%lu"), dwValue);

	// DWORDをセーブする
	return (::WritePrivateProfileString(m_szSectName, lpszItem, szValue, m_szFileName))? true : false;
}

const bool CIniCfgStorage::SetTextItem(LPCTSTR lpszItem, LPCTSTR lpszValue)
{
	if(!m_szFileName[0] || !lpszItem || !lpszValue)return false;

	// 文字列をセーブする
	return (::WritePrivateProfileString(m_szSectName, lpszItem, lpszValue, m_szFileName))? true : false;
}

const bool CIniCfgStorage::GetBoolItem(LPCTSTR lpszItem, const bool bDefault)
{
	if(!m_szFileName[0] || !lpszItem)return false;

	// BOOLをロードする
	return (::GetPrivateProfileInt(m_szSectName, lpszItem, (bDefault)? 1UL : 0UL, m_szFileName))? true : false;
}

const DWORD CIniCfgStorage::GetIntItem(LPCTSTR lpszItem, const DWORD dwDefault, const DWORD dwMax, const DWORD dwMin)
{
	if(!m_szFileName[0] || !lpszItem)return false;

	// DWORDをロードする
	const DWORD dwValue = (DWORD)::GetPrivateProfileInt(m_szSectName, lpszItem, dwDefault, m_szFileName);
	
	// 範囲をチェック
	return ((dwValue >= dwMin) && (dwValue <= dwMax))? dwValue : dwDefault;
}

const DWORD CIniCfgStorage::GetTextItem(LPCTSTR lpszItem, LPTSTR lpszValue, LPCTSTR lpszDefault, const DWORD dwMaxLen)
{
	if(!m_szFileName[0] || !lpszItem)return false;

	TCHAR szValue[MAX_TEXTVAL_LEN] = TEXT("\0");
	if(lpszDefault)::_tcscpy(szValue, lpszDefault);

	// 文字列をロードする
	const DWORD dwRet = ::GetPrivateProfileString(m_szSectName, lpszItem, (lpszDefault)? lpszDefault : TEXT(""), szValue, sizeof(szValue) / sizeof(szValue[0]), m_szFileName);

	// 文字数をチェック
	if(dwRet > dwMaxLen){
		::_tcscpy(szValue, (lpszDefault)? lpszDefault : TEXT(""));
		}
		
	// バッファにコピー
	if(lpszValue)::_tcscpy(lpszValue, szValue);
	
	// 文字数を返す
	return ::_tcslen(szValue);
}

CIniCfgStorage::CIniCfgStorage(IBonObject *pOwner)
	: CBonObject(pOwner)
{
	m_szFileName[0] = TEXT('\0');
	m_szSectName[0] = TEXT('\0');
}

CIniCfgStorage::~CIniCfgStorage(void)
{
	// ストレージをクローズする
	CloseStorage();
}

void CIniCfgStorage::GetSectionPath(LPTSTR lpszPath, LPCTSTR lpszSection) const
{
	if(!m_szFileName[0] || !lpszPath || !lpszSection)::DebugBreak();

	// セクションパスを生成する
	if(!::_tcscmp(m_szSectName, ROOT_SECTNAME)){
		// ルート直下
		::_tcscpy(lpszPath, lpszSection);
		}
	else{
		// サブセクション以下
		::_stprintf(lpszPath, TEXT("%s\\%s"), m_szSectName, lpszSection);
		}
}

const bool CIniCfgStorage::IsSectionExist(LPCTSTR lpszSection) const
{
	if(!m_szFileName[0] || !lpszSection)::DebugBreak();

	// セクション名を列挙する
	TCHAR szSections[MAX_SECTNAME_LEN * MAX_SECTNAME_NUM] = TEXT("\0");

	const DWORD dwRet = ::GetPrivateProfileSectionNames(szSections, sizeof(szSections) / sizeof(szSections[0]), m_szFileName);
	if(!dwRet || ((dwRet + 2UL) == (sizeof(szSections) / sizeof(szSections[0]))))return false;

	// 検索セクション名生成
	TCHAR szFindSection[MAX_SECTNAME_NUM];

	if(!::_tcscmp(m_szSectName, ROOT_SECTNAME)){
		// ルート直下
		::_tcscpy(szFindSection, lpszSection);
		}
	else{
		// サブセクション以下
		::_stprintf(szFindSection, TEXT("%s\\%s"), m_szSectName, lpszSection);
		}

	// セクションを検索する
	LPCTSTR lpszFindPos = szSections;

	while(*lpszFindPos){
		if(!::_tcscmp(lpszFindPos, szFindSection)){
			// セクション名一致
			return true;
			}
		
		// 検索位置更新
		lpszFindPos += (::_tcslen(lpszFindPos) + 1UL);
		}

	return false;
}

const bool CIniCfgStorage::IsSubSectionPath(LPCTSTR lpszPath) const
{
	if(!m_szFileName[0] || !lpszPath)::DebugBreak();

	if(!::_tcscmp(m_szSectName, ROOT_SECTNAME)){
		// ルート直下
		if(!::_tcschr(lpszPath, TEXT('\\')))return true;
		}
	else{
		// サブセクション以下
		const DWORD dwParentLen = ::_tcslen(m_szSectName) + 1UL;

		if((DWORD)::_tcslen(lpszPath) > dwParentLen){
			// 親セクション名より長い
			if(!::_tcsnicmp(lpszPath, m_szSectName, dwParentLen)){
				// 検索セクション名で始まる					
				if(!::_tcschr(&lpszPath[dwParentLen], TEXT('\\')))return true;
				}
			}
		}

	return false;
}

const bool CIniCfgStorage::IsItemExist(LPCTSTR lpszItem) const
{
	if(!m_szFileName[0] || !lpszItem)::DebugBreak();

	// アイテムを列挙する
	TCHAR szItems[MAX_VALNAME_LEN * MAX_VALNAME_NUM] = TEXT("\0");
	const DWORD dwRet = ::GetPrivateProfileString(m_szSectName, NULL, NULL, szItems, sizeof(szItems) / sizeof(szItems[0]), m_szFileName);

	if(!dwRet || ((dwRet + 2UL) == (sizeof(szItems) / sizeof(szItems[0]))))return false;

	// アイテムを検索する
	LPCTSTR lpszFindPos = szItems;

	while(*lpszFindPos){
		if(!::_tcsicmp(lpszFindPos, lpszItem))return true;
		
		// 検索位置更新
		lpszFindPos += (::_tcslen(lpszFindPos) + 1UL);
		}

	return false;
}
