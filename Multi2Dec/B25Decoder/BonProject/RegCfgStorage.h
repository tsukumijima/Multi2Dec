// RegCfgStorage.h: 汎用コンフィギュレーションストレージクラス(レジストリ版)
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// 汎用コンフィギュレーションストレージクラス(レジストリ版)
/////////////////////////////////////////////////////////////////////////////

class CRegCfgStorage :	public CBonObject,
						public IConfigStorage
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CRegCfgStorage)

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

// CRegCfgStorage
	CRegCfgStorage(IBonObject *pOwner);
	virtual ~CRegCfgStorage(void);

protected:
	HKEY m_hRegKey;
};
