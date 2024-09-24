// BonConfigIf.h: コンフィギュレーション関連インタフェース定義
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// コンフィギュレーションストレージインタフェース
/////////////////////////////////////////////////////////////////////////////

class IConfigStorage : public IBonObject
{
public:
	enum	// GetItemType() の戻り値
	{
		ITEM_TYPE_INVALID	= 0x00000000UL,		// 無効な型
		ITEM_TYPE_INT		= 0x00000001UL,		// DWORD
		ITEM_TYPE_TEXT		= 0x00000002UL		// 文字列
	};

	virtual const bool OpenStorage(LPCTSTR lpszPathName = NULL, const bool bCreate = true) = 0;
	virtual const bool CloseStorage(void) = 0;

	virtual IConfigStorage * OpenSection(LPCTSTR lpszSection, const bool bCreate = true) = 0;
	virtual const DWORD GetSectionNum(void) = 0;
	virtual const DWORD EnumSection(const DWORD dwIndex, LPTSTR lpszSection) = 0;
	virtual const bool DeleteSection(LPCTSTR lpszSection) = 0;

	virtual const DWORD GetItemNum(void) = 0;
	virtual const DWORD EnumItem(const DWORD dwIndex, LPTSTR lpszItem) = 0;
	virtual const DWORD GetItemType(LPCTSTR lpszItem) = 0;
	virtual const bool DeleteItem(LPCTSTR lpszItem) = 0;

	virtual const bool SetBoolItem(LPCTSTR lpszItem, const bool bValue) = 0;
	virtual const bool SetIntItem(LPCTSTR lpszItem, const DWORD dwValue) = 0;
	virtual const bool SetTextItem(LPCTSTR lpszItem, LPCTSTR lpszValue) = 0;
	
	virtual const bool GetBoolItem(LPCTSTR lpszItem, const bool bDefault = false) = 0;
	virtual const DWORD GetIntItem(LPCTSTR lpszItem, const DWORD dwDefault = 0UL, const DWORD dwMax = 0xFFFFFFFFUL, const DWORD dwMin = 0x00000000UL) = 0;
	virtual const DWORD GetTextItem(LPCTSTR lpszItem, LPTSTR lpszValue, LPCTSTR lpszDefault = NULL, const DWORD dwMaxLen = 0UL) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// コンフィギュレーションプロパティインタフェース
/////////////////////////////////////////////////////////////////////////////

class IConfigTarget;

class IConfigProperty : public IBonObject
{
public:
	virtual const bool LoadProperty(IConfigStorage *pStorage) = 0;
	virtual const bool SaveProperty(IConfigStorage *pStorage) = 0;
	virtual const bool CopyProperty(const IConfigProperty *pProperty) = 0;

	virtual IConfigTarget * GetConfigTarget(void) = 0;
	virtual const DWORD GetDialogClassName(LPTSTR lpszClassName) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// コンフィギュレーションターゲットインタフェース
/////////////////////////////////////////////////////////////////////////////

class IConfigTarget : public IBonObject
{
public:
	virtual const bool SetProperty(const IConfigProperty *pProperty) = 0;
	virtual const bool GetProperty(IConfigProperty *pProperty) = 0;
	virtual const DWORD GetPropertyClassName(LPTSTR lpszClassName) = 0;
};
