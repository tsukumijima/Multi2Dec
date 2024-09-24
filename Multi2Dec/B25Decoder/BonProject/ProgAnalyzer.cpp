// ProgAnalyzer.cpp: �v���O�����A�i���C�U�f�R�[�_
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "TsTable.h"
#include "ProgAnalyzer.h"
#include <TChar.h>


/////////////////////////////////////////////////////////////////////////////
// �v���O�����A�i���C�U�f�R�[�_
/////////////////////////////////////////////////////////////////////////////

const bool CProgAnalyzer::InputMedia(IMediaData *pMediaData, const DWORD dwInputIndex)
{
	// ���̓p�����[�^�`�F�b�N
	ITsPacket * const pTspacket = dynamic_cast<ITsPacket *>(pMediaData);
	if(!pTspacket || (dwInputIndex >= GetInputNum()))return false;

	// PID�}�b�v�ɓ���
	m_TsPidMapper.StorePacket(pTspacket);

	// ���ʃf�R�[�_�Ƀf�[�^�o��
	OutputMedia(pMediaData);

	return true;
}

const WORD CProgAnalyzer::GetNetworkID(void)
{
	// �I���W�i���l�b�g���[�NID��Ԃ�
	return m_ProgContext.wNetworkID;
}

const WORD CProgAnalyzer::GetTsID(void)
{
	// TS ID��Ԃ�
	return m_ProgContext.wTsID;
}

const DWORD CProgAnalyzer::GetProgramNum(void)
{
	// �L���ȃv���O��������Ԃ�
	DWORD dwNum = 0UL;
	
	for(ProgramList::iterator itProgram = m_ProgContext.ProgList.begin() ; itProgram != m_ProgContext.ProgList.end() ; itProgram++){
		if(itProgram->bAvailable)dwNum++;
		}
		
	return dwNum;
}

const WORD CProgAnalyzer::GetProgramID(const DWORD dwIndex)
{
	// �v���O����ID��Ԃ�
	DWORD dwPos = 0UL;
	
	for(ProgramList::iterator itProgram = m_ProgContext.ProgList.begin() ; itProgram != m_ProgContext.ProgList.end() ; itProgram++){
		if(itProgram->bAvailable){
			if(dwPos++ == dwIndex)return itProgram->wProgramID;
			}
		}
		
	return 0x0000U;
}

const WORD CProgAnalyzer::GetProgramVideoPID(const WORD wProgramID)
{
	// �f��PID��Ԃ�
	const PROGRAM_ITEM * pItem = FindProgramItem(wProgramID);
	return (pItem)? pItem->wVideoPID : TS_INVALID_PID;
}

const WORD CProgAnalyzer::GetProgramAudioPID(const WORD wProgramID)
{
	// ����PID��Ԃ�
	const PROGRAM_ITEM * pItem = FindProgramItem(wProgramID);
	return (pItem)? pItem->wAudioPID : TS_INVALID_PID;
}

const WORD CProgAnalyzer::GetProgramPcrPID(const WORD wProgramID)
{
	// PCR��PID��Ԃ�
	const PROGRAM_ITEM * pItem = FindProgramItem(wProgramID);
	return (pItem)? pItem->wPcrPID : TS_INVALID_PID;
}

const DWORD CProgAnalyzer::GetNetworkName(LPTSTR lpszNetworkName)
{
	// �l�b�g���[�N����Ԃ�
	if(lpszNetworkName)::_tcscpy(lpszNetworkName, m_ProgContext.szNetworkName);

	// �o�b�t�@�ɕK�v�ȕ�������Ԃ�
	return ::_tcslen(m_ProgContext.szNetworkName);
}

const DWORD CProgAnalyzer::GetServiceName(LPTSTR lpszServiceName, const WORD wProgramID)
{
	// �T�[�r�X����Ԃ�
	const PROGRAM_ITEM * pItem = FindProgramItem(wProgramID);
	if(!pItem)return 0UL;
	
	// �o�b�t�@�ɃR�s�[
	if(lpszServiceName)::_tcscpy(lpszServiceName, pItem->szServiceName);

	// �o�b�t�@�ɕK�v�ȕ�������Ԃ�
	return ::_tcslen(pItem->szServiceName);
}

const bool CProgAnalyzer::GetStreamTime(SYSTEMTIME *pStreamTime)
{
	// �X�g���[���̎��Ԃ�Ԃ�
	ITotTable *pTotTable = dynamic_cast<ITotTable *>(m_TsPidMapper.GetMapTarget(0x0014U));
	::BON_ASSERT(pTotTable != NULL);
	
	CTsTime StreamTime = pTotTable->GetDateTime();
	if(StreamTime.IsEmpty())return false;

	// ���Ԃ��Z�b�g	
	if(pStreamTime)*pStreamTime = StreamTime;
	
	return true;
}

const WORD CProgAnalyzer::GetCurrentProgramID(void)
{
	// ���݂̃v���O����ID��Ԃ�
	return m_ProgContext.wCurProgramID;
}

const bool CProgAnalyzer::SetCurrentProgramID(const WORD wProgramID)
{
	// ���݂̃v���O����ID��ݒ肷��
	if(wProgramID){
		// �w��v���O����ID�ɕύX
		if(FindProgramItem(wProgramID))return false;
		m_ProgContext.wCurProgramID = wProgramID;
		}
	else{
		// �f�t�H���g�v���O����ID�ɕύX(PAT�̍ŏ��̃v���O����ID)
		if(!m_ProgContext.ProgList.size())return false;
		m_ProgContext.wCurProgramID = m_ProgContext.ProgList[0].wProgramID;
		}

	return true;
}

const IEpgItem * CProgAnalyzer::GetCurrentEpgInfo(const WORD wProgramID)
{
	// ���݂�EPG����Ԃ�
	const PROGRAM_ITEM * pItem = FindProgramItem(wProgramID);
	return (pItem)? pItem->pCurEpgItem : NULL;
}

const IEpgItem * CProgAnalyzer::GetNextEpgInfo(const WORD wProgramID)
{
	// ����EPG����Ԃ�
	const PROGRAM_ITEM * pItem = FindProgramItem(wProgramID);
	return (pItem)? pItem->pNextEpgItem : NULL;
}

CProgAnalyzer::CProgAnalyzer(IBonObject *pOwner)
	: CMediaDecoder(pOwner, 1UL, 1UL)
	, m_TsPidMapper(NULL)
{
	// PAT�e�[�u����PID�}�b�v�ɒǉ�
	m_TsPidMapper.MapPid(0x0000U, dynamic_cast<ITsPidMapTarget *>(CPatTable::CreateInstance(dynamic_cast<IPatTable::IHandler *>(this))));

	// NIT�e�[�u����PID�}�b�v�ɒǉ�
	m_TsPidMapper.MapPid(0x0010U, dynamic_cast<ITsPidMapTarget *>(CNitTable::CreateInstance(dynamic_cast<INitTable::IHandler *>(this))));

	// SDT�e�[�u����PID�}�b�v�ɒǉ�
	m_TsPidMapper.MapPid(0x0011U, dynamic_cast<ITsPidMapTarget *>(CSdtTable::CreateInstance(dynamic_cast<ISdtTable::IHandler *>(this))));

	// EIT�e�[�u����PID�}�b�v�ɒǉ�
	m_TsPidMapper.MapPid(0x0012U, dynamic_cast<ITsPidMapTarget *>(CEitTable::CreateInstance(dynamic_cast<IEitTable::IHandler *>(this))));

	// TOT�e�[�u����PID�}�b�v�ɒǉ�
	m_TsPidMapper.MapPid(0x0014U, dynamic_cast<ITsPidMapTarget *>(CTotTable::CreateInstance(dynamic_cast<ITotTable::IHandler *>(this))));

	// ��ԃ��Z�b�g
	OnReset();
}

CProgAnalyzer::~CProgAnalyzer(void)
{
	// �C���X�^���X�J��
	for(ProgramList::iterator itProgram = m_ProgContext.ProgList.begin() ; itProgram != m_ProgContext.ProgList.end() ; itProgram++){
		BON_SAFE_RELEASE(itProgram->pCurEpgItem);
		BON_SAFE_RELEASE(itProgram->pNextEpgItem);
		}
}

const bool CProgAnalyzer::OnPlay(void)
{
	return OnReset();
}

const bool CProgAnalyzer::OnReset(void)
{
	// �R���e�L�X�g���Z�b�g
	m_ProgContext.wNetworkID = 0x0000U;
	m_ProgContext.wTsID = 0x0000U;
	m_ProgContext.wCurProgramID = 0x0000U;
	m_ProgContext.szNetworkName[0] = TEXT('\0');

	for(ProgramList::iterator itProgram = m_ProgContext.ProgList.begin() ; itProgram != m_ProgContext.ProgList.end() ; itProgram++){
		BON_SAFE_RELEASE(itProgram->pCurEpgItem);
		BON_SAFE_RELEASE(itProgram->pNextEpgItem);
		}

	m_ProgContext.ProgList.clear();

	// PID�}�b�v���Z�b�g
	ResetPidMap();

	// PAT���Z�b�g
	m_TsPidMapper.ResetPid(0x0000U);

	return true;
}

void CProgAnalyzer::OnPatTable(const IPatTable *pPatTable)
{
	// TS ID�̕ω����`�F�b�N����
	if(m_ProgContext.wTsID != pPatTable->GetTsID()){
		// TS ID���ω������Ƃ��͏�Ԃ����Z�b�g����
		ResetPidMap();
		m_ProgContext.wTsID = pPatTable->GetTsID();
		}

	// PMT�e�[�u�����}�b�v
	for(DWORD dwIndex = 0UL ; dwIndex < pPatTable->GetProgramNum() ; dwIndex++){
		m_TsPidMapper.MapPid(pPatTable->GetPmtPID(dwIndex), dynamic_cast<ITsPidMapTarget *>(CPmtTable::CreateInstance(dynamic_cast<IPmtTable::IHandler *>(this))));
		}

	// ���݂��Ȃ��v���O����ID���폜����
	for(ProgramList::iterator itProgram = m_ProgContext.ProgList.begin() ; itProgram != m_ProgContext.ProgList.end() ; ){

		// PAT������
		bool bFound = false;
	
		for(DWORD dwIndex = 0UL ; (dwIndex < pPatTable->GetProgramNum()) && !bFound; dwIndex++){
			if(itProgram->wProgramID == pPatTable->GetProgramID(dwIndex)){
				bFound = true;
				}			
			}

		if(!bFound){
			// �v���O�������X�g����폜
			BON_SAFE_RELEASE(itProgram->pCurEpgItem);
			BON_SAFE_RELEASE(itProgram->pNextEpgItem);

			m_ProgContext.ProgList.erase(itProgram);
			itProgram = m_ProgContext.ProgList.begin();
			}
		else{
			itProgram++;
			}
		}

	// �V�K�̃v���O����ID��ǉ�����
	for(DWORD dwIndex = 0UL ; dwIndex < pPatTable->GetProgramNum() ; dwIndex++){
		if(FindProgramItem(pPatTable->GetProgramID(dwIndex)))continue;

		// �v���O����ID�������ɂȂ�悤�ɑ}��
		ProgramList::iterator itProgram = m_ProgContext.ProgList.begin();

		for( ; itProgram != m_ProgContext.ProgList.end() ; itProgram++){
			if(itProgram->wProgramID > pPatTable->GetProgramID(dwIndex))break;
			}
		
		const PROGRAM_ITEM NewItem =
		{
			false,														// bAvailable
			pPatTable->GetProgramID(dwIndex),							// wProgramID
			TS_INVALID_PID,												// wVideoPID
			TS_INVALID_PID,												// wAudioPID
			TS_INVALID_PID,												// wPcrPID
			TEXT(""),													// szServiceName[]
			dynamic_cast<IEpgItem *>(CEpgItem::CreateInstance(NULL)),	// pCurEpgItem
			dynamic_cast<IEpgItem *>(CEpgItem::CreateInstance(NULL))	// pNextEpgItem
		};

		m_ProgContext.ProgList.insert(itProgram, NewItem);
		}

	// ���݂̃v���O����ID�̕ύX�𔻒肷��
	if(!FindProgramItem(m_ProgContext.wCurProgramID)){
		// �V�K�̃v���O����ID�ɕύX
		m_ProgContext.wCurProgramID = (m_ProgContext.ProgList.size())? m_ProgContext.ProgList[0].wProgramID : 0x0000U;
		}
}

void CProgAnalyzer::OnPmtTable(const IPmtTable *pPmtTable)
{
	// �v���O���������X�V����
	PROGRAM_ITEM *pProgItem = FindProgramItem(pPmtTable->GetProgramID());
	if(!pProgItem)return;

	// PCR��PID�X�V
	pProgItem->wPcrPID = pPmtTable->GetPcrPID();

	// �f��/������PID�X�V
	for(DWORD dwIndex = 0UL ; dwIndex < pPmtTable->GetEsNum() ; dwIndex++){
		switch(pPmtTable->GetStreamTypeID(dwIndex)){
			case 0x02U :	// ITU-T����H.262|ISO/IEC 13818-2�f��
				pProgItem->wVideoPID = pPmtTable->GetEsPID(dwIndex);
				break;
				
			case 0x0FU :	// ISO/IEC 13818-7�����iADTS�g�����X�|�[�g�\���j
				pProgItem->wAudioPID = pPmtTable->GetEsPID(dwIndex);
				break;
			}
		}

	// �L���t���O�X�V
	if((pProgItem->wVideoPID != TS_INVALID_PID) && (pProgItem->wAudioPID != TS_INVALID_PID)){
		// MPEG2�f����AAC����������v���O������L���Ƃ���
		pProgItem->bAvailable = true;
		}
}

void CProgAnalyzer::OnNitTable(const INitTable *pNitTable)
{
	// �l�b�g���[�NID�X�V
	m_ProgContext.wNetworkID = pNitTable->GetNetworkID();

	// �l�b�g���[�N���X�V
	const INetworkNameDesc *pNetworkNameDesc = dynamic_cast<const INetworkNameDesc *>(pNitTable->GetNetworkDesc()->GetDescByTag(INetworkNameDesc::DESC_TAG));
	if(pNetworkNameDesc)pNetworkNameDesc->GetNetworkName(m_ProgContext.szNetworkName);
}

void CProgAnalyzer::OnSdtTable(const ISdtTable *pSdtTable)
{
	// �T�[�r�X�����X�V����
	for(DWORD dwIndex = 0UL ; dwIndex < pSdtTable->GetServiceNum() ; dwIndex++){
		const WORD wServiceID = pSdtTable->GetServiceID(dwIndex);
		PROGRAM_ITEM *pProgItem = FindProgramItem(wServiceID);
		if(!pProgItem)continue;

		// �T�[�r�X���X�V	
		const IServiceDesc *pServiceDesc = dynamic_cast<const IServiceDesc *>(pSdtTable->GetServiceDesc(wServiceID)->GetDescByTag(IServiceDesc::DESC_TAG));
		if(!pServiceDesc)continue;

		pServiceDesc->GetServiceName(pProgItem->szServiceName);
		}
}

void CProgAnalyzer::OnTotTable(const ITotTable *pTotTable)
{
	// �������Ȃ�
}

void CProgAnalyzer::OnEitTable(const IEitTable *pEitTable, const IEitItem *pEitItem)
{
	// EPG�����X�V����
	if(pEitTable->GetOrgNetworkID() != GetNetworkID())return;
	if(pEitTable->GetTsID() != GetTsID())return;
	
	PROGRAM_ITEM *pProgItem = FindProgramItem(pEitTable->GetServiceID());
	if(!pProgItem)return;

	if(pEitTable->GetTableID() == IEitTable::TABLE_ID_CURORNEXT_SELF){
		// ���� or ����EPG���X�g�A
		switch(pEitTable->GetSectionNumber()){
			case 0U :
				if(pProgItem->pCurEpgItem->StoreEitItem(pEitItem)){
					// �ԑg���X�V�C�x���g
//					TCHAR szInfo[4096] = TEXT("����M");
//					pProgItem->pCurEpgItem->GetEventDetail(szInfo);
//					::BON_TRACE(TEXT("Info: %04X\n%s\n\n"), pProgItem->pCurEpgItem->GetEventID(), szInfo);
					}
				break;

			case 1U :
				if(pProgItem->pNextEpgItem->StoreEitItem(pEitItem)){
					// �ԑg���X�V�C�x���g
					}
				break; 

			default :
				// �K�i��͂��肦�Ȃ�
				break;
			}
		}
	else{
		// �X�P�W���[���ς�EPG���X�g�A
		}	
}

void CProgAnalyzer::ResetPidMap(void)
{
	// NIT/SDT/TOT���Z�b�g�A����ȊO�̓A���}�b�v
	for(WORD wPID = 0x0000U ; wPID < 0x2000U ; wPID++){
		switch(wPID){
			case 0x0000 :	// PAT
				break;
			
			case 0x0010 :	// NIT
			case 0x0011 :	// SDT
			case 0x0012 :	// EIT
			case 0x0014 :	// TOT
				// ���Z�b�g
				m_TsPidMapper.ResetPid(wPID);
				break;
			
			default :
				// �A���}�b�v
				m_TsPidMapper.UnmapPid(wPID);
				break;			
			}
		}
}

CProgAnalyzer::PROGRAM_ITEM * CProgAnalyzer::FindProgramItem(WORD wProgramID)
{
	// �v���O����ID�ȗ����͌��݂̃v���O��������������
	if(!wProgramID)wProgramID = GetCurrentProgramID();

	// �v���O����ID����Ή�����A�C�e�����������ĕԂ�
	for(ProgramList::iterator itProgram = m_ProgContext.ProgList.begin() ; itProgram != m_ProgContext.ProgList.end() ; itProgram++){
		if(itProgram->wProgramID == wProgramID){
			return &(*itProgram);
			}
		}

	return NULL;
}
