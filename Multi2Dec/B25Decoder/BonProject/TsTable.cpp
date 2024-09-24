// TsTable.cpp: TS�e�[�u�����N���X
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include <TChar.h>
#include "TsTable.h"


/////////////////////////////////////////////////////////////////////////////
// [0x00] PAT�e�[�u�����N���X
/////////////////////////////////////////////////////////////////////////////

void CPatTable::Reset(void)
{
	// ��Ԃ��N���A����
	m_wTsID = 0x0000U;
	
	m_NitPIDArray.clear();
	m_PmtPIDArray.clear();

	CPsiTableBase::Reset();
}

const WORD CPatTable::GetTsID(void) const
{
	// TS ID��Ԃ�
	return m_wTsID;
}

const WORD CPatTable::GetNitPID(const DWORD dwIndex) const
{
	// NIT��PID��Ԃ�
	return (dwIndex < m_NitPIDArray.size())? m_NitPIDArray[dwIndex].wPID : TS_INVALID_PID;
}

const DWORD CPatTable::GetNitNum(void) const
{
	// NIT�̐���Ԃ�
	return m_NitPIDArray.size();
}

const WORD CPatTable::GetPmtPID(const DWORD dwIndex) const
{
	// PMT��PID��Ԃ�
	return (dwIndex < m_PmtPIDArray.size())? m_PmtPIDArray[dwIndex].wPID : TS_INVALID_PID;
}

const WORD CPatTable::GetProgramID(const DWORD dwIndex) const
{
	// Program Number ID��Ԃ�
	return (dwIndex < m_PmtPIDArray.size())? m_PmtPIDArray[dwIndex].wProgramID : 0x0000U;
}

const DWORD CPatTable::GetProgramNum(void) const
{
	// PMT�̐���Ԃ�
	return m_PmtPIDArray.size();
}

CPatTable::CPatTable(IBonObject *pOwner)
	: CPsiTableBase(pOwner)
	, m_pHandler(dynamic_cast<IPatTable::IHandler *>(pOwner))
	, m_wTsID(0UL)
{
	// �������Ȃ�
}

CPatTable::~CPatTable(void)
{
	// �������Ȃ�
}

const bool CPatTable::OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection)
{
	const WORD wDataSize = pNewSection->GetPayloadSize();
	const BYTE *pHexData = pNewSection->GetPayloadData();

	if(wDataSize % 4U)return false;										// �e�[�u���̃T�C�Y���s��
	if(pNewSection->GetTableID() != IPatTable::TABLE_ID)return false;	// �e�[�u��ID���s��

	// PID���N���A����
	m_NitPIDArray.clear();
	m_PmtPIDArray.clear();

	TAG_PATITEM PatItem;

	// �e�[�u������͂���
	m_wTsID = pNewSection->GetTableIdExtension();

	::BON_TRACE(TEXT("\n------- PAT Table -------\nTS ID = %04X\n"), m_wTsID);

	for(WORD wPos = 0U ; wPos < wDataSize ; wPos += 4U, pHexData += 4){
		PatItem.wProgramID	= ((WORD)pHexData[0] << 8) | (WORD)pHexData[1];				// +1,2
		PatItem.wPID		= ((WORD)(pHexData[2] & 0x1FU) << 8) | (WORD)pHexData[3];	// +3,4

		if(!PatItem.wProgramID){
			// NIT��PID
			::BON_TRACE(TEXT("NIT #%u [ID:%04X][PID:%04X]\n"), m_NitPIDArray.size(), PatItem.wProgramID, PatItem.wPID);
			m_NitPIDArray.push_back(PatItem);
			}
		else{
			// PMT��PID
			::BON_TRACE(TEXT("PMT #%u [ID:%04X][PID:%04X]\n"), m_PmtPIDArray.size(), PatItem.wProgramID, PatItem.wPID);
			m_PmtPIDArray.push_back(PatItem);
			}
		}

	// �n���h���֒ʒm
	if(m_pHandler)m_pHandler->OnPatTable(this);

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// [0x01] CAT�e�[�u�����N���X
/////////////////////////////////////////////////////////////////////////////

void CCatTable::Reset(void)
{
	// ��Ԃ��N���A����
	m_DescBlock.Reset();

	CPsiTableBase::Reset();
}

const IDescBlock * CCatTable::GetCatDesc(void) const
{
	// �L�q�q�̈��Ԃ�
	return &m_DescBlock;
}

CCatTable::CCatTable(IBonObject *pOwner)
	: CPsiTableBase(pOwner)
	, m_DescBlock(NULL)
	, m_pHandler(dynamic_cast<ICatTable::IHandler *>(pOwner))
{
	// �������Ȃ�
}

CCatTable::~CCatTable(void)
{
	// �������Ȃ�
}

const bool CCatTable::OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection)
{
	if(pNewSection->GetTableID() != ICatTable::TABLE_ID)return false;	// �e�[�u��ID���s��
	
	// �e�[�u������͂���
	m_DescBlock.SetData(pNewSection->GetPayloadData(), pNewSection->GetPayloadSize());
	m_DescBlock.ParseDescs();

	::BON_TRACE(TEXT("\n------- CAT Table -------\n"));

	for(DWORD dwIndex = 0UL ; dwIndex < m_DescBlock.GetDescNum() ; dwIndex++){
		const IDescBase *pDescBase = m_DescBlock.GetDescByIndex(dwIndex);
		::BON_TRACE(TEXT("[%lu] TAG = 0x%02X LEN = %lu\n"), dwIndex, pDescBase->GetTag(), pDescBase->GetLength());
		}

	// �n���h���֒ʒm
	if(m_pHandler)m_pHandler->OnCatTable(this);

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// [0x02] PMT�e�[�u�����N���X
/////////////////////////////////////////////////////////////////////////////

void CPmtTable::Reset(void)
{
	// ��Ԃ��N���A����
	m_wProgramID = 0x0000U;
	m_wPcrPID = TS_INVALID_PID;
	m_DescBlock.Reset();

	for(DWORD dwIndex = 0UL ; dwIndex < m_EsInfoArray.size() ; dwIndex++){
		BON_SAFE_RELEASE(m_EsInfoArray[dwIndex].pDescBlock);
		}

	m_EsInfoArray.clear();
	
	CPsiTableBase::Reset();
}

const WORD CPmtTable::GetProgramID(void) const
{
	// �����ԑg�ԍ�ID��Ԃ�
	return m_wProgramID;
}

const WORD CPmtTable::GetPcrPID(void) const
{
	// PCR PID��Ԃ�
	return m_wPcrPID;
}

const IDescBlock * CPmtTable::GetPmtDesc(void) const
{
	// �L�q�q�̈�1��Ԃ�
	return &m_DescBlock;
}

const DWORD CPmtTable::GetEsNum(void) const
{
	// �G�������^���X�g���[������Ԃ�
	return m_EsInfoArray.size();
}

const BYTE CPmtTable::GetStreamTypeID(const DWORD dwIndex) const
{
	// �X�g���[���`��ID��Ԃ�
	return (dwIndex < m_EsInfoArray.size())? m_EsInfoArray[dwIndex].byStreamTypeID : 0x00U;
}

const WORD CPmtTable::GetEsPID(const DWORD dwIndex) const
{
	// �G�������^��PID��Ԃ�
	return (dwIndex < m_EsInfoArray.size())? m_EsInfoArray[dwIndex].wEsPID : TS_INVALID_PID;
}

const IDescBlock * CPmtTable::GetEsDesc(const DWORD dwIndex) const
{
	// �L�q�q�̈�2��Ԃ�
	return (dwIndex < m_EsInfoArray.size())? m_EsInfoArray[dwIndex].pDescBlock : NULL;
}

CPmtTable::CPmtTable(IBonObject *pOwner)
	: CPsiTableBase(pOwner)
	, m_wProgramID(0x0000U)
	, m_wPcrPID(TS_INVALID_PID)
	, m_DescBlock(NULL)
	, m_pHandler(dynamic_cast<IPmtTable::IHandler *>(pOwner))
{
	// �������Ȃ�
}

CPmtTable::~CPmtTable(void)
{
	// �C���X�^���X�J��
	Reset();
}

const bool CPmtTable::OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection)
{
	const WORD wDataSize = pNewSection->GetPayloadSize();
	const BYTE *pHexData = pNewSection->GetPayloadData();

	if(pNewSection->GetTableID() != IPmtTable::TABLE_ID)return false;	// �e�[�u��ID���s��
	
	// ES�����N���A����
	for(DWORD dwIndex = 0UL ; dwIndex < m_EsInfoArray.size() ; dwIndex++){
		BON_SAFE_RELEASE(m_EsInfoArray[dwIndex].pDescBlock);
		}

	m_EsInfoArray.clear();
	
	// �e�[�u������͂���
	m_wProgramID = pNewSection->GetTableIdExtension();
	m_wPcrPID = ((WORD)(pHexData[0] & 0x1FU) << 8) | (WORD)pHexData[1];

	WORD wDescLen = ((WORD)(pHexData[2] & 0x0FU) << 8) | (WORD)pHexData[3];
	m_DescBlock.SetData(&pHexData[4], wDescLen);
	m_DescBlock.ParseDescs();

	::BON_TRACE(TEXT("\n------- PMT Table -------\nProgram Number ID = %04X\nPCR PID = %04X\nECM PID = %04X\nDescriptor Field:\n"), m_wProgramID, m_wPcrPID, (m_DescBlock.GetDescByTag(CCaMethodDesc::DESC_TAG))? dynamic_cast<const CCaMethodDesc *>(m_DescBlock.GetDescByTag(CCaMethodDesc::DESC_TAG))->GetCaPID() : TS_INVALID_PID);

	for(DWORD dwIndex = 0UL ; dwIndex < m_DescBlock.GetDescNum() ; dwIndex++){
		const IDescBase *pDescBase = m_DescBlock.GetDescByIndex(dwIndex);
		::BON_TRACE(TEXT("    [%lu] TAG = 0x%02X LEN = %lu\n"), dwIndex, pDescBase->GetTag(), pDescBase->GetLength());
		}

	// �X�g���[�����̊J�n�ʒu���v�Z
	WORD wPos = wDescLen + 4U;
	TAG_PMTITEM PmtItem;

	// ES�������
	while(wPos < wDataSize){
		PmtItem.byStreamTypeID = pHexData[wPos + 0U];													// +0
		PmtItem.wEsPID = ((WORD)(pHexData[wPos + 1U] & 0x1FU) << 8) | (WORD)pHexData[wPos + 2U];		// +1,2	
		wDescLen = ((WORD)(pHexData[wPos + 3U] & 0x0FU) << 8) | (WORD)pHexData[wPos + 4U];				// +3,4

		// �L�q�q�u���b�N
		PmtItem.pDescBlock = dynamic_cast<CDescBlock *>(CDescBlock::CreateInstance(NULL));
		PmtItem.pDescBlock->SetData(&pHexData[wPos + 5U], wDescLen);
		PmtItem.pDescBlock->ParseDescs();

		::BON_TRACE(TEXT("[%u] Stream Type ID = %02X  PID = %04X\n"), m_EsInfoArray.size(), PmtItem.byStreamTypeID, PmtItem.wEsPID);		

		for(DWORD dwIndex = 0UL ; dwIndex < PmtItem.pDescBlock->GetDescNum() ; dwIndex++){
			const IDescBase *pDescBase = PmtItem.pDescBlock->GetDescByIndex(dwIndex);
			::BON_TRACE(TEXT("    [%lu] TAG = 0x%02X LEN = %lu\n"), dwIndex, pDescBase->GetTag(), pDescBase->GetLength());
			}

		// �e�[�u���ɒǉ�����
		m_EsInfoArray.push_back(PmtItem);
		wPos += (wDescLen + 5U);
		}

	// �n���h���֒ʒm
	if(m_pHandler)m_pHandler->OnPmtTable(this);

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// [0x40] NIT�e�[�u�����N���X
/////////////////////////////////////////////////////////////////////////////

void CNitTable::Reset(void)
{
	// ��Ԃ��N���A����
	m_wNetworkID = 0x0000U;
	m_NwDescBlock.Reset();

	for(DWORD dwIndex = 0UL ; dwIndex < m_TsInfoArray.size() ; dwIndex++){
		BON_SAFE_RELEASE(m_TsInfoArray[dwIndex].pDescBlock);
		}

	m_TsInfoArray.clear();
	
	CPsiTableBase::Reset();
}

const WORD CNitTable::GetNetworkID(void) const
{
	// �l�b�g���[�NID��Ԃ�
	return m_wNetworkID;
}

const IDescBlock * CNitTable::GetNetworkDesc(void) const
{
	// �l�b�g���[�N�L�q�q�̈��Ԃ�
	return (m_NwDescBlock.GetSize())? &m_NwDescBlock : NULL;
}

const DWORD CNitTable::GetTsNum(void) const
{
	// TS��񐔂�Ԃ�
	return m_TsInfoArray.size();
}

const BYTE CNitTable::GetTsID(const DWORD dwIndex) const
{
	// TS ID��Ԃ�
	return (dwIndex < m_TsInfoArray.size())? m_TsInfoArray[dwIndex].wTsID : 0x0000U;
}

const BYTE CNitTable::GetOrgNetworkID(const DWORD dwIndex) const
{
	// �I���W�i���l�b�g���[�NID��Ԃ�
	return (dwIndex < m_TsInfoArray.size())? m_TsInfoArray[dwIndex].wOrgNetworkID : 0x0000U;
}

const IDescBlock * CNitTable::GetTsDesc(const DWORD dwIndex) const
{
	// �l�b�g���[�N�L�q�q�̈��Ԃ�
	return (dwIndex < m_TsInfoArray.size())? m_TsInfoArray[dwIndex].pDescBlock : NULL;
}

CNitTable::CNitTable(IBonObject *pOwner)
	: CPsiTableBase(pOwner)
	, m_wNetworkID(0x0000U)
	, m_NwDescBlock(NULL)
	, m_pHandler(dynamic_cast<INitTable::IHandler *>(pOwner))
{
	// �������Ȃ�
}

CNitTable::~CNitTable(void)
{
	// �C���X�^���X�J��
	Reset();
}

const bool CNitTable::OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection)
{
	const WORD wDataSize = pNewSection->GetPayloadSize();
	const BYTE *pHexData = pNewSection->GetPayloadData();

	if(pNewSection->GetTableID() != INitTable::TABLE_ID)return false;	// �e�[�u��ID���s��
	
	// TS�����N���A����
	for(DWORD dwIndex = 0UL ; dwIndex < m_TsInfoArray.size() ; dwIndex++){
		BON_SAFE_RELEASE(m_TsInfoArray[dwIndex].pDescBlock);
		}

	m_TsInfoArray.clear();
	
	// �e�[�u������͂���
	m_wNetworkID = pNewSection->GetTableIdExtension();

	WORD wDescLen = ((WORD)(pHexData[0] & 0x0FU) << 8) | (WORD)pHexData[1];
	m_NwDescBlock.SetData(&pHexData[2], wDescLen);
	m_NwDescBlock.ParseDescs();

	::BON_TRACE(TEXT("\n------- NIT Table -------\nNetwork ID = %04X\n"), m_wNetworkID);
	
	TCHAR szTemp[256];
	::_tcscpy(szTemp, TEXT("Unknown"));
	if(m_NwDescBlock.GetDescByTag(INetworkNameDesc::DESC_TAG)){
		dynamic_cast<const INetworkNameDesc *>(m_NwDescBlock.GetDescByTag(INetworkNameDesc::DESC_TAG))->GetNetworkName(szTemp);
		}
	::BON_TRACE(TEXT("Network Name = %s\n"), szTemp);

	::BON_TRACE(TEXT("Descriptor Field:\n"));
	for(DWORD dwIndex = 0UL ; dwIndex < m_NwDescBlock.GetDescNum() ; dwIndex++){
		const IDescBase *pDescBase = m_NwDescBlock.GetDescByIndex(dwIndex);
		::BON_TRACE(TEXT("    [%lu] TAG = 0x%02X LEN = %lu\n"), dwIndex, pDescBase->GetTag(), pDescBase->GetLength());
		}

	// TS���̊J�n�ʒu���v�Z
	WORD wPos = wDescLen + 4U;
	TAG_NITITEM NitItem;

	// TS�������
	while(wPos < wDataSize){
		NitItem.wTsID = ((WORD)pHexData[wPos + 0U] << 8) | (WORD)pHexData[wPos + 1U];
		NitItem.wOrgNetworkID = ((WORD)pHexData[wPos + 2U] << 8) | (WORD)pHexData[wPos + 3U];

		wDescLen = ((WORD)(pHexData[wPos + 4U] & 0x0FU) << 8) | (WORD)pHexData[wPos + 5U];

		// �L�q�q�u���b�N
		NitItem.pDescBlock = dynamic_cast<CDescBlock *>(CDescBlock::CreateInstance(NULL));
		NitItem.pDescBlock->SetData(&pHexData[wPos + 6U], wDescLen);
		NitItem.pDescBlock->ParseDescs();

		::BON_TRACE(TEXT("[%u] TS ID = %04X  Original Network ID = %04X\n    Descriptor Field:\n"), m_TsInfoArray.size(), NitItem.wTsID, NitItem.wOrgNetworkID);		

		for(DWORD dwIndex = 0UL ; dwIndex < NitItem.pDescBlock->GetDescNum() ; dwIndex++){
			const IDescBase *pDescBase = NitItem.pDescBlock->GetDescByIndex(dwIndex);
			::BON_TRACE(TEXT("        [%lu] TAG = 0x%02X LEN = %lu\n"), dwIndex, pDescBase->GetTag(), pDescBase->GetLength());
			}

		// �e�[�u���ɒǉ�����
		m_TsInfoArray.push_back(NitItem);
		wPos += (wDescLen + 6U);
		}

	// �n���h���֒ʒm
	if(m_pHandler)m_pHandler->OnNitTable(this);

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// [0x42] SDT�e�[�u�����N���X
/////////////////////////////////////////////////////////////////////////////

void CSdtTable::Reset(void)
{
	// ��Ԃ��N���A����
	m_wTsID = 0x0000U;
	m_wOrgNetworkID = 0x0000U;

	for(DWORD dwIndex = 0UL ; dwIndex != m_SdtItemArray.size() ; dwIndex++){
		BON_SAFE_RELEASE(m_SdtItemArray[dwIndex].pDescBlock);
		}

	m_SdtItemArray.clear();
	
	CPsiTableBase::Reset();
}

const WORD CSdtTable::GetTsID(void) const
{
	// TS ID��Ԃ�
	return m_wTsID;
}

const WORD CSdtTable::GetOrgNetworkID(void) const
{
	// �I���W�i���l�b�g���[�NID��Ԃ�
	return m_wOrgNetworkID;
}

const DWORD CSdtTable::GetServiceNum(void) const
{
	// �T�[�r�X����Ԃ�
	return m_SdtItemArray.size();
}

const WORD CSdtTable::GetServiceID(const DWORD dwIndex) const
{
	// �T�[�r�XID��Ԃ�
	return (dwIndex < m_SdtItemArray.size())? m_SdtItemArray[dwIndex].wServiceID : 0x0000U;
}

const bool CSdtTable::IsEitSchedule(const WORD wServiceID) const
{
	// EIT�X�P�W���[���t���O��Ԃ�
	const DWORD dwIndex = GetIndexByServiceID(wServiceID);
	return (dwIndex < m_SdtItemArray.size())? m_SdtItemArray[wServiceID].bIsEitSchedule : false;
}

const bool CSdtTable::IsEitPresentFollowing(const WORD wServiceID) const
{
	// EIT����/���t���O��Ԃ�
	const DWORD dwIndex = GetIndexByServiceID(wServiceID);
	return (dwIndex < m_SdtItemArray.size())? m_SdtItemArray[dwIndex].IsEitPresentFollowing : false;
}

const BYTE CSdtTable::GetRunningStatus(const WORD wServiceID) const
{
	// �i�s��Ԃ�Ԃ�
	const DWORD dwIndex = GetIndexByServiceID(wServiceID);
	return (dwIndex < m_SdtItemArray.size())? m_SdtItemArray[dwIndex].byRunningStatus : ISdtTable::RST_UNDEFINED;
}

const bool CSdtTable::IsScrambling(const WORD wServiceID) const
{
	// �X�N�����u���t���O��Ԃ�
	const DWORD dwIndex = GetIndexByServiceID(wServiceID);
	return (dwIndex < m_SdtItemArray.size())? m_SdtItemArray[dwIndex].bIsScrambling : false;
}

const IDescBlock * CSdtTable::GetServiceDesc(const WORD wServiceID) const
{
	// �L�q�q�̈��Ԃ�
	const DWORD dwIndex = GetIndexByServiceID(wServiceID);
	return (dwIndex < m_SdtItemArray.size())? m_SdtItemArray[dwIndex].pDescBlock : NULL;
}

CSdtTable::CSdtTable(IBonObject *pOwner)
	: CPsiTableBase(pOwner)
	, m_wTsID(0x0000U)
	, m_wOrgNetworkID(0x0000U)
	, m_pHandler(dynamic_cast<ISdtTable::IHandler *>(pOwner))
{
	// �������Ȃ�
}

CSdtTable::~CSdtTable(void)
{
	// �C���X�^���X�J��
	Reset();
}

const bool CSdtTable::OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection)
{
	const WORD wDataSize = pNewSection->GetPayloadSize();
	const BYTE *pHexData = pNewSection->GetPayloadData();

	if(pNewSection->GetTableID() != ISdtTable::TABLE_ID)return false;	// �e�[�u��ID���s��
	
	// �T�[�r�X�����N���A����
	for(DWORD dwIndex = 0UL ; dwIndex != m_SdtItemArray.size() ; dwIndex++){
		BON_SAFE_RELEASE(m_SdtItemArray[dwIndex].pDescBlock);
		}

	m_SdtItemArray.clear();
	
	// �e�[�u������͂���
	m_wTsID = pNewSection->GetTableIdExtension();
	m_wOrgNetworkID = ((WORD)pHexData[0] << 8) | (WORD)pHexData[1];

	::BON_TRACE(TEXT("\n------- SDT Table -------\nTS ID = %04X\nOriginal Network ID = %04X\n"), m_wTsID, m_wOrgNetworkID);

	WORD wPos = 3U;
	TAG_SDTITEM SdtItem;
	
	// �T�[�r�X�������
	while(wPos < wDataSize){
		SdtItem.wServiceID = ((WORD)pHexData[wPos + 0U] << 8) | (WORD)pHexData[wPos + 1U];
		SdtItem.bIsEitSchedule = (pHexData[wPos + 2U] & 0x02U)? true : false;
		SdtItem.IsEitPresentFollowing = (pHexData[wPos + 2U] & 0x01U)? true : false;
		SdtItem.byRunningStatus = (pHexData[wPos + 3U]) >> 5;
		SdtItem.bIsScrambling = (pHexData[wPos + 3U] & 0x10U)? true : false;

		// �L�q�q�u���b�N
		const WORD wDescLen = ((WORD)(pHexData[wPos + 3U] & 0x0FU) << 8) | (WORD)pHexData[wPos + 4U];
		SdtItem.pDescBlock = dynamic_cast<CDescBlock *>(CDescBlock::CreateInstance(NULL));
		SdtItem.pDescBlock->SetData(&pHexData[wPos + 5U], wDescLen);
		SdtItem.pDescBlock->ParseDescs();

		// �e�[�u���ɒǉ�����
		m_SdtItemArray.push_back(SdtItem);
		wPos += (wDescLen + 5U);

		// �ȉ��f�o�b�O�p
		::BON_TRACE(TEXT("\n[%u] Service ID = %04X\n    Eit Schedule = %u\n    Eit Present Following = %u\n    Running Status = %u\n    Scrambling = %u\n"), m_SdtItemArray.size() - 1U, SdtItem.wServiceID, SdtItem.bIsEitSchedule, SdtItem.IsEitPresentFollowing, SdtItem.byRunningStatus, SdtItem.bIsScrambling);		

		TCHAR szTemp[256];
		::_tcscpy(szTemp, TEXT("Unknown"));
		if(SdtItem.pDescBlock->GetDescByTag(IServiceDesc::DESC_TAG)){
			dynamic_cast<const IServiceDesc *>(SdtItem.pDescBlock->GetDescByTag(IServiceDesc::DESC_TAG))->GetProviderName(szTemp);
			}
		::BON_TRACE(TEXT("    Provider Name = %s\n"), szTemp);

		::_tcscpy(szTemp, TEXT("Unknown"));
		if(SdtItem.pDescBlock->GetDescByTag(IServiceDesc::DESC_TAG)){
			dynamic_cast<const IServiceDesc *>(SdtItem.pDescBlock->GetDescByTag(IServiceDesc::DESC_TAG))->GetServiceName(szTemp);
			}
		::BON_TRACE(TEXT("    Service Name = %s\n"), szTemp);

		for(DWORD dwIndex = 0UL ; dwIndex < SdtItem.pDescBlock->GetDescNum() ; dwIndex++){
			const IDescBase *pDescBase = SdtItem.pDescBlock->GetDescByIndex(dwIndex);
			::BON_TRACE(TEXT("        [%lu] TAG = 0x%02X LEN = %lu\n"), dwIndex, pDescBase->GetTag(), pDescBase->GetLength());
			}
		}

	// �n���h���֒ʒm
	if(m_pHandler)m_pHandler->OnSdtTable(this);

	return true;
}

const DWORD CSdtTable::GetIndexByServiceID(const WORD wServiceID) const
{
	// �T�[�r�XID����C���f�b�N�X���擾����
	DWORD dwIndex;
	
	for(dwIndex = 0UL ; dwIndex < m_SdtItemArray.size() ; dwIndex++){
		if(m_SdtItemArray[dwIndex].wServiceID == wServiceID)break;
		}

	return dwIndex;
}


/////////////////////////////////////////////////////////////////////////////
// [0x4E-0x6F] EIT�e�[�u�����N���X
/////////////////////////////////////////////////////////////////////////////

void CEitTable::Reset(void)
{
	// ��Ԃ��N���A����
	m_EitItem.ClearSize();
	
	CPsiTableBase::Reset();
}

const WORD CEitTable::GetServiceID(void) const
{
	// �T�[�r�XID��Ԃ�
	return m_LastSection.GetTableIdExtension();
}

const WORD CEitTable::GetTsID(void) const
{
	// TS ID��Ԃ�
	if(m_LastSection.GetPayloadSize() < 6U)return 0x0000U;

	const BYTE *pHexData = m_LastSection.GetPayloadData();
	return ((WORD)pHexData[0] << 8) | (WORD)pHexData[1];
}

const WORD CEitTable::GetOrgNetworkID(void) const
{
	// Original Network ID��Ԃ�
	if(m_LastSection.GetPayloadSize() < 6U)return 0x0000U;

	const BYTE *pHexData = m_LastSection.GetPayloadData();
	return ((WORD)pHexData[2] << 8) | (WORD)pHexData[3];
}

const BYTE CEitTable::GetTableID(void) const
{
	// �e�[�u��ID��Ԃ�
	return m_LastSection.GetTableID();
}

const BYTE CEitTable::GetLastTableID(void) const
{
	// �ŏI�e�[�u��ID��Ԃ�
	if(m_LastSection.GetPayloadSize() < 6U)return 0x00U;

	const BYTE *pHexData = m_LastSection.GetPayloadData();
	return pHexData[5];
}

const BYTE CEitTable::GetSectionNumber(void) const
{
	// �Z�N�V�����ԍ���Ԃ�
	return m_LastSection.GetSectionNumber();
}

const BYTE CEitTable::GetLastSectionNumber(void) const
{
	// �ŏI�Z�N�V�����ԍ���Ԃ�
	return m_LastSection.GetLastSectionNumber();
}

const BYTE CEitTable::GetSegmentLastSectionNumber(void) const
{
	// �Z�O�����g�ŏI�Z�N�V�����ԍ���Ԃ�
	if(m_LastSection.GetPayloadSize() < 6U)return 0x00U;

	const BYTE *pHexData = m_LastSection.GetPayloadData();
	return pHexData[4];
}

CEitTable::CEitTable(IBonObject *pOwner)
	: CPsiTableBase(pOwner)
	, m_pHandler(dynamic_cast<IEitTable::IHandler *>(pOwner))
	, m_EitItem(NULL)
{
	// �������Ȃ�
}

CEitTable::~CEitTable(void)
{
	// �������Ȃ�
}

void CEitTable::OnPsiSection(const IPsiSectionParser *pPsiSectionParser, const IPsiSection *pPsiSection)
{
	// CPsiTableBase�̎������I�[�o�[���C�h����
	if(m_LastSection.CompareSection(pPsiSection)){
		// �Z�N�V�������X�V���ꂽ
		OnTableUpdate(pPsiSection, &m_LastSection);
		}
}

const bool CEitTable::OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection)
{
	const WORD wDataSize = pNewSection->GetPayloadSize();
	const BYTE *pHexData = pNewSection->GetPayloadData();

	if(pNewSection->GetTableID() < IEitTable::TABLE_ID_START)return false;	// �e�[�u��ID���s��
	if(pNewSection->GetTableID() > IEitTable::TABLE_ID_END)return false;	// �e�[�u��ID���s��
	if(wDataSize < 18U)return false;										// �y�C���[�h�����s��

	// �Z�N�V�����X�g�A
	m_LastSection.CopySection(pNewSection);

	// �A�C�e�����Ƀn���h���ɒʒm
	if(!m_pHandler)return true;
	
	for(WORD wPos = 6U ; (wPos < wDataSize) && ((wDataSize - wPos) >= 12U) ; ){
		// �A�C�e���T�C�Y�v�Z
		const WORD wItemLen = (((WORD)(pHexData[wPos + 10] & 0x0FU) << 8) | (WORD)pHexData[wPos + 11]) + 12U;
		m_EitItem.SetData(&pHexData[wPos], wItemLen);
		wPos += wItemLen;
		
		// �n���h���֒ʒm
		if(m_EitItem.ParseItem()){
			m_pHandler->OnEitTable(this, &m_EitItem);
			}
		}	

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// [0x73] TOT�e�[�u�����N���X
/////////////////////////////////////////////////////////////////////////////

void CTotTable::Reset(void)
{
	// ��Ԃ��N���A����
	m_DateTime.ClearTime();
	m_DescBlock.Reset();
	
	CPsiTableBase::Reset();
}

const SYSTEMTIME & CTotTable::GetDateTime(void) const
{
	// ���ݓ��t/������Ԃ�
	return m_DateTime;
}

const IDescBlock * CTotTable::GetTotDesc(void) const
{
	// �L�q�q�̈��Ԃ�
	return &m_DescBlock;
}

CTotTable::CTotTable(IBonObject *pOwner)
	: CPsiTableBase(pOwner)
	, m_DescBlock(NULL)
	, m_pHandler(dynamic_cast<ITotTable::IHandler *>(pOwner))
{
	// �W���`���Z�N�V������I��
	m_SectionParser.SetExtSection(false);
}

CTotTable::~CTotTable(void)
{
	// �������Ȃ�
}

const bool CTotTable::OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection)
{
	const WORD wDataSize = pNewSection->GetPayloadSize();
	const BYTE *pHexData = pNewSection->GetPayloadData();

	if(pNewSection->GetTableID() != ITotTable::TABLE_ID)return false;	// �e�[�u��ID���s��

	// �e�[�u������͂���
	m_DateTime.SetAribTime(pHexData);
	
	const WORD wDescLen = ((WORD)(pHexData[5] & 0x0FU) << 8) | (WORD)pHexData[6];
	m_DescBlock.SetData(&pHexData[7], wDescLen);
	m_DescBlock.ParseDescs();

	// �n���h���֒ʒm
	if(m_pHandler)m_pHandler->OnTotTable(this);

	return true;
}
