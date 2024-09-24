// BonClassEnumerator.h: Bonクラス列挙クラス
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Vector>


/////////////////////////////////////////////////////////////////////////////
// Bonクラス列挙クラス
/////////////////////////////////////////////////////////////////////////////

class CBonClassEnumerator :	public CBonObject,
							public IBonClassEnumerator
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CBonClassEnumerator)

// IBonClassEnumerator
	virtual const DWORD EnumBonClass(LPCTSTR lpszBIId);
	virtual const DWORD GetBonClassNum(void);
	virtual const DWORD GetBonModuleName(DWORD dwIndex, LPTSTR lpszModuleName);
	virtual const DWORD GetBonClassName(DWORD dwIndex, LPTSTR lpszClassName);
	virtual const DWORD GetBonClassDesc(DWORD dwIndex, LPTSTR lpszDescription);
	virtual const DWORD GetBonModuleComment(DWORD dwIndex, LPTSTR lpszComment);
	virtual const DWORD GetBonModuleAuthor(DWORD dwIndex, LPTSTR lpszAuthor);
	virtual const DWORD GetBonModuleWebsite(DWORD dwIndex, LPTSTR lpszWebsite);

// CBonClassEnumerator
	CBonClassEnumerator(IBonObject *pOwner);
	virtual ~CBonClassEnumerator(void);

protected:
	void ReleaseCatalog(void);

	struct CLASS_CAT_INFO
	{
		IBonModCatalog *pModCatalog;
		DWORD dwClassIndex;
	};

	TCHAR m_szEnumBIId[BON_MAX_CLASS_NAME];
	std::vector<IBonModCatalog *> m_CatalogList;
	std::vector<CLASS_CAT_INFO> m_ClassList;
};
