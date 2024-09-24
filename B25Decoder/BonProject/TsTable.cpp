// TsTable.cpp: TSテーブル基底クラス
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "TsTable.h"


/////////////////////////////////////////////////////////////////////////////
// [0x00] PATテーブル基底クラス
/////////////////////////////////////////////////////////////////////////////

void CPatTable::Reset(void)
{
	// 状態をクリアする
	m_wTsID = 0x0000U;
	
	m_NitPIDArray.clear();
	m_PmtPIDArray.clear();

	CPsiTableBase::Reset();
}

const WORD CPatTable::GetTsID(void) const
{
	// TS IDを返す
	return m_wTsID;
}

const WORD CPatTable::GetNitPID(const DWORD dwIndex) const
{
	// NITのPIDを返す
	return (dwIndex < m_NitPIDArray.size())? m_NitPIDArray[dwIndex].wPID : TS_INVALID_PID;
}

const DWORD CPatTable::GetNitNum(void) const
{
	// NITの数を返す
	return m_NitPIDArray.size();
}

const WORD CPatTable::GetPmtPID(const DWORD dwIndex) const
{
	// PMTのPIDを返す
	return (dwIndex < m_PmtPIDArray.size())? m_PmtPIDArray[dwIndex].wPID : TS_INVALID_PID;
}

const WORD CPatTable::GetProgramID(const DWORD dwIndex) const
{
	// Program Number IDを返す
	return (dwIndex < m_PmtPIDArray.size())? m_PmtPIDArray[dwIndex].wProgramID : 0x0000U;
}

const DWORD CPatTable::GetProgramNum(void) const
{
	// PMTの数を返す
	return m_PmtPIDArray.size();
}

CPatTable::CPatTable(IBonObject *pOwner)
	: CPsiTableBase(pOwner)
	, m_pHandler(dynamic_cast<IPatTable::IHandler *>(pOwner))
	, m_wTsID(0UL)
{
	// 何もしない
}

CPatTable::~CPatTable(void)
{
	// 何もしない
}

const bool CPatTable::OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection)
{
	const WORD wDataSize = pNewSection->GetPayloadSize();
	const BYTE *pHexData = pNewSection->GetPayloadData();

	if(wDataSize % 4U)return false;										// テーブルのサイズが不正
	if(pNewSection->GetTableID() != IPatTable::TABLE_ID)return false;	// テーブルIDが不正

	// PIDをクリアする
	m_NitPIDArray.clear();
	m_PmtPIDArray.clear();

	TAG_PATITEM PatItem;

	// テーブルを解析する
	m_wTsID = pNewSection->GetTableIdExtension();

	::BON_TRACE(TEXT("\n------- PAT Table -------\nTS ID = %04X\n"), m_wTsID);

	for(WORD wPos = 0U ; wPos < wDataSize ; wPos += 4U, pHexData += 4){
		PatItem.wProgramID	= ((WORD)pHexData[0] << 8) | (WORD)pHexData[1];				// +1,2
		PatItem.wPID		= ((WORD)(pHexData[2] & 0x1FU) << 8) | (WORD)pHexData[3];	// +3,4

		if(!PatItem.wProgramID){
			// NITのPID
			::BON_TRACE(TEXT("NIT #%u [ID:%04X][PID:%04X]\n"), m_NitPIDArray.size(), PatItem.wProgramID, PatItem.wPID);
			m_NitPIDArray.push_back(PatItem);
			}
		else{
			// PMTのPID
			::BON_TRACE(TEXT("PMT #%u [ID:%04X][PID:%04X]\n"), m_PmtPIDArray.size(), PatItem.wProgramID, PatItem.wPID);
			m_PmtPIDArray.push_back(PatItem);
			}
		}

	// ハンドラへ通知
	if(m_pHandler)m_pHandler->OnPatTable(this);

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// [0x01] CATテーブル基底クラス
/////////////////////////////////////////////////////////////////////////////

void CCatTable::Reset(void)
{
	// 状態をクリアする
	m_DescBlock.Reset();

	CPsiTableBase::Reset();
}

const IDescBlock * CCatTable::GetCatDesc(void) const
{
	// 記述子領域を返す
	return &m_DescBlock;
}

CCatTable::CCatTable(IBonObject *pOwner)
	: CPsiTableBase(pOwner)
	, m_DescBlock(NULL)
	, m_pHandler(dynamic_cast<ICatTable::IHandler *>(pOwner))
{
	// 何もしない
}

CCatTable::~CCatTable(void)
{
	// 何もしない
}

const bool CCatTable::OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection)
{
	if(pNewSection->GetTableID() != ICatTable::TABLE_ID)return false;	// テーブルIDが不正
	
	// テーブルを解析する
	m_DescBlock.SetData(pNewSection->GetPayloadData(), pNewSection->GetPayloadSize());
	m_DescBlock.ParseDescs();

	::BON_TRACE(TEXT("\n------- CAT Table -------\n"));

	for(DWORD dwIndex = 0UL ; dwIndex < m_DescBlock.GetDescNum() ; dwIndex++){
		const IDescBase *pDescBase = m_DescBlock.GetDescByIndex(dwIndex);
		::BON_TRACE(TEXT("[%lu] TAG = 0x%02X LEN = %lu\n"), dwIndex, pDescBase->GetTag(), pDescBase->GetLength());
		}

	// ハンドラへ通知
	if(m_pHandler)m_pHandler->OnCatTable(this);

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// [0x02] PMTテーブル基底クラス
/////////////////////////////////////////////////////////////////////////////

void CPmtTable::Reset(void)
{
	// 状態をクリアする
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
	// 放送番組番号IDを返す
	return m_wProgramID;
}

const WORD CPmtTable::GetPcrPID(void) const
{
	// PCR PIDを返す
	return m_wPcrPID;
}

const IDescBlock * CPmtTable::GetPmtDesc(void) const
{
	// 記述子領域1を返す
	return &m_DescBlock;
}

const DWORD CPmtTable::GetEsNum(void) const
{
	// エレメンタリストリーム数を返す
	return m_EsInfoArray.size();
}

const BYTE CPmtTable::GetStreamTypeID(const DWORD dwIndex) const
{
	// ストリーム形式IDを返す
	return (dwIndex < m_EsInfoArray.size())? m_EsInfoArray[dwIndex].byStreamTypeID : 0x00U;
}

const WORD CPmtTable::GetEsPID(const DWORD dwIndex) const
{
	// エレメンタリPIDを返す
	return (dwIndex < m_EsInfoArray.size())? m_EsInfoArray[dwIndex].wEsPID : TS_INVALID_PID;
}

const IDescBlock * CPmtTable::GetEsDesc(const DWORD dwIndex) const
{
	// 記述子領域2を返す
	return (dwIndex < m_EsInfoArray.size())? m_EsInfoArray[dwIndex].pDescBlock : NULL;
}

CPmtTable::CPmtTable(IBonObject *pOwner)
	: CPsiTableBase(pOwner)
	, m_wProgramID(0x0000U)
	, m_wPcrPID(TS_INVALID_PID)
	, m_DescBlock(NULL)
	, m_pHandler(dynamic_cast<IPmtTable::IHandler *>(pOwner))
{
	// 何もしない
}

CPmtTable::~CPmtTable(void)
{
	// インスタンス開放
	Reset();
}

const bool CPmtTable::OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection)
{
	const WORD wDataSize = pNewSection->GetPayloadSize();
	const BYTE *pHexData = pNewSection->GetPayloadData();

	if(pNewSection->GetTableID() != IPmtTable::TABLE_ID)return false;	// テーブルIDが不正
	
	// ES情報をクリアする
	for(DWORD dwIndex = 0UL ; dwIndex < m_EsInfoArray.size() ; dwIndex++){
		BON_SAFE_RELEASE(m_EsInfoArray[dwIndex].pDescBlock);
		}

	m_EsInfoArray.clear();
	
	// テーブルを解析する
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

	// ストリーム情報の開始位置を計算
	WORD wPos = wDescLen + 4U;
	TAG_PMTITEM PmtItem;

	// ES情報を解析
	while(wPos < wDataSize){
		PmtItem.byStreamTypeID = pHexData[wPos + 0U];													// +0
		PmtItem.wEsPID = ((WORD)(pHexData[wPos + 1U] & 0x1FU) << 8) | (WORD)pHexData[wPos + 2U];		// +1,2	
		wDescLen = ((WORD)(pHexData[wPos + 3U] & 0x0FU) << 8) | (WORD)pHexData[wPos + 4U];				// +3,4

		// 記述子ブロック
		PmtItem.pDescBlock = dynamic_cast<CDescBlock *>(CDescBlock::CreateInstance(NULL));
		PmtItem.pDescBlock->SetData(&pHexData[wPos + 5U], wDescLen);
		PmtItem.pDescBlock->ParseDescs();

		::BON_TRACE(TEXT("[%u] Stream Type ID = %02X  PID = %04X\n"), m_EsInfoArray.size(), PmtItem.byStreamTypeID, PmtItem.wEsPID);		

		for(DWORD dwIndex = 0UL ; dwIndex < PmtItem.pDescBlock->GetDescNum() ; dwIndex++){
			const IDescBase *pDescBase = PmtItem.pDescBlock->GetDescByIndex(dwIndex);
			::BON_TRACE(TEXT("    [%lu] TAG = 0x%02X LEN = %lu\n"), dwIndex, pDescBase->GetTag(), pDescBase->GetLength());
			}

		// テーブルに追加する
		m_EsInfoArray.push_back(PmtItem);
		wPos += (wDescLen + 5U);
		}

	// ハンドラへ通知
	if(m_pHandler)m_pHandler->OnPmtTable(this);

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// [0x40] NITテーブル基底クラス
/////////////////////////////////////////////////////////////////////////////

void CNitTable::Reset(void)
{
	// 状態をクリアする
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
	// ネットワークIDを返す
	return m_wNetworkID;
}

const IDescBlock * CNitTable::GetNetworkDesc(void) const
{
	// ネットワーク記述子領域を返す
	return (m_NwDescBlock.GetSize())? &m_NwDescBlock : NULL;
}

const DWORD CNitTable::GetTsNum(void) const
{
	// TS情報数を返す
	return m_TsInfoArray.size();
}

const BYTE CNitTable::GetTsID(const DWORD dwIndex) const
{
	// TS IDを返す
	return (dwIndex < m_TsInfoArray.size())? m_TsInfoArray[dwIndex].wTsID : 0x0000U;
}

const BYTE CNitTable::GetOrgNetworkID(const DWORD dwIndex) const
{
	// オリジナルネットワークIDを返す
	return (dwIndex < m_TsInfoArray.size())? m_TsInfoArray[dwIndex].wOrgNetworkID : 0x0000U;
}

const IDescBlock * CNitTable::GetTsDesc(const DWORD dwIndex) const
{
	// ネットワーク記述子領域を返す
	return (dwIndex < m_TsInfoArray.size())? m_TsInfoArray[dwIndex].pDescBlock : NULL;
}

CNitTable::CNitTable(IBonObject *pOwner)
	: CPsiTableBase(pOwner)
	, m_wNetworkID(0x0000U)
	, m_NwDescBlock(NULL)
	, m_pHandler(dynamic_cast<INitTable::IHandler *>(pOwner))
{
	// 何もしない
}

CNitTable::~CNitTable(void)
{
	// インスタンス開放
	Reset();
}

const bool CNitTable::OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection)
{
	const WORD wDataSize = pNewSection->GetPayloadSize();
	const BYTE *pHexData = pNewSection->GetPayloadData();

	if(pNewSection->GetTableID() != INitTable::TABLE_ID)return false;	// テーブルIDが不正
	
	// TS情報をクリアする
	for(DWORD dwIndex = 0UL ; dwIndex < m_TsInfoArray.size() ; dwIndex++){
		BON_SAFE_RELEASE(m_TsInfoArray[dwIndex].pDescBlock);
		}

	m_TsInfoArray.clear();
	
	// テーブルを解析する
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

	// TS情報の開始位置を計算
	WORD wPos = wDescLen + 4U;
	TAG_NITITEM NitItem;

	// TS情報を解析
	while(wPos < wDataSize){
		NitItem.wTsID = ((WORD)pHexData[wPos + 0U] << 8) | (WORD)pHexData[wPos + 1U];
		NitItem.wOrgNetworkID = ((WORD)pHexData[wPos + 2U] << 8) | (WORD)pHexData[wPos + 3U];

		wDescLen = ((WORD)(pHexData[wPos + 4U] & 0x0FU) << 8) | (WORD)pHexData[wPos + 5U];

		// 記述子ブロック
		NitItem.pDescBlock = dynamic_cast<CDescBlock *>(CDescBlock::CreateInstance(NULL));
		NitItem.pDescBlock->SetData(&pHexData[wPos + 6U], wDescLen);
		NitItem.pDescBlock->ParseDescs();

		::BON_TRACE(TEXT("[%u] TS ID = %04X  Original Network ID = %04X\n    Descriptor Field:\n"), m_TsInfoArray.size(), NitItem.wTsID, NitItem.wOrgNetworkID);		

		for(DWORD dwIndex = 0UL ; dwIndex < NitItem.pDescBlock->GetDescNum() ; dwIndex++){
			const IDescBase *pDescBase = NitItem.pDescBlock->GetDescByIndex(dwIndex);
			::BON_TRACE(TEXT("        [%lu] TAG = 0x%02X LEN = %lu\n"), dwIndex, pDescBase->GetTag(), pDescBase->GetLength());
			}

		// テーブルに追加する
		m_TsInfoArray.push_back(NitItem);
		wPos += (wDescLen + 6U);
		}

	// ハンドラへ通知
	if(m_pHandler)m_pHandler->OnNitTable(this);

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// [0x42] SDTテーブル基底クラス
/////////////////////////////////////////////////////////////////////////////

void CSdtTable::Reset(void)
{
	// 状態をクリアする
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
	// TS IDを返す
	return m_wTsID;
}

const WORD CSdtTable::GetOrgNetworkID(void) const
{
	// オリジナルネットワークIDを返す
	return m_wOrgNetworkID;
}

const DWORD CSdtTable::GetServiceNum(void) const
{
	// サービス数を返す
	return m_SdtItemArray.size();
}

const WORD CSdtTable::GetServiceID(const DWORD dwIndex) const
{
	// サービスIDを返す
	return (dwIndex < m_SdtItemArray.size())? m_SdtItemArray[dwIndex].wServiceID : 0x0000U;
}

const bool CSdtTable::IsEitSchedule(const WORD wServiceID) const
{
	// EITスケジュールフラグを返す
	const DWORD dwIndex = GetIndexByServiceID(wServiceID);
	return (dwIndex < m_SdtItemArray.size())? m_SdtItemArray[wServiceID].bIsEitSchedule : false;
}

const bool CSdtTable::IsEitPresentFollowing(const WORD wServiceID) const
{
	// EIT現在/次フラグを返す
	const DWORD dwIndex = GetIndexByServiceID(wServiceID);
	return (dwIndex < m_SdtItemArray.size())? m_SdtItemArray[dwIndex].IsEitPresentFollowing : false;
}

const BYTE CSdtTable::GetRunningStatus(const WORD wServiceID) const
{
	// 進行状態を返す
	const DWORD dwIndex = GetIndexByServiceID(wServiceID);
	return (dwIndex < m_SdtItemArray.size())? m_SdtItemArray[dwIndex].byRunningStatus : ISdtTable::RST_UNDEFINED;
}

const bool CSdtTable::IsScrambling(const WORD wServiceID) const
{
	// スクランブルフラグを返す
	const DWORD dwIndex = GetIndexByServiceID(wServiceID);
	return (dwIndex < m_SdtItemArray.size())? m_SdtItemArray[dwIndex].bIsScrambling : false;
}

const IDescBlock * CSdtTable::GetServiceDesc(const WORD wServiceID) const
{
	// 記述子領域を返す
	const DWORD dwIndex = GetIndexByServiceID(wServiceID);
	return (dwIndex < m_SdtItemArray.size())? m_SdtItemArray[dwIndex].pDescBlock : NULL;
}

CSdtTable::CSdtTable(IBonObject *pOwner)
	: CPsiTableBase(pOwner)
	, m_wTsID(0x0000U)
	, m_wOrgNetworkID(0x0000U)
	, m_pHandler(dynamic_cast<ISdtTable::IHandler *>(pOwner))
{
	// 何もしない
}

CSdtTable::~CSdtTable(void)
{
	// インスタンス開放
	Reset();
}

const bool CSdtTable::OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection)
{
	const WORD wDataSize = pNewSection->GetPayloadSize();
	const BYTE *pHexData = pNewSection->GetPayloadData();

	if(pNewSection->GetTableID() != ISdtTable::TABLE_ID)return false;	// テーブルIDが不正
	
	// サービス情報をクリアする
	for(DWORD dwIndex = 0UL ; dwIndex != m_SdtItemArray.size() ; dwIndex++){
		BON_SAFE_RELEASE(m_SdtItemArray[dwIndex].pDescBlock);
		}

	m_SdtItemArray.clear();
	
	// テーブルを解析する
	m_wTsID = pNewSection->GetTableIdExtension();
	m_wOrgNetworkID = ((WORD)pHexData[0] << 8) | (WORD)pHexData[1];

	::BON_TRACE(TEXT("\n------- SDT Table -------\nTS ID = %04X\nOriginal Network ID = %04X\n"), m_wTsID, m_wOrgNetworkID);

	WORD wPos = 3U;
	TAG_SDTITEM SdtItem;
	
	// サービス情報を解析
	while(wPos < wDataSize){
		SdtItem.wServiceID = ((WORD)pHexData[wPos + 0U] << 8) | (WORD)pHexData[wPos + 1U];
		SdtItem.bIsEitSchedule = (pHexData[wPos + 2U] & 0x02U)? true : false;
		SdtItem.IsEitPresentFollowing = (pHexData[wPos + 2U] & 0x01U)? true : false;
		SdtItem.byRunningStatus = (pHexData[wPos + 3U]) >> 5;
		SdtItem.bIsScrambling = (pHexData[wPos + 3U] & 0x10U)? true : false;

		// 記述子ブロック
		const WORD wDescLen = ((WORD)(pHexData[wPos + 3U] & 0x0FU) << 8) | (WORD)pHexData[wPos + 4U];
		SdtItem.pDescBlock = dynamic_cast<CDescBlock *>(CDescBlock::CreateInstance(NULL));
		SdtItem.pDescBlock->SetData(&pHexData[wPos + 5U], wDescLen);
		SdtItem.pDescBlock->ParseDescs();

		// テーブルに追加する
		m_SdtItemArray.push_back(SdtItem);
		wPos += (wDescLen + 5U);

		// 以下デバッグ用
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

	// ハンドラへ通知
	if(m_pHandler)m_pHandler->OnSdtTable(this);

	return true;
}

const DWORD CSdtTable::GetIndexByServiceID(const WORD wServiceID) const
{
	// サービスIDからインデックスを取得する
	DWORD dwIndex;
	
	for(dwIndex = 0UL ; dwIndex < m_SdtItemArray.size() ; dwIndex++){
		if(m_SdtItemArray[dwIndex].wServiceID == wServiceID)break;
		}

	return dwIndex;
}


/////////////////////////////////////////////////////////////////////////////
// [0x4E-0x6F] EITテーブル基底クラス
/////////////////////////////////////////////////////////////////////////////

void CEitTable::Reset(void)
{
	// 状態をクリアする
	m_EitItem.ClearSize();
	
	CPsiTableBase::Reset();
}

const WORD CEitTable::GetServiceID(void) const
{
	// サービスIDを返す
	return m_LastSection.GetTableIdExtension();
}

const WORD CEitTable::GetTsID(void) const
{
	// TS IDを返す
	if(m_LastSection.GetPayloadSize() < 6U)return 0x0000U;

	const BYTE *pHexData = m_LastSection.GetPayloadData();
	return ((WORD)pHexData[0] << 8) | (WORD)pHexData[1];
}

const WORD CEitTable::GetOrgNetworkID(void) const
{
	// Original Network IDを返す
	if(m_LastSection.GetPayloadSize() < 6U)return 0x0000U;

	const BYTE *pHexData = m_LastSection.GetPayloadData();
	return ((WORD)pHexData[2] << 8) | (WORD)pHexData[3];
}

const BYTE CEitTable::GetTableID(void) const
{
	// テーブルIDを返す
	return m_LastSection.GetTableID();
}

const BYTE CEitTable::GetLastTableID(void) const
{
	// 最終テーブルIDを返す
	if(m_LastSection.GetPayloadSize() < 6U)return 0x00U;

	const BYTE *pHexData = m_LastSection.GetPayloadData();
	return pHexData[5];
}

const BYTE CEitTable::GetSectionNumber(void) const
{
	// セクション番号を返す
	return m_LastSection.GetSectionNumber();
}

const BYTE CEitTable::GetLastSectionNumber(void) const
{
	// 最終セクション番号を返す
	return m_LastSection.GetLastSectionNumber();
}

const BYTE CEitTable::GetSegmentLastSectionNumber(void) const
{
	// セグメント最終セクション番号を返す
	if(m_LastSection.GetPayloadSize() < 6U)return 0x00U;

	const BYTE *pHexData = m_LastSection.GetPayloadData();
	return pHexData[4];
}

CEitTable::CEitTable(IBonObject *pOwner)
	: CPsiTableBase(pOwner)
	, m_pHandler(dynamic_cast<IEitTable::IHandler *>(pOwner))
	, m_EitItem(NULL)
{
	// 何もしない
}

CEitTable::~CEitTable(void)
{
	// 何もしない
}

void CEitTable::OnPsiSection(const IPsiSectionParser *pPsiSectionParser, const IPsiSection *pPsiSection)
{
	// CPsiTableBaseの実装をオーバーライドする
	if(m_LastSection.CompareSection(pPsiSection)){
		// セクションが更新された
		OnTableUpdate(pPsiSection, &m_LastSection);
		}
}

const bool CEitTable::OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection)
{
	const WORD wDataSize = pNewSection->GetPayloadSize();
	const BYTE *pHexData = pNewSection->GetPayloadData();

	if(pNewSection->GetTableID() < IEitTable::TABLE_ID_START)return false;	// テーブルIDが不正
	if(pNewSection->GetTableID() > IEitTable::TABLE_ID_END)return false;	// テーブルIDが不正
	if(wDataSize < 18U)return false;										// ペイロード長が不正

	// セクションストア
	m_LastSection.CopySection(pNewSection);

	// アイテム毎にハンドラに通知
	if(!m_pHandler)return true;
	
	for(WORD wPos = 6U ; (wPos < wDataSize) && ((wDataSize - wPos) >= 12U) ; ){
		// アイテムサイズ計算
		const WORD wItemLen = (((WORD)(pHexData[wPos + 10] & 0x0FU) << 8) | (WORD)pHexData[wPos + 11]) + 12U;
		m_EitItem.SetData(&pHexData[wPos], wItemLen);
		wPos += wItemLen;
		
		// ハンドラへ通知
		if(m_EitItem.ParseItem()){
			m_pHandler->OnEitTable(this, &m_EitItem);
			}
		}	

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// [0x73] TOTテーブル基底クラス
/////////////////////////////////////////////////////////////////////////////

void CTotTable::Reset(void)
{
	// 状態をクリアする
	m_DateTime.ClearTime();
	m_DescBlock.Reset();
	
	CPsiTableBase::Reset();
}

const SYSTEMTIME & CTotTable::GetDateTime(void) const
{
	// 現在日付/時刻を返す
	return m_DateTime;
}

const IDescBlock * CTotTable::GetTotDesc(void) const
{
	// 記述子領域を返す
	return &m_DescBlock;
}

CTotTable::CTotTable(IBonObject *pOwner)
	: CPsiTableBase(pOwner)
	, m_DescBlock(NULL)
	, m_pHandler(dynamic_cast<ITotTable::IHandler *>(pOwner))
{
	// 標準形式セクションを選択
	m_SectionParser.SetExtSection(false);
}

CTotTable::~CTotTable(void)
{
	// 何もしない
}

const bool CTotTable::OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection)
{
	const WORD wDataSize = pNewSection->GetPayloadSize();
	const BYTE *pHexData = pNewSection->GetPayloadData();

	if(pNewSection->GetTableID() != ITotTable::TABLE_ID)return false;	// テーブルIDが不正

	// テーブルを解析する
	m_DateTime.SetAribTime(pHexData);
	
	const WORD wDescLen = ((WORD)(pHexData[5] & 0x0FU) << 8) | (WORD)pHexData[6];
	m_DescBlock.SetData(&pHexData[7], wDescLen);
	m_DescBlock.ParseDescs();

	// ハンドラへ通知
	if(m_pHandler)m_pHandler->OnTotTable(this);

	return true;
}
