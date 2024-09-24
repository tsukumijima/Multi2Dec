// BonBaseIf.h: �x�[�X�C���^�t�F�[�X��`
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// Bon�I�u�W�F�N�g���C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IBonObject
{
public:
	virtual void Release(void) = 0;
	virtual const BONGUID GetGuid(void) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// �N���X�t�@�N�g���[���\�b�h�^
/////////////////////////////////////////////////////////////////////////////

typedef IBonObject * (* CLASSFACTORYMETHOD)(IBonObject *pOwner);


/////////////////////////////////////////////////////////////////////////////
// Bon�R�A�G���W���C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IBonCoreEngine : public IBonObject
{
public:
	virtual const BONGUID BonNameToGuid(LPCTSTR lpszName) = 0;
	virtual IBonObject * BonCreateInstance(LPCTSTR lpszClassName, IBonObject *pOwner) = 0;

	virtual const bool RegisterBonModule(LPCTSTR lpszModulePath = NULL) = 0;
	virtual const bool QueryBonModule(LPCTSTR lpszModuleName) = 0;
	virtual const DWORD GetBonModuleNum(void) = 0;
	virtual const BONGUID EnumBonModule(const DWORD dwIndex) = 0;
	virtual const DWORD GetBonModuleName(const BONGUID ModuleId, LPTSTR lpszModuleName) = 0;
	virtual const BONGUID EnumBonModuleClass(const BONGUID ModuleId, const DWORD dwIndex) = 0;

	virtual const bool RegisterBonClass(LPCTSTR lpszClassName, const CLASSFACTORYMETHOD pfnClassFactory, const DWORD dwPriority = 0UL) = 0;
	virtual const bool QueryBonClass(LPCTSTR lpszClassName) = 0;
	virtual const DWORD GetBonClassNum(void) = 0;
	virtual const BONGUID EnumBonClass(const DWORD dwIndex) = 0;
	virtual const DWORD GetBonClassName(const BONGUID ClassId, LPTSTR lpszClassName) = 0;
	virtual const DWORD GetBonClassPriority(const BONGUID ClassId) = 0;
	virtual const BONGUID GetBonClassModule(const BONGUID ClassId) = 0;

	virtual IBonObject * GetStockInstance(LPCTSTR lpszClassName) = 0;
	virtual const bool RegisterStockInstance(LPCTSTR lpszClassName, IBonObject *pInstance) = 0;
	virtual const bool UnregisterStockInstance(LPCTSTR lpszClassName) = 0;
	virtual const DWORD EnumStockInstance(const DWORD dwIndex, LPTSTR lpszClassName) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// Bon���W���[���J�^���O�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IBonModCatalog : public IBonObject
{
public:
	virtual const WORD GetVersion(void) = 0;
	virtual const DWORD GetComment(LPTSTR lpszComment) = 0;
	virtual const DWORD GetAuthor(LPTSTR lpszAuthor) = 0;
	virtual const DWORD GetWebsite(LPTSTR lpszWebsite) = 0;
	virtual const BONGUID EnumClass(LPCTSTR lpszBIId, const DWORD dwIndex) = 0;
	virtual const DWORD GetClassDesc(const BONGUID ClassId, LPTSTR lpszDescription) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// Bon�N���X�񋓃C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IBonClassEnumerator : public IBonObject
{
public:
	virtual const DWORD EnumBonClass(LPCTSTR lpszBIId) = 0;
	virtual const DWORD GetBonClassNum(void) = 0;
	
	virtual const DWORD GetBonModuleName(DWORD dwIndex, LPTSTR lpszModuleName) = 0;
	virtual const DWORD GetBonClassName(DWORD dwIndex, LPTSTR lpszClassName) = 0;
	virtual const DWORD GetBonClassDesc(DWORD dwIndex, LPTSTR lpszDescription) = 0;
	virtual const DWORD GetBonModuleComment(DWORD dwIndex, LPTSTR lpszComment) = 0;
	virtual const DWORD GetBonModuleAuthor(DWORD dwIndex, LPTSTR lpszAuthor) = 0;
	virtual const DWORD GetBonModuleWebsite(DWORD dwIndex, LPTSTR lpszWebsite) = 0;
};
