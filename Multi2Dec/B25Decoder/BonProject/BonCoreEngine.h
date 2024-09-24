// BonCoreEngine.h: Bon�R�A�G���W���N���X
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Map>
#include <Vector>
#include <String>


/////////////////////////////////////////////////////////////////////////////
// �}�N����`
/////////////////////////////////////////////////////////////////////////////

#ifdef _UNICODE
	#define BonString std::wstring		// UNICODE
#else
	#define BonString std::string		// char
#endif


/////////////////////////////////////////////////////////////////////////////
// CBonCoreEngine�N���X
/////////////////////////////////////////////////////////////////////////////

class CBonCoreEngine :	public CBonObject,
						public IBonCoreEngine
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CBonCoreEngine)

// IBonCoreEngine
	virtual const BONGUID BonNameToGuid(LPCTSTR lpszName);
	virtual IBonObject * BonCreateInstance(LPCTSTR lpszClassName, IBonObject *pOwner);

	virtual const bool RegisterBonModule(LPCTSTR lpszModulePath = NULL);
	virtual const bool QueryBonModule(LPCTSTR lpszModuleName);
	virtual const DWORD GetBonModuleNum(void);
	virtual const BONGUID EnumBonModule(const DWORD dwIndex);
	virtual const DWORD GetBonModuleName(const BONGUID ModuleId, LPTSTR lpszModuleName);
	virtual const BONGUID EnumBonModuleClass(const BONGUID ModuleId, const DWORD dwIndex);

	virtual const bool RegisterBonClass(LPCTSTR lpszClassName, const CLASSFACTORYMETHOD pfnClassFactory, const DWORD dwPriority = 0UL);
	virtual const bool QueryBonClass(LPCTSTR lpszClassName);
	virtual const DWORD GetBonClassNum(void);
	virtual const BONGUID EnumBonClass(const DWORD dwIndex);
	virtual const DWORD GetBonClassName(const BONGUID ClassId, LPTSTR lpszClassName);
	virtual const DWORD GetBonClassPriority(const BONGUID ClassId);
	virtual const BONGUID GetBonClassModule(const BONGUID ClassId);

	virtual IBonObject * GetStockInstance(LPCTSTR lpszClassName);
	virtual const bool RegisterStockInstance(LPCTSTR lpszClassName, IBonObject *pInstance);
	virtual const bool UnregisterStockInstance(LPCTSTR lpszClassName);
	virtual const DWORD EnumStockInstance(const DWORD dwIndex, LPTSTR lpszClassName);

// CBonCoreEngine
	CBonCoreEngine(IBonObject *pOwner);
	virtual ~CBonCoreEngine(void);

	void Startup(HINSTANCE hInstance);
	void Shutdown(void);

protected:
	static const DWORD CalcCrc(const BYTE *pData, const DWORD dwDataSize);

	// Bon���W���[�����
	struct BON_MODULE_INFO
	{
		BonString strModuleName;				// ���W���[����
		HINSTANCE hModule;						// DLL�C���X�^���X
		std::vector<BONGUID> ClassList;			// �N���X���X�g
	};
	
	// Bon�N���X���
	struct BON_CLASS_INFO
	{
		BonString strClassName;					// �N���X��
		CLASSFACTORYMETHOD pfnClassFactory;		// �N���X�t�@�N�g���[
		DWORD dwPriority;						// �v���C�I���e�B
		BONGUID ModuleId;						// ���W���[����BONGUID
	};

	// Bon�X�g�b�N�C���X�^���X���
	struct BON_INSTANCE_INFO
	{
		BonString strClassName;					// �N���X��
		IBonObject *pInstance;					// �C���X�^���X
	};

	typedef std::map<const BONGUID, BON_MODULE_INFO> BonModuleMap;
	typedef std::map<const BONGUID, BON_CLASS_INFO> BonClassMap;
	typedef std::map<const BONGUID, BON_INSTANCE_INFO> BonInstanceMap;

	BonModuleMap m_ModuleMap;
	BonClassMap m_ClassMap;
	BonInstanceMap m_InstanceMap;

	CSmartLock m_Lock;
	BON_MODULE_INFO m_CurModuleInfo;

	HINSTANCE m_hInstance;
};
