// Multi2Decoder.cpp: MULTI2暗号デコーダクラス
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Multi2Decoder.h"
#include <stdlib.h>


/////////////////////////////////////////////////////////////////////////////
// ファイルローカル定数設定
/////////////////////////////////////////////////////////////////////////////

#define SCRAMBLE_ROUND		4UL


/////////////////////////////////////////////////////////////////////////////
// MULTI2暗号デコーダクラス
/////////////////////////////////////////////////////////////////////////////

static inline DWORD HexToDword(const BYTE *pHexData)
{
#ifdef _WIN32
	return ::_byteswap_ulong(*reinterpret_cast<const DWORD *>(pHexData));
#else
	return (DWORD)pHexData[0] << 24 | (DWORD)pHexData[1] << 16 | (DWORD)pHexData[2] << 8 | (DWORD)pHexData[3];
#endif
}

static inline void DwordToHex(DWORD dwSrc, BYTE *pHexData)
{
#ifdef _WIN32
	*reinterpret_cast<DWORD *>(pHexData) = ::_byteswap_ulong(dwSrc);
#else
	pHexData[0] = (BYTE)(dwSrc >> 24);
	pHexData[1] = (BYTE)(dwSrc >> 16);
	pHexData[2] = (BYTE)(dwSrc >> 8);
	pHexData[3] = (BYTE)dwSrc;
#endif
}

inline void CMulti2Decoder::DATKEY::SetHexData(const BYTE *pHexData)
{
	// バイトオーダー変換
	dwRight = HexToDword(pHexData + 4);
	dwLeft = HexToDword(pHexData);
}

inline void CMulti2Decoder::DATKEY::GetHexData(BYTE *pHexData) const
{
	// バイトオーダー変換
	DwordToHex(dwRight, pHexData + 4);
	DwordToHex(dwLeft, pHexData);
}

void CMulti2Decoder::SYSKEY::SetHexData(const BYTE *pHexData)
{
	// バイトオーダー変換
	for(int i = 0; i < 8; i++){
		SetKey(i, HexToDword(pHexData + i * 4));
		}
}

void CMulti2Decoder::SYSKEY::SetKey(int nIndex, DWORD dwKey)
{
	dwKeys[nIndex] = dwKey;
}

CMulti2Decoder::CMulti2Decoder(void)
	: m_bIsSysKeyValid(false)
	, m_bIsWorkKeyValid(false)
{

}

CMulti2Decoder::~CMulti2Decoder(void)
{

}

void CMulti2Decoder::Initialize(const BYTE *pSystemKey, const BYTE *pInitialCbc)
{
	m_bIsSysKeyValid = true;
	m_bIsWorkKeyValid = false;

	// Descrambling System Keyセット
	m_SystemKey.SetHexData(pSystemKey);
	
	// Descrambler CBC Initial Valueセット
	m_InitialCbc.SetHexData(pInitialCbc);
}

const bool CMulti2Decoder::SetScrambleKey(const BYTE *pScrambleKey)
{
	if(!m_bIsSysKeyValid)return false;
	
	if(!pScrambleKey){
		// キーが設定されない場合はデコード不能にする(不正な復号による破損防止のため)
		m_bIsWorkKeyValid = false;
		return false;
		}

	// Scramble Key Odd/Even をセットする
	DATKEY ScrKeyOdd, ScrKeyEven;
	
	// バイトオーダー変換
	ScrKeyOdd.SetHexData(&pScrambleKey[0]);
	ScrKeyEven.SetHexData(&pScrambleKey[8]);

	// キースケジュール
	KeySchedule(m_WorkKeyOdd, m_SystemKey, ScrKeyOdd);
	KeySchedule(m_WorkKeyEven, m_SystemKey, ScrKeyEven);
	
	m_bIsWorkKeyValid = true;
	
	return true;
}

const bool CMulti2Decoder::Decode(BYTE *pData, const DWORD dwSize, const BYTE byScrCtrl) const
{
	if(!byScrCtrl)return true;										// スクランブルなし
	else if(!m_bIsSysKeyValid || !m_bIsWorkKeyValid)return false;	// スクランブルキー未設定
	else if((byScrCtrl != 2U) && (byScrCtrl != 3U))return false;	// スクランブル制御不正

	// ワークキー選択
	const SYSKEY &WorkKey = (byScrCtrl == 3)? m_WorkKeyOdd : m_WorkKeyEven;

	DWORD dwPos;
	DATKEY CbcData = m_InitialCbc;
	DATKEY SrcData1, SrcData2;

	// CBCモード
	for(dwPos = 0UL ; (dwSize - dwPos) >= sizeof(DATKEY) * 2 ; dwPos += sizeof(DATKEY) * 2){
		SrcData1.SetHexData(&pData[dwPos]);
		SrcData2.SetHexData(&pData[dwPos + sizeof(DATKEY)]);
		DATKEY x1 = SrcData1;
		DATKEY x2 = SrcData2;
		DecryptBlockX2(x1, x2, WorkKey);
		x1.dwLeft ^= CbcData.dwLeft;
		x1.dwRight ^= CbcData.dwRight;
		x2.dwLeft ^= SrcData1.dwLeft;
		x2.dwRight ^= SrcData1.dwRight;
		CbcData = SrcData2;
		x1.GetHexData(&pData[dwPos]);
		x2.GetHexData(&pData[dwPos + sizeof(DATKEY)]);
		}

	if((dwSize - dwPos) >= sizeof(DATKEY)){
		SrcData1.SetHexData(&pData[dwPos]);
		DATKEY x1 = SrcData1;
		DecryptBlock(x1, WorkKey);
		x1.dwLeft ^= CbcData.dwLeft;
		x1.dwRight ^= CbcData.dwRight;
		CbcData = SrcData1;
		x1.GetHexData(&pData[dwPos]);
		dwPos += sizeof(DATKEY);
		}

	// OFBモード
	if(dwPos < dwSize){
		BYTE Remain[sizeof(DATKEY)];
		EncryptBlock(CbcData, WorkKey);
		CbcData.GetHexData(Remain);

		for(BYTE *pRemain = Remain ; (dwSize - dwPos) > 0UL ; dwPos++, pRemain++){
			pData[dwPos] ^= *pRemain;
			}
		}

	return true;
}

inline void CMulti2Decoder::DecryptBlock(DATKEY &Block, const SYSKEY &WorkKey)
{
	// Block Decryption
	for(DWORD dwRound = 0UL ; dwRound < SCRAMBLE_ROUND ; dwRound++){
		RoundFuncPi4(Block, WorkKey.dwKeys[7]);
		RoundFuncPi3(Block, WorkKey.dwKeys[5], WorkKey.dwKeys[6]);
		RoundFuncPi2(Block, WorkKey.dwKeys[4]);
		RoundFuncPi1(Block);
		RoundFuncPi4(Block, WorkKey.dwKeys[3]);
		RoundFuncPi3(Block, WorkKey.dwKeys[1], WorkKey.dwKeys[2]);
		RoundFuncPi2(Block, WorkKey.dwKeys[0]);
		RoundFuncPi1(Block);
		}
}

inline void CMulti2Decoder::DecryptBlockX2(DATKEY &Block1, DATKEY &Block2, const SYSKEY &WorkKey)
{
	// Block Decryption
	for(DWORD dwRound = 0UL ; dwRound < SCRAMBLE_ROUND ; dwRound++){
		RoundFuncPi4X2(Block1, Block2, WorkKey.dwKeys[7]);
		RoundFuncPi3X2(Block1, Block2, WorkKey.dwKeys[5], WorkKey.dwKeys[6]);
		RoundFuncPi2X2(Block1, Block2, WorkKey.dwKeys[4]);
		RoundFuncPi1X2(Block1, Block2);
		RoundFuncPi4X2(Block1, Block2, WorkKey.dwKeys[3]);
		RoundFuncPi3X2(Block1, Block2, WorkKey.dwKeys[1], WorkKey.dwKeys[2]);
		RoundFuncPi2X2(Block1, Block2, WorkKey.dwKeys[0]);
		RoundFuncPi1X2(Block1, Block2);
		}
}

inline void CMulti2Decoder::EncryptBlock(DATKEY &Block, const SYSKEY &WorkKey)
{
	// Block Encryption
	for(DWORD dwRound = 0UL ; dwRound < SCRAMBLE_ROUND ; dwRound++){
		RoundFuncPi1(Block);
		RoundFuncPi2(Block, WorkKey.dwKeys[0]);
		RoundFuncPi3(Block, WorkKey.dwKeys[1], WorkKey.dwKeys[2]);
		RoundFuncPi4(Block, WorkKey.dwKeys[3]);
		RoundFuncPi1(Block);
		RoundFuncPi2(Block, WorkKey.dwKeys[4]);
		RoundFuncPi3(Block, WorkKey.dwKeys[5], WorkKey.dwKeys[6]);
		RoundFuncPi4(Block, WorkKey.dwKeys[7]);
		}
}

void CMulti2Decoder::KeySchedule(SYSKEY &WorkKey, const SYSKEY &SysKey, DATKEY &DataKey)
{
	// Key Schedule
	RoundFuncPi1(DataKey);									// π1

	RoundFuncPi2(DataKey, SysKey.dwKeys[0]);					// π2
	WorkKey.SetKey(0, DataKey.dwLeft);

	RoundFuncPi3(DataKey, SysKey.dwKeys[1], SysKey.dwKeys[2]);	// π3
	WorkKey.SetKey(1, DataKey.dwRight);

	RoundFuncPi4(DataKey, SysKey.dwKeys[3]);					// π4
	WorkKey.SetKey(2, DataKey.dwLeft);
	
	RoundFuncPi1(DataKey);									// π1
	WorkKey.SetKey(3, DataKey.dwRight);
	
	RoundFuncPi2(DataKey, SysKey.dwKeys[4]);					// π2
	WorkKey.SetKey(4, DataKey.dwLeft);

	RoundFuncPi3(DataKey, SysKey.dwKeys[5], SysKey.dwKeys[6]);	// π3
	WorkKey.SetKey(5, DataKey.dwRight);

	RoundFuncPi4(DataKey, SysKey.dwKeys[7]);					// π4
	WorkKey.SetKey(6, DataKey.dwLeft);

	RoundFuncPi1(DataKey);									// π1
	WorkKey.SetKey(7, DataKey.dwRight);
}

template <int Rotate>
static inline const DWORD LeftRotate(const DWORD dwValue)
{
	// 左ローテート
	return (dwValue << Rotate) | (dwValue >> (32 - Rotate));
}

inline void CMulti2Decoder::RoundFuncPi1(DATKEY &Block)
{
	// Elementary Encryption Function π1
	Block.dwRight ^= Block.dwLeft;
}

inline void CMulti2Decoder::RoundFuncPi1X2(DATKEY &Block1, DATKEY &Block2)
{
	Block1.dwRight ^= Block1.dwLeft;
	Block2.dwRight ^= Block2.dwLeft;
}

inline void CMulti2Decoder::RoundFuncPi2(DATKEY &Block, const DWORD dwK1)
{
	// Elementary Encryption Function π2
	const DWORD dwY = Block.dwRight + dwK1;
	const DWORD dwZ = LeftRotate<1>(dwY) + dwY - 1;
	Block.dwLeft ^= LeftRotate<4>(dwZ) ^ dwZ;
}

inline void CMulti2Decoder::RoundFuncPi2X2(DATKEY &Block1, DATKEY &Block2, const DWORD dwK1)
{
	DWORD t1 = Block1.dwRight + dwK1;
	DWORD t2 = Block2.dwRight + dwK1;
	t1 = LeftRotate<1>(t1) + t1 - 1;
	t2 = LeftRotate<1>(t2) + t2 - 1;
	Block1.dwLeft ^= LeftRotate<4>(t1) ^ t1;
	Block2.dwLeft ^= LeftRotate<4>(t2) ^ t2;
}

inline void CMulti2Decoder::RoundFuncPi3(DATKEY &Block, const DWORD dwK2, const DWORD dwK3)
{
	// Elementary Encryption Function π3
	const DWORD dwY = Block.dwLeft + dwK2;
	const DWORD dwZ = LeftRotate<2>(dwY) + dwY + 1;
	const DWORD dwA = LeftRotate<8>(dwZ) ^ dwZ;
	const DWORD dwB = dwA + dwK3;
	//const DWORD dwC = LeftRotate<1>(dwB) - dwB;
	// 法2^32の合同式において、MSB(dwB)==0のとき dwC=dwB*2-dwB=dwB、MSB(dwB)==1のとき dwC=dwB*2+1-dwB=dwB+1
	// まとめると dwC=dwB+MSB(dwB)=dwB+(dwB>>31)
	const DWORD dwC = dwB + (dwB >> 31);
	Block.dwRight ^= (LeftRotate<16>(dwC) ^ (dwC | Block.dwLeft));
}

inline void CMulti2Decoder::RoundFuncPi3X2(DATKEY &Block1, DATKEY &Block2, const DWORD dwK2, const DWORD dwK3)
{
	DWORD t1 = Block1.dwLeft + dwK2;
	DWORD t2 = Block2.dwLeft + dwK2;
	t1 = LeftRotate<2>(t1) + t1 + 1;
	t2 = LeftRotate<2>(t2) + t2 + 1;
	t1 = LeftRotate<8>(t1) ^ t1;
	t2 = LeftRotate<8>(t2) ^ t2;
	t1 = t1 + dwK3;
	t2 = t2 + dwK3;
	t1 = t1 + (t1 >> 31);
	t2 = t2 + (t2 >> 31);
	Block1.dwRight ^= (LeftRotate<16>(t1) ^ (t1 | Block1.dwLeft));
	Block2.dwRight ^= (LeftRotate<16>(t2) ^ (t2 | Block2.dwLeft));
}

inline void CMulti2Decoder::RoundFuncPi4(DATKEY &Block, const DWORD dwK4)
{
	// Elementary Encryption Function π4
	const DWORD dwY = Block.dwRight + dwK4;
	Block.dwLeft ^= (LeftRotate<2>(dwY) + dwY + 1);
}

inline void CMulti2Decoder::RoundFuncPi4X2(DATKEY &Block1, DATKEY &Block2, const DWORD dwK4)
{
	DWORD t1 = Block1.dwRight + dwK4;
	DWORD t2 = Block2.dwRight + dwK4;
	Block1.dwLeft ^= (LeftRotate<2>(t1) + t1 + 1);
	Block2.dwLeft ^= (LeftRotate<2>(t2) + t2 + 1);
}
