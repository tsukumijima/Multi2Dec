// TsDescriptor.cpp: �f�X�N���v�^�N���X
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "TsDescriptor.h"
#include <TChar.h>


/////////////////////////////////////////////////////////////////////////////
// �����̎����N���X�͋L�q�q�̃f�[�^��ێ������AIDescBlock�C���^�t�F�[�X
// �����N���X���ێ����郁�����u���b�N���Q�Ƃ���B
// ���̂��߂����̃N���X��IDescBlock�C���^�t�F�[�X����擾���Ďg�p���Ȃ����
// �Ȃ�Ȃ��B(�P�ƂŃC���X�^���X�𐶐����邱�Ƃ͂ł��Ȃ�)
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// [0x09] Conditional Access Method �L�q�q�N���X
/////////////////////////////////////////////////////////////////////////////

const WORD CCaMethodDesc::GetCaMethodID(void) const
{
	// �����M����ID��Ԃ�
	return (WORD)m_pDescData[0] << 8 | (WORD)m_pDescData[1];
}

const WORD CCaMethodDesc::GetCaPID(void) const
{
	// �����MPID��Ԃ�
	return (WORD)(m_pDescData[2] & 0x1FU) << 8 | (WORD)m_pDescData[3];
}

const BYTE * CCaMethodDesc::GetPrivateData(void) const
{
	// �v���C�x�[�g�f�[�^��Ԃ�
	return &m_pDescData[4];
}

const BYTE CCaMethodDesc::GetPrivateDataSize(void) const
{
	// �v���C�x�[�g�f�[�^����Ԃ�
	return m_byDescLen - 4U;
}

CCaMethodDesc::CCaMethodDesc(IBonObject *pOwner)
	: CDescBase(pOwner)
{
	// �������Ȃ�
}

CCaMethodDesc::~CCaMethodDesc(void)
{
	// �������Ȃ�
}

const bool CCaMethodDesc::ParseData(void)
{
	// �T�C�Y�̂݃`�F�b�N����
	return (m_byDescLen >= 4U)? true : false;
}


/////////////////////////////////////////////////////////////////////////////
// [0x40] �l�b�g���[�N�� �L�q�q�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

const DWORD CNetworkNameDesc::GetNetworkName(LPTSTR lpszDst) const
{
	// �l�b�g���[�N�������`�F�b�N
	if(!m_byDescLen)return 0UL;

	// �l�b�g���[�N�����擾����
	TCHAR szName[256] = TEXT("\0");
	CAribString::AribToString(szName, &m_pDescData[0], m_byDescLen);

	// �o�b�t�@�ɃR�s�[
	if(lpszDst)::_tcscpy(lpszDst, szName);

	// �o�b�t�@�ɕK�v�ȕ�������Ԃ�
	return ::_tcslen(szName);
}

CNetworkNameDesc::CNetworkNameDesc(IBonObject *pOwner)
	: CDescBase(pOwner)
{
	// �������Ȃ�
}

CNetworkNameDesc::~CNetworkNameDesc(void)
{
	// �������Ȃ�
}

const bool CNetworkNameDesc::ParseData(void)
{
	// �T�C�Y�̂݃`�F�b�N����
	return (m_byDescLen >= 1U)? true : false;
}


/////////////////////////////////////////////////////////////////////////////
// [0x41] �T�[�r�X���X�g �L�q�q�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

const DWORD CServiceListDesc::GetServiceNum(void) const
{
	// �T�[�r�X����Ԃ�
	return m_byDescLen / 3UL;
}

const WORD CServiceListDesc::GetServiceID(const DWORD dwIndex) const
{
	// �T�[�r�XID��Ԃ�
	return (dwIndex < GetServiceNum())? ((WORD)m_pDescData[dwIndex * 3] << 8 | (WORD)m_pDescData[dwIndex * 3 + 1]) : 0x0000U;
}

const BYTE CServiceListDesc::GetServiceType(const DWORD dwIndex) const
{
	// �T�[�r�X�`����ʂ�Ԃ�
	return (dwIndex < GetServiceNum())? m_pDescData[dwIndex * 3 + 2] : 0x00U;
}

CServiceListDesc::CServiceListDesc(IBonObject *pOwner)
	: CDescBase(pOwner)
{
	// �������Ȃ�
}

CServiceListDesc::~CServiceListDesc(void)
{
	// �������Ȃ�
}

const bool CServiceListDesc::ParseData(void)
{
	// �T�C�Y�̂݃`�F�b�N����
	return (m_byDescLen && !(m_byDescLen % 3U))? true : false;
}


/////////////////////////////////////////////////////////////////////////////
// [0x48] Service �L�q�q�N���X
/////////////////////////////////////////////////////////////////////////////

const BYTE CServiceDesc::GetServiceType(void) const
{
	// �T�[�r�X�`��ID��Ԃ�
	return m_pDescData[0];
}

const DWORD CServiceDesc::GetProviderName(LPTSTR lpszDst) const
{
	// ���ƎҖ������`�F�b�N
	const DWORD dwPos = 1UL;
	if(!m_pDescData[dwPos])return 0UL;

	// ���ƎҖ����擾����
	TCHAR szName[256] = TEXT("\0");
	CAribString::AribToString(szName, &m_pDescData[dwPos + 1UL], m_pDescData[dwPos]);

	// �o�b�t�@�ɃR�s�[
	if(lpszDst)::_tcscpy(lpszDst, szName);

	// �o�b�t�@�ɕK�v�ȕ�������Ԃ�
	return ::_tcslen(szName);
}

const DWORD CServiceDesc::GetServiceName(LPTSTR lpszDst) const
{
	// �T�[�r�X�������`�F�b�N
	const DWORD dwPos = (DWORD)m_pDescData[1] + 2UL;
	if(!m_pDescData[dwPos])return 0UL;

	// �T�[�r�X�����擾����
	TCHAR szName[256] = TEXT("\0");
	CAribString::AribToString(szName, &m_pDescData[dwPos + 1UL], m_pDescData[dwPos]);

	// �o�b�t�@�ɃR�s�[
	if(lpszDst)::_tcscpy(lpszDst, szName);

	// �o�b�t�@�ɕK�v�ȕ�������Ԃ�
	return ::_tcslen(szName);
}

CServiceDesc::CServiceDesc(IBonObject *pOwner)
	: CDescBase(pOwner)
{
	// �������Ȃ�
}

CServiceDesc::~CServiceDesc(void)
{
	// �������Ȃ�
}

const bool CServiceDesc::ParseData(void)
{
	// �T�C�Y�̂݃`�F�b�N����
	return (m_byDescLen >= 3U)? true : false;
}


/////////////////////////////////////////////////////////////////////////////
// [0x4D] �Z�`���C�x���g �L�q�q�N���X
/////////////////////////////////////////////////////////////////////////////

const DWORD CShortEventDesc::GetLanguageCode(void) const
{
	// ����R�[�h��Ԃ�
	return ((DWORD)m_pDescData[0] << 16) | ((DWORD)m_pDescData[1] << 8) | (DWORD)m_pDescData[2];
}

const DWORD CShortEventDesc::GetEventName(LPTSTR lpszDst) const
{
	// �ԑg�������`�F�b�N
	const DWORD dwPos = 3UL;
	if(!m_pDescData[dwPos])return 0UL;

	// �ԑg�����擾����
	TCHAR szName[256] = TEXT("\0");
	CAribString::AribToString(szName, &m_pDescData[dwPos + 1UL], m_pDescData[dwPos]);

	// �o�b�t�@�ɃR�s�[
	if(lpszDst)::_tcscpy(lpszDst, szName);

	// �o�b�t�@�ɕK�v�ȕ�������Ԃ�
	return ::_tcslen(szName);
}

const DWORD CShortEventDesc::GetEventDesc(LPTSTR lpszDst) const
{
	// �ԑg�L�q�����`�F�b�N
	const DWORD dwPos = (DWORD)m_pDescData[3] + 4UL;
	if(!m_pDescData[dwPos])return 0UL;

	// �ԑg�L�q���擾����
	TCHAR szName[256] = TEXT("\0");
	CAribString::AribToString(szName, &m_pDescData[dwPos + 1UL], m_pDescData[dwPos]);

	// �o�b�t�@�ɃR�s�[
	if(lpszDst)::_tcscpy(lpszDst, szName);

	// �o�b�t�@�ɕK�v�ȕ�������Ԃ�
	return ::_tcslen(szName);
}

CShortEventDesc::CShortEventDesc(IBonObject *pOwner)
	: CDescBase(pOwner)
{
	// �������Ȃ�
}

CShortEventDesc::~CShortEventDesc(void)
{
	// �������Ȃ�
}

const bool CShortEventDesc::ParseData(void)
{
	// �T�C�Y�̂݃`�F�b�N����
	return (m_byDescLen >= 5U)? true : false;
}


/////////////////////////////////////////////////////////////////////////////
// [0x4E] �g���`���C�x���g �L�q�q�N���X
/////////////////////////////////////////////////////////////////////////////

const BYTE CExtendEventDesc::GetDescNumber(void) const
{
	// �L�q�q�ԍ���Ԃ�
	return m_pDescData[0] >> 4;
}

const BYTE CExtendEventDesc::GetLastDescNumber(void) const
{
	// �ŏI�L�q�q�ԍ���Ԃ�
	return m_pDescData[0] & 0x0FU;
}

const DWORD CExtendEventDesc::GetLanguageCode(void) const
{
	// ����R�[�h��Ԃ�
	return ((DWORD)m_pDescData[1] << 16) | ((DWORD)m_pDescData[2] << 8) | (DWORD)m_pDescData[3];
}

const DWORD CExtendEventDesc::GetItemNum(void) const
{
	// �A�C�e������Ԃ�
	const BYTE *pItem = &m_pDescData[5];
	DWORD dwItem = 0UL;

	for(DWORD dwPos = 0UL ; dwPos < (DWORD)m_pDescData[4] ; dwItem++){
		// �|�C���^�ʒu�X�V
		if((dwPos + (DWORD)pItem[dwPos] + 1U) >= (DWORD)m_pDescData[4])break;
		dwPos += (DWORD)pItem[dwPos] + 1UL;
		dwPos += (DWORD)pItem[dwPos] + 1UL;
		}
		
	return dwItem;
}

const DWORD CExtendEventDesc::GetItemName(LPTSTR lpszDst, const DWORD dwIndex) const
{
	// �A�C�e���|�C���^���擾
	const BYTE *pItem = GetItemPointer(dwIndex);
	if(!pItem)return 0UL;
	if(!pItem[0])return 0UL;

	// �A�C�e������Ԃ�
	TCHAR szName[512] = TEXT("\0");
	CAribString::AribToString(szName, &pItem[1], pItem[0]);

	// �o�b�t�@�ɃR�s�[
	if(lpszDst)::_tcscpy(lpszDst, szName);

	// �o�b�t�@�ɕK�v�ȕ�������Ԃ�
	return ::_tcslen(szName);	
}

const DWORD CExtendEventDesc::GetItemText(BYTE *pDst, const DWORD dwIndex) const
{
	// �A�C�e���|�C���^���擾
	const BYTE *pItem = GetItemPointer(dwIndex);
	if(!pItem)return 0UL;
	
	pItem = &pItem[(DWORD)pItem[0] + 1UL];
	
	if(!pItem[0])return 0UL;

	// �o�b�t�@�ɃR�s�[
	if(pDst)::CopyMemory(pDst, &pItem[1], pItem[0]);

	// �o�b�t�@�ɕK�v�ȃo�C�g����Ԃ�
	return pItem[0];	
}

const DWORD CExtendEventDesc::GetExtDesc(LPTSTR lpszDst) const
{
	const BYTE *pItem = &m_pDescData[5];
	DWORD dwPos = 0UL;

	while(true){
		// �|�C���^�ʒu�X�V
		if(dwPos >= (DWORD)m_pDescData[4]){
			// Description�Ȃ�
			return 0UL;
			}
		else if((dwPos + (DWORD)pItem[dwPos] + 1U) >= (DWORD)m_pDescData[4]){
			// Description����
			pItem = &pItem[dwPos];
			break;
			}
		else{			
			dwPos += (DWORD)pItem[dwPos] + 1UL;
			dwPos += (DWORD)pItem[dwPos] + 1UL;
			}
		}

	// �A�C�e���̃e�L�X�g��Ԃ�
	TCHAR szDesc[256] = TEXT("\0");
	CAribString::AribToString(szDesc, &pItem[1], pItem[0]);

	// �o�b�t�@�ɃR�s�[
	if(lpszDst)::_tcscpy(lpszDst, szDesc);

	// �o�b�t�@�ɕK�v�ȕ�������Ԃ�
	return ::_tcslen(szDesc);	
}

CExtendEventDesc::CExtendEventDesc(IBonObject *pOwner)
	: CDescBase(pOwner)
{
	// �������Ȃ�
}

CExtendEventDesc::~CExtendEventDesc(void)
{
	// �������Ȃ�
}

const bool CExtendEventDesc::ParseData(void)
{
	// �T�C�Y�̂݃`�F�b�N����
	return (m_byDescLen >= 8U)? true : false;
}

const BYTE * CExtendEventDesc::GetItemPointer(const DWORD dwIndex) const
{
	// �w�肵���A�C�e���̐擪�|�C���^��Ԃ�
	const BYTE *pItem = &m_pDescData[5];
	
	for(DWORD dwPos = 0UL, dwItem = 0UL ; dwPos < (DWORD)m_pDescData[4] ; ){
		if((dwPos + (DWORD)pItem[dwPos] + 1U) >= (DWORD)m_pDescData[4])break;

		if(dwItem++ == dwIndex){
			// �C���f�b�N�X���B�A�|�C���^��Ԃ�
			return &pItem[dwPos];
			}
		else{
			// �|�C���^�ʒu�X�V
			dwPos += (DWORD)pItem[dwPos] + 1UL;
			dwPos += (DWORD)pItem[dwPos] + 1UL;
			}
		}
		
	return NULL;
}


/////////////////////////////////////////////////////////////////////////////
// [0x50] Component �L�q�q�N���X
/////////////////////////////////////////////////////////////////////////////

const BYTE CComponentDesc::GetStreamContent(void) const
{
	// �R���|�[�l���g���e��Ԃ�
	return m_pDescData[0] & 0x0FU;
}

const BYTE CComponentDesc::GetComponentType(void) const
{
	// �R���|�[�l���g�^�C�v��Ԃ�
	return m_pDescData[1];
}

const BYTE CComponentDesc::GetComponentTag(void) const
{
	// �R���|�[�l���g�^�O��Ԃ�
	return m_pDescData[2];
}

const DWORD CComponentDesc::GetLanguageCode(void) const
{
	// ����R�[�h��Ԃ�
	return ((WORD)m_pDescData[3] << 16) | ((WORD)m_pDescData[4] << 8) | (WORD)m_pDescData[5];
}

const DWORD CComponentDesc::GetComponentText(LPTSTR lpszDst) const
{
	// �R���|�[�l���g�L�q��Ԃ�
	if(m_byDescLen < 7U)return 0UL;
	
	TCHAR szText[256] = TEXT("\0");
	CAribString::AribToString(szText, &m_pDescData[6], m_byDescLen - 6UL);

	// �o�b�t�@�ɃR�s�[
	if(lpszDst)::_tcscpy(lpszDst, szText);

	// �o�b�t�@�ɕK�v�ȕ�������Ԃ�
	return ::_tcslen(szText);
}

const bool CComponentDesc::IsVideoComponent(void) const
{
	// �R���|�[�l���g���e���f���Ɋւ�����̂��ǂ�����Ԃ�
	return (GetStreamContent() == 0x01U)? true : false;
}

const WORD CComponentDesc::GetVideoResolution(void) const
{
	// �𑜓x(�������̃s�N�Z����)��Ԃ�
	if(!IsVideoComponent())return 0U;
	
	switch(GetComponentType() & 0x0FU){
		case 0x01U :
		case 0x02U :
		case 0x03U :				
		case 0x04U :
			switch(GetComponentType() & 0xF0U){
				case 0xD0U :	// 240p
					return 240U;

				case 0x00U :	// 480i
				case 0xA0U :	// 480p
					return 480U;
					
				case 0xC0U :	// 720p
					return 720U;
				
				case 0xB0U :	// 1080i
					return 1080U;
				
				default :		// ����`�̉𑜓x
					return 0U;
				}
			break;
		
		default :
			// ����`�̉𑜓x
			return 0U;
		}
}

const bool CComponentDesc::IsVideoProgressive(void) const
{
	// �v���O���b�V�u�̗L����Ԃ�
	if(!IsVideoComponent())return false;
	
	switch(GetComponentType() & 0x0FU){
		case 0x01U :
		case 0x02U :
		case 0x03U :				
		case 0x04U :
			switch(GetComponentType() & 0xF0U){
				case 0x00U :	// 480i
				case 0xB0U :	// 1080i
					return false;

				case 0xD0U :	// 240p
				case 0xA0U :	// 480p
				case 0xC0U :	// 720p
					return true;

				default :		// ����`�̉𑜓x
					return false;
				}
			break;
		
		default :
			// ����`�̉𑜓x
			return false;
		}
}

const bool CComponentDesc::IsVideoAspectWide(void) const
{
	// �A�X�y�N�g��16:9�̗L����Ԃ�
	if(!IsVideoComponent())return true;

	switch(GetComponentType() & 0xF0U){
		case 0xD0U :	// 240p
		case 0x00U :	// 480i
		case 0xA0U :	// 480p
		case 0xC0U :	// 720p
		case 0xB0U :	// 1080i
			switch(GetComponentType() & 0x0FU){
				case 0x01U :
					return false;	// 4:3
				
				case 0x02U :
				case 0x03U :				
				case 0x04U :
					return true;	// 16:9
		
				default :
					// ����`�̉𑜓x
					return true;
				}
			break;

		default :	// ����`�̉𑜓x
			return true;
		}
}

const bool CComponentDesc::IsVideoPanAndScan(void) const
{
	// �p���X�L�����̗L����Ԃ�
	if(!IsVideoComponent())return false;

	switch(GetComponentType() & 0xF0U){
		case 0xD0U :	// 240p
		case 0x00U :	// 480i
		case 0xA0U :	// 480p
		case 0xC0U :	// 720p
		case 0xB0U :	// 1080i
			switch(GetComponentType() & 0x0FU){
				case 0x02U :
					return true;	// �p���x�N�g������
				
				case 0x01U :
				case 0x03U :				
				case 0x04U :
					return false;	// �p���x�N�g���Ȃ�
		
				default :
					// ����`�̉𑜓x
					return false;
				}
			break;

		default :	// ����`�̉𑜓x
			return false;
		}
}

CComponentDesc::CComponentDesc(IBonObject *pOwner)
	: CDescBase(pOwner)
{
	// �������Ȃ�
}

CComponentDesc::~CComponentDesc(void)
{
	// �������Ȃ�
}

const bool CComponentDesc::ParseData(void)
{
	// �T�C�Y�̂݃`�F�b�N����
	return (m_byDescLen >= 6U)? true : false;
}


/////////////////////////////////////////////////////////////////////////////
// [0x52] Stream Identifier �L�q�q�N���X
/////////////////////////////////////////////////////////////////////////////

const BYTE CStreamIdDesc::GetComponentTag(void) const
{
	// �R���|�[�l���g�^�O��Ԃ�
	return m_pDescData[0];
}

CStreamIdDesc::CStreamIdDesc(IBonObject *pOwner)
	: CDescBase(pOwner)
{
	// �������Ȃ�
}

CStreamIdDesc::~CStreamIdDesc(void)
{
	// �������Ȃ�
}

const bool CStreamIdDesc::ParseData(void)
{
	// �T�C�Y�̂݃`�F�b�N����
	return (m_byDescLen == 1U)? true : false;
}


/////////////////////////////////////////////////////////////////////////////
// [0x54] Content �L�q�q�N���X
/////////////////////////////////////////////////////////////////////////////

const DWORD CContentDesc::GetGenreNum(void) const
{
	// �W�������̐���Ԃ�
	return m_byDescLen / 2U;
}

const BYTE CContentDesc::GetGenreLevel1(const DWORD dwIndex) const
{
	// �W������1��Ԃ�
	return (dwIndex < GetGenreNum())? (m_pDescData[dwIndex / 2UL + 0UL] >> 4) : 0x0FU;
}

const BYTE CContentDesc::GetGenreLevel2(const DWORD dwIndex) const
{
	// �W������2��Ԃ�
	return (dwIndex < GetGenreNum())? (m_pDescData[dwIndex / 2UL + 0UL] & 0x0F) : 0x0FU;
}

const BYTE CContentDesc::GetUserGenre1(const DWORD dwIndex) const
{
	// ���[�U�W������1��Ԃ�
	return (dwIndex < GetGenreNum())? (m_pDescData[dwIndex / 2UL + 1UL] >> 4) : 0x0FU;
}

const BYTE CContentDesc::GetUserGenre2(const DWORD dwIndex) const
{
	// ���[�U�W������2��Ԃ�
	return (dwIndex < GetGenreNum())? (m_pDescData[dwIndex / 2UL + 1UL] & 0x0F) : 0x0FU;
}

CContentDesc::CContentDesc(IBonObject *pOwner)
	: CDescBase(pOwner)
{
	// �������Ȃ�
}

CContentDesc::~CContentDesc(void)
{
	// �������Ȃ�
}

const bool CContentDesc::ParseData(void)
{
	// �T�C�Y�̂݃`�F�b�N����
	return (m_byDescLen && !(m_byDescLen % 2U))? true : false;
}


/////////////////////////////////////////////////////////////////////////////
// [0xC4] ���� Component �L�q�q�N���X
/////////////////////////////////////////////////////////////////////////////

const BYTE CAudioComponentDesc::GetComponentType(void) const
{
	// �R���|�[�l���g��ʂ�Ԃ�
	return m_pDescData[1];
}

const BYTE CAudioComponentDesc::GetComponentTag(void) const
{
	// �R���|�[�l���g�^�O��Ԃ�
	return m_pDescData[2];
}

const BYTE CAudioComponentDesc::GetStreamType(void) const
{
	// �X�g���[���`����Ԃ�
	return m_pDescData[3];
}

const BYTE CAudioComponentDesc::GetSimulcastGroupTag(void) const
{
	// �T�C�}���L���X�g�O���[�v�^�O��Ԃ�
	return m_pDescData[4];
}

const bool CAudioComponentDesc::IsEsMultiLanguage(void) const
{
	// ES������t���O��Ԃ�
	return (m_pDescData[5] & 0x80U)? true : false;
}

const bool CAudioComponentDesc::IsMainComponent(void) const
{
	// ��R���|�[�l���g�t���O��Ԃ�
	return (m_pDescData[5] & 0x40U)? true : false;
}

const BYTE CAudioComponentDesc::GetQualityIndicator(void) const
{
	// �����\����Ԃ�
	return (m_pDescData[5] >> 4) & 0x03U;
}

const BYTE CAudioComponentDesc::GetSamplingRate(void) const
{
	// �T���v�����O���g����Ԃ�
	return (m_pDescData[5] >> 1) & 0x07U;
}

const DWORD CAudioComponentDesc::GetMainLanguageCode(void) const
{
	// ��1�����̌���R�[�h��Ԃ�
	return ((WORD)m_pDescData[6] << 16) | ((WORD)m_pDescData[7] << 8) | (WORD)m_pDescData[8];
}

const DWORD CAudioComponentDesc::GetSubLanguageCode(void) const
{
	// ��2�����̌���R�[�h��Ԃ�
	if(IsEsMultiLanguage() && (m_byDescLen >= 12U)){
		// ��2��������
		return ((WORD)m_pDescData[9] << 16) | ((WORD)m_pDescData[10] << 8) | (WORD)m_pDescData[11];
		}
	else{
		// ��2�����Ȃ�
		return 0x00000000UL;
		}
}

const DWORD CAudioComponentDesc::GetComponentText(LPTSTR lpszDst) const
{
	// �R���|�[�l���g�L�q��Ԃ�
	TCHAR szText[256] = TEXT("\0");

	if(IsEsMultiLanguage() && (m_byDescLen >= 13U)){
		// ��2��������
		CAribString::AribToString(szText, &m_pDescData[12], m_byDescLen - 12UL);
		}
	else if(m_byDescLen >= 10U){
		// ��2�����Ȃ�
		CAribString::AribToString(szText, &m_pDescData[9], m_byDescLen - 9UL);
		}
	else{
		// �R���|�[�l���g�L�q�Ȃ�
		return 0UL;
		}

	// �o�b�t�@�ɃR�s�[
	if(lpszDst)::_tcscpy(lpszDst, szText);

	// �o�b�t�@�ɕK�v�ȕ�������Ԃ�
	return ::_tcslen(szText);
}

const bool CAudioComponentDesc::IsDualMonoMode(void) const
{
	// ��d�����̗L����Ԃ�
	return (GetComponentType() == 0x02U)? true : false;
}

const bool CAudioComponentDesc::IsEnableCenter(void) const
{
	// �Z���^�[�X�s�[�J�[�̗L��
	switch(GetComponentType()){
		case 0x01U :	// 1/0
		case 0x02U :	// 1/0+1/0
		case 0x05U :	// 3/0
		case 0x07U :	// 3/1
		case 0x08U :	// 3/2
		case 0x09U :	// 3/2+LFE
			return true;
		
		default	:
			return false;		
		}
}

const bool CAudioComponentDesc::IsEnableFront(void) const
{
	// �t�����g���C���X�s�[�J�[�̗L��
	switch(GetComponentType()){
		case 0x03U :	// 2/0		Stereo
		case 0x04U :	// 2/1
		case 0x05U :	// 3/0
		case 0x06U :	// 2/2
		case 0x07U :	// 3/1
		case 0x08U :	// 3/2
		case 0x09U :	// 3/2+LFE
			return true;
		
		default	:
			return false;		
		}
}

const bool CAudioComponentDesc::IsEnableSurround(void) const
{
	// ���A�T���E���h�X�s�[�J�[�̗L��
	switch(GetComponentType()){
		case 0x04U :	// 2/1
		case 0x06U :	// 2/2
		case 0x07U :	// 3/1
		case 0x08U :	// 3/2
		case 0x09U :	// 3/2+LFE
			return true;
		
		default	:
			return false;		
		}
}

const bool CAudioComponentDesc::IsEnableLFE(void) const
{
	// LFE�X�s�[�J�[�̗L��
	switch(GetComponentType()){
		case 0x09U :	// 3/2+LFE
			return true;
		
		default	:
			return false;		
		}
}

CAudioComponentDesc::CAudioComponentDesc(IBonObject *pOwner)
	: CDescBase(pOwner)
{
	// �������Ȃ�
}

CAudioComponentDesc::~CAudioComponentDesc(void)
{
	// �������Ȃ�
}

const bool CAudioComponentDesc::ParseData(void)
{
	// �T�C�Y�̂݃`�F�b�N����
	if(m_byDescLen < 9U){
		// �T�C�Y���s��
		return false;
		}
	else if(IsEsMultiLanguage()){
		// ��2��������
		return (m_byDescLen >= 12U)? true : false;
		}
	else{
		// ��2�����Ȃ�
		return (m_byDescLen >= 9U)? true : false;
		}
}
