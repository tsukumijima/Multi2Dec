// TsDescrambler.cpp: TSデスクランブルデコーダ
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BonSDK.h"
#include "TsDescrambler.h"
#include "TsTable.h"
#include "TsPacket.h"
#ifndef _WIN32
#include "BcasCardReader.h"
#include <chrono>
namespace chrono = std::chrono;
#endif


/////////////////////////////////////////////////////////////////////////////
// ファイルローカル定数設定
/////////////////////////////////////////////////////////////////////////////

// EMM処理有効期間
#define EMM_VALID_PERIOD			7UL			// 7day

// B-CASカード再初期化ガードインターバル
#define BCAS_REFRESH_INTERVAL		1000UL		// 1s

// PMT待ちパケットカウント数
#define PMT_WAIT_PACKET_COUNT		(3250 *  200 / TS_PACKET_SIZE)	//  200ms @ 26Mbps
#define ECM_WAIT_PACKET_COUNT		(3250 * 2000 / TS_PACKET_SIZE)	// 2000ms @ 26Mbps

// クリップ付きインクリメントマクロ
#define CLIPED_INCREMENT(V)			{if((V) < 0xFFFFFFFFUL)(V)++;}


/////////////////////////////////////////////////////////////////////////////
// TSデスクランブルデコーダ
/////////////////////////////////////////////////////////////////////////////

const bool CTsDescrambler::InputMedia(IMediaData *pMediaData, const DWORD dwInputIndex)
{
	// 入力パラメータチェック
	ITsPacket * const pPacket = dynamic_cast<ITsPacket *>(pMediaData);
	if(!pPacket || (dwInputIndex >= GetInputNum()))return false;

	// PS処理
	InputPacket(pPacket);

	if(!m_bEnableBuffering || (m_DecBufState == BDS_RUNNING)){
		// パケット復号
		OutputPacket(pPacket);
		}
	else{
		// 未処理パケットバッファリング
		PushUnprocPacket(pPacket);

		// バッファリング制御
		BufManagement(pPacket);
		}

	return true;
}

const bool CTsDescrambler::OpenDescrambler(LPCTSTR lpszBCId)
{
	// 一旦クローズ
	CloseDescrambler();

	IHalDevice *pHalDevice = NULL;

	try{
#ifdef _WIN32
		// B-CASカードのインスタンスを生成する
		if(!(pHalDevice = ::BON_SAFE_CREATE<IHalDevice *>(lpszBCId)))throw __LINE__;

		// デバイスタイプをチェック
		if(pHalDevice->GetDeviceType() != ::BON_NAME_TO_GUID(TEXT("IHalBcasCard")))throw __LINE__;
#else
		// 名前解決によるインスタンス生成は未実装なので直接生成する
		if(::_tcscmp(lpszBCId, TEXT("CBcasCardReader")))throw __LINE__;
		pHalDevice = new CBcasCardReader(NULL);
#endif

		// デバイスの存在をチェック
		if(!pHalDevice->GetTotalDeviceNum())throw __LINE__;

		// デバイスの空きをチェック
		if(pHalDevice->GetActiveDeviceNum() >= pHalDevice->GetTotalDeviceNum())throw __LINE__;
	
		// IHalBcasCardインタフェース取得
		if(!(m_pHalBcasCard = dynamic_cast<IHalBcasCard *>(pHalDevice)))throw __LINE__;

		// B-CASカードをオープン
		if(!m_pHalBcasCard->OpenCard())throw __LINE__;
		
		// 全状態リセット
		OnReset();
		}
	catch(...){
		// エラー発生
		BON_SAFE_RELEASE(pHalDevice);
		m_pHalBcasCard = NULL;
		
		CloseDescrambler();
		return false;
		}

	return true;
}

const bool CTsDescrambler::CloseDescrambler(void)
{
	// B-CASカードをクローズ
	BON_SAFE_RELEASE(m_pHalBcasCard);

	return true;
}

void CTsDescrambler::DiscardScramblePacket(const bool bEnable)
{
	// スクランブル解除漏れパケットを破棄するか設定
	m_bDiscardScramblePacket = (bEnable)? true : false;
}

void CTsDescrambler::EnableEmmProcess(const bool bEnable)
{
	// EMMを処理するか設定
	m_bEnableEmmProcess = (bEnable)? true : false;
}

void CTsDescrambler::EnableBuffering(const bool bEnable)
{
	// バッファリングするか設定
	m_bEnableBuffering = (bEnable)? true : false;
}

void CTsDescrambler::ResetStatistics(void)
{
	// 統計データをリセットする
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
	// PATテーブルを取得
	const IPatTable *pPatTable = dynamic_cast<const IPatTable *>(m_TsInputMap.GetMapTarget(0x0000U));
	if(!pPatTable)return IHalBcasCard::EC_BADARGUMENT;
		
	// 引数に一致するPMTのPIDを検索
	for(WORD wIndex = 0U ; wIndex < pPatTable->GetProgramNum() ; wIndex++){
		if(pPatTable->GetProgramID(wIndex) == wProgramID){
			
			// PMTテーブルを取得
			const IPmtTable *pPmtTable = dynamic_cast<const IPmtTable *>(m_TsInputMap.GetMapTarget(pPatTable->GetPmtPID(wIndex)));
			if(!pPmtTable)return IHalBcasCard::EC_BADARGUMENT;
			
			// ECMのPIDを取得
			const IDescBlock * const pDescBlock = pPmtTable->GetPmtDesc();
			if(!pDescBlock)return IHalBcasCard::EC_BADARGUMENT;
	
			const ICaMethodDesc * const pCaMethodDesc = dynamic_cast<const ICaMethodDesc *>(pDescBlock->GetDescByTag(ICaMethodDesc::DESC_TAG));
			if(!pCaMethodDesc)return IHalBcasCard::EC_BADARGUMENT;

			const WORD wEcmPID = pCaMethodDesc->GetCaPID();
			if(wEcmPID >= TS_INVALID_PID)return IHalBcasCard::EC_NO_ERROR;

			// ECMプロセッサを取得
			const CEcmProcessor *pEcmProcessor = dynamic_cast<const CEcmProcessor *>(m_TsInputMap.GetMapTarget(pPatTable->GetPmtPID(wEcmPID)));
			if(!pEcmProcessor)return IHalBcasCard::EC_BADARGUMENT;
			
			// 復号状態を返す
			return pEcmProcessor->m_dwDescramblingState;
			}		
		}

	return IHalBcasCard::EC_BADARGUMENT;
}

const DWORD CTsDescrambler::GetInputPacketNum(const WORD wPID)
{
	// 入力パケット数を返す
	return (wPID < 0x2000U)? m_adwInputPacketNum[wPID] : m_dwInputPacketNum;
}

const DWORD CTsDescrambler::GetOutputPacketNum(const WORD wPID)
{
	// 出力パケット数を返す
	return (wPID < 0x2000U)? m_adwOutputPacketNum[wPID] : m_dwOutputPacketNum;
}

const DWORD CTsDescrambler::GetScramblePacketNum(const WORD wPID)
{
	// 復号漏れパケット数を返す
	return (wPID < 0x2000U)? m_adwScramblePacketNum[wPID] : m_dwScramblePacketNum;
}

const DWORD CTsDescrambler::GetEcmProcessNum(void)
{
	// ECM処理回数を返す
	return m_dwEcmProcessNum;
}

const DWORD CTsDescrambler::GetEmmProcessNum(void)
{
	// EMM処理回数を返す
	return m_dwEmmProcessNum;
}

IHalBcasCard * CTsDescrambler::GetHalBcasCard(void)
{
	// B-CASカードリーダデバイスを返す
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
	// 統計データを初期化する
	ResetStatistics();

	// PATテーブルをPIDマップに追加
	m_TsInputMap.MapPid(0x0000U, dynamic_cast<ITsPidMapTarget *>(CPatTable::CreateInstance(dynamic_cast<IPatTable::IHandler *>(this))));

	// CATテーブルをPIDマップに追加
	m_TsInputMap.MapPid(0x0001U, dynamic_cast<ITsPidMapTarget *>(CCatTable::CreateInstance(dynamic_cast<ICatTable::IHandler *>(this))));

	// TOTテーブルをPIDマップに追加
	m_TsInputMap.MapPid(0x0014U, dynamic_cast<ITsPidMapTarget *>(CTotTable::CreateInstance(dynamic_cast<ITotTable::IHandler *>(this))));
}

CTsDescrambler::~CTsDescrambler(void)
{
	// B-CASカードクローズ
	CloseDescrambler();
	
	// バッファ開放
	ClearUnprocPacket();
}

const bool CTsDescrambler::OnPlay(void)
{
	// 内部状態を初期化する
	return OnReset();
}

const bool CTsDescrambler::OnStop(void)
{
	// 未処理パケットをフラッシュする
	FlushUnprocPacket();

	return CMediaDecoder::OnStop();
}

const bool CTsDescrambler::OnReset(void)
{
	// PIDマップリセット
	ResetPidMap();

	// TSIDリセット
	m_wLastTsID = TS_INVALID_PID;

	// PATリセット
	m_TsInputMap.ResetPid(0x0000U);

	// バッファリング状態リセット
	m_DecBufState = BDS_INITIAL;
	m_dwPmtWaitCount = 0UL;
	ClearUnprocPacket();

	// 統計データリセット
	ResetStatistics();

	return true;
}

void CTsDescrambler::OnPatTable(const IPatTable *pPatTable)
{
	// TSID更新確認
	if(m_bEnableBuffering && (m_wLastTsID != pPatTable->GetTsID())){
		if(m_wLastTsID != TS_INVALID_PID){
			// 未処理パケットをフラッシュする
			FlushUnprocPacket();
			m_DecBufState = BDS_INITIAL;
			m_dwPmtWaitCount = 0UL;
			
			// PIDマップリセット
			ResetPidMap();
			}
		m_wLastTsID = pPatTable->GetTsID();
		}

	// PMTテーブルPIDマップ追加
	for(WORD wIndex = 0U ; wIndex < pPatTable->GetProgramNum() ; wIndex++){
		m_TsInputMap.MapPid(pPatTable->GetPmtPID(wIndex), dynamic_cast<ITsPidMapTarget *>(CPmtTable::CreateInstance(dynamic_cast<IPmtTable::IHandler *>(this))));
		}

	// バッファリング状態状態更新
	if(m_DecBufState == BDS_INITIAL){
		m_DecBufState = BDS_STORING_PMT;
		m_dwPmtWaitCount = 0UL;
		}
}

void CTsDescrambler::OnCatTable(const ICatTable *pCatTable)
{
	// EMMのPIDを取得する
	const IDescBlock * const pDescBlock = pCatTable->GetCatDesc();
	if(!pDescBlock)return;
	
	const ICaMethodDesc * const pCaMethodDesc = dynamic_cast<const ICaMethodDesc *>(pDescBlock->GetDescByTag(ICaMethodDesc::DESC_TAG));
	if(!pCaMethodDesc)return;

	const WORD wEmmPID = pCaMethodDesc->GetCaPID();
	if(wEmmPID >= TS_INVALID_PID)return;

	// EMMプロセッサをPIDマップに追加
	m_TsInputMap.MapPid(wEmmPID, dynamic_cast<ITsPidMapTarget *>(CEmmProcessor::CreateInstance(dynamic_cast<ITsDescrambler *>(this))));
}

void CTsDescrambler::OnPmtTable(const IPmtTable *pPmtTable)
{
	// ECMのPIDを取得する
	const IDescBlock * const pDescBlock = pPmtTable->GetPmtDesc();
	if(!pDescBlock)return;
	
	const ICaMethodDesc * const pCaMethodDesc = dynamic_cast<const ICaMethodDesc *>(pDescBlock->GetDescByTag(ICaMethodDesc::DESC_TAG));
	if(!pCaMethodDesc)return;

	const WORD wEcmPID = pCaMethodDesc->GetCaPID();
	if(wEcmPID >= TS_INVALID_PID)return;

	// 既存のECMプロセッサを確認
	if(!m_TsInputMap.GetMapTarget(wEcmPID)){
		// ECMプロセッサをPIDマップに追加
		m_TsInputMap.MapPid(wEcmPID, dynamic_cast<ITsPidMapTarget *>(CEcmProcessor::CreateInstance(dynamic_cast<ITsDescrambler *>(this))));
		}

	// ESプロセッサをPIDマップに追加
	for(WORD wIndex = 0U ; wIndex < pPmtTable->GetEsNum() ; wIndex++){
		m_TsOutputMap.MapPid(pPmtTable->GetEsPID(wIndex), dynamic_cast<ITsPidMapTarget *>(CEsProcessor::CreateInstance(m_TsInputMap.GetMapTarget(wEcmPID))));
		}
}

void CTsDescrambler::OnTotTable(const ITotTable *pTotTable)
{
	// 何もしない
}

void CTsDescrambler::InputPacket(ITsPacket * const pPacket)
{
	// PIDマップに入力
	if(m_pHalBcasCard)m_TsInputMap.StorePacket(pPacket);

	// 入力パケット数インクリメント
	CLIPED_INCREMENT(m_dwInputPacketNum);
	CLIPED_INCREMENT(m_adwInputPacketNum[pPacket->GetPID()]);
}

void CTsDescrambler::BufManagement(ITsPacket * const pPacket)
{
	if(m_PacketBuf.size() >= ECM_WAIT_PACKET_COUNT){
		// バッファリングリミット超過
		FlushUnprocPacket();
		m_DecBufState = BDS_RUNNING;
		}
	else if(m_DecBufState == BDS_STORING_PMT){
		// PMT待ちカウンタインクリメント
		if(++m_dwPmtWaitCount >= PMT_WAIT_PACKET_COUNT){
			m_DecBufState = BDS_STORING_ECM;
			}
		}
	else if(m_DecBufState == BDS_STORING_ECM){
		// ECMストア状態モニタ
		if(dynamic_cast<CEcmProcessor *>(m_TsInputMap.GetMapTarget(pPacket->GetPID()))){
			bool bNotStored = false;
		
			for(WORD wPID = 0x0000U ; wPID < 0x2000U ; wPID++){
				const CEcmProcessor *pEcmProcessor = dynamic_cast<CEcmProcessor *>(m_TsInputMap.GetMapTarget(wPID));
				if(pEcmProcessor){
					if(!pEcmProcessor->IsEcmReceived()){
						// ECM未受信
						bNotStored = true;
						}
					}
				}

			if(!bNotStored){
				m_DecBufState = BDS_RUNNING;
			
				// 未処理パケット出力
				FlushUnprocPacket();
				}
			}
		}
}

void CTsDescrambler::OutputPacket(ITsPacket * const pPacket)
{
	const WORD wPID = pPacket->GetPID();

	// ESプロセッサに入力
	m_TsOutputMap.StorePacket(pPacket);

	if(pPacket->IsScrambled()){
		// 復号漏れパケット数インクリメント
		CLIPED_INCREMENT(m_dwScramblePacketNum);
		CLIPED_INCREMENT(m_adwScramblePacketNum[wPID]);

		SendDecoderEvent(EID_CANT_DESCRAMBLE, reinterpret_cast<PVOID>(m_dwScramblePacketNum));
		
		// 復号漏れパケット破棄有効時は下位デコーダに出力しない
		if(m_bDiscardScramblePacket)return;
		}

	// 出力パケット数インクリメント
	CLIPED_INCREMENT(m_dwOutputPacketNum);
	CLIPED_INCREMENT(m_adwOutputPacketNum[wPID]);

	// 下位デコーダにデータ出力
	OutputMedia(dynamic_cast<IMediaData *>(pPacket));
}

void CTsDescrambler::ResetPidMap(void)
{
	// PAT/CAT/TOT以外アンマップ
	for(WORD wPID = 0x0002U ; wPID < 0x2000U ; wPID++){
		if(wPID != 0x0014U){
			m_TsInputMap.UnmapPid(wPID);
			}	
		}

	// CATリセット
	m_TsInputMap.ResetPid(0x0001U);

	// TOTリセット
	m_TsInputMap.ResetPid(0x0014U);
	
	// ESプロセッサリセット
	m_TsOutputMap.ClearPid();
}

void CTsDescrambler::FlushUnprocPacket(void)
{
	// 未処理パケットをフラッシュする
	for(DWORD dwIndex = 0UL ; dwIndex < m_PacketBuf.size() ; dwIndex++){
		OutputPacket(m_PacketBuf[dwIndex]);
		}

	// バッファをクリア
	ClearUnprocPacket();
}

void CTsDescrambler::PushUnprocPacket(const ITsPacket *pPacket)
{
	// 未処理パケットをバッファにプッシュする
	CTsPacket * const pNewPacket = new CTsPacket(NULL);

	if(pNewPacket->CopyPacket(pPacket)){	
		m_PacketBuf.push_back(pNewPacket);
		}
}

void CTsDescrambler::ClearUnprocPacket(void)
{
	// 未処理パケットをクリア
	for(DWORD dwIndex = 0UL ; dwIndex < m_PacketBuf.size() ; dwIndex++){
		BON_SAFE_RELEASE(m_PacketBuf[dwIndex]);
		}
	
	m_PacketBuf.clear();
}


/////////////////////////////////////////////////////////////////////////////
// ECM処理内部クラス
/////////////////////////////////////////////////////////////////////////////

void CTsDescrambler::CEcmProcessor::OnPidReset(ITsPidMapper *pTsPidMapper, const WORD wPID)
{
	// キークリアする(誤ったキーによる破損を防止するため)
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
	, m_dwLastRetryTime(
#ifdef _WIN32
		::GetTickCount() - BCAS_REFRESH_INTERVAL
#else
		(DWORD)chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count() - (DWORD)BCAS_REFRESH_INTERVAL
#endif
		)
	, m_dwDescramblingState(IHalBcasCard::EC_NO_ERROR)
{
	// MULTI2デコーダを初期化
	m_Multi2Decoder.Initialize(m_pHalBcasCard->GetSystemKey(), m_pHalBcasCard->GetInitialCbc());
}

CTsDescrambler::CEcmProcessor::~CEcmProcessor(void)
{
	// このインスタンスを参照しているCEsProcessorをアンマップする
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
	// ECMの受信有無を返す
	return m_bIsEcmReceived;
}

const bool CTsDescrambler::CEcmProcessor::DescramblePacket(ITsPacket * const pTsPacket)
{
	// IMediaDataインタフェース問い合わせ
	IMediaData *pMediaData = dynamic_cast<IMediaData *>(pTsPacket);

	// スクランブル解除
	if(pMediaData){
		if(m_Multi2Decoder.Decode(pTsPacket->GetPayloadData(), (DWORD)pTsPacket->GetPayloadSize(), pTsPacket->m_Header.byTransportScramblingCtrl, CTsPacket::EXTRA_BUFFER_ALLOCATION_SIZE)){
			// トランスポートスクランブル制御再設定
			pMediaData->SetAt(3UL, pMediaData->GetAt(3UL) & 0x3FU);
			pTsPacket->m_Header.byTransportScramblingCtrl = 0U;

			return true;
			}
		}
	
	return false;
}

const bool CTsDescrambler::CEcmProcessor::OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection)
{
	// テーブルIDをチェック
	if(pNewSection->GetTableID() != TABLE_ID)return false;

	// ECMをB-CASカードに渡してキー取得
	const BYTE *pKsData = m_pHalBcasCard->GetKsFromEcm(pNewSection->GetPayloadData(), pNewSection->GetPayloadSize());

	// ECM処理失敗時は一度だけB-CASカードを再初期化する
	if(!pKsData && (m_pHalBcasCard->GetEcmError() != IHalBcasCard::EC_NOT_CONTRACTED)){
		DWORD dwTick =
#ifdef _WIN32
			::GetTickCount();
#else
			(DWORD)chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count();
#endif
		if((dwTick - m_dwLastRetryTime) >= BCAS_REFRESH_INTERVAL){
			// 再初期化ガードインターバル経過なら			
			if(m_pHalBcasCard->OpenCard()){
				pKsData = m_pHalBcasCard->GetKsFromEcm(pNewSection->GetPayloadData(), pNewSection->GetPayloadSize());
				}
				
			// 最終リトライ時間更新
			m_dwLastRetryTime = dwTick;
			}
		}

	// スクランブルキー更新
	m_Multi2Decoder.SetScrambleKey(pKsData);

	// イベント通知
	if(pKsData)CLIPED_INCREMENT(m_pTsDescrambler->m_dwEcmProcessNum);
	m_pTsDescrambler->SendDecoderEvent(EID_ECM_PROCESSED, reinterpret_cast<PVOID>(m_pHalBcasCard->GetEcmError()));

	// 復号状態更新
	m_dwDescramblingState = m_pHalBcasCard->GetEcmError();
	m_bIsEcmReceived = true;

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// EMM処理内部クラス
/////////////////////////////////////////////////////////////////////////////

CTsDescrambler::CEmmProcessor::CEmmProcessor(IBonObject *pOwner)
	: CPsiTableBase(NULL)
	, m_pTsDescrambler(dynamic_cast<CTsDescrambler *>(pOwner))
	, m_pHalBcasCard(m_pTsDescrambler->m_pHalBcasCard)
{
	// 何もしない
}

CTsDescrambler::CEmmProcessor::~CEmmProcessor(void)
{
	// 何もしない
}

const bool CTsDescrambler::CEmmProcessor::OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection)
{
	// テーブルIDをチェック
	if(pNewSection->GetTableID() != SECTION_TABLE_ID)return false;

	// EMM処理が無効のときは何もしない
	if(!m_pTsDescrambler->m_bEnableEmmProcess)return true;

	// セクションを解析
	const WORD wDataSize = pNewSection->GetPayloadSize();
	const BYTE *pHexData = pNewSection->GetPayloadData();
	
	WORD wPos = 0U;
	
	while((wDataSize - wPos) >= 17U){
		// EMMサイズをチェック
		const WORD wEmmSize = (WORD)pHexData[wPos + 6U] + 7U;
		if(((wDataSize - wPos) < wEmmSize) || (wEmmSize < 17U))break;

		// ハンドラ呼び出し
		if(OnEmmBody(&pHexData[wPos], wEmmSize))break;
		
		// 解析位置更新
		wPos += wEmmSize;
		}

	return true;
}

const bool CTsDescrambler::CEmmProcessor::OnEmmBody(const BYTE *pEmmData, const WORD wEmmSize)
{
	// 自B-CASカードID取得
	const BYTE * const pCardID = m_pHalBcasCard->GetBcasCardID();

	// 自B-CASカードIDと照合
	for(DWORD dwPos = 0UL ; dwPos < 6UL ; dwPos++){
		if(pCardID[dwPos] != pEmmData[dwPos])return false;
		}

	// ストリームの時間を確認する
	CTsTime ValidTime;
	ValidTime.SetNowTime();									// 現在時刻を取得
	ValidTime -= EMM_VALID_PERIOD * 24UL * 60UL * 60UL;		// 有効期間だけ過去に戻す
	
	const ITotTable *pTotTable = dynamic_cast<const ITotTable *>(m_pTsDescrambler->m_TsInputMap.GetMapTarget(0x0014U));
	if(!pTotTable)return true;
	CTsTime EmmTime = pTotTable->GetDateTime();				// TOTから時刻を取得
	
	// 有効期間より過去のEMMデータは処理しない
	if(EmmTime < ValidTime)return true;
	
	// B-CASカードにEMMデータ送信
	if(!m_pHalBcasCard->SendEmmSection(pEmmData, wEmmSize)){
		// リトライ
		if(!m_pHalBcasCard->SendEmmSection(pEmmData, wEmmSize)){
			// EMM処理失敗
			m_pTsDescrambler->SendDecoderEvent(EID_EMM_PROCESSED, reinterpret_cast<PVOID>(false));
			return true;
			}
		}

	// イベント通知
	CLIPED_INCREMENT(m_pTsDescrambler->m_dwEmmProcessNum);
	m_pTsDescrambler->SendDecoderEvent(EID_EMM_PROCESSED, reinterpret_cast<PVOID>(true));

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// ES処理内部クラス
/////////////////////////////////////////////////////////////////////////////

CTsDescrambler::CEsProcessor::CEsProcessor(IBonObject *pOwner)
	: CBonObject(NULL)
	, m_pEcmProcessor(dynamic_cast<CEcmProcessor *>(pOwner))
{
	// 何もしない
}

CTsDescrambler::CEsProcessor::~CEsProcessor(void)
{
	// 何もしない
}

const bool CTsDescrambler::CEsProcessor::StorePacket(const ITsPacket *pTsPacket)
{
	// スクランブル解除
	return m_pEcmProcessor->DescramblePacket(const_cast<ITsPacket *>(pTsPacket));
}

void CTsDescrambler::CEsProcessor::OnPidReset(ITsPidMapper *pTsPidMapper, const WORD wPID)
{
	// 何もしない
}

void CTsDescrambler::CEsProcessor::OnPidMapped(ITsPidMapper *pTsPidMapper, const WORD wPID)
{
	// 何もしない
}

void CTsDescrambler::CEsProcessor::OnPidUnmapped(ITsPidMapper *pTsPidMapper, const WORD wPID)
{
	// インスタンス開放
	Release();
}
