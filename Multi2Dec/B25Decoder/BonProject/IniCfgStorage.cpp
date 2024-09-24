// IniCfgStorage.cpp: �ėp�R���t�B�M�����[�V�����X�g���[�W�N���X(INI�t�@�C����)
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include <StdLib.h>
#include "TChar.h"
#include "IniCfgStorage.h"


/////////////////////////////////////////////////////////////////////////////
// �t�@�C�����[�J���萔�ݒ�
/////////////////////////////////////////////////////////////////////////////

// �f�t�H���g�̃Z�N�V������
#define ROOT_SECTNAME		TEXT("General")		// [General]

#define MAX_SECTNAME_LEN	256UL				// �ő�Z�N�V��������
#define MAX_SECTNAME_NUM	256UL				// �ő�Z�N�V������
#define MAX_VALNAME_LEN		256UL				// �ő�A�C�e������
#define MAX_VALNAME_NUM		256UL				// �ő�A�C�e����
#define MAX_TEXTVAL_LEN		65536UL				// �ő�e�L�X�g�A�C�e����


/////////////////////////////////////////////////////////////////////////////
// �ėp�R���t�B�M�����[�V�����X�g���[�W�N���X(INI�t�@�C����)
/////////////////////////////////////////////////////////////////////////////

const bool CIniCfgStorage::OpenStorage(LPCTSTR lpszPathName, const bool bCreate)
{
	// ��U�N���[�Y
	CloseStorage();

	// ���s�t�@�C���̃p�X�����擾
	TCHAR szDrive[_MAX_DRIVE + 1] = TEXT("\0");
	TCHAR szDir[_MAX_DIR + 1]	  = TEXT("\0");
	TCHAR szFile[_MAX_FNAME + 1]  = TEXT("\0");

	if(!::GetModuleFileName(NULL, m_szFileName, sizeof(m_szFileName) - 1))return false;
	::_tsplitpath(m_szFileName, szDrive, szDir, szFile, NULL);
	::_tmakepath(m_szFileName, szDrive, szDir, szFile, TEXT("ini"));	

	// �Z�N�V��������ݒ�
	::_tcscpy(m_szSectName, (lpszPathName)? lpszPathName : ROOT_SECTNAME);

	// �Z�N�V�����̑��݂��`�F�b�N
	if(!bCreate){
		if(!IsSectionExist(m_szSectName)){
			// �Z�N�V������������Ȃ�
			CloseStorage();
			return false;
			}
		}

	return true;
}

const bool CIniCfgStorage::CloseStorage(void)
{
	// �t�@�C�����A�Z�N�V���������N���A����
	m_szFileName[0] = TEXT('\0');
	m_szSectName[0] = TEXT('\0');

	return true;
}

IConfigStorage * CIniCfgStorage::OpenSection(LPCTSTR lpszSection, const bool bCreate)
{
	if(!m_szFileName[0] || !lpszSection)return NULL;

	// �Z�N�V�����̑��݂��`�F�b�N
	if(!bCreate){
		// �쐬���[�h�łȂ��Ƃ��̓Z�N�V�������Ȃ���΃G���[
		if(!IsSectionExist(lpszSection))return NULL;
		}

	// �V�����C���X�^���X����
	CIniCfgStorage *pNewInstance = new CIniCfgStorage(NULL);
	if(!pNewInstance)return NULL;
		
	// �t�@�C������ݒ�
	::_tcscpy(pNewInstance->m_szFileName, m_szFileName);

	// �V�����C���X�^���X�ɃZ�N�V�������ݒ�
	GetSectionPath(pNewInstance->m_szSectName, lpszSection);
		
	return pNewInstance;
}

const DWORD CIniCfgStorage::GetSectionNum(void)
{
	if(!m_szFileName[0])return false;

	// �Z�N�V��������񋓂���
	TCHAR szSections[MAX_SECTNAME_LEN * MAX_SECTNAME_NUM] = TEXT("\0");

	const DWORD dwRet = ::GetPrivateProfileSectionNames(szSections, sizeof(szSections) / sizeof(szSections[0]), m_szFileName);
	if(!dwRet || ((dwRet + 2UL) == (sizeof(szSections) / sizeof(szSections[0]))))return false;

	// �Z�N�V���������J�E���g����
	DWORD dwSectNum = 0UL;
	LPCTSTR lpszFindPos = szSections;

	while(*lpszFindPos){
		// �Z�N�V�������C���N�������g
		if(IsSubSectionPath(lpszFindPos))dwSectNum++;
		
		// �����ʒu�X�V
		lpszFindPos += (::_tcslen(lpszFindPos) + 1UL);
		}

	return dwSectNum;
}

const DWORD CIniCfgStorage::EnumSection(const DWORD dwIndex, LPTSTR lpszSection)
{
	if(!m_szFileName[0])return 0UL;

	// �Z�N�V��������񋓂���
	TCHAR szSections[MAX_SECTNAME_LEN * MAX_SECTNAME_NUM] = TEXT("\0");

	const DWORD dwRet = ::GetPrivateProfileSectionNames(szSections, sizeof(szSections) / sizeof(szSections[0]), m_szFileName);
	if(!dwRet || ((dwRet + 2UL) == (sizeof(szSections) / sizeof(szSections[0]))))return 0UL;

	// �Z�N�V���������J�E���g����
	DWORD dwSectNum = 0UL;
	LPCTSTR lpszFindPos = szSections;

	while(*lpszFindPos){
		// �Z�N�V�������C���N�������g
		if(IsSubSectionPath(lpszFindPos)){
			if(dwSectNum == dwIndex){
				// �C���f�b�N�X��v
				if(lpszSection)::_tcscpy(lpszSection, lpszFindPos);
				return ::_tcslen(lpszFindPos);
				}

			dwSectNum++;
			}
		
		// �����ʒu�X�V
		lpszFindPos += (::_tcslen(lpszFindPos) + 1UL);
		}

	return 0UL;
}

const bool CIniCfgStorage::DeleteSection(LPCTSTR lpszSection)
{
	if(!m_szFileName[0] || !lpszSection)return false;

	// �Z�N�V�����̑��݂��`�F�b�N
	if(!IsSectionExist(lpszSection))return false;

	// �Z�N�V�����p�X����
	TCHAR szPath[MAX_SECTNAME_NUM];
	GetSectionPath(szPath, lpszSection);

	// �Z�N�V�������폜
	return (::WritePrivateProfileSection(szPath, NULL, m_szFileName))? true : false;
}

const DWORD CIniCfgStorage::GetItemNum(void)
{
	if(!m_szFileName[0])return 0UL;

	// �A�C�e����񋓂���
	TCHAR szItems[MAX_VALNAME_LEN * MAX_VALNAME_NUM] = TEXT("\0");
	const DWORD dwRet = ::GetPrivateProfileString(m_szSectName, NULL, NULL, szItems, sizeof(szItems) / sizeof(szItems[0]), m_szFileName);

	if(!dwRet || ((dwRet + 2UL) == (sizeof(szItems) / sizeof(szItems[0]))))return 0UL;

	// �A�C�e�������J�E���g����
	DWORD dwItemNum = 0UL;
	LPCTSTR lpszFindPos = szItems;

	while(*lpszFindPos){
		// �A�C�e�����C���N�������g
		dwItemNum++;
		
		// �����ʒu�X�V
		lpszFindPos += (::_tcslen(lpszFindPos) + 1UL);
		}

	// �A�C�e������Ԃ�
	return dwItemNum;
}

const DWORD CIniCfgStorage::EnumItem(const DWORD dwIndex, LPTSTR lpszItem)
{
	if(!m_szFileName[0])return 0UL;

	// �A�C�e����񋓂���
	TCHAR szItems[MAX_VALNAME_LEN * MAX_VALNAME_NUM] = TEXT("\0");
	const DWORD dwRet = ::GetPrivateProfileString(m_szSectName, NULL, NULL, szItems, sizeof(szItems) / sizeof(szItems[0]), m_szFileName);

	if(!dwRet || ((dwRet + 2UL) == (sizeof(szItems) / sizeof(szItems[0]))))return 0UL;

	// �A�C�e�������J�E���g����
	DWORD dwItemNum = 0UL;
	LPTSTR lpszFindPos = szItems;

	while(*lpszFindPos){
		// �A�C�e�����C���N�������g
		if(dwItemNum == dwIndex){
			// �C���f�b�N�X��v
			if(lpszItem)::_tcscpy(lpszItem, lpszFindPos);
			return ::_tcslen(lpszFindPos);
			}		
		
		// �����ʒu�X�V
		lpszFindPos += (::_tcslen(lpszFindPos) + 1UL);
		dwItemNum++;
		}

	return 0UL;
}

const DWORD CIniCfgStorage::GetItemType(LPCTSTR lpszItem)
{
	if(!m_szFileName[0] || !lpszItem)return ITEM_TYPE_INVALID;

	// �A�C�e���^�C�v��Ԃ�(INI�t�@�C���̏ꍇ�͏�Ƀe�L�X�g)
	return (IsItemExist(lpszItem))? ITEM_TYPE_TEXT : ITEM_TYPE_INVALID;
}

const bool CIniCfgStorage::DeleteItem(LPCTSTR lpszItem)
{
	if(!m_szFileName[0] || !lpszItem)return false;

	// �A�C�e�����폜����
	return (::WritePrivateProfileString(m_szSectName, lpszItem, NULL, m_szFileName))? true : false;
}

const bool CIniCfgStorage::SetBoolItem(LPCTSTR lpszItem, const bool bValue)
{
	if(!m_szFileName[0] || !lpszItem)return false;
	
	// BOOL���Z�[�u����
	return (::WritePrivateProfileString(m_szSectName, lpszItem, (bValue)? TEXT("1") : TEXT("0"), m_szFileName))? true : false;
}

const bool CIniCfgStorage::SetIntItem(LPCTSTR lpszItem, const DWORD dwValue)
{
	if(!m_szFileName[0] || !lpszItem)return false;

	TCHAR szValue[16];
	::_stprintf(szValue, TEXT("%lu"), dwValue);

	// DWORD���Z�[�u����
	return (::WritePrivateProfileString(m_szSectName, lpszItem, szValue, m_szFileName))? true : false;
}

const bool CIniCfgStorage::SetTextItem(LPCTSTR lpszItem, LPCTSTR lpszValue)
{
	if(!m_szFileName[0] || !lpszItem || !lpszValue)return false;

	// ��������Z�[�u����
	return (::WritePrivateProfileString(m_szSectName, lpszItem, lpszValue, m_szFileName))? true : false;
}

const bool CIniCfgStorage::GetBoolItem(LPCTSTR lpszItem, const bool bDefault)
{
	if(!m_szFileName[0] || !lpszItem)return false;

	// BOOL�����[�h����
	return (::GetPrivateProfileInt(m_szSectName, lpszItem, (bDefault)? 1UL : 0UL, m_szFileName))? true : false;
}

const DWORD CIniCfgStorage::GetIntItem(LPCTSTR lpszItem, const DWORD dwDefault, const DWORD dwMax, const DWORD dwMin)
{
	if(!m_szFileName[0] || !lpszItem)return false;

	// DWORD�����[�h����
	const DWORD dwValue = (DWORD)::GetPrivateProfileInt(m_szSectName, lpszItem, dwDefault, m_szFileName);
	
	// �͈͂��`�F�b�N
	return ((dwValue >= dwMin) && (dwValue <= dwMax))? dwValue : dwDefault;
}

const DWORD CIniCfgStorage::GetTextItem(LPCTSTR lpszItem, LPTSTR lpszValue, LPCTSTR lpszDefault, const DWORD dwMaxLen)
{
	if(!m_szFileName[0] || !lpszItem)return false;

	TCHAR szValue[MAX_TEXTVAL_LEN] = TEXT("\0");
	if(lpszDefault)::_tcscpy(szValue, lpszDefault);

	// ����������[�h����
	const DWORD dwRet = ::GetPrivateProfileString(m_szSectName, lpszItem, (lpszDefault)? lpszDefault : TEXT(""), szValue, sizeof(szValue) / sizeof(szValue[0]), m_szFileName);

	// ���������`�F�b�N
	if(dwRet > dwMaxLen){
		::_tcscpy(szValue, (lpszDefault)? lpszDefault : TEXT(""));
		}
		
	// �o�b�t�@�ɃR�s�[
	if(lpszValue)::_tcscpy(lpszValue, szValue);
	
	// ��������Ԃ�
	return ::_tcslen(szValue);
}

CIniCfgStorage::CIniCfgStorage(IBonObject *pOwner)
	: CBonObject(pOwner)
{
	m_szFileName[0] = TEXT('\0');
	m_szSectName[0] = TEXT('\0');
}

CIniCfgStorage::~CIniCfgStorage(void)
{
	// �X�g���[�W���N���[�Y����
	CloseStorage();
}

void CIniCfgStorage::GetSectionPath(LPTSTR lpszPath, LPCTSTR lpszSection) const
{
	if(!m_szFileName[0] || !lpszPath || !lpszSection)::DebugBreak();

	// �Z�N�V�����p�X�𐶐�����
	if(!::_tcscmp(m_szSectName, ROOT_SECTNAME)){
		// ���[�g����
		::_tcscpy(lpszPath, lpszSection);
		}
	else{
		// �T�u�Z�N�V�����ȉ�
		::_stprintf(lpszPath, TEXT("%s\\%s"), m_szSectName, lpszSection);
		}
}

const bool CIniCfgStorage::IsSectionExist(LPCTSTR lpszSection) const
{
	if(!m_szFileName[0] || !lpszSection)::DebugBreak();

	// �Z�N�V��������񋓂���
	TCHAR szSections[MAX_SECTNAME_LEN * MAX_SECTNAME_NUM] = TEXT("\0");

	const DWORD dwRet = ::GetPrivateProfileSectionNames(szSections, sizeof(szSections) / sizeof(szSections[0]), m_szFileName);
	if(!dwRet || ((dwRet + 2UL) == (sizeof(szSections) / sizeof(szSections[0]))))return false;

	// �����Z�N�V����������
	TCHAR szFindSection[MAX_SECTNAME_NUM];

	if(!::_tcscmp(m_szSectName, ROOT_SECTNAME)){
		// ���[�g����
		::_tcscpy(szFindSection, lpszSection);
		}
	else{
		// �T�u�Z�N�V�����ȉ�
		::_stprintf(szFindSection, TEXT("%s\\%s"), m_szSectName, lpszSection);
		}

	// �Z�N�V��������������
	LPCTSTR lpszFindPos = szSections;

	while(*lpszFindPos){
		if(!::_tcscmp(lpszFindPos, szFindSection)){
			// �Z�N�V��������v
			return true;
			}
		
		// �����ʒu�X�V
		lpszFindPos += (::_tcslen(lpszFindPos) + 1UL);
		}

	return false;
}

const bool CIniCfgStorage::IsSubSectionPath(LPCTSTR lpszPath) const
{
	if(!m_szFileName[0] || !lpszPath)::DebugBreak();

	if(!::_tcscmp(m_szSectName, ROOT_SECTNAME)){
		// ���[�g����
		if(!::_tcschr(lpszPath, TEXT('\\')))return true;
		}
	else{
		// �T�u�Z�N�V�����ȉ�
		const DWORD dwParentLen = ::_tcslen(m_szSectName) + 1UL;

		if((DWORD)::_tcslen(lpszPath) > dwParentLen){
			// �e�Z�N�V��������蒷��
			if(!::_tcsnicmp(lpszPath, m_szSectName, dwParentLen)){
				// �����Z�N�V�������Ŏn�܂�					
				if(!::_tcschr(&lpszPath[dwParentLen], TEXT('\\')))return true;
				}
			}
		}

	return false;
}

const bool CIniCfgStorage::IsItemExist(LPCTSTR lpszItem) const
{
	if(!m_szFileName[0] || !lpszItem)::DebugBreak();

	// �A�C�e����񋓂���
	TCHAR szItems[MAX_VALNAME_LEN * MAX_VALNAME_NUM] = TEXT("\0");
	const DWORD dwRet = ::GetPrivateProfileString(m_szSectName, NULL, NULL, szItems, sizeof(szItems) / sizeof(szItems[0]), m_szFileName);

	if(!dwRet || ((dwRet + 2UL) == (sizeof(szItems) / sizeof(szItems[0]))))return false;

	// �A�C�e������������
	LPCTSTR lpszFindPos = szItems;

	while(*lpszFindPos){
		if(!::_tcsicmp(lpszFindPos, lpszItem))return true;
		
		// �����ʒu�X�V
		lpszFindPos += (::_tcslen(lpszFindPos) + 1UL);
		}

	return false;
}
