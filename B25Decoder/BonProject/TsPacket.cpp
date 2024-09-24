// TsPacket.cpp: TSパケット基底クラス
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BonSDK.h"
#include "TsPacket.h"


/////////////////////////////////////////////////////////////////////////////
// ITsPacket基底クラス
/////////////////////////////////////////////////////////////////////////////

const DWORD CTsPacket::ParsePacket(BYTE *pContinuityCounter)
{
	// TSパケットヘッダ解析
	m_Header.bySyncByte					= m_pData[0];							// +0
	m_Header.bTransportErrorIndicator	= (m_pData[1] & 0x80U)? true : false;	// +1 bit7
	m_Header.bPayloadUnitStartIndicator	= (m_pData[1] & 0x40U)? true : false;	// +1 bit6
	m_Header.bTransportPriority			= (m_pData[1] & 0x20U)? true : false;	// +1 bit5
	m_Header.wPID = ((WORD)(m_pData[1] & 0x1F) << 8) | (WORD)m_pData[2];		// +1 bit4-0, +2
	m_Header.byTransportScramblingCtrl	= (m_pData[3] & 0xC0U) >> 6;			// +3 bit7-6
	m_Header.byAdaptationFieldCtrl		= (m_pData[3] & 0x30U) >> 4;			// +3 bit5-4
	m_Header.byContinuityCounter		= m_pData[3] & 0x0FU;					// +3 bit3-0
	
	// アダプテーションフィールド解析
	::ZeroMemory(&m_AdaptationField, sizeof(m_AdaptationField));
	
	if(m_Header.byAdaptationFieldCtrl & 0x02){
		// アダプテーションフィールドあり
		if(m_AdaptationField.byAdaptationFieldLength = m_pData[4]){								// +4
			// フィールド長以降あり
			m_AdaptationField.bDiscontinuityIndicator	= (m_pData[5] & 0x80U)? true : false;	// +5 bit7
			m_AdaptationField.bRamdomAccessIndicator	= (m_pData[5] & 0x40U)? true : false;	// +5 bit6
			m_AdaptationField.bEsPriorityIndicator		= (m_pData[5] & 0x20U)? true : false;	// +5 bit5
			m_AdaptationField.bPcrFlag					= (m_pData[5] & 0x10U)? true : false;	// +5 bit4
			m_AdaptationField.bOpcrFlag					= (m_pData[5] & 0x08U)? true : false;	// +5 bit3
			m_AdaptationField.bSplicingPointFlag		= (m_pData[5] & 0x04U)? true : false;	// +5 bit2
			m_AdaptationField.bTransportPrivateDataFlag	= (m_pData[5] & 0x02U)? true : false;	// +5 bit1
			m_AdaptationField.bAdaptationFieldExtFlag	= (m_pData[5] & 0x01U)? true : false;	// +5 bit0
			
			if(m_pData[4] > 1U){
				m_AdaptationField.pOptionData			= &m_pData[6];
				m_AdaptationField.byOptionSize			= m_pData[4] - 1U;
				}
			}
		}

	// パケットのフォーマット適合性をチェックする
	if(m_Header.bySyncByte != 0x47U)return EC_FORMAT;								// 同期バイト不正
	if(m_Header.bTransportErrorIndicator)return EC_TRANSPORT;						// ビット誤りあり
	if((m_Header.wPID >= 0x0002U) && (m_Header.wPID <= 0x000FU))return EC_FORMAT;	// 未定義PID範囲
	if(m_Header.byTransportScramblingCtrl == 0x01U)return EC_FORMAT;				// 未定義スクランブル制御値
	if(m_Header.byAdaptationFieldCtrl == 0x00U)return EC_FORMAT;					// 未定義アダプテーションフィールド制御値
	if((m_Header.byAdaptationFieldCtrl == 0x02U) && (m_AdaptationField.byAdaptationFieldLength > 183U))return EC_FORMAT;	// アダプテーションフィールド長異常
	if((m_Header.byAdaptationFieldCtrl == 0x03U) && (m_AdaptationField.byAdaptationFieldLength > 182U))return EC_FORMAT;	// アダプテーションフィールド長異常

	if(!pContinuityCounter || m_Header.wPID == 0x1FFFU)return EC_VALID;

	// 連続性チェック
	const BYTE byOldCounter = pContinuityCounter[m_Header.wPID];
	const BYTE byNewCounter = (m_Header.byAdaptationFieldCtrl & 0x01U)? m_Header.byContinuityCounter : 0x10U;
	pContinuityCounter[m_Header.wPID] = byNewCounter;

	if(!m_AdaptationField.bDiscontinuityIndicator){
		if((byOldCounter < 0x10U) && (byNewCounter < 0x10U)){
			if(((byOldCounter + 1U) & 0x0FU) != byNewCounter){
				return EC_CONTINUITY;
				}
			}
		}

	return EC_VALID;
}

BYTE * CTsPacket::GetRawData(void) const
{
	// パケットのバイナリデータを返す
	return (m_dwDataSize == TS_PACKET_SIZE)? m_pData : NULL;
}

const WORD CTsPacket::GetPID(void) const
{
	// PIDを返す
	return m_Header.wPID;
}

const bool CTsPacket::IsScrambled(void) const
{
	// スクランブル有無を返す
	return (m_Header.byTransportScramblingCtrl & 0x02U)? true : false;
}

const bool CTsPacket::HaveAdaptationField(void) const
{
	// アダプテーションフィールドの有無を返す
	return (m_Header.byAdaptationFieldCtrl & 0x02U)? true : false;
}

BYTE * CTsPacket::GetPayloadData(void) const
{
	// ペイロードポインタを返す
	switch(m_Header.byAdaptationFieldCtrl){
		case 1U :	// ペイロードのみ
			return &m_pData[4];
		
		case 3U :	// アダプテーションフィールド、ペイロードあり
			return &m_pData[m_AdaptationField.byAdaptationFieldLength + 5U];
		
		default :	// アダプテーションフィールドのみ or 例外
			return NULL;
		}
}

const BYTE CTsPacket::GetPayloadSize(void) const
{
	// ペイロードサイズを返す
	switch(m_Header.byAdaptationFieldCtrl){
		case 1U :	// ペイロードのみ
			return (TS_PACKET_SIZE - 4U);
		
		case 3U :	// アダプテーションフィールド、ペイロードあり
			return (TS_PACKET_SIZE - m_AdaptationField.byAdaptationFieldLength - 5U);
		
		default :	// アダプテーションフィールドのみ or 例外
			return 0U;
		}
}

const bool CTsPacket::CopyPacket(const ITsPacket *pSrc)
{
	if(!pSrc)return false;
	
	// バイナリデータを取得
	const BYTE *pSrcRawData = pSrc->GetRawData();
	if(!pSrcRawData)return false;

	// データをコピー
	SetData(pSrcRawData, TS_PACKET_SIZE);
	
	// パケット解析
	ParsePacket();

	return true;
}

const bool CTsPacket::ComparePacket(const ITsPacket *pSrc) const
{
	// パケット比較する
	if(!pSrc)return false;

	const BYTE *pSrcRawData = pSrc->GetRawData();
	
	// サイズをチェック
	if(!pSrcRawData || (m_dwDataSize != TS_PACKET_SIZE))return false;

	// パケットを比較
	return (!::memcmp(pSrcRawData, m_pData, TS_PACKET_SIZE))? true : false;
}

const DWORD CTsPacket::CopyData(const IMediaData *pSrc)
{
	// ITsPacketインタフェース取得
	const ITsPacket * const pTsPacket = dynamic_cast<const ITsPacket *>(pSrc);

	if(pTsPacket){
		// ITsPacketとしてコピー
		return (CopyPacket(pTsPacket))? TS_PACKET_SIZE : 0UL;
		}
	else{
		// IMediaDataとしてコピー
		return CMediaData::CopyData(pSrc);
		}
}

CTsPacket::CTsPacket(IBonObject *pOwner)
	: CMediaData(pOwner)
{
	// バッファ確保
	GetBuffer(TS_PACKET_SIZE);
	
	// ヘッダ情報クリア
	::ZeroMemory(&m_Header, sizeof(m_Header));
	::ZeroMemory(&m_AdaptationField, sizeof(m_AdaptationField));
}

CTsPacket::~CTsPacket(void)
{

}
