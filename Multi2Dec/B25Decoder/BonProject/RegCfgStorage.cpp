// RegCfgStorage.cpp: �ėp�R���t�B�M�����[�V�����X�g���[�W�N���X(���W�X�g����)
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "TChar.h"
#include "RegCfgStorage.h"


/////////////////////////////////////////////////////////////////////////////
// �t�@�C�����[�J���萔�ݒ�
/////////////////////////////////////////////////////////////////////////////

// �f�t�H���g�̃Z�N�V������
#define DEF_PATHNAME		TEXT("Bon")		// /HKEY_CURRENT_USER/Software/Bon/

#define MAX_KEYNAME_LEN		256UL			// �ő�L�[����
#define MAX_VALNAME_LEN		256UL			// �ő�A�C�e������


/////////////////////////////////////////////////////////////////////////////
// �ėp�R���t�B�M�����[�V�����X�g���[�W�N���X(���W�X�g����)
/////////////////////////////////////////////////////////////////////////////

const bool CRegCfgStorage::OpenStorage(LPCTSTR lpszPathName, const bool bCreate)
{
	// ��U�N���[�Y
	CloseStorage();

	// �p�X���𐶐�
	TCHAR szKeyPath[MAX_KEYNAME_LEN] = {TEXT('\0')};
	::_stprintf(szKeyPath, TEXT("Software\\%s"), (lpszPathName)? lpszPathName : DEF_PATHNAME);
	
	// ���W�X�g�����I�[�v������
	if(bCreate){
		// �L�[�����݂��Ȃ��ꍇ�̓L�[���쐬����
		if(::RegCreateKeyEx(HKEY_CURRENT_USER, szKeyPath, 0UL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &m_hRegKey, NULL) != ERROR_SUCCESS)return false;
		}
	else{
		// �L�[�����݂��Ȃ��ꍇ�͎��s����
		if(::RegOpenKeyEx(HKEY_CURRENT_USER, szKeyPath, 0UL, KEY_ALL_ACCESS, &m_hRegKey) != ERROR_SUCCESS)return false;
		}

	return true;
}

const bool CRegCfgStorage::CloseStorage(void)
{
	if(!m_hRegKey)return false;

	// ���W�X�g�������
	::RegCloseKey(m_hRegKey);
	m_hRegKey = NULL;

	return true;
}

IConfigStorage * CRegCfgStorage::OpenSection(LPCTSTR lpszSection, const bool bCreate)
{
	if(!m_hRegKey || !lpszSection)return NULL;

	// �T�u�L�[���I�[�v������
	HKEY hSubKey = NULL;

	if(bCreate){
		// �T�u�L�[�����݂��Ȃ��ꍇ�̓T�u�L�[���쐬����
		if(::RegCreateKeyEx(m_hRegKey, lpszSection, 0UL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hSubKey, NULL) != ERROR_SUCCESS)return NULL;
		}
	else{
		// �T�u�L�[�����݂��Ȃ��ꍇ�͎��s����
		if(::RegOpenKeyEx(m_hRegKey, lpszSection, 0UL, KEY_ALL_ACCESS, &hSubKey) != ERROR_SUCCESS)return NULL;
		}

	// �V�����C���X�^���X����
	CRegCfgStorage *pNewInstance = new CRegCfgStorage(NULL);
	
	if(!pNewInstance){
		::RegCloseKey(hSubKey);
		return NULL;
		}

	// �V�����C���X�^���X�Ƀn���h���ݒ�
	pNewInstance->m_hRegKey = hSubKey;
	
	return pNewInstance;
}

const DWORD CRegCfgStorage::GetSectionNum(void)
{
	if(!m_hRegKey)return 0UL;

	DWORD dwSubKeyNum = 0UL;

	// �T�u�L�[�����擾����
	if(::RegQueryInfoKey(m_hRegKey, NULL, NULL, NULL, &dwSubKeyNum, NULL, NULL, NULL, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)return 0UL;

	// �T�u�L�[����Ԃ�
	return dwSubKeyNum;
}

const DWORD CRegCfgStorage::EnumSection(const DWORD dwIndex, LPTSTR lpszSection)
{
	if(!m_hRegKey)return 0UL;

	DWORD dwSectionLen = MAX_KEYNAME_LEN;

	// �T�u�L�[����񋓂���
	if(::RegEnumKeyEx(m_hRegKey, dwIndex, lpszSection, &dwSectionLen, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)return 0UL;

	return dwSectionLen;
}

const bool CRegCfgStorage::DeleteSection(LPCTSTR lpszSection)
{
	if(!m_hRegKey)return 0UL;

	// �T�u�L�[���폜����
	return (::RegDeleteKey(m_hRegKey, lpszSection) == ERROR_SUCCESS)? true : false;
}

const DWORD CRegCfgStorage::GetItemNum(void)
{
	if(!m_hRegKey)return 0UL;

	DWORD dwItemNum = 0UL;

	// �A�C�e�������擾����
	if(::RegQueryInfoKey(m_hRegKey, NULL, NULL, NULL, NULL, NULL, NULL, &dwItemNum, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)return 0UL;

	// �A�C�e������Ԃ�
	return dwItemNum;
}

const DWORD CRegCfgStorage::EnumItem(const DWORD dwIndex, LPTSTR lpszItem)
{
	if(!m_hRegKey)return 0UL;

	DWORD dwValueLen = MAX_VALNAME_LEN;

	// �G���g������񋓂���
	if(::RegEnumValue(m_hRegKey, dwIndex, lpszItem, &dwValueLen, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)return 0UL;

	return dwValueLen;
}

const DWORD CRegCfgStorage::GetItemType(LPCTSTR lpszItem)
{
	if(!m_hRegKey)return 0UL;

	DWORD dwType = REG_NONE;
	
	// �G���g���̌^���擾����
	if(::RegQueryValueEx(m_hRegKey, lpszItem, NULL, &dwType, NULL, NULL) != ERROR_SUCCESS)return 0UL;

	// BOOL��DWORD�͋�ʂł��Ȃ�(���[�U�̈����Ɉς˂���)
	switch(dwType){
		case REG_DWORD	: return ITEM_TYPE_INT;
		case REG_SZ		: return ITEM_TYPE_TEXT;		
		default			: return ITEM_TYPE_INVALID;
		}
}

const bool CRegCfgStorage::DeleteItem(LPCTSTR lpszItem)
{
	if(!m_hRegKey)return 0UL;
	
	// �G���g�����폜����
	return (::RegDeleteValue(m_hRegKey, lpszItem))? true : false;
}

const bool CRegCfgStorage::SetBoolItem(LPCTSTR lpszItem, const bool bValue)
{
	if(!m_hRegKey)return 0UL;
	
	const DWORD dwValue = (bValue)? 1UL : 0UL;
	
	// BOOL���Z�[�u����
	return (::RegSetValueEx(m_hRegKey, lpszItem, 0UL, REG_DWORD, (const BYTE *)&dwValue, sizeof(dwValue)))? true : false;
}

const bool CRegCfgStorage::SetIntItem(LPCTSTR lpszItem, const DWORD dwValue)
{
	if(!m_hRegKey)return 0UL;

	// DWORD���Z�[�u����
	return (::RegSetValueEx(m_hRegKey, lpszItem, 0UL, REG_DWORD, (const BYTE *)&dwValue, sizeof(dwValue)))? true : false;
}

const bool CRegCfgStorage::SetTextItem(LPCTSTR lpszItem, LPCTSTR lpszValue)
{
	if(!m_hRegKey)return 0UL;

	// ��������Z�[�u����
	return (::RegSetValueEx(m_hRegKey, lpszItem, 0UL, REG_SZ, (const BYTE *)lpszValue, (::_tcslen(lpszValue) + 1UL) * sizeof(lpszValue[0])))? true : false;
}

const bool CRegCfgStorage::GetBoolItem(LPCTSTR lpszItem, const bool bDefault)
{
	if(!m_hRegKey)return bDefault;

	DWORD dwType = REG_NONE;
	DWORD dwValue = 0UL;

	// BOOL�����[�h����
	if(::RegQueryValueEx(m_hRegKey, lpszItem, NULL, &dwType, (BYTE *)&dwValue, NULL) != ERROR_SUCCESS)return bDefault;

	// �^���`�F�b�N
	if(dwType != REG_DWORD)return bDefault;

	// �l��Ԃ�
	return (dwValue)? true : false;
}

const DWORD CRegCfgStorage::GetIntItem(LPCTSTR lpszItem, const DWORD dwDefault, const DWORD dwMax, const DWORD dwMin)
{
	if(!m_hRegKey)return dwDefault;

	DWORD dwType = REG_NONE;
	DWORD dwValue = 0UL;

	// DWORD�����[�h����
	if(::RegQueryValueEx(m_hRegKey, lpszItem, NULL, &dwType, (BYTE *)&dwValue, NULL) != ERROR_SUCCESS)return dwDefault;

	// �^���`�F�b�N
	if(dwType != REG_DWORD)return dwDefault;

	// �l��Ԃ�
	return ((dwValue >= dwMin) && (dwValue <= dwMax))? dwValue : dwDefault;
}

const DWORD CRegCfgStorage::GetTextItem(LPCTSTR lpszItem, LPTSTR lpszValue, LPCTSTR lpszDefault, const DWORD dwMaxLen)
{
	if(!m_hRegKey)return 0UL;

	DWORD dwType = REG_SZ;
	DWORD dwSize = 0xFFFFFFFFUL;
	
	// �T�C�Y���擾
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

	// ��������擾
	if(::RegQueryValueEx(m_hRegKey, lpszItem, NULL, &dwType, (BYTE *)lpszValue, &dwSize) != ERROR_SUCCESS){
		if(lpszDefault){
			if(lpszValue){
				::_tcscpy(lpszValue, lpszDefault);
				}
			return ::_tcslen(lpszDefault);
			}

		return 0UL;
		}
	
	// �i�[������������Ԃ�
	return dwSize / sizeof(lpszValue[0]) - 1UL;
}

CRegCfgStorage::CRegCfgStorage(IBonObject *pOwner)
	: CBonObject(pOwner)
	, m_hRegKey(NULL)
{

}

CRegCfgStorage::~CRegCfgStorage(void)
{
	// �X�g���[�W���N���[�Y����
	CloseStorage();
}
