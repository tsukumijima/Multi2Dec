// TsDescrambler.cpp: TS�f�X�N�����u���f�R�[�_
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "TsDescrambler.h"
#include "TsTable.h"
#include "TsPacket.h"


/////////////////////////////////////////////////////////////////////////////
// �t�@�C�����[�J���萔�ݒ�
/////////////////////////////////////////////////////////////////////////////

// EMM�����L������
#define EMM_VALID_PERIOD			7UL			// 7day

// B-CAS�J�[�h�ď������K�[�h�C���^�[�o��
#define BCAS_REFRESH_INTERVAL		1000UL		// 1s

// PMT�҂��p�P�b�g�J�E���g��
#define PMT_WAIT_PACKET_COUNT		(3250 *  200 / TS_PACKET_SIZE)	//  200ms @ 26Mbps
#define ECM_WAIT_PACKET_COUNT		(3250 * 2000 / TS_PACKET_SIZE)	// 2000ms @ 26Mbps

// �N���b�v�t���C���N�������g�}�N��
#define CLIPED_INCREMENT(V)			{if((V) < 0xFFFFFFFFUL)(V)++;}


/////////////////////////////////////////////////////////////////////////////
// TS�f�X�N�����u���f�R�[�_
/////////////////////////////////////////////////////////////////////////////

const bool CTsDescrambler::InputMedia(IMediaData *pMediaData, const DWORD dwInputIndex)
{
	// ���̓p�����[�^�`�F�b�N
	ITsPacket * const pPacket = dynamic_cast<ITsPacket *>(pMediaData);
	if(!pPacket || (dwInputIndex >= GetInputNum()))return false;

	// PS����
	InputPacket(pPacket);

	if(!m_bEnableBuffering || (m_DecBufState == BDS_RUNNING)){
		// �p�P�b�g����
		OutputPacket(pPacket);
		}
	else{
		// �������p�P�b�g�o�b�t�@�����O
		PushUnprocPacket(pPacket);

		// �o�b�t�@�����O����
		BufManagement(pPacket);
		}

	return true;
}

const bool CTsDescrambler::OpenDescrambler(LPCTSTR lpszBCId)
{
	// ��U�N���[�Y
	CloseDescrambler();

	IHalDevice *pHalDevice = NULL;

	try{
		// B-CAS�J�[�h�̃C���X�^���X�𐶐�����
		if(!(pHalDevice = ::BON_SAFE_CREATE<IHalDevice *>(lpszBCId)))throw __LINE__;

		// �f�o�C�X�^�C�v���`�F�b�N
		if(pHalDevice->GetDeviceType() != ::BON_NAME_TO_GUID(TEXT("IHalBcasCard")))throw __LINE__;

		// �f�o�C�X�̑��݂��`�F�b�N
		if(!pHalDevice->GetTotalDeviceNum())throw __LINE__;

		// �f�o�C�X�̋󂫂��`�F�b�N
		if(pHalDevice->GetActiveDeviceNum() >= pHalDevice->GetTotalDeviceNum())throw __LINE__;
	
		// IHalBcasCard�C���^�t�F�[�X�擾
		if(!(m_pHalBcasCard = dynamic_cast<IHalBcasCard *>(pHalDevice)))throw __LINE__;

		// B-CAS�J�[�h���I�[�v��
		if(!m_pHalBcasCard->OpenCard())throw __LINE__;
		
		// �S��ԃ��Z�b�g
		OnReset();
		}
	catch(...){
		// �G���[����
		BON_SAFE_RELEASE(pHalDevice);
		m_pHalBcasCard = NULL;
		
		CloseDescrambler();
		return false;
		}

	return true;
}

const bool CTsDescrambler::CloseDescrambler(void)
{
	// B-CAS�J�[�h���N���[�Y
	BON_SAFE_RELEASE(m_pHalBcasCard);

	return true;
}

void CTsDescrambler::DiscardScramblePacket(const bool bEnable)
{
	// �X�N�����u�������R��p�P�b�g��j�����邩�ݒ�
	m_bDiscardScramblePacket = (bEnable)? true : false;
}

void CTsDescrambler::EnableEmmProcess(const bool bEnable)
{
	// EMM���������邩�ݒ�
	m_bEnableEmmProcess = (bEnable)? true : false;
}

void CTsDescrambler::EnableBuffering(const bool bEnable)
{
	// �o�b�t�@�����O���邩�ݒ�
	m_bEnableBuffering = (bEnable)? true : false;
}

void CTsDescrambler::ResetStatistics(void)
{
	// ���v�f�[�^�����Z�b�g����
	m_dwInputPacketNum = 0UL;
	m_dwOutputPacketNum = 0UL;
	m_dwScramblePacketNum = 0UL;
	m_dwEcmProcessNum = 0UL;
	m_dwEmmProcessNum = 0UL;
	
	::ZeroMemory(m_adwInputPacketNum, sizeof(m_adwInputPacketNum));
	::ZeroMemory(m_adwOutputPacketNum, sizeof(m_adwOutputPacketNum));
	::ZeroMemory(m_adwScramblePacketNum, sizeof(m_adwScramblePacketNum));
}

const DWORD CTsDescrambler::GetDescramblingState(const WORD wProgramID)
{
	// PAT�e�[�u�����擾
	const IPatTable *pPatTable = dynamic_cast<const IPatTable *>(m_TsInputMap.GetMapTarget(0x0000U));
	if(!pPatTable)return IHalBcasCard::EC_BADARGUMENT;
		
	// �����Ɉ�v����PMT��PID������
	for(WORD wIndex = 0U ; wIndex < pPatTable->GetProgramNum() ; wIndex++){
		if(pPatTable->GetProgramID(wIndex) == wProgramID){
			
			// PMT�e�[�u�����擾
			const IPmtTable *pPmtTable = dynamic_cast<const IPmtTable *>(m_TsInputMap.GetMapTarget(pPatTable->GetPmtPID(wIndex)));
			if(!pPmtTable)return IHalBcasCard::EC_BADARGUMENT;
			
			// ECM��PID���擾
			const IDescBlock * const pDescBlock = pPmtTable->GetPmtDesc();
			if(!pDescBlock)return IHalBcasCard::EC_BADARGUMENT;
	
			const ICaMethodDesc * const pCaMethodDesc = dynamic_cast<const ICaMethodDesc *>(pDescBlock->GetDescByTag(ICaMethodDesc::DESC_TAG));
			if(!pCaMethodDesc)return IHalBcasCard::EC_BADARGUMENT;

			const WORD wEcmPID = pCaMethodDesc->GetCaPID();
			if(wEcmPID >= TS_INVALID_PID)return IHalBcasCard::EC_NO_ERROR;

			// ECM�v���Z�b�T���擾
			const CEcmProcessor *pEcmProcessor = dynamic_cast<const CEcmProcessor *>(m_TsInputMap.GetMapTarget(pPatTable->GetPmtPID(wEcmPID)));
			if(!pEcmProcessor)return IHalBcasCard::EC_BADARGUMENT;
			
			// ������Ԃ�Ԃ�
			return pEcmProcessor->m_dwDescramblingState;
			}		
		}

	return IHalBcasCard::EC_BADARGUMENT;
}

const DWORD CTsDescrambler::GetInputPacketNum(const WORD wPID)
{
	// ���̓p�P�b�g����Ԃ�
	return (wPID < 0x2000U)? m_adwInputPacketNum[wPID] : m_dwInputPacketNum;
}

const DWORD CTsDescrambler::GetOutputPacketNum(const WORD wPID)
{
	// �o�̓p�P�b�g����Ԃ�
	return (wPID < 0x2000U)? m_adwOutputPacketNum[wPID] : m_dwOutputPacketNum;
}

const DWORD CTsDescrambler::GetScramblePacketNum(const WORD wPID)
{
	// �����R��p�P�b�g����Ԃ�
	return (wPID < 0x2000U)? m_adwScramblePacketNum[wPID] : m_dwScramblePacketNum;
}

const DWORD CTsDescrambler::GetEcmProcessNum(void)
{
	// ECM�����񐔂�Ԃ�
	return m_dwEcmProcessNum;
}

const DWORD CTsDescrambler::GetEmmProcessNum(void)
{
	// EMM�����񐔂�Ԃ�
	return m_dwEmmProcessNum;
}

IHalBcasCard * CTsDescrambler::GetHalBcasCard(void)
{
	// B-CAS�J�[�h���[�_�f�o�C�X��Ԃ�
	return m_pHalBcasCard;
}

CTsDescrambler::CTsDescrambler(IBonObject *pOwner)
	: CMediaDecoder(pOwner, 1UL, 1UL)
	, m_TsInputMap(NULL)
	, m_TsOutputMap(NULL)
	, m_pHalBcasCard(NULL)
	, m_bDiscardScramblePacket(false)
	, m_bEnableEmmProcess(false)
	, m_wLastTsID(TS_INVALID_PID)
	, m_bEnableBuffering(false)
	, m_DecBufState(BDS_INITIAL)
	, m_dwPmtWaitCount(0UL)
{
	// ���v�f�[�^������������
	ResetStatistics();

	// PAT�e�[�u����PID�}�b�v�ɒǉ�
	m_TsInputMap.MapPid(0x0000U, dynamic_cast<ITsPidMapTarget *>(CPatTable::CreateInstance(dynamic_cast<IPatTable::IHandler *>(this))));

	// CAT�e�[�u����PID�}�b�v�ɒǉ�
	m_TsInputMap.MapPid(0x0001U, dynamic_cast<ITsPidMapTarget *>(CCatTable::CreateInstance(dynamic_cast<ICatTable::IHandler *>(this))));

	// TOT�e�[�u����PID�}�b�v�ɒǉ�
	m_TsInputMap.MapPid(0x0014U, dynamic_cast<ITsPidMapTarget *>(CTotTable::CreateInstance(dynamic_cast<ITotTable::IHandler *>(this))));
}

CTsDescrambler::~CTsDescrambler(void)
{
	// B-CAS�J�[�h�N���[�Y
	CloseDescrambler();
	
	// �o�b�t�@�J��
	ClearUnprocPacket();
}

const bool CTsDescrambler::OnPlay(void)
{
	// ������Ԃ�����������
	return OnReset();
}

const bool CTsDescrambler::OnStop(void)
{
	// �������p�P�b�g���t���b�V������
	FlushUnprocPacket();

	return CMediaDecoder::OnStop();
}

const bool CTsDescrambler::OnReset(void)
{
	// PID�}�b�v���Z�b�g
	ResetPidMap();

	// TSID���Z�b�g
	m_wLastTsID = TS_INVALID_PID;

	// PAT���Z�b�g
	m_TsInputMap.ResetPid(0x0000U);

	// �o�b�t�@�����O��ԃ��Z�b�g
	m_DecBufState = BDS_INITIAL;
	m_dwPmtWaitCount = 0UL;
	ClearUnprocPacket();

	// ���v�f�[�^���Z�b�g
	ResetStatistics();

	return true;
}

void CTsDescrambler::OnPatTable(const IPatTable *pPatTable)
{
	// TSID�X�V�m�F
	if(m_bEnableBuffering && (m_wLastTsID != pPatTable->GetTsID())){
		if(m_wLastTsID != TS_INVALID_PID){
			// �������p�P�b�g���t���b�V������
			FlushUnprocPacket();
			m_DecBufState = BDS_INITIAL;
			m_dwPmtWaitCount = 0UL;
			
			// PID�}�b�v���Z�b�g
			ResetPidMap();
			}
		m_wLastTsID = pPatTable->GetTsID();
		}

	// PMT�e�[�u��PID�}�b�v�ǉ�
	for(WORD wIndex = 0U ; wIndex < pPatTable->GetProgramNum() ; wIndex++){
		m_TsInputMap.MapPid(pPatTable->GetPmtPID(wIndex), dynamic_cast<ITsPidMapTarget *>(CPmtTable::CreateInstance(dynamic_cast<IPmtTable::IHandler *>(this))));
		}

	// �o�b�t�@�����O��ԏ�ԍX�V
	if(m_DecBufState == BDS_INITIAL){
		m_DecBufState = BDS_STORING_PMT;
		m_dwPmtWaitCount = 0UL;
		}
}

void CTsDescrambler::OnCatTable(const ICatTable *pCatTable)
{
	// EMM��PID���擾����
	const IDescBlock * const pDescBlock = pCatTable->GetCatDesc();
	if(!pDescBlock)return;
	
	const ICaMethodDesc * const pCaMethodDesc = dynamic_cast<const ICaMethodDesc *>(pDescBlock->GetDescByTag(ICaMethodDesc::DESC_TAG));
	if(!pCaMethodDesc)return;

	const WORD wEmmPID = pCaMethodDesc->GetCaPID();
	if(wEmmPID >= TS_INVALID_PID)return;

	// EMM�v���Z�b�T��PID�}�b�v�ɒǉ�
	m_TsInputMap.MapPid(wEmmPID, dynamic_cast<ITsPidMapTarget *>(CEmmProcessor::CreateInstance(dynamic_cast<ITsDescrambler *>(this))));
}

void CTsDescrambler::OnPmtTable(const IPmtTable *pPmtTable)
{
	// ECM��PID���擾����
	const IDescBlock * const pDescBlock = pPmtTable->GetPmtDesc();
	if(!pDescBlock)return;
	
	const ICaMethodDesc * const pCaMethodDesc = dynamic_cast<const ICaMethodDesc *>(pDescBlock->GetDescByTag(ICaMethodDesc::DESC_TAG));
	if(!pCaMethodDesc)return;

	const WORD wEcmPID = pCaMethodDesc->GetCaPID();
	if(wEcmPID >= TS_INVALID_PID)return;

	// ������ECM�v���Z�b�T���m�F
	if(!m_TsInputMap.GetMapTarget(wEcmPID)){
		// ECM�v���Z�b�T��PID�}�b�v�ɒǉ�
		m_TsInputMap.MapPid(wEcmPID, dynamic_cast<ITsPidMapTarget *>(CEcmProcessor::CreateInstance(dynamic_cast<ITsDescrambler *>(this))));
		}

	// ES�v���Z�b�T��PID�}�b�v�ɒǉ�
	for(WORD wIndex = 0U ; wIndex < pPmtTable->GetEsNum() ; wIndex++){
		m_TsOutputMap.MapPid(pPmtTable->GetEsPID(wIndex), dynamic_cast<ITsPidMapTarget *>(CEsProcessor::CreateInstance(m_TsInputMap.GetMapTarget(wEcmPID))));
		}
}

void CTsDescrambler::OnTotTable(const ITotTable *pTotTable)
{
	// �������Ȃ�
}

void CTsDescrambler::InputPacket(ITsPacket * const pPacket)
{
	// PID�}�b�v�ɓ���
	if(m_pHalBcasCard)m_TsInputMap.StorePacket(pPacket);

	// ���̓p�P�b�g���C���N�������g
	CLIPED_INCREMENT(m_dwInputPacketNum);
	CLIPED_INCREMENT(m_adwInputPacketNum[pPacket->GetPID()]);
}

void CTsDescrambler::BufManagement(ITsPacket * const pPacket)
{
	if(m_PacketBuf.size() >= ECM_WAIT_PACKET_COUNT){
		// �o�b�t�@�����O���~�b�g����
		FlushUnprocPacket();
		m_DecBufState = BDS_RUNNING;
		}
	else if(m_DecBufState == BDS_STORING_PMT){
		// PMT�҂��J�E���^�C���N�������g
		if(++m_dwPmtWaitCount >= PMT_WAIT_PACKET_COUNT){
			m_DecBufState = BDS_STORING_ECM;
			}
		}
	else if(m_DecBufState == BDS_STORING_ECM){
		// ECM�X�g�A��ԃ��j�^
		if(dynamic_cast<CEcmProcessor *>(m_TsInputMap.GetMapTarget(pPacket->GetPID()))){
			bool bNotStored = false;
		
			for(WORD wPID = 0x0000U ; wPID < 0x2000U ; wPID++){
				const CEcmProcessor *pEcmProcessor = dynamic_cast<CEcmProcessor *>(m_TsInputMap.GetMapTarget(wPID));
				if(pEcmProcessor){
					if(!pEcmProcessor->IsEcmReceived()){
						// ECM����M
						bNotStored = true;
						}
					}
				}

			if(!bNotStored){
				m_DecBufState = BDS_RUNNING;
			
				// �������p�P�b�g�o��
				FlushUnprocPacket();
				}
			}
		}
}

void CTsDescrambler::OutputPacket(ITsPacket * const pPacket)
{
	const WORD wPID = pPacket->GetPID();

	// ES�v���Z�b�T�ɓ���
	m_TsOutputMap.StorePacket(pPacket);

	if(pPacket->IsScrambled()){
		// �����R��p�P�b�g���C���N�������g
		CLIPED_INCREMENT(m_dwScramblePacketNum);
		CLIPED_INCREMENT(m_adwScramblePacketNum[wPID]);

		SendDecoderEvent(EID_CANT_DESCRAMBLE, reinterpret_cast<PVOID>(m_dwScramblePacketNum));
		
		// �����R��p�P�b�g�j���L�����͉��ʃf�R�[�_�ɏo�͂��Ȃ�
		if(m_bDiscardScramblePacket)return;
		}

	// �o�̓p�P�b�g���C���N�������g
	CLIPED_INCREMENT(m_dwOutputPacketNum);
	CLIPED_INCREMENT(m_adwOutputPacketNum[wPID]);

	// ���ʃf�R�[�_�Ƀf�[�^�o��
	OutputMedia(dynamic_cast<IMediaData *>(pPacket));
}

void CTsDescrambler::ResetPidMap(void)
{
	// PAT/CAT/TOT�ȊO�A���}�b�v
	for(WORD wPID = 0x0002U ; wPID < 0x2000U ; wPID++){
		if(wPID != 0x0014U){
			m_TsInputMap.UnmapPid(wPID);
			}	
		}

	// CAT���Z�b�g
	m_TsInputMap.ResetPid(0x0001U);

	// TOT���Z�b�g
	m_TsInputMap.ResetPid(0x0014U);
	
	// ES�v���Z�b�T���Z�b�g
	m_TsOutputMap.ClearPid();
}

void CTsDescrambler::FlushUnprocPacket(void)
{
	// �������p�P�b�g���t���b�V������
	for(DWORD dwIndex = 0UL ; dwIndex < m_PacketBuf.size() ; dwIndex++){
		OutputPacket(m_PacketBuf[dwIndex]);
		}

	// �o�b�t�@���N���A
	ClearUnprocPacket();
}

void CTsDescrambler::PushUnprocPacket(const ITsPacket *pPacket)
{
	// �������p�P�b�g���o�b�t�@�Ƀv�b�V������
	CTsPacket * const pNewPacket = new CTsPacket(NULL);
	::BON_ASSERT(pNewPacket);

	if(pNewPacket->CopyPacket(pPacket)){	
		m_PacketBuf.push_back(pNewPacket);
		}
}

void CTsDescrambler::ClearUnprocPacket(void)
{
	// �������p�P�b�g���N���A
	for(DWORD dwIndex = 0UL ; dwIndex < m_PacketBuf.size() ; dwIndex++){
		BON_SAFE_RELEASE(m_PacketBuf[dwIndex]);
		}
	
	m_PacketBuf.clear();
}


/////////////////////////////////////////////////////////////////////////////
// ECM���������N���X
/////////////////////////////////////////////////////////////////////////////

void CTsDescrambler::CEcmProcessor::OnPidReset(ITsPidMapper *pTsPidMapper, const WORD wPID)
{
	// �L�[�N���A����(������L�[�ɂ��j����h�~���邽��)
	m_Multi2Decoder.SetScrambleKey(NULL);
	m_dwDescramblingState = IHalBcasCard::EC_NO_ERROR;
	
	CPsiTableBase::OnPidReset(pTsPidMapper, wPID);
}

void CTsDescrambler::CEcmProcessor::Reset(void)
{
	m_bIsEcmReceived = false;
	
	CPsiTableBase::Reset();
}

CTsDescrambler::CEcmProcessor::CEcmProcessor(IBonObject *pOwner)
	: CPsiTableBase(NULL)
	, m_pTsDescrambler(dynamic_cast<CTsDescrambler *>(pOwner))
	, m_pHalBcasCard(m_pTsDescrambler->m_pHalBcasCard)
	, m_dwLastRetryTime(::GetTickCount() - BCAS_REFRESH_INTERVAL)
	, m_dwDescramblingState(IHalBcasCard::EC_NO_ERROR)
{
	// MULTI2�f�R�[�_��������
	m_Multi2Decoder.Initialize(m_pHalBcasCard->GetSystemKey(), m_pHalBcasCard->GetInitialCbc());
}

CTsDescrambler::CEcmProcessor::~CEcmProcessor(void)
{
	// ���̃C���X�^���X���Q�Ƃ��Ă���CEsProcessor���A���}�b�v����
	CEsProcessor *pEsProcessor;
	
	for(WORD wPID = 0x0000U ; wPID < 0x2000U ; wPID++){
		if(pEsProcessor = dynamic_cast<CEsProcessor *>(m_pTsDescrambler->m_TsOutputMap.GetMapTarget(wPID))){
			if(pEsProcessor->m_pEcmProcessor == this){
				m_pTsDescrambler->m_TsOutputMap.UnmapPid(wPID);
				}
			}
		}
}

const bool CTsDescrambler::CEcmProcessor::IsEcmReceived(void) const
{
	// ECM�̎�M�L����Ԃ�
	return m_bIsEcmReceived;
}

const bool CTsDescrambler::CEcmProcessor::DescramblePacket(ITsPacket * const pTsPacket)
{
	// IMediaData�C���^�t�F�[�X�₢���킹
	IMediaData *pMediaData = dynamic_cast<IMediaData *>(pTsPacket);

	// �X�N�����u������
	if(pMediaData){
		if(m_Multi2Decoder.Decode(pTsPacket->GetPayloadData(), (DWORD)pTsPacket->GetPayloadSize(), pTsPacket->m_Header.byTransportScramblingCtrl)){
			// �g�����X�|�[�g�X�N�����u������Đݒ�
			pMediaData->SetAt(3UL, pMediaData->GetAt(3UL) & 0x3FU);
			pTsPacket->m_Header.byTransportScramblingCtrl = 0U;

			return true;
			}
		}
	
	return false;
}

const bool CTsDescrambler::CEcmProcessor::OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection)
{
	// �e�[�u��ID���`�F�b�N
	if(pNewSection->GetTableID() != TABLE_ID)return false;

	// ECM��B-CAS�J�[�h�ɓn���ăL�[�擾
	const BYTE *pKsData = m_pHalBcasCard->GetKsFromEcm(pNewSection->GetPayloadData(), pNewSection->GetPayloadSize());

	// ECM�������s���͈�x����B-CAS�J�[�h���ď���������
	if(!pKsData && (m_pHalBcasCard->GetEcmError() != IHalBcasCard::EC_NOT_CONTRACTED)){
		if((::GetTickCount() - m_dwLastRetryTime) >= BCAS_REFRESH_INTERVAL){
			// �ď������K�[�h�C���^�[�o���o�߂Ȃ�			
			if(m_pHalBcasCard->OpenCard()){
				pKsData = m_pHalBcasCard->GetKsFromEcm(pNewSection->GetPayloadData(), pNewSection->GetPayloadSize());
				}
				
			// �ŏI���g���C���ԍX�V
			m_dwLastRetryTime = ::GetTickCount();
			}
		}

	// �X�N�����u���L�[�X�V
	m_Multi2Decoder.SetScrambleKey(pKsData);

	// �C�x���g�ʒm
	if(pKsData)CLIPED_INCREMENT(m_pTsDescrambler->m_dwEcmProcessNum);
	m_pTsDescrambler->SendDecoderEvent(EID_ECM_PROCESSED, reinterpret_cast<PVOID>(m_pHalBcasCard->GetEcmError()));

	// ������ԍX�V
	m_dwDescramblingState = m_pHalBcasCard->GetEcmError();
	m_bIsEcmReceived = true;

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// EMM���������N���X
/////////////////////////////////////////////////////////////////////////////

CTsDescrambler::CEmmProcessor::CEmmProcessor(IBonObject *pOwner)
	: CPsiTableBase(NULL)
	, m_pTsDescrambler(dynamic_cast<CTsDescrambler *>(pOwner))
	, m_pHalBcasCard(m_pTsDescrambler->m_pHalBcasCard)
{
	// �������Ȃ�
}

CTsDescrambler::CEmmProcessor::~CEmmProcessor(void)
{
	// �������Ȃ�
}

const bool CTsDescrambler::CEmmProcessor::OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection)
{
	// �e�[�u��ID���`�F�b�N
	if(pNewSection->GetTableID() != SECTION_TABLE_ID)return false;

	// EMM�����������̂Ƃ��͉������Ȃ�
	if(!m_pTsDescrambler->m_bEnableEmmProcess)return true;

	// �Z�N�V���������
	const WORD wDataSize = pNewSection->GetPayloadSize();
	const BYTE *pHexData = pNewSection->GetPayloadData();
	
	WORD wPos = 0U;
	
	while((wDataSize - wPos) >= 17U){
		// EMM�T�C�Y���`�F�b�N
		const WORD wEmmSize = (WORD)pHexData[wPos + 6U] + 7U;
		if(((wDataSize - wPos) < wEmmSize) || (wEmmSize < 17U))break;

		// �n���h���Ăяo��
		if(OnEmmBody(&pHexData[wPos], wEmmSize))break;
		
		// ��͈ʒu�X�V
		wPos += wEmmSize;
		}

	return true;
}

const bool CTsDescrambler::CEmmProcessor::OnEmmBody(const BYTE *pEmmData, const WORD wEmmSize)
{
	// ��B-CAS�J�[�hID�擾
	const BYTE * const pCardID = m_pHalBcasCard->GetBcasCardID();

	// ��B-CAS�J�[�hID�Əƍ�
	for(DWORD dwPos = 0UL ; dwPos < 6UL ; dwPos++){
		if(pCardID[dwPos] != pEmmData[dwPos])return false;
		}

	// �X�g���[���̎��Ԃ��m�F����
	CTsTime ValidTime;
	ValidTime.SetNowTime();									// ���ݎ������擾
	ValidTime -= EMM_VALID_PERIOD * 24UL * 60UL * 60UL;		// �L�����Ԃ����ߋ��ɖ߂�
	
	const ITotTable *pTotTable = dynamic_cast<const ITotTable *>(m_pTsDescrambler->m_TsInputMap.GetMapTarget(0x0014U));
	if(!pTotTable)return true;
	CTsTime EmmTime = pTotTable->GetDateTime();				// TOT���玞�����擾
	
	// �L�����Ԃ��ߋ���EMM�f�[�^�͏������Ȃ�
	if(EmmTime < ValidTime)return true;
	
	// B-CAS�J�[�h��EMM�f�[�^���M
	if(!m_pHalBcasCard->SendEmmSection(pEmmData, wEmmSize)){
		// ���g���C
		if(!m_pHalBcasCard->SendEmmSection(pEmmData, wEmmSize)){
			// EMM�������s
			m_pTsDescrambler->SendDecoderEvent(EID_EMM_PROCESSED, reinterpret_cast<PVOID>(false));
			return true;
			}
		}

	// �C�x���g�ʒm
	CLIPED_INCREMENT(m_pTsDescrambler->m_dwEmmProcessNum);
	m_pTsDescrambler->SendDecoderEvent(EID_EMM_PROCESSED, reinterpret_cast<PVOID>(true));

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// ES���������N���X
/////////////////////////////////////////////////////////////////////////////

CTsDescrambler::CEsProcessor::CEsProcessor(IBonObject *pOwner)
	: CBonObject(NULL)
	, m_pEcmProcessor(dynamic_cast<CEcmProcessor *>(pOwner))
{
	// �������Ȃ�
}

CTsDescrambler::CEsProcessor::~CEsProcessor(void)
{
	// �������Ȃ�
}

const bool CTsDescrambler::CEsProcessor::StorePacket(const ITsPacket *pTsPacket)
{
	// �X�N�����u������
	return m_pEcmProcessor->DescramblePacket(const_cast<ITsPacket *>(pTsPacket));
}

void CTsDescrambler::CEsProcessor::OnPidReset(ITsPidMapper *pTsPidMapper, const WORD wPID)
{
	// �������Ȃ�
}

void CTsDescrambler::CEsProcessor::OnPidMapped(ITsPidMapper *pTsPidMapper, const WORD wPID)
{
	// �������Ȃ�
}

void CTsDescrambler::CEsProcessor::OnPidUnmapped(ITsPidMapper *pTsPidMapper, const WORD wPID)
{
	// �C���X�^���X�J��
	Release();
}
