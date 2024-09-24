// Multi2Decoder.cpp: MULTI2暗号デコーダクラス
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Multi2Decoder.h"
#include <stdlib.h>
#include <memory>
#include <new>
#if defined(MULTI2_SIMD_SSE2)
#include <emmintrin.h>
#elif defined(MULTI2_SIMD_NEON)
#include <arm_neon.h>
#endif


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

#if defined(MULTI2_SIMD_SSE2)

// 以下、x86/x64向け最適化コードはTVCAS_attachment(20120915)のMulti2DecoderSIMD.cppを移植したもの

template <int Rotate>
static inline __m128i LeftRotateX4(const __m128i &Value)
{
	return _mm_or_si128(_mm_slli_epi32(Value, Rotate), _mm_srli_epi32(Value, 32 - Rotate));
}

static inline __m128i ByteSwapX4(const __m128i &Value)
{
	__m128i t0 = _mm_srli_epi16(Value, 8);
	__m128i t1 = _mm_slli_epi16(Value, 8);
	return LeftRotateX4<16>(_mm_or_si128(t0, t1));
}

static inline void RoundFuncPi1X8(__m128i &Left1, __m128i &Right1, __m128i &Left2, __m128i &Right2)
{
	Right1 = _mm_xor_si128(Right1, Left1);
	Right2 = _mm_xor_si128(Right2, Left2);
}

static inline void RoundFuncPi2X8(__m128i &Left1, __m128i &Right1, __m128i &Left2, __m128i &Right2, const __m128i &Key1)
{
	__m128i t1 = _mm_add_epi32(Right1, Key1);
	__m128i t2 = _mm_add_epi32(Right2, Key1);
	t1 = _mm_add_epi32(LeftRotateX4<1>(t1), t1);
	t2 = _mm_add_epi32(LeftRotateX4<1>(t2), t2);
	t1 = _mm_sub_epi32(t1, _mm_set1_epi32(1));
	t2 = _mm_sub_epi32(t2, _mm_set1_epi32(1));
	t1 = _mm_xor_si128(LeftRotateX4<4>(t1), t1);
	t2 = _mm_xor_si128(LeftRotateX4<4>(t2), t2);
	Left1 = _mm_xor_si128(Left1, t1);
	Left2 = _mm_xor_si128(Left2, t2);
}

static inline void RoundFuncPi3X8(__m128i &Left1, __m128i &Right1, __m128i &Left2, __m128i &Right2, const __m128i &Key2, const __m128i &Key3)
{
	__m128i t1 = _mm_add_epi32(Left1, Key2);
	__m128i t2 = _mm_add_epi32(Left2, Key2);
	t1 = _mm_add_epi32(LeftRotateX4<2>(t1), t1);
	t2 = _mm_add_epi32(LeftRotateX4<2>(t2), t2);
	t1 = _mm_add_epi32(t1, _mm_set1_epi32(1));
	t2 = _mm_add_epi32(t2, _mm_set1_epi32(1));
	t1 = _mm_xor_si128(LeftRotateX4<8>(t1), t1);
	t2 = _mm_xor_si128(LeftRotateX4<8>(t2), t2);
	t1 = _mm_add_epi32(t1, Key3);
	t2 = _mm_add_epi32(t2, Key3);
	t1 = _mm_add_epi32(t1, _mm_srli_epi32(t1, 31));
	t2 = _mm_add_epi32(t2, _mm_srli_epi32(t2, 31));
	t1 = _mm_xor_si128(LeftRotateX4<16>(t1), _mm_or_si128(t1, Left1));
	t2 = _mm_xor_si128(LeftRotateX4<16>(t2), _mm_or_si128(t2, Left2));
	Right1 = _mm_xor_si128(Right1, t1);
	Right2 = _mm_xor_si128(Right2, t2);
}

static inline void RoundFuncPi4X8(__m128i &Left1, __m128i &Right1, __m128i &Left2, __m128i &Right2, const __m128i &Key4)
{
	__m128i t1 = _mm_add_epi32(Right1, Key4);
	__m128i t2 = _mm_add_epi32(Right2, Key4);
	t1 = _mm_add_epi32(LeftRotateX4<2>(t1), t1);
	t2 = _mm_add_epi32(LeftRotateX4<2>(t2), t2);
	t1 = _mm_add_epi32(t1, _mm_set1_epi32(1));
	t2 = _mm_add_epi32(t2, _mm_set1_epi32(1));
	Left1 = _mm_xor_si128(Left1, t1);
	Left2 = _mm_xor_si128(Left2, t2);
}

#elif defined(MULTI2_SIMD_NEON)

// 以下、ARM向け最適化コードはx86/x64向け最適化コードをほとんど直訳したもの

template <int Rotate>
static inline uint32x4_t LeftRotateX4(const uint32x4_t &Value)
{
	return vsliq_n_u32(vshrq_n_u32(Value, 32 - Rotate), Value, Rotate);
}

static inline uint32x4_t ByteSwapX4(const uint32x4_t &Value)
{
	return vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(Value)));
}

static inline void RoundFuncPi1X8(uint32x4_t &Left1, uint32x4_t &Right1, uint32x4_t &Left2, uint32x4_t &Right2)
{
	Right1 = veorq_u32(Right1, Left1);
	Right2 = veorq_u32(Right2, Left2);
}

static inline void RoundFuncPi2X8(uint32x4_t &Left1, uint32x4_t &Right1, uint32x4_t &Left2, uint32x4_t &Right2, const uint32x4_t &Key1)
{
	uint32x4_t t1 = vaddq_u32(Right1, Key1);
	uint32x4_t t2 = vaddq_u32(Right2, Key1);
	t1 = vaddq_u32(LeftRotateX4<1>(t1), t1);
	t2 = vaddq_u32(LeftRotateX4<1>(t2), t2);
	t1 = vsubq_u32(t1, vdupq_n_u32(1));
	t2 = vsubq_u32(t2, vdupq_n_u32(1));
	t1 = veorq_u32(LeftRotateX4<4>(t1), t1);
	t2 = veorq_u32(LeftRotateX4<4>(t2), t2);
	Left1 = veorq_u32(Left1, t1);
	Left2 = veorq_u32(Left2, t2);
}

static inline void RoundFuncPi3X8(uint32x4_t &Left1, uint32x4_t &Right1, uint32x4_t &Left2, uint32x4_t &Right2, const uint32x4_t &Key2, const uint32x4_t &Key3)
{
	uint32x4_t t1 = vaddq_u32(Left1, Key2);
	uint32x4_t t2 = vaddq_u32(Left2, Key2);
	t1 = vaddq_u32(LeftRotateX4<2>(t1), t1);
	t2 = vaddq_u32(LeftRotateX4<2>(t2), t2);
	t1 = vaddq_u32(t1, vdupq_n_u32(1));
	t2 = vaddq_u32(t2, vdupq_n_u32(1));
	t1 = veorq_u32(LeftRotateX4<8>(t1), t1);
	t2 = veorq_u32(LeftRotateX4<8>(t2), t2);
	t1 = vaddq_u32(t1, Key3);
	t2 = vaddq_u32(t2, Key3);
	t1 = vsraq_n_u32(t1, t1, 31);
	t2 = vsraq_n_u32(t2, t2, 31);
	t1 = veorq_u32(LeftRotateX4<16>(t1), vorrq_u32(t1, Left1));
	t2 = veorq_u32(LeftRotateX4<16>(t2), vorrq_u32(t2, Left2));
	Right1 = veorq_u32(Right1, t1);
	Right2 = veorq_u32(Right2, t2);
}

static inline void RoundFuncPi4X8(uint32x4_t &Left1, uint32x4_t &Right1, uint32x4_t &Left2, uint32x4_t &Right2, const uint32x4_t &Key4)
{
	uint32x4_t t1 = vaddq_u32(Right1, Key4);
	uint32x4_t t2 = vaddq_u32(Right2, Key4);
	t1 = vaddq_u32(LeftRotateX4<2>(t1), t1);
	t2 = vaddq_u32(LeftRotateX4<2>(t2), t2);
	t1 = vaddq_u32(t1, vdupq_n_u32(1));
	t2 = vaddq_u32(t2, vdupq_n_u32(1));
	Left1 = veorq_u32(Left1, t1);
	Left2 = veorq_u32(Left2, t2);
}

#endif

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
#if defined(MULTI2_SIMD_SSE2) || defined(MULTI2_SIMD_NEON)
	pdw4Keys = byKeys;
	size_t nSpace = sizeof(byKeys);
	if(!std::align(16, 8, pdw4Keys, nSpace)){
		// 通常ありえない
		throw std::bad_alloc();
		}
#ifdef MULTI2_SIMD_SSE2
	(static_cast<__m128i *>(pdw4Keys))[nIndex] = _mm_set1_epi32(dwKey);
#else
	(static_cast<uint32x4_t *>(pdw4Keys))[nIndex] = vdupq_n_u32(dwKey);
#endif
#endif
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

const bool CMulti2Decoder::Decode(BYTE *pData, const DWORD dwSize, const BYTE byScrCtrl, const DWORD dwExtraSize) const
{
	if(!byScrCtrl)return true;										// スクランブルなし
	else if(!m_bIsSysKeyValid || !m_bIsWorkKeyValid)return false;	// スクランブルキー未設定
	else if((byScrCtrl != 2U) && (byScrCtrl != 3U))return false;	// スクランブル制御不正

	// ワークキー選択
	const SYSKEY &WorkKey = (byScrCtrl == 3)? m_WorkKeyOdd : m_WorkKeyEven;

	DWORD dwPos;

#if defined(MULTI2_SIMD_SSE2) || defined(MULTI2_SIMD_NEON)
	if(dwSize == 184 && dwExtraSize >= 8){
#ifdef MULTI2_SIMD_SSE2
		__m128i Cbc = _mm_set_epi32(0, 0, m_InitialCbc.dwRight, m_InitialCbc.dwLeft);
		__m128i *pdw4Keys = static_cast<__m128i *>(WorkKey.pdw4Keys);
#else
		uint32x4_t CbcLeft = vdupq_n_u32(m_InitialCbc.dwLeft);
		uint32x4_t CbcRight = vdupq_n_u32(m_InitialCbc.dwRight);
		uint32x4_t *pdw4Keys = static_cast<uint32x4_t *>(WorkKey.pdw4Keys);
#endif

		for(dwPos = 0; dwPos < sizeof(DATKEY) * 8 * 3; dwPos += sizeof(DATKEY) * 8){
#ifdef MULTI2_SIMD_SSE2
			__m128i Src1 = _mm_loadu_si128(reinterpret_cast<__m128i *>(pData + dwPos));
			__m128i Src2 = _mm_loadu_si128(reinterpret_cast<__m128i *>(pData + dwPos + sizeof(DATKEY) * 2));
			__m128i Src3 = _mm_loadu_si128(reinterpret_cast<__m128i *>(pData + dwPos + sizeof(DATKEY) * 4));
			__m128i Src4 = _mm_loadu_si128(reinterpret_cast<__m128i *>(pData + dwPos + sizeof(DATKEY) * 6));

			// r2 l2 r1 l1 .. r8 l8 r7 l7
			Src1 = ByteSwapX4(Src1);
			Src2 = ByteSwapX4(Src2);
			Src3 = ByteSwapX4(Src3);
			Src4 = ByteSwapX4(Src4);

			// r2 r1 l2 l1 .. r8 r7 l8 l7
			__m128i x1 = _mm_shuffle_epi32(Src1, (3 << 6) | (1 << 4) | (2 << 2) | 0);
			__m128i y1 = _mm_shuffle_epi32(Src2, (3 << 6) | (1 << 4) | (2 << 2) | 0);
			__m128i x2 = _mm_shuffle_epi32(Src3, (3 << 6) | (1 << 4) | (2 << 2) | 0);
			__m128i y2 = _mm_shuffle_epi32(Src4, (3 << 6) | (1 << 4) | (2 << 2) | 0);

			// l4 l3 l2 l1 .. r8 r7 r6 r5
			__m128i Left1 = _mm_unpacklo_epi64(x1, y1);
			__m128i Right1 = _mm_unpackhi_epi64(x1, y1);
			__m128i Left2 = _mm_unpacklo_epi64(x2, y2);
			__m128i Right2 = _mm_unpackhi_epi64(x2, y2);
#else
			uint32x4x2_t Src1 = vld2q_u32(reinterpret_cast<DWORD *>(pData + dwPos));
			uint32x4x2_t Src2 = vld2q_u32(reinterpret_cast<DWORD *>(pData + dwPos + sizeof(DATKEY) * 4));

			// {l1,l2,l3,l4} .. {r5,r6,r7,r8}
			uint32x4_t SrcLeft1 = ByteSwapX4(Src1.val[0]);
			uint32x4_t SrcRight1 = ByteSwapX4(Src1.val[1]);
			uint32x4_t SrcLeft2 = ByteSwapX4(Src2.val[0]);
			uint32x4_t SrcRight2 = ByteSwapX4(Src2.val[1]);

			uint32x4_t Left1 = SrcLeft1;
			uint32x4_t Right1 = SrcRight1;
			uint32x4_t Left2 = SrcLeft2;
			uint32x4_t Right2 = SrcRight2;
#endif

			for(DWORD dwRound = 0; dwRound < SCRAMBLE_ROUND; dwRound++){
				RoundFuncPi4X8(Left1, Right1, Left2, Right2, pdw4Keys[7]);
				RoundFuncPi3X8(Left1, Right1, Left2, Right2, pdw4Keys[5], pdw4Keys[6]);
				RoundFuncPi2X8(Left1, Right1, Left2, Right2, pdw4Keys[4]);
				RoundFuncPi1X8(Left1, Right1, Left2, Right2);
				RoundFuncPi4X8(Left1, Right1, Left2, Right2, pdw4Keys[3]);
				RoundFuncPi3X8(Left1, Right1, Left2, Right2, pdw4Keys[1], pdw4Keys[2]);
				RoundFuncPi2X8(Left1, Right1, Left2, Right2, pdw4Keys[0]);
				RoundFuncPi1X8(Left1, Right1, Left2, Right2);
				}

#ifdef MULTI2_SIMD_SSE2
			// r2 l2 r1 l1 .. r8 l8 r7 l7
			x1 = _mm_unpacklo_epi32(Left1, Right1);
			y1 = _mm_unpackhi_epi32(Left1, Right1);
			x2 = _mm_unpacklo_epi32(Left2, Right2);
			y2 = _mm_unpackhi_epi32(Left2, Right2);

			Src2 = _mm_shuffle_epi32(Src2, (1 << 6) | (0 << 4) | (3 << 2) | 2);
			Src4 = _mm_shuffle_epi32(Src4, (1 << 6) | (0 << 4) | (3 << 2) | 2);
			x1 = _mm_xor_si128(x1, _mm_unpacklo_epi64(Cbc, Src1));
			y1 = _mm_xor_si128(y1, _mm_unpackhi_epi64(Src1, Src2));
			x2 = _mm_xor_si128(x2, _mm_unpacklo_epi64(Src2, Src3));
			y2 = _mm_xor_si128(y2, _mm_unpackhi_epi64(Src3, Src4));
			Cbc = Src4;

			x1 = ByteSwapX4(x1);
			y1 = ByteSwapX4(y1);
			x2 = ByteSwapX4(x2);
			y2 = ByteSwapX4(y2);

			_mm_storeu_si128(reinterpret_cast<__m128i *>(pData + dwPos), x1);
			_mm_storeu_si128(reinterpret_cast<__m128i *>(pData + dwPos + sizeof(DATKEY) * 2), y1);
			_mm_storeu_si128(reinterpret_cast<__m128i *>(pData + dwPos + sizeof(DATKEY) * 4), x2);
			_mm_storeu_si128(reinterpret_cast<__m128i *>(pData + dwPos + sizeof(DATKEY) * 6), y2);
#else
			// {l0,l1,l2,l3} .. {r4,r5,r6,r7}
			uint32x4_t x1 = vextq_u32(CbcLeft, SrcLeft1, 3);
			uint32x4_t y1 = vextq_u32(CbcRight, SrcRight1, 3);
			uint32x4_t x2 = vextq_u32(SrcLeft1, SrcLeft2, 3);
			uint32x4_t y2 = vextq_u32(SrcRight1, SrcRight2, 3);

			x1 = veorq_u32(Left1, x1);
			y1 = veorq_u32(Right1, y1);
			x2 = veorq_u32(Left2, x2);
			y2 = veorq_u32(Right2, y2);
			CbcLeft = SrcLeft2;
			CbcRight = SrcRight2;

			uint32x4x2_t z1 = {ByteSwapX4(x1), ByteSwapX4(y1)};
			uint32x4x2_t z2 = {ByteSwapX4(x2), ByteSwapX4(y2)};

			vst2q_u32(reinterpret_cast<DWORD *>(pData + dwPos), z1);
			vst2q_u32(reinterpret_cast<DWORD *>(pData + dwPos + sizeof(DATKEY) * 4), z2);
#endif
			}

		return true;
		}
#endif

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
