// ProgAnalyzer.cpp: プログラムアナライザデコーダ
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "TsTable.h"
#include "ProgAnalyzer.h"
#include <TChar.h>


/////////////////////////////////////////////////////////////////////////////
// プログラムアナライザデコーダ
/////////////////////////////////////////////////////////////////////////////

const bool CProgAnalyzer::InputMedia(IMediaData *pMediaData, const DWORD dwInputIndex)
{
	// 入力パラメータチェック
	ITsPacket * const pTspacket = dynamic_cast<ITsPacket *>(pMediaData);
	if(!pTspacket || (dwInputIndex >= GetInputNum()))return false;

	// PIDマップに入力
	m_TsPidMapper.StorePacket(pTspacket);

	// 下位デコーダにデータ出力
	OutputMedia(pMediaData);

	return true;
}

const WORD CProgAnalyzer::GetNetworkID(void)
{
	// オリジナルネットワークIDを返す
	return m_ProgContext.wNetworkID;
}

const WORD CProgAnalyzer::GetTsID(void)
{
	// TS IDを返す
	return m_ProgContext.wTsID;
}

const DWORD CProgAnalyzer::GetProgramNum(void)
{
	// 有効なプログラム数を返す
	DWORD dwNum = 0UL;

	for(ProgramList::iterator itProgram = m_ProgContext.ProgList.begin() ; itProgram != m_ProgContext.ProgList.end() ; itProgram++){
		if(itProgram->bAvailable)dwNum++;
		}

	return dwNum;
}

const WORD CProgAnalyzer::GetProgramID(const DWORD dwIndex)
{
	// プログラムIDを返す
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
	// 映像PIDを返す
	const PROGRAM_ITEM * pItem = FindProgramItem(wProgramID);
	return (pItem)? pItem->wVideoPID : TS_INVALID_PID;
}

const WORD CProgAnalyzer::GetProgramAudioPID(const WORD wProgramID)
{
	// 音声PIDを返す
	const PROGRAM_ITEM * pItem = FindProgramItem(wProgramID);
	return (pItem)? pItem->wAudioPID : TS_INVALID_PID;
}

const WORD CProgAnalyzer::GetProgramPcrPID(const WORD wProgramID)
{
	// PCRのPIDを返す
	const PROGRAM_ITEM * pItem = FindProgramItem(wProgramID);
	return (pItem)? pItem->wPcrPID : TS_INVALID_PID;
}

const DWORD CProgAnalyzer::GetNetworkName(LPTSTR lpszNetworkName)
{
	// ネットワーク名を返す
	if(lpszNetworkName)::_tcscpy(lpszNetworkName, m_ProgContext.szNetworkName);

	// バッファに必要な文字数を返す
	return ::_tcslen(m_ProgContext.szNetworkName);
}

const DWORD CProgAnalyzer::GetServiceName(LPTSTR lpszServiceName, const WORD wProgramID)
{
	// サービス名を返す
	const PROGRAM_ITEM * pItem = FindProgramItem(wProgramID);
	if(!pItem)return 0UL;

	// バッファにコピー
	if(lpszServiceName)::_tcscpy(lpszServiceName, pItem->szServiceName);

	// バッファに必要な文字数を返す
	return ::_tcslen(pItem->szServiceName);
}

const bool CProgAnalyzer::GetStreamTime(SYSTEMTIME *pStreamTime)
{
	// ストリームの時間を返す
	ITotTable *pTotTable = dynamic_cast<ITotTable *>(m_TsPidMapper.GetMapTarget(0x0014U));
	::BON_ASSERT(pTotTable != NULL);

	CTsTime StreamTime = pTotTable->GetDateTime();
	if(StreamTime.IsEmpty())return false;

	// 時間をセット
	if(pStreamTime)*pStreamTime = StreamTime;

	return true;
}

const WORD CProgAnalyzer::GetCurrentProgramID(void)
{
	// 現在のプログラムIDを返す
	return m_ProgContext.wCurProgramID;
}

const bool CProgAnalyzer::SetCurrentProgramID(const WORD wProgramID)
{
	// 現在のプログラムIDを設定する
	if(wProgramID){
		// 指定プログラムIDに変更
		if(FindProgramItem(wProgramID))return false;
		m_ProgContext.wCurProgramID = wProgramID;
		}
	else{
		// デフォルトプログラムIDに変更(PATの最初のプログラムID)
		if(!m_ProgContext.ProgList.size())return false;
		m_ProgContext.wCurProgramID = m_ProgContext.ProgList[0].wProgramID;
		}

	return true;
}

const IEpgItem * CProgAnalyzer::GetCurrentEpgInfo(const WORD wProgramID)
{
	// 現在のEPG情報を返す
	const PROGRAM_ITEM * pItem = FindProgramItem(wProgramID);
	return (pItem)? pItem->pCurEpgItem : NULL;
}

const IEpgItem * CProgAnalyzer::GetNextEpgInfo(const WORD wProgramID)
{
	// 次のEPG情報を返す
	const PROGRAM_ITEM * pItem = FindProgramItem(wProgramID);
	return (pItem)? pItem->pNextEpgItem : NULL;
}

CProgAnalyzer::CProgAnalyzer(IBonObject *pOwner)
	: CMediaDecoder(pOwner, 1UL, 1UL)
	, m_TsPidMapper(NULL)
{
	// PATテーブルをPIDマップに追加
	m_TsPidMapper.MapPid(0x0000U, dynamic_cast<ITsPidMapTarget *>(CPatTable::CreateInstance(dynamic_cast<IPatTable::IHandler *>(this))));

	// NITテーブルをPIDマップに追加
	m_TsPidMapper.MapPid(0x0010U, dynamic_cast<ITsPidMapTarget *>(CNitTable::CreateInstance(dynamic_cast<INitTable::IHandler *>(this))));

	// SDTテーブルをPIDマップに追加
	m_TsPidMapper.MapPid(0x0011U, dynamic_cast<ITsPidMapTarget *>(CSdtTable::CreateInstance(dynamic_cast<ISdtTable::IHandler *>(this))));

	// EITテーブルをPIDマップに追加
	m_TsPidMapper.MapPid(0x0012U, dynamic_cast<ITsPidMapTarget *>(CEitTable::CreateInstance(dynamic_cast<IEitTable::IHandler *>(this))));

	// TOTテーブルをPIDマップに追加
	m_TsPidMapper.MapPid(0x0014U, dynamic_cast<ITsPidMapTarget *>(CTotTable::CreateInstance(dynamic_cast<ITotTable::IHandler *>(this))));

	// 状態リセット
	OnReset();
}

CProgAnalyzer::~CProgAnalyzer(void)
{
	// インスタンス開放
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
	// コンテキストリセット
	m_ProgContext.wNetworkID = 0x0000U;
	m_ProgContext.wTsID = 0x0000U;
	m_ProgContext.wCurProgramID = 0x0000U;
	m_ProgContext.szNetworkName[0] = TEXT('\0');

	for(ProgramList::iterator itProgram = m_ProgContext.ProgList.begin() ; itProgram != m_ProgContext.ProgList.end() ; itProgram++){
		BON_SAFE_RELEASE(itProgram->pCurEpgItem);
		BON_SAFE_RELEASE(itProgram->pNextEpgItem);
		}

	m_ProgContext.ProgList.clear();

	// PIDマップリセット
	ResetPidMap();

	// PATリセット
	m_TsPidMapper.ResetPid(0x0000U);

	return true;
}

void CProgAnalyzer::OnPatTable(const IPatTable *pPatTable)
{
	// TS IDの変化をチェックする
	if(m_ProgContext.wTsID != pPatTable->GetTsID()){
		// TS IDが変化したときは状態をリセットする
		ResetPidMap();
		m_ProgContext.wTsID = pPatTable->GetTsID();
		}

	// PMTテーブルをマップ
	for(DWORD dwIndex = 0UL ; dwIndex < pPatTable->GetProgramNum() ; dwIndex++){
		m_TsPidMapper.MapPid(pPatTable->GetPmtPID(dwIndex), dynamic_cast<ITsPidMapTarget *>(CPmtTable::CreateInstance(dynamic_cast<IPmtTable::IHandler *>(this))));
		}

	// 存在しないプログラムIDを削除する
	for(ProgramList::iterator itProgram = m_ProgContext.ProgList.begin() ; itProgram != m_ProgContext.ProgList.end() ; ){

		// PATを検索
		bool bFound = false;

		for(DWORD dwIndex = 0UL ; (dwIndex < pPatTable->GetProgramNum()) && !bFound; dwIndex++){
			if(itProgram->wProgramID == pPatTable->GetProgramID(dwIndex)){
				bFound = true;
				}
			}

		if(!bFound){
			// プログラムリストから削除
			BON_SAFE_RELEASE(itProgram->pCurEpgItem);
			BON_SAFE_RELEASE(itProgram->pNextEpgItem);

			m_ProgContext.ProgList.erase(itProgram);
			itProgram = m_ProgContext.ProgList.begin();
			}
		else{
			itProgram++;
			}
		}

	// 新規のプログラムIDを追加する
	for(DWORD dwIndex = 0UL ; dwIndex < pPatTable->GetProgramNum() ; dwIndex++){
		if(FindProgramItem(pPatTable->GetProgramID(dwIndex)))continue;

		// プログラムIDが昇順になるように挿入
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

	// 現在のプログラムIDの変更を判定する
	if(!FindProgramItem(m_ProgContext.wCurProgramID)){
		// 新規のプログラムIDに変更
		m_ProgContext.wCurProgramID = (m_ProgContext.ProgList.size())? m_ProgContext.ProgList[0].wProgramID : 0x0000U;
		}
}

void CProgAnalyzer::OnPmtTable(const IPmtTable *pPmtTable)
{
	// プログラム情報を更新する
	PROGRAM_ITEM *pProgItem = FindProgramItem(pPmtTable->GetProgramID());
	if(!pProgItem)return;

	// PCRのPID更新
	pProgItem->wPcrPID = pPmtTable->GetPcrPID();

	// 映像/音声のPID更新
	for(DWORD dwIndex = 0UL ; dwIndex < pPmtTable->GetEsNum() ; dwIndex++){
		switch(pPmtTable->GetStreamTypeID(dwIndex)){
			case 0x02U :	// ITU-T勧告H.262|ISO/IEC 13818-2映像
				pProgItem->wVideoPID = pPmtTable->GetEsPID(dwIndex);
				break;

			case 0x0FU :	// ISO/IEC 13818-7音声（ADTSトランスポート構造）
				pProgItem->wAudioPID = pPmtTable->GetEsPID(dwIndex);
				break;
			}
		}

	// 有効フラグ更新
	if((pProgItem->wVideoPID != TS_INVALID_PID) && (pProgItem->wAudioPID != TS_INVALID_PID)){
		// MPEG2映像とAAC音声があるプログラムを有効とする
		pProgItem->bAvailable = true;
		}
}

void CProgAnalyzer::OnNitTable(const INitTable *pNitTable)
{
	// ネットワークID更新
	m_ProgContext.wNetworkID = pNitTable->GetNetworkID();

	// ネットワーク名更新
	const INetworkNameDesc *pNetworkNameDesc = dynamic_cast<const INetworkNameDesc *>(pNitTable->GetNetworkDesc()->GetDescByTag(INetworkNameDesc::DESC_TAG));
	if(pNetworkNameDesc)pNetworkNameDesc->GetNetworkName(m_ProgContext.szNetworkName);
}

void CProgAnalyzer::OnSdtTable(const ISdtTable *pSdtTable)
{
	// サービス情報を更新する
	for(DWORD dwIndex = 0UL ; dwIndex < pSdtTable->GetServiceNum() ; dwIndex++){
		const WORD wServiceID = pSdtTable->GetServiceID(dwIndex);
		PROGRAM_ITEM *pProgItem = FindProgramItem(wServiceID);
		if(!pProgItem)continue;

		// サービス情報更新
		const IServiceDesc *pServiceDesc = dynamic_cast<const IServiceDesc *>(pSdtTable->GetServiceDesc(wServiceID)->GetDescByTag(IServiceDesc::DESC_TAG));
		if(!pServiceDesc)continue;

		pServiceDesc->GetServiceName(pProgItem->szServiceName);
		}
}

void CProgAnalyzer::OnTotTable(const ITotTable *pTotTable)
{
	// 何もしない
}

void CProgAnalyzer::OnEitTable(const IEitTable *pEitTable, const IEitItem *pEitItem)
{
	// EPG情報を更新する
	if(pEitTable->GetOrgNetworkID() != GetNetworkID())return;
	if(pEitTable->GetTsID() != GetTsID())return;

	PROGRAM_ITEM *pProgItem = FindProgramItem(pEitTable->GetServiceID());
	if(!pProgItem)return;

	if(pEitTable->GetTableID() == IEitTable::TABLE_ID_CURORNEXT_SELF){
		// 現在 or 次のEPG情報ストア
		switch(pEitTable->GetSectionNumber()){
			case 0U :
				if(pProgItem->pCurEpgItem->StoreEitItem(pEitItem)){
					// 番組情報更新イベント
//					TCHAR szInfo[4096] = TEXT("未受信");
//					pProgItem->pCurEpgItem->GetEventDetail(szInfo);
//					::BON_TRACE(TEXT("Info: %04X\n%s\n\n"), pProgItem->pCurEpgItem->GetEventID(), szInfo);
					}
				break;

			case 1U :
				if(pProgItem->pNextEpgItem->StoreEitItem(pEitItem)){
					// 番組情報更新イベント
					}
				break; 

			default :
				// 規格上はありえない
				break;
			}
		}
	else{
		// スケジュール済みEPG情報ストア
		}
}

void CProgAnalyzer::ResetPidMap(void)
{
	// NIT/SDT/TOTリセット、それ以外はアンマップ
	for(WORD wPID = 0x0000U ; wPID < 0x2000U ; wPID++){
		switch(wPID){
			case 0x0000 :	// PAT
				break;

			case 0x0010 :	// NIT
			case 0x0011 :	// SDT
			case 0x0012 :	// EIT
			case 0x0014 :	// TOT
				// リセット
				m_TsPidMapper.ResetPid(wPID);
				break;

			default :
				// アンマップ
				m_TsPidMapper.UnmapPid(wPID);
				break;
			}
		}
}

CProgAnalyzer::PROGRAM_ITEM * CProgAnalyzer::FindProgramItem(WORD wProgramID)
{
	// プログラムID省略時は現在のプログラムを検索する
	if(!wProgramID)wProgramID = GetCurrentProgramID();

	// プログラムIDから対応するアイテムを検索して返す
	for(ProgramList::iterator itProgram = m_ProgContext.ProgList.begin() ; itProgram != m_ProgContext.ProgList.end() ; itProgram++){
		if(itProgram->wProgramID == wProgramID){
			return &(*itProgram);
			}
		}

	return NULL;
}
