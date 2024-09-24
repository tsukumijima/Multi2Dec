// B25Decoder.cpp: CCB25Decoder クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "B25Decoder.h"


//////////////////////////////////////////////////////////////////////
// インスタンス生成メソッド
//////////////////////////////////////////////////////////////////////

extern "C"
{

B25DECAPI IB25Decoder * CreateB25Decoder()
{
	// インスタンス生成
	return dynamic_cast<IB25Decoder *>(new CB25Decoder(NULL));
}

B25DECAPI IB25Decoder2 * CreateB25Decoder2()
{
	// インスタンス生成
	return dynamic_cast<IB25Decoder2 *>(new CB25Decoder(NULL));
}

}


//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

CB25Decoder::CB25Decoder(IBonObject * const pOwner)
	: CBonObject(pOwner)
	, m_TsPacketSync(NULL)
	, m_TsDescrambler(NULL)
	, m_MediaGrabber(dynamic_cast<IMediaDecoderHandler *>(this))
	, m_InputBuff((BYTE)0x00U, 0x100000UL)	// 1MBのバッファを確保
	, m_OutputBuff((BYTE)0x00U, 0x100000UL)	// 1MBのバッファを確保
{
	// デコーダグラフ構築
	m_TsPacketSync.SetOutputDecoder(&m_TsDescrambler);
	m_TsDescrambler.SetOutputDecoder(&m_MediaGrabber);
}

CB25Decoder::~CB25Decoder(void)
{
	// 何もしない
}

const BOOL CB25Decoder::Initialize(DWORD dwRound)
{
	// デコーダオープン、デコーダグラフ再生
	return Reset();
}

const BOOL CB25Decoder::Decode(BYTE *pSrcBuf, const DWORD dwSrcSize, BYTE **ppDstBuf, DWORD *pdwDstSize)
{
	CBlockLock AutoLock(&m_GraphLock);

	if(!pSrcBuf || !pdwDstSize || !ppDstBuf || !pdwDstSize){
		// 引数が不正
		return FALSE;
		}

	if(!m_TsDescrambler.GetHalBcasCard()){
		// B-CASカードがオープンされていない
		return FALSE;
		}

	// 出力バッファクリア
	m_OutputBuff.ClearSize();

	// メディアデータセット
	m_InputBuff.SetData(pSrcBuf, dwSrcSize);
	
	// デコーダに入力する
	if(!m_TsPacketSync.InputMedia(&m_InputBuff)){
		// デコーダグラフでエラー発生
		return FALSE;
		}

	// 出力データセット
	*ppDstBuf = m_OutputBuff.GetData();
	*pdwDstSize = m_OutputBuff.GetSize();

	return TRUE;
}

const BOOL CB25Decoder::Flush(BYTE **ppDstBuf, DWORD *pdwDstSize)
{
	// このメソッドは効果がない(互換性のため実装)
	if(!ppDstBuf || !pdwDstSize){
		// 引数が不正
		return FALSE;
		}

	// 出力データセット(データは残らないため常に空)
	*ppDstBuf = NULL;
	*pdwDstSize = 0;

	return TRUE;
}

const BOOL CB25Decoder::Reset(void)
{
	CBlockLock AutoLock(&m_GraphLock);

	// 一旦デコーダグラフ停止
	m_TsPacketSync.StopDecoder();
	
	// パケット同期デコーダ設定
	m_TsPacketSync.DiscardNullPacket(false);
	
	// デスクランブラデコーダオープン
	if(!m_TsDescrambler.OpenDescrambler(TEXT("CBcasCardReader")))return FALSE;

	// デスクランブラデコーダ設定
	m_TsDescrambler.EnableEmmProcess(false);
	m_TsDescrambler.DiscardScramblePacket(false);

	// 出力バッファクリア
	m_OutputBuff.ClearSize();

	// 統計情報リセット
	ResetStatistics();

	// デコーダグラフ再生
	return (m_TsPacketSync.PlayDecoder())? TRUE : FALSE;
}

void CB25Decoder::DiscardNullPacket(const bool bEnable)
{
	// NULLパケット破棄の有無を設定
	m_TsPacketSync.DiscardNullPacket(bEnable);
}

void CB25Decoder::DiscardScramblePacket(const bool bEnable)
{
	// 復号漏れパケット破棄の有無を設定
	m_TsDescrambler.DiscardScramblePacket(bEnable);
}

void CB25Decoder::EnableEmmProcess(const bool bEnable)
{
	// EMM処理の有効/無効を設定
	m_TsDescrambler.EnableEmmProcess(bEnable);
}

const DWORD CB25Decoder::GetDescramblingState(const WORD wProgramID)
{
	// ↓本当は必要
	// CBlockLock AutoLock(&m_GraphLock);

	// 指定したプログラムIDの復号状態を返す
	return m_TsDescrambler.GetDescramblingState(wProgramID);
}

void CB25Decoder::ResetStatistics(void)
{
	// 統計情報をリセットする
	m_TsPacketSync.ResetStatistics();
	m_TsDescrambler.ResetStatistics();
}

const DWORD CB25Decoder::GetPacketStride(void)
{
	// パケット周期を返す
	return m_TsPacketSync.GetPacketStride();
}

const DWORD CB25Decoder::GetInputPacketNum(const WORD wPID)
{
	// 入力パケット数を返す　※TS_INVALID_PID指定時は全PIDの合計を返す
	return m_TsPacketSync.GetInputPacketNum(wPID);
}

const DWORD CB25Decoder::GetOutputPacketNum(const WORD wPID)
{
	// 出力パケット数を返す　※TS_INVALID_PID指定時は全PIDの合計を返す
	return m_TsPacketSync.GetOutputPacketNum(wPID);
}

const DWORD CB25Decoder::GetSyncErrNum(void)
{
	// 同期エラー数を返す
	return m_TsPacketSync.GetSyncErrNum();
}

const DWORD CB25Decoder::GetFormatErrNum(void)
{
	// フォーマットエラー数を返す
	return m_TsPacketSync.GetFormatErrNum();
}

const DWORD CB25Decoder::GetTransportErrNum(void)
{
	// ビットエラー数を返す
	return m_TsPacketSync.GetTransportErrNum();
}

const DWORD CB25Decoder::GetContinuityErrNum(const WORD wPID)
{
	// ドロップパケット数を返す
	return m_TsPacketSync.GetContinuityErrNum(wPID);
}

const DWORD CB25Decoder::GetScramblePacketNum(const WORD wPID)
{
	// 復号漏れパケット数を返す
	return m_TsDescrambler.GetScramblePacketNum(wPID);
}

const DWORD CB25Decoder::GetEcmProcessNum(void)
{
	// ECM処理数を返す(B-CASカードアクセス回数)
	return m_TsDescrambler.GetEcmProcessNum();
}

const DWORD CB25Decoder::GetEmmProcessNum(void)
{
	// EMM処理数を返す(B-CASカードアクセス回数)
	return m_TsDescrambler.GetEmmProcessNum();
}

const DWORD CB25Decoder::OnDecoderEvent(IMediaDecoder *pDecoder, const DWORD dwEventID, PVOID pParam)
{
	if(pDecoder == dynamic_cast<IMediaDecoder *>(&m_MediaGrabber)){
		if(dwEventID == IMediaGrabber::EID_ON_MEDIADATA){	
			// 出力バッファにデータを追加
			m_OutputBuff.AddData(reinterpret_cast<IMediaData *>(pParam));		
			}
		}

	return 0UL;
}


/////////////////////////////////////////////////////////////////////////////
// For Delphi and General Non C++ Laguages
// From MobileHackerz (http://mobilehackerz.jp/contents/Software/cap_hdus)
/////////////////////////////////////////////////////////////////////////////

extern "C"
{

B25DECAPI const BOOL B25Decoder_Initialize(IB25Decoder *pB25, DWORD dwRound)
{
	return pB25->Initialize(dwRound);
}

B25DECAPI void B25Decoder_Release(IB25Decoder *pB25)
{
	pB25->Release();
}

B25DECAPI const BOOL B25Decoder_Decode(IB25Decoder *pB25, BYTE *pSrcBuf, const DWORD dwSrcSize, BYTE **ppDstBuf, DWORD *pdwDstSize)
{
	return pB25->Decode(pSrcBuf, dwSrcSize, ppDstBuf, pdwDstSize);
}

B25DECAPI const BOOL B25Decoder_Flush(IB25Decoder *pB25, BYTE **ppDstBuf, DWORD *pdwDstSize)
{
	return pB25->Flush(ppDstBuf, pdwDstSize);
}

B25DECAPI const BOOL B25Decoder_Reset(IB25Decoder *pB25)
{
	return pB25->Reset();
}

B25DECAPI const BOOL B25Decoder2_Initialize(IB25Decoder2 *pB25, DWORD dwRound)
{
	return pB25->Initialize(dwRound);
}

B25DECAPI void B25Decoder2_Release(IB25Decoder2 *pB25)
{
	pB25->Release();
}

B25DECAPI const BOOL B25Decoder2_Decode(IB25Decoder2 *pB25, BYTE *pSrcBuf, const DWORD dwSrcSize, BYTE **ppDstBuf, DWORD *pdwDstSize)
{
	return pB25->Decode(pSrcBuf, dwSrcSize, ppDstBuf, pdwDstSize);
}

B25DECAPI const BOOL B25Decoder2_Flush(IB25Decoder2 *pB25, BYTE **ppDstBuf, DWORD *pdwDstSize)
{
	return pB25->Flush(ppDstBuf, pdwDstSize);
}

B25DECAPI const BOOL B25Decoder2_Reset(IB25Decoder2 *pB25)
{
	return pB25->Reset();
}

B25DECAPI void B25Decoder2_DiscardNullPacket(IB25Decoder2 *pB25, const bool bEnable)
{
	pB25->DiscardNullPacket(bEnable);
}

B25DECAPI void B25Decoder2_DiscardScramblePacket(IB25Decoder2 *pB25, const bool bEnable)
{
	pB25->DiscardScramblePacket(bEnable);
}

B25DECAPI void B25Decoder2_EnableEmmProcess(IB25Decoder2 *pB25, const bool bEnable)
{
	pB25->EnableEmmProcess(bEnable);
}

B25DECAPI const DWORD B25Decoder2_GetDescramblingState(IB25Decoder2 *pB25, const WORD wProgramID)
{
	return pB25->GetDescramblingState(wProgramID);
}

B25DECAPI void B25Decoder2_ResetStatistics(IB25Decoder2 *pB25)
{
	pB25->ResetStatistics();
}

B25DECAPI const DWORD B25Decoder2_GetPacketStride(IB25Decoder2 *pB25)
{
	return pB25->GetPacketStride();
}

B25DECAPI const DWORD B25Decoder2_GetInputPacketNum(IB25Decoder2 *pB25, const WORD wPID)
{
	return pB25->GetInputPacketNum(wPID);
}

B25DECAPI const DWORD B25Decoder2_GetOutputPacketNum(IB25Decoder2 *pB25, const WORD wPID)
{
	return pB25->GetOutputPacketNum(wPID);
}

B25DECAPI const DWORD B25Decoder2_GetSyncErrNum(IB25Decoder2 *pB25)
{
	return pB25->GetSyncErrNum();
}

B25DECAPI const DWORD B25Decoder2_GetFormatErrNum(IB25Decoder2 *pB25)
{
	return pB25->GetFormatErrNum();
}

B25DECAPI const DWORD B25Decoder2_GetTransportErrNum(IB25Decoder2 *pB25)
{
	return pB25->GetTransportErrNum();
}

B25DECAPI const DWORD B25Decoder2_GetContinuityErrNum(IB25Decoder2 *pB25, const WORD wPID)
{
	return pB25->GetContinuityErrNum(wPID);
}

B25DECAPI const DWORD B25Decoder2_GetScramblePacketNum(IB25Decoder2 *pB25, const WORD wPID)
{
	return pB25->GetScramblePacketNum(wPID);
}

B25DECAPI const DWORD B25Decoder2_GetEcmProcessNum(IB25Decoder2 *pB25)
{
	return pB25->GetEcmProcessNum();
}

B25DECAPI const DWORD B25Decoder2_GetEmmProcessNum(IB25Decoder2 *pB25)
{
	return pB25->GetEmmProcessNum();
}

}