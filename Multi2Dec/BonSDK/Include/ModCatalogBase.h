// ModCatalogBase.h: ���W���[���J�^���O���N���X
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

/////////////////////////////////////////////////////////////////////////////
// ���W���[���J�^���O���N���X
/////////////////////////////////////////////////////////////////////////////

class CModCatalogBase	:	public CBonObject,
							public IBonModCatalog
{
public:
	struct MOD_CLASS_INFO
	{
		TCHAR szBIId[BON_MAX_CLASS_NAME];	// �C���^�t�F�[�X��
		TCHAR szBCId[BON_MAX_CLASS_NAME];	// �N���X��
		TCHAR szDesc[256];					// �N���X�̐���
	};

// CBonObject
	DECLARE_IBONOBJECT(CModCatalogBase)

// IBonModCatalog
	virtual const WORD GetVersion(void);
	virtual const DWORD GetComment(LPTSTR lpszComment);
	virtual const DWORD GetAuthor(LPTSTR lpszAuthor);
	virtual const DWORD GetWebsite(LPTSTR lpszWebsite);
	virtual const BONGUID EnumClass(LPCTSTR lpszBIId, const DWORD dwIndex);
	virtual const DWORD GetClassDesc(const BONGUID ClassId, LPTSTR lpszDescription);

// CModCatalogBase
	CModCatalogBase(IBonObject *pOwner);
	CModCatalogBase(IBonObject *pOwner, const MOD_CLASS_INFO *pClassInfo, const DWORD dwClassNum, const HMODULE hModule, const LPCTSTR lpszComment, const LPCTSTR lpszAuthor, const LPCTSTR lpszWebsite);
	CModCatalogBase(IBonObject *pOwner, const MOD_CLASS_INFO *pClassInfo, const DWORD dwClassNum, const WORD wVersion, const LPCTSTR lpszComment, const LPCTSTR lpszAuthor, const LPCTSTR lpszWebsite);
	virtual ~CModCatalogBase(void);
	
	const MOD_CLASS_INFO *m_pClassInfo;
	const DWORD m_dwClassNum;
	const HMODULE m_hModule;
	const WORD m_wVersion;
	const LPCTSTR m_lpszComment;
	const LPCTSTR m_lpszAuthor;
	const LPCTSTR m_lpszWebsite;
};
