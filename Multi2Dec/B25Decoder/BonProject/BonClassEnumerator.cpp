// BonClassEnumerator.cpp: Bon�N���X�񋓃N���X
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "BonClassEnumerator.h"
#include <TChar.h>


/////////////////////////////////////////////////////////////////////////////
// Bon�N���X�񋓃N���X
/////////////////////////////////////////////////////////////////////////////

const DWORD CBonClassEnumerator::EnumBonClass(LPCTSTR lpszBIId)
{
	if(!lpszBIId)return 0UL;
	if(!lpszBIId[0])return 0UL;

	// ���X�g���N���A
	ReleaseCatalog();

	// �C���^�t�F�[�X���ۑ�
	::_tcscpy(m_szEnumBIId, lpszBIId);

	// �R�A�G���W���C���X�^���X�擾
	IBonCoreEngine *pCoreEngine = dynamic_cast<IBonCoreEngine *>(::GET_STOCK_INSTANCE(TEXT("CBonCoreEngine")));
	::BON_ASSERT(pCoreEngine != NULL);

	DWORD dwModIndex = 0UL;
	TCHAR szModName[BON_MAX_MODULE_NAME] = TEXT("");
	
	// ���W���[����
	while(pCoreEngine->GetBonModuleName(pCoreEngine->EnumBonModule(dwModIndex++), szModName)){
		// ���W���[���J�^���O�C���X�^���X����
		IBonModCatalog *pModCatalog = ::BON_SAFE_CREATE<IBonModCatalog *>(szModName);

		// ���X�g�ɒǉ�
		if(pModCatalog)m_CatalogList.push_back(pModCatalog);
		}

	// �N���X���
	for(dwModIndex = 0UL ; dwModIndex < m_CatalogList.size() ; dwModIndex++){
		DWORD dwClassIndex = 0UL;
		
		while(m_CatalogList[dwModIndex]->EnumClass(lpszBIId, dwClassIndex) != BCID_NULL){
			// �N���X���X�g�ɒǉ�
			CLASS_CAT_INFO NewInfo = {m_CatalogList[dwModIndex], dwClassIndex++};
			m_ClassList.push_back(NewInfo);
			}
		}

	// �w�肳�ꂽ�C���^�t�F�[�X�̃N���X����Ԃ�
	return m_ClassList.size();
}

const DWORD CBonClassEnumerator::GetBonClassNum(void)
{
	// �񋓂��ꂽ�N���X����Ԃ�
	return m_ClassList.size();
}

const DWORD CBonClassEnumerator::GetBonModuleName(DWORD dwIndex, LPTSTR lpszModuleName)
{
	// ���W���[������Ԃ�
	return (dwIndex < m_ClassList.size())? ::GET_BON_CLASS_NAME(m_ClassList[dwIndex].pModCatalog->GetGuid(), lpszModuleName) : 0UL;
}

const DWORD CBonClassEnumerator::GetBonClassName(DWORD dwIndex, LPTSTR lpszClassName)
{
	// �N���X����Ԃ�
	return (dwIndex < m_ClassList.size())? ::GET_BON_CLASS_NAME(m_ClassList[dwIndex].pModCatalog->EnumClass(m_szEnumBIId, m_ClassList[dwIndex].dwClassIndex), lpszClassName) : 0UL;
}

const DWORD CBonClassEnumerator::GetBonClassDesc(DWORD dwIndex, LPTSTR lpszDescription)
{
	// �N���X�̐�����Ԃ�
	return (dwIndex < m_ClassList.size())? m_ClassList[dwIndex].pModCatalog->GetClassDesc(m_ClassList[dwIndex].pModCatalog->EnumClass(m_szEnumBIId, m_ClassList[dwIndex].dwClassIndex), lpszDescription) : 0UL;
}

const DWORD CBonClassEnumerator::GetBonModuleComment(DWORD dwIndex, LPTSTR lpszComment)
{
	// ���W���[���̐�����Ԃ�
	return (dwIndex < m_ClassList.size())? m_ClassList[dwIndex].pModCatalog->GetComment(lpszComment) : 0UL;
}

const DWORD CBonClassEnumerator::GetBonModuleAuthor(DWORD dwIndex, LPTSTR lpszAuthor)
{
	// ���W���[���̍�Җ���Ԃ�
	return (dwIndex < m_ClassList.size())? m_ClassList[dwIndex].pModCatalog->GetAuthor(lpszAuthor) : 0UL;
}

const DWORD CBonClassEnumerator::GetBonModuleWebsite(DWORD dwIndex, LPTSTR lpszWebsite)
{
	// ���W���[���̘A�����Ԃ�
	return (dwIndex < m_ClassList.size())? m_ClassList[dwIndex].pModCatalog->GetWebsite(lpszWebsite) : 0UL;
}

CBonClassEnumerator::CBonClassEnumerator(IBonObject *pOwner)
	: CBonObject(pOwner)
{
	// �������Ȃ�
}

CBonClassEnumerator::~CBonClassEnumerator(void)
{
	// �J�^���O�C���X�^���X�J��
	ReleaseCatalog();
}

void CBonClassEnumerator::ReleaseCatalog(void)
{
	// �C���^�t�F�[�X���N���A
	m_szEnumBIId[0] = TEXT('\0');

	// �N���X���X�g�N���A
	m_ClassList.clear();
	
	// �J�^���O�C���X�^���X�J��
	for(DWORD dwIndex = 0UL ; dwIndex < m_CatalogList.size() ; dwIndex++){
		m_CatalogList[dwIndex]->Release();
		}

	m_CatalogList.clear();
}
