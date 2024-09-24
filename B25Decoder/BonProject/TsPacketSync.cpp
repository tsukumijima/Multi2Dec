// TsPacketSync.cpp: TSパケット同期デコーダ
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BonSDK.h"
#include "TsPacketSync.h"
#ifndef _WIN32
#include <chrono>
namespace chrono = std::chrono;
#endif


/////////////////////////////////////////////////////////////////////////////
// ファイルローカル定数設定
/////////////////////////////////////////////////////////////////////////////

#define STRIDE_FILTER	10UL		// パケット周期判定フィルタ回数
#define BITRATE_PERIOD	1000UL		// ビットレート更新周期(1000ms)

// クリップ付きインクリメントマクロ
#define CLIPED_INCREMENT(V)			{if((V) < 0xFFFFFFFFUL)(V)++;}


/////////////////////////////////////////////////////////////////////////////
// TSパケット同期デコーダ
/////////////////////////////////////////////////////////////////////////////

const bool CTsPacketSync::InputMedia(IMediaData *pMediaData, const DWORD dwInputIndex)
{
	// 入力パラメータチェック
	if(!pMediaData || (dwInputIndex >= GetInputNum()))return false;

	// パケット同期
	SyncPacket(pMediaData->GetData(), pMediaData->GetSize());

	return true;
}

void CTsPacketSync::DiscardNullPacket(const bool bEnable)
{
	// NULLパケットの破棄を有効/無効化設定
	m_bDiscardNullPacket = (bEnable)? true : false;
}

void CTsPacketSync::ResetStatistics(void)
{
	// パケット周期カウンタをリセット
	m_dwCurStrideSize = 0UL;
	m_dwPrvStrideSize = 0UL;
	m_dwFixStrideSize = 0UL;
	m_dwFixStrideCount = 0UL;

	// 統計データをリセットする
	m_dwInputPacketNum = 0UL;
	m_dwOutputPacketNum = 0UL;
	m_dwSyncErrNum = 0UL;
	m_dwFormatErrNum = 0UL;
	m_dwTransportErrNum = 0UL;
	m_dwContinuityErrNum = 0UL;
	m_dwLastUpdateTime =
#ifdef _WIN32
		::GetTickCount();
#else
		(DWORD)chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count();
#endif
	m_dwLastInputPacket = 0UL;
	m_dwLastOutputPacket = 0UL;
	m_dwCurInputPacket = 0UL;
	m_dwCurOutputPacket = 0UL;

	::ZeroMemory(m_adwInputPacketNum, sizeof(m_adwInputPacketNum));
	::ZeroMemory(m_adwOutputPacketNum, sizeof(m_adwOutputPacketNum));
	::ZeroMemory(m_adwContinuityErrNum, sizeof(m_adwContinuityErrNum));

	// パケット連続性カウンタを初期化する
	::FillMemory(m_abyContCounter, sizeof(m_abyContCounter), 0x10UL);
}

const DWORD CTsPacketSync::GetPacketStride(void)
{
	// パケット周期を返す
	return m_dwFixStrideSize;
}

const DWORD CTsPacketSync::GetInputPacketNum(const WORD wPID)
{
	// 入力パケット数を返す(エラーパケット除く、NULLパケット含む)
	return (wPID < 0x2000U)? m_adwInputPacketNum[wPID] : m_dwInputPacketNum;
}

const DWORD CTsPacketSync::GetOutputPacketNum(const WORD wPID)
{
	// 出力パケット数を返す(NULLパケット除く)
	return (wPID < 0x2000U)? m_adwOutputPacketNum[wPID] : m_dwOutputPacketNum;
}

const DWORD CTsPacketSync::GetInputBitrate(void)
{
	// 入力ビットレートを返す(パケットサイズ×1秒間の入力パケット数×8bit)
	return m_dwLastInputPacket * TS_PACKET_SIZE * 8UL;
}

const DWORD CTsPacketSync::GetOutputBitrate(void)
{
	// 出力ビットレートを返す(パケットサイズ×1秒間の入力パケット数×8bit)
	return m_dwLastInputPacket * TS_PACKET_SIZE * 8UL;
}

const DWORD CTsPacketSync::GetSyncErrNum(void)
{
	// 同期エラー数を返す(パケット境界が乱れるモード)
	return m_dwSyncErrNum;
}

const DWORD CTsPacketSync::GetFormatErrNum(void)
{
	// フォーマットエラー数を返す(パケットヘッダが規格外のモード)
	return m_dwFormatErrNum;
}

const DWORD CTsPacketSync::GetTransportErrNum(void)
{
	// トランスポートエラー数を返す(Transport Error Indicatorが立つモード)
	return m_dwTransportErrNum;
}

const DWORD CTsPacketSync::GetContinuityErrNum(const WORD wPID)
{
	// 連続性エラー数を返す(Continuity Counterが不連続になるモード)
	return (wPID < 0x2000U)? m_adwContinuityErrNum[wPID] : m_dwContinuityErrNum;
}

CTsPacketSync::CTsPacketSync(IBonObject *pOwner)
	: CMediaDecoder(pOwner, 1UL, 1UL)
	, m_TsPacket(NULL)
	, m_dwCurStrideSize(0UL)
	, m_dwPrvStrideSize(0UL)
	, m_dwFixStrideSize(0UL)
	, m_dwFixStrideCount(0UL)
	, m_bDiscardNullPacket(true)
	, m_dwInputPacketNum(0UL)
	, m_dwOutputPacketNum(0UL)
	, m_dwSyncErrNum(0UL)
	, m_dwFormatErrNum(0UL)
	, m_dwTransportErrNum(0UL)
	, m_dwContinuityErrNum(0UL)
	, m_dwLastUpdateTime(0UL)
	, m_dwLastInputPacket(0UL)
	, m_dwLastOutputPacket(0UL)
	, m_dwCurInputPacket(0UL)
	, m_dwCurOutputPacket(0UL)
{
	// 統計データ初期化
	ResetStatistics();
}

CTsPacketSync::~CTsPacketSync(void)
{
	// 何もしない
}

const bool CTsPacketSync::OnPlay(void)
{
	// 内部状態を初期化する
	return OnReset();
}

const bool CTsPacketSync::OnReset(void)
{
	// パケットサイズクリア
	m_TsPacket.ClearSize();

	// 統計データリセット
	ResetStatistics();

	return true;
}

void CTsPacketSync::SyncPacket(const BYTE *pStream, const DWORD dwSize)
{
	DWORD dwCurPos = 0UL;
	DWORD dwCurSize;

	while(dwCurPos < dwSize){
		dwCurSize = m_TsPacket.GetSize();

		if(!dwCurSize){
			// 同期バイト待ち中
			do{
				m_dwCurStrideSize++;

				if(pStream[dwCurPos++] == 0x47U){
					// 同期バイト発見
					m_TsPacket.AddByte(0x47U);
					break;
					}
				}
			while(dwCurPos < dwSize);
			}
		else{
			// データ待ち
			if((dwSize - dwCurPos) >= (TS_PACKET_SIZE - dwCurSize)){
				// パケットサイズ分データがそろった
				m_TsPacket.AddData(&pStream[dwCurPos], TS_PACKET_SIZE - dwCurSize);
				m_dwCurStrideSize += (TS_PACKET_SIZE - dwCurSize);
				dwCurPos += (TS_PACKET_SIZE - dwCurSize);

				// ヘッダ解析
				if(ParseTsPacket()){
					// パケット正常

					// 同期エラー判定
					if((m_dwFixStrideSize != m_dwCurStrideSize) && m_dwFixStrideSize){
						// パケット周期異常
						CLIPED_INCREMENT(m_dwSyncErrNum);
						SendDecoderEvent(EID_SYNC_ERR, reinterpret_cast<PVOID>(m_dwSyncErrNum));
						}

					// パケット周期フィルタリング
					if(m_dwCurStrideSize == m_dwPrvStrideSize){
						if(++m_dwFixStrideCount >= STRIDE_FILTER){
							// パケット周期確定
							m_dwFixStrideSize = m_dwCurStrideSize;
							m_dwFixStrideCount = STRIDE_FILTER;
							}
						}
					else{
						m_dwPrvStrideSize = m_dwCurStrideSize;
						m_dwFixStrideCount = 0UL;
						}

					// パケット出力
					OnTsPacket();

					// ビットレート更新
					UpdateBitrate();
					}

				// パケットサイズクリア
				m_TsPacket.ClearSize();
				m_dwCurStrideSize = 0UL;
				}
			else{
				// 残りのデータはパケットサイズに満たない
				m_TsPacket.AddData(&pStream[dwCurPos], dwSize - dwCurPos);
				m_dwCurStrideSize += (dwSize - dwCurPos);
				dwCurPos += (dwSize - dwCurPos);
				}
			}
		}
}

const bool CTsPacketSync::ParseTsPacket(void)
{
	// パケットを解析する
	switch(m_TsPacket.ParsePacket(m_abyContCounter)){
		case ITsPacket::EC_VALID :
			// パケット正常
			CLIPED_INCREMENT(m_dwInputPacketNum);
			CLIPED_INCREMENT(m_adwInputPacketNum[m_TsPacket.GetPID()]);
			CLIPED_INCREMENT(m_dwCurInputPacket);
			return true;

		case ITsPacket::EC_FORMAT :
			// フォーマット異常
			CLIPED_INCREMENT(m_dwFormatErrNum);
			SendDecoderEvent(EID_FORMAT_ERR, reinterpret_cast<PVOID>(m_dwFormatErrNum));
			return false;

		case ITsPacket::EC_TRANSPORT :
			// トランスポートエラー異常
			CLIPED_INCREMENT(m_dwTransportErrNum);
			SendDecoderEvent(EID_TRANSPORT_ERR, reinterpret_cast<PVOID>(m_dwTransportErrNum));
			return false;

		case ITsPacket::EC_CONTINUITY :
			// 連続性異常(ドロップ)　※パケットの破棄はしない
			CLIPED_INCREMENT(m_dwInputPacketNum);
			CLIPED_INCREMENT(m_adwInputPacketNum[m_TsPacket.GetPID()]);
			CLIPED_INCREMENT(m_dwContinuityErrNum);
			CLIPED_INCREMENT(m_adwContinuityErrNum[m_TsPacket.GetPID()]);
			CLIPED_INCREMENT(m_dwCurInputPacket);
			SendDecoderEvent(EID_CONTINUITY_ERR, reinterpret_cast<PVOID>(m_dwContinuityErrNum));
			return true;

		default :
			// 例外
#ifdef _WIN32
			::DebugBreak();
#endif
			return false;
		}
}

void CTsPacketSync::OnTsPacket(void)
{
	// 正常パケット出力
	const WORD wPID = m_TsPacket.GetPID();

	// NULLパケットは破棄する
	if(m_bDiscardNullPacket && (wPID == 0x1FFFU))return;

	// 出力パケット数インクリメント
	CLIPED_INCREMENT(m_dwOutputPacketNum);
	CLIPED_INCREMENT(m_adwOutputPacketNum[wPID]);
	CLIPED_INCREMENT(m_dwCurOutputPacket);

	// 下位デコーダにデータ出力
	OutputMedia(&m_TsPacket);
}

void CTsPacketSync::UpdateBitrate(void)
{
	// ビットレート更新
	const DWORD dwCurUpdateTime =
#ifdef _WIN32
		::GetTickCount();
#else
		(DWORD)chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count();
#endif

	// 更新周期チェック
	if((dwCurUpdateTime - m_dwLastUpdateTime) < BITRATE_PERIOD)return;

	// カウンタ更新
	m_dwLastInputPacket = m_dwCurInputPacket;
	m_dwLastOutputPacket = m_dwCurOutputPacket;

	// カウンタリセット
	m_dwCurInputPacket = 0UL;
	m_dwCurOutputPacket = 0UL;

	// 周期更新
	m_dwLastUpdateTime = dwCurUpdateTime;
}
