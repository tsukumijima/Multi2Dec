// IniCfgStorage.h: 汎用コンフィギュレーションストレージクラス(INIファイル版)
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// 汎用コンフィギュレーションストレージクラス(INIファイル版)
/////////////////////////////////////////////////////////////////////////////

class CIniCfgStorage :	public CBonObject,
						public IConfigStorage
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CIniCfgStorage)

// IConfigStorage
	virtual const bool OpenStorage(LPCTSTR lpszPathName = NULL, const bool bCreate = true);
	virtual const bool CloseStorage(void);

	virtual IConfigStorage * OpenSection(LPCTSTR lpszSection, const bool bCreate = true);
	virtual const DWORD GetSectionNum(void);
	virtual const DWORD EnumSection(const DWORD dwIndex, LPTSTR lpszSection);
	virtual const bool DeleteSection(LPCTSTR lpszSection);

	virtual const DWORD GetItemNum(void);
	virtual const DWORD EnumItem(const DWORD dwIndex, LPTSTR lpszItem);
	virtual const DWORD GetItemType(LPCTSTR lpszItem);
	virtual const bool DeleteItem(LPCTSTR lpszItem);

	virtual const bool SetBoolItem(LPCTSTR lpszItem, const bool bValue);
	virtual const bool SetIntItem(LPCTSTR lpszItem, const DWORD dwValue);
	virtual const bool SetTextItem(LPCTSTR lpszItem, LPCTSTR lpszValue);
	
	virtual const bool GetBoolItem(LPCTSTR lpszItem, const bool bDefault = false);
	virtual const DWORD GetIntItem(LPCTSTR lpszItem, const DWORD dwDefault = 0UL, const DWORD dwMax = 0xFFFFFFFFUL, const DWORD dwMin = 0x00000000UL);
	virtual const DWORD GetTextItem(LPCTSTR lpszItem, LPTSTR lpszValue, LPCTSTR lpszDefault = NULL, const DWORD dwMaxLen = 0UL);

// CIniCfgStorage
	CIniCfgStorage(IBonObject *pOwner);
	virtual ~CIniCfgStorage(void);

protected:
	void GetSectionPath(LPTSTR lpszPath, LPCTSTR lpszSection) const;
	const bool IsSectionExist(LPCTSTR lpszSection) const;
	const bool IsSubSectionPath(LPCTSTR lpszPath) const;
	const bool IsItemExist(LPCTSTR lpszItem) const;

	TCHAR m_szFileName[1024];
	TCHAR m_szSectName[1024];
};
