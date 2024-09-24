// TsEpgCore.cpp: TS EPG �R�A�N���X
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "TsEpgCore.h"
#include <TChar.h>
#include <Vector>


/////////////////////////////////////////////////////////////////////////////
// IEitItem���N���X
/////////////////////////////////////////////////////////////////////////////

const bool CEitItem::ParseItem(void)
{
	// �T�C�Y���`�F�b�N
	if(m_dwDataSize < 12UL)return false;

	// �f�[�^����͂���
	m_wEventID = ((WORD)m_pData[0] << 8) | (WORD)m_pData[1];
	m_StartTime.SetAribTime(&m_pData[2]);
	m_dwDuration = CAribTime::AribBcdToSecond(&m_pData[7]);
	m_byRunningStatus = m_pData[10] >> 5;
	m_bIsScrambled = (m_pData[10] & 0x10U)? true : false;
	
	const WORD wDescLen = ((WORD)(m_pData[10] & 0x0FU) << 8) | (WORD)m_pData[11];
	m_EventDesc.SetData(&m_pData[12], wDescLen);
	m_EventDesc.ParseDescs();

	return true;
}

const WORD CEitItem::GetEventID(void) const
{
	// �C�x���gID��Ԃ�
	return m_wEventID;
}

const CTsTime & CEitItem::GetStartTime(void) const
{
	// �J�n���Ԃ�Ԃ�
	return m_StartTime;
}

const DWORD CEitItem::GetDuration(void) const
{
	// �ԑg����Ԃ�
	return m_dwDuration;
}

const BYTE CEitItem::GetRunningStatus(void) const
{
	// �i�s��Ԃ�Ԃ�
	return m_byRunningStatus;
}

const bool CEitItem::IsScrambled(void) const
{
	// �X�N�����u���t���O��Ԃ�
	return m_bIsScrambled;
}

const IDescBlock * CEitItem::GetEventDesc(void) const
{
	// �L�q�q�u���b�N��Ԃ�
	return &m_EventDesc;
}

const bool CEitItem::CopyItem(const IEitItem *pSrc)
{
	// IMediaData�C���^�t�F�[�X���擾
	const IMediaData * const pMediaData = dynamic_cast<const IMediaData *>(pSrc);
	if(!pMediaData)return false;

	// �A�C�e���f�[�^�R�s�[
	if(CMediaData::CopyData(pMediaData) != GetSize())return false;
	
	// �f�[�^���
	return ParseItem();
}

const DWORD CEitItem::CopyData(const IMediaData *pSrc)
{
	// IPsiSection�C���^�t�F�[�X�擾
	const IEitItem * const pEitItem = dynamic_cast<const IEitItem *>(pSrc);

	if(pEitItem){
		// IEitItem�Ƃ��ăR�s�[
		return (CopyItem(pEitItem))? GetSize() : 0UL;
		}
	else{
		// IMediaData�Ƃ��ăR�s�[
		return CMediaData::CopyData(pSrc);
		}
}

CEitItem::CEitItem(IBonObject *pOwner)
	: CMediaData(25UL)
	, m_wEventID(0x0000U)
	, m_dwDuration(0x00000000UL)
	, m_byRunningStatus(0x00U)
	, m_bIsScrambled(false)
	, m_EventDesc(NULL)
{
	// �������Ȃ�
}

CEitItem::~CEitItem(void)
{
	// �������Ȃ�
}


/////////////////////////////////////////////////////////////////////////////
// IEpgItem���N���X
/////////////////////////////////////////////////////////////////////////////

const BYTE CEpgItem::StoreEitItem(const IEitItem *pEitItem)
{
	if(!pEitItem)return SST_EMPTY;

	// ��{�������
	BYTE byLastState = m_byStoreState;
	m_byStoreState |= StoreEventID(pEitItem);

	// �L�q�q�����
	const IDescBlock *pDescBlock = pEitItem->GetEventDesc();





//	for(DWORD dwIndex = 0UL ; dwIndex < pDescBlock->GetDescNum() ; dwIndex++){
//		const IDescBase *pDesc = pDescBlock->GetDescByIndex(dwIndex);
//		::BON_TRACE(TEXT("[%lu] TAG = %02X, LEN = %u\n"), dwIndex, pDesc->GetTag(), pDesc->GetLength());		
//		}
//
//	::BON_TRACE(TEXT("\n"));







	if(pDescBlock){
		m_byStoreState |= StoreShortDesc(pDescBlock);
		m_byStoreState |= StoreExtendDesc(pDescBlock);
		m_byStoreState |= StoreComponentDesc(pDescBlock);
		m_byStoreState |= StoreAudioComponentDesc(pDescBlock);
		m_byStoreState |= StoreContentDesc(pDescBlock);
		}

	// �X�V���ꂽ����Ԃ�
	return m_byStoreState & (~byLastState);
}

void CEpgItem::Reset(void)
{
	// ��Ԃ����Z�b�g����
	m_byStoreState = SST_EMPTY;
	m_wEventID = 0x0000U;
	m_StartTime.ClearTime();
	m_dwDuration = 0x00000000UL;
	m_bIsScrambled = false;

	m_strEventTitle = TEXT("");
	m_strEventInfo = TEXT("");
	m_strEventDetail = TEXT("");

	m_wVideoResolution = 0U;
	m_bIsVideoProgressive = false;
	m_bIsVideoAspectWide = true;
	m_bIsVideoPanAndScan = false;
	
	m_byAudioMode = ADM_UNDEFINED;
	
	m_GenreArray.clear();
}

const BYTE CEpgItem::GetStoreState(void) const
{
	// �f�[�^�̃X�g�A�i����Ԃ�Ԃ�
	return m_byStoreState;
}

const WORD CEpgItem::GetEventID(void) const
{
	// �C�x���gID��Ԃ�
	return m_wEventID;
}

const CTsTime & CEpgItem::GetStartTime(void) const
{
	// �J�n���Ԃ�Ԃ�
	return m_StartTime;
}

const DWORD CEpgItem::GetDuration(void) const
{
	// �ԑg����Ԃ�
	return m_dwDuration;
}

const bool CEpgItem::IsScrambled(void) const
{
	// �X�N�����u���t���O��Ԃ�
	return m_bIsScrambled;
}

const DWORD CEpgItem::GetEventTitle(LPTSTR lpszDst) const
{
	// �ԑg�̃^�C�g����Ԃ�
	if(lpszDst)::_tcscpy(lpszDst, m_strEventTitle.c_str());

	return m_strEventTitle.length();
}

const DWORD CEpgItem::GetEventInfo(LPTSTR lpszDst) const
{
	// �ԑg�̏���Ԃ�
	if(lpszDst)::_tcscpy(lpszDst, m_strEventInfo.c_str());

	return m_strEventInfo.length();
}

const DWORD CEpgItem::GetEventDetail(LPTSTR lpszDst) const
{
	// �ԑg�̏ڍׂ�Ԃ�
	if(lpszDst)::_tcscpy(lpszDst, m_strEventDetail.c_str());

	return m_strEventDetail.length();
}

const WORD CEpgItem::GetVideoResolution(void) const
{
	// �𑜓x(�������̃s�N�Z����)��Ԃ�
	return m_wVideoResolution;
}

const bool CEpgItem::IsVideoProgressive(void) const
{
	// �v���O���b�V�u�̗L����Ԃ�
	return m_bIsVideoProgressive;
}

const bool CEpgItem::IsVideoAspectWide(void) const
{
	// �A�X�y�N�g��16:9�̗L����Ԃ�
	return m_bIsVideoAspectWide;
}

const bool CEpgItem::IsVideoPanAndScan(void) const
{
	// �p���X�L�����̗L����Ԃ�
	return m_bIsVideoPanAndScan;
}

const BYTE CEpgItem::GetAudioMode(void) const
{
	// �������[�h��Ԃ�
	return m_byAudioMode;
}

const DWORD CEpgItem::GetGenreNum(void) const
{
	// �W����������Ԃ�
	return m_GenreArray.size();
}

const BYTE CEpgItem::GetGenreLevel1(const DWORD dwIndex) const
{
	// �W������1��Ԃ�
	return (dwIndex < m_GenreArray.size())? (m_GenreArray[dwIndex] >> 4) : 0x0FU;
}

const BYTE CEpgItem::GetGenreLevel2(const DWORD dwIndex) const
{
	// �W������2��Ԃ�
	return (dwIndex < m_GenreArray.size())? (m_GenreArray[dwIndex] & 0x0F) : 0x0FU;
}

const bool CEpgItem::CopyItem(const IEpgItem *pSrc)
{
	if(!pSrc)return false;

	// �C���X�^���X���R�s�[����
	m_byStoreState = pSrc->GetStoreState();
	m_wEventID = pSrc->GetEventID();
	m_StartTime = pSrc->GetStartTime();
	m_dwDuration = pSrc->GetDuration();
	m_bIsScrambled = pSrc->IsScrambled();

	m_wVideoResolution = pSrc->GetVideoResolution();
	m_bIsVideoProgressive = pSrc->IsVideoProgressive();
	m_bIsVideoAspectWide = pSrc->IsVideoAspectWide();
	m_bIsVideoPanAndScan = pSrc->IsVideoPanAndScan();

	m_byAudioMode = pSrc->GetAudioMode();

	// �ʓ|���������N���X�Ɉˑ����Ȃ��悤�ɂ���ɂ͂������邵���Ȃ�
	std::vector<TCHAR> szTempString;
	DWORD dwLength;

	// �ԑg�̃^�C�g�����R�s�[
	if(dwLength = pSrc->GetEventTitle(NULL)){
		szTempString.resize(dwLength + 1UL);
		pSrc->GetEventTitle(&szTempString[0]);
		m_strEventTitle = &szTempString[0];
		}
	else{
		m_strEventTitle = TEXT("");
		}
		
	// �ԑg�̏����R�s�[
	if(dwLength = pSrc->GetEventInfo(NULL)){
		szTempString.resize(dwLength + 1UL);
		pSrc->GetEventInfo(&szTempString[0]);
		m_strEventInfo = &szTempString[0];
		}
	else{
		m_strEventInfo = TEXT("");
		}

	// �ԑg�̏ڍׂ��R�s�[				
	if(dwLength = pSrc->GetEventDetail(NULL)){
		szTempString.resize(dwLength + 1UL);
		pSrc->GetEventDetail(&szTempString[0]);
		m_strEventDetail = &szTempString[0];
		}
	else{
		m_strEventDetail = TEXT("");
		}

	// �W���������R�s�[
	const DWORD dwGenreNum = pSrc->GetGenreNum();
	m_GenreArray.resize(dwGenreNum);
	
	for(DWORD dwIndex = 0UL ; dwIndex < dwGenreNum ; dwIndex++){
		m_GenreArray[dwIndex] = (pSrc->GetGenreLevel1() << 4) | (pSrc->GetGenreLevel2() & 0x0FU);
		}

	return true;
}

CEpgItem::CEpgItem(IBonObject *pOwner)
	: CBonObject(pOwner)
{
	Reset();
}

CEpgItem::~CEpgItem(void)
{
	// �������Ȃ�
}

const BYTE CEpgItem::StoreEventID(const IEitItem *pEitItem)
{
	if((m_byStoreState == SST_EMPTY) || (pEitItem->GetEventID() != m_wEventID)){
		// �C�x���gID���m�� or �C�x���gID�ω���
		Reset();

		m_wEventID = pEitItem->GetEventID();
		m_StartTime = pEitItem->GetStartTime();
		m_dwDuration = pEitItem->GetDuration();
		m_bIsScrambled = pEitItem->IsScrambled();

		return SST_EVENT_ID;
		}
	else if((pEitItem->GetStartTime() != m_StartTime) || (pEitItem->GetDuration() != m_dwDuration)){
		// �����X�P�W���[���ω���
		m_StartTime = pEitItem->GetStartTime();
		m_dwDuration = pEitItem->GetDuration();

		return SST_SCHEDULE_CHANGE;
		}
	else{
		// �ω��Ȃ��A�Ȃɂ����Ȃ�
		return SST_EMPTY;
		}
}

const BYTE CEpgItem::StoreShortDesc(const IDescBlock *pDescBlock)
{
	if(m_byStoreState & SST_SHORT_INFO)return SST_EMPTY;

	// �Z�`���C�x���g�L�q�q���X�g�A
	const IShortEventDesc *pShortEventDesc = dynamic_cast<const IShortEventDesc *>(pDescBlock->GetDescByTag(IShortEventDesc::DESC_TAG));
	if(!pShortEventDesc)return SST_EMPTY;

	std::vector<TCHAR> szTempString;
	DWORD dwLength;

	// �ԑg�̃^�C�g�����X�g�A
	if(dwLength = pShortEventDesc->GetEventName(NULL)){
		szTempString.resize(dwLength + 1UL);
		pShortEventDesc->GetEventName(&szTempString[0]);
		m_strEventTitle = &szTempString[0];
		}

	// �ԑg�̏����X�g�A
	if(dwLength = pShortEventDesc->GetEventDesc(NULL)){
		szTempString.resize(dwLength + 1UL);
		pShortEventDesc->GetEventDesc(&szTempString[0]);
		m_strEventInfo = &szTempString[0];
		}

	return SST_SHORT_INFO;
}

const BYTE CEpgItem::StoreExtendDesc(const IDescBlock *pDescBlock)
{
	if(m_byStoreState & SST_EXTEND_INFO)return SST_EMPTY;

	// �g���`���C�x���g�L�q�q���X�g�A
	DWORD dwIndex = 0UL;
	const IExtendEventDesc *pExtendEventDesc;

	TCHAR szTempString[256 * 16];	// �l������ő咷(�}���`�o�C�g�����ł͂����2�{�K�v����) 
	BYTE AribString[256 * 16];		// �l������ő咷
	DWORD dwLength = 0UL;

	while(pExtendEventDesc = dynamic_cast<const IExtendEventDesc *>(pDescBlock->GetDescByTag(IExtendEventDesc::DESC_TAG, dwIndex++))){
		if(dwIndex == 1UL)m_strEventDetail = TEXT("");

		for(DWORD dwItem = 0UL ; dwItem < pExtendEventDesc->GetItemNum() ; dwItem++){
			if(pExtendEventDesc->GetItemName(NULL, dwItem)){
				// �O�A�C�e���̃A�C�e�����e�X�g�A
				if(dwLength){
					if(CAribString::AribToString(szTempString, AribString, dwLength)){
						m_strEventDetail += szTempString;
						m_strEventDetail += TEXT("\r\n\r\n");
						}
						
					dwLength = 0UL;
					}

				// �A�C�e����
				pExtendEventDesc->GetItemName(szTempString, dwItem);
				m_strEventDetail += TEXT("�|");
				m_strEventDetail += szTempString;
				m_strEventDetail += TEXT("�|\r\n");
				}

			// �A�C�e�����e
			dwLength += pExtendEventDesc->GetItemText(&AribString[dwLength], dwItem);
			}
		}

	// �ŏI�A�C�e�����e�X�g�A
	if(dwLength){
		if(CAribString::AribToString(szTempString, AribString, dwLength)){
			m_strEventDetail += szTempString;
			}
		}

	return (dwIndex > 1UL)? SST_EXTEND_INFO : SST_EMPTY;
}

const BYTE CEpgItem::StoreComponentDesc(const IDescBlock *pDescBlock)
{
	if(m_byStoreState & SST_VIDEO_INFO)return SST_EMPTY;

	// �R���|�[�l���g�L�q�q���X�g�A
	DWORD dwIndex = 0UL;
	const IComponentDesc *pComponentDesc;

	while(pComponentDesc = dynamic_cast<const IComponentDesc *>(pDescBlock->GetDescByTag(IComponentDesc::DESC_TAG, dwIndex++))){
		if(pComponentDesc->IsVideoComponent()){
			// �f�������X�g�A
			m_wVideoResolution = pComponentDesc->GetVideoResolution();
			m_bIsVideoProgressive = pComponentDesc->IsVideoProgressive();
			m_bIsVideoAspectWide = pComponentDesc->IsVideoAspectWide();
			m_bIsVideoPanAndScan = pComponentDesc->IsVideoPanAndScan();
			
			return SST_VIDEO_INFO;
			}
		}

	return SST_EMPTY;
}

const BYTE CEpgItem::StoreAudioComponentDesc(const IDescBlock *pDescBlock)
{
	if(m_byStoreState & SST_AUDIO_INFO)return SST_EMPTY;

	// �����R���|�[�l���g�L�q�q���X�g�A
	const IAudioComponentDesc *pAudioComponentDesc = dynamic_cast<const IAudioComponentDesc *>(pDescBlock->GetDescByTag(IAudioComponentDesc::DESC_TAG));
	if(!pAudioComponentDesc)return SST_EMPTY;
	
	switch(pAudioComponentDesc->GetComponentType()){
		case 0x01U :	// 1/0���[�h�i�V���O�����m�j
			m_byAudioMode = ADM_MONO;
			break;
			
		case 0x02U :	// 1/0�{1/0���[�h�i�f���A�����m�j
			m_byAudioMode = ADM_DUAL;
			break;

		case 0x03U :	// 2/0���[�h�i�X�e���I�j
		case 0x05U :	// 3/0���[�h
			m_byAudioMode = ADM_STEREO;
			break;

		case 0x04U :	// 2/1���[�h
		case 0x06U :	// 2/2���[�h
		case 0x07U :	// 3/1���[�h
		case 0x08U :	// 3/2���[�h
		case 0x09U :	// 3/2�{LFE���[�h
			m_byAudioMode = ADM_SURROUND;
			break;

		default :
			m_byAudioMode = ADM_UNDEFINED;
			break;
		}

	return SST_AUDIO_INFO;
}

const BYTE CEpgItem::StoreContentDesc(const IDescBlock *pDescBlock)
{
	if(m_byStoreState & SST_CONTENT_INFO)return SST_EMPTY;

	// �R���e���g�L�q�q���X�g�A
	const IContentDesc *pContentDesc = dynamic_cast<const IContentDesc *>(pDescBlock->GetDescByTag(IContentDesc::DESC_TAG));
	if(!pContentDesc)return SST_EMPTY;

	const DWORD dwGenreNum = pContentDesc->GetGenreNum();
	m_GenreArray.resize(dwGenreNum);
	
	for(DWORD dwIndex = 0UL ; dwIndex < dwGenreNum ; dwIndex++){
		m_GenreArray[dwIndex] = (pContentDesc->GetGenreLevel1() << 4) | (pContentDesc->GetGenreLevel2() & 0x0FU);
		}

	return SST_CONTENT_INFO;
}
