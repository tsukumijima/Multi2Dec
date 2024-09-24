// RegCfgStorage.cpp: 汎用コンフィギュレーションストレージクラス(レジストリ版)
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "TChar.h"
#include "RegCfgStorage.h"


/////////////////////////////////////////////////////////////////////////////
// ファイルローカル定数設定
/////////////////////////////////////////////////////////////////////////////

// デフォルトのセクション名
#define DEF_PATHNAME		TEXT("Bon")		// /HKEY_CURRENT_USER/Software/Bon/

#define MAX_KEYNAME_LEN		256UL			// 最大キー名長
#define MAX_VALNAME_LEN		256UL			// 最大アイテム名長


/////////////////////////////////////////////////////////////////////////////
// 汎用コンフィギュレーションストレージクラス(レジストリ版)
/////////////////////////////////////////////////////////////////////////////

const bool CRegCfgStorage::OpenStorage(LPCTSTR lpszPathName, const bool bCreate)
{
	// 一旦クローズ
	CloseStorage();

	// パス名を生成
	TCHAR szKeyPath[MAX_KEYNAME_LEN] = {TEXT('\0')};
	::_stprintf(szKeyPath, TEXT("Software\\%s"), (lpszPathName)? lpszPathName : DEF_PATHNAME);

	// レジストリをオープンする
	if(bCreate){
		// キーが存在しない場合はキーを作成する
		if(::RegCreateKeyEx(HKEY_CURRENT_USER, szKeyPath, 0UL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hRegKey, NULL) != ERROR_SUCCESS)return false;
		}
	else{
		// キーが存在しない場合は失敗する
		if(::RegOpenKeyEx(HKEY_CURRENT_USER, szKeyPath, 0UL, KEY_ALL_ACCESS, &m_hRegKey) != ERROR_SUCCESS)return false;
		}

	return true;
}

const bool CRegCfgStorage::CloseStorage(void)
{
	if(!m_hRegKey)return false;

	// レジストリを閉じる
	::RegCloseKey(m_hRegKey);
	m_hRegKey = NULL;

	return true;
}

IConfigStorage * CRegCfgStorage::OpenSection(LPCTSTR lpszSection, const bool bCreate)
{
	if(!m_hRegKey || !lpszSection)return NULL;

	// サブキーをオープンする
	HKEY hSubKey = NULL;

	if(bCreate){
		// サブキーが存在しない場合はサブキーを作成する
		if(::RegCreateKeyEx(m_hRegKey, lpszSection, 0UL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hSubKey, NULL) != ERROR_SUCCESS)return NULL;
		}
	else{
		// サブキーが存在しない場合は失敗する
		if(::RegOpenKeyEx(m_hRegKey, lpszSection, 0UL, KEY_ALL_ACCESS, &hSubKey) != ERROR_SUCCESS)return NULL;
		}

	// 新しいインスタンス生成
	CRegCfgStorage *pNewInstance = new CRegCfgStorage(NULL);

	if(!pNewInstance){
		::RegCloseKey(hSubKey);
		return NULL;
		}

	// 新しいインスタンスにハンドル設定
	pNewInstance->m_hRegKey = hSubKey;

	return pNewInstance;
}

const DWORD CRegCfgStorage::GetSectionNum(void)
{
	if(!m_hRegKey)return 0UL;

	DWORD dwSubKeyNum = 0UL;

	// サブキー数を取得する
	if(::RegQueryInfoKey(m_hRegKey, NULL, NULL, NULL, &dwSubKeyNum, NULL, NULL, NULL, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)return 0UL;

	// サブキー数を返す
	return dwSubKeyNum;
}

const DWORD CRegCfgStorage::EnumSection(const DWORD dwIndex, LPTSTR lpszSection)
{
	if(!m_hRegKey)return 0UL;

	DWORD dwSectionLen = MAX_KEYNAME_LEN;

	// サブキー名を列挙する
	if(::RegEnumKeyEx(m_hRegKey, dwIndex, lpszSection, &dwSectionLen, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)return 0UL;

	return dwSectionLen;
}

const bool CRegCfgStorage::DeleteSection(LPCTSTR lpszSection)
{
	if(!m_hRegKey)return 0UL;

	// サブキーを削除する
	return (::RegDeleteKey(m_hRegKey, lpszSection) == ERROR_SUCCESS)? true : false;
}

const DWORD CRegCfgStorage::GetItemNum(void)
{
	if(!m_hRegKey)return 0UL;

	DWORD dwItemNum = 0UL;

	// アイテム数を取得する
	if(::RegQueryInfoKey(m_hRegKey, NULL, NULL, NULL, NULL, NULL, NULL, &dwItemNum, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)return 0UL;

	// アイテム数を返す
	return dwItemNum;
}

const DWORD CRegCfgStorage::EnumItem(const DWORD dwIndex, LPTSTR lpszItem)
{
	if(!m_hRegKey)return 0UL;

	DWORD dwValueLen = MAX_VALNAME_LEN;

	// エントリ名を列挙する
	if(::RegEnumValue(m_hRegKey, dwIndex, lpszItem, &dwValueLen, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)return 0UL;

	return dwValueLen;
}

const DWORD CRegCfgStorage::GetItemType(LPCTSTR lpszItem)
{
	if(!m_hRegKey)return 0UL;

	DWORD dwType = REG_NONE;

	// エントリの型を取得する
	if(::RegQueryValueEx(m_hRegKey, lpszItem, NULL, &dwType, NULL, NULL) != ERROR_SUCCESS)return 0UL;

	// BOOLとDWORDは区別できない(ユーザの扱いに委ねられる)
	switch(dwType){
		case REG_DWORD	: return ITEM_TYPE_INT;
		case REG_SZ		: return ITEM_TYPE_TEXT;
		default			: return ITEM_TYPE_INVALID;
		}
}

const bool CRegCfgStorage::DeleteItem(LPCTSTR lpszItem)
{
	if(!m_hRegKey)return 0UL;

	// エントリを削除する
	return (::RegDeleteValue(m_hRegKey, lpszItem))? true : false;
}

const bool CRegCfgStorage::SetBoolItem(LPCTSTR lpszItem, const bool bValue)
{
	if(!m_hRegKey)return 0UL;

	const DWORD dwValue = (bValue)? 1UL : 0UL;

	// BOOLをセーブする
	return (::RegSetValueEx(m_hRegKey, lpszItem, 0UL, REG_DWORD, (const BYTE *)&dwValue, sizeof(dwValue)))? true : false;
}

const bool CRegCfgStorage::SetIntItem(LPCTSTR lpszItem, const DWORD dwValue)
{
	if(!m_hRegKey)return 0UL;

	// DWORDをセーブする
	return (::RegSetValueEx(m_hRegKey, lpszItem, 0UL, REG_DWORD, (const BYTE *)&dwValue, sizeof(dwValue)))? true : false;
}

const bool CRegCfgStorage::SetTextItem(LPCTSTR lpszItem, LPCTSTR lpszValue)
{
	if(!m_hRegKey)return 0UL;

	// 文字列をセーブする
	return (::RegSetValueEx(m_hRegKey, lpszItem, 0UL, REG_SZ, (const BYTE *)lpszValue, (::_tcslen(lpszValue) + 1UL) * sizeof(lpszValue[0])))? true : false;
}

const bool CRegCfgStorage::GetBoolItem(LPCTSTR lpszItem, const bool bDefault)
{
	if(!m_hRegKey)return bDefault;

	DWORD dwType = REG_NONE;
	DWORD dwValue = 0UL;

	// BOOLをロードする
	if(::RegQueryValueEx(m_hRegKey, lpszItem, NULL, &dwType, (BYTE *)&dwValue, NULL) != ERROR_SUCCESS)return bDefault;

	// 型をチェック
	if(dwType != REG_DWORD)return bDefault;

	// 値を返す
	return (dwValue)? true : false;
}

const DWORD CRegCfgStorage::GetIntItem(LPCTSTR lpszItem, const DWORD dwDefault, const DWORD dwMax, const DWORD dwMin)
{
	if(!m_hRegKey)return dwDefault;

	DWORD dwType = REG_NONE;
	DWORD dwValue = 0UL;

	// DWORDをロードする
	if(::RegQueryValueEx(m_hRegKey, lpszItem, NULL, &dwType, (BYTE *)&dwValue, NULL) != ERROR_SUCCESS)return dwDefault;

	// 型をチェック
	if(dwType != REG_DWORD)return dwDefault;

	// 値を返す
	return ((dwValue >= dwMin) && (dwValue <= dwMax))? dwValue : dwDefault;
}

const DWORD CRegCfgStorage::GetTextItem(LPCTSTR lpszItem, LPTSTR lpszValue, LPCTSTR lpszDefault, const DWORD dwMaxLen)
{
	if(!m_hRegKey)return 0UL;

	DWORD dwType = REG_SZ;
	DWORD dwSize = 0xFFFFFFFFUL;

	// サイズを取得
	const LONG lReturn = ::RegQueryValueEx(m_hRegKey, lpszItem, NULL, &dwType, NULL, &dwSize);

	if((lReturn != ERROR_SUCCESS) || (dwType != REG_SZ) || (dwMaxLen && (((dwSize / sizeof(lpszValue[0])) - 1UL) > dwMaxLen))){
		if(lpszDefault){
			if(lpszValue){
				::_tcscpy(lpszValue, lpszDefault);
				}
			return ::_tcslen(lpszDefault);
			}

		return 0UL;
		}

	// 文字列を取得
	if(::RegQueryValueEx(m_hRegKey, lpszItem, NULL, &dwType, (BYTE *)lpszValue, &dwSize) != ERROR_SUCCESS){
		if(lpszDefault){
			if(lpszValue){
				::_tcscpy(lpszValue, lpszDefault);
				}
			return ::_tcslen(lpszDefault);
			}

		return 0UL;
		}

	// 格納した文字数を返す
	return dwSize / sizeof(lpszValue[0]) - 1UL;
}

CRegCfgStorage::CRegCfgStorage(IBonObject *pOwner)
	: CBonObject(pOwner)
	, m_hRegKey(NULL)
{

}

CRegCfgStorage::~CRegCfgStorage(void)
{
	// ストレージをクローズする
	CloseStorage();
}
