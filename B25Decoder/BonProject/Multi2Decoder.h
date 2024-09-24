// Multi2Decoder.h: MULTI2暗号デコーダクラス
//
//////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// MULTI2暗号デコーダクラス
/////////////////////////////////////////////////////////////////////////////

#ifdef MULTI2_SIMD
#if !defined(MULTI2_SIMD_SSE2) && (defined(_WIN32) || defined(__SSE2__))
#define MULTI2_SIMD_SSE2
#elif !defined(MULTI2_SIMD_NEON) && defined(__ARM_NEON) && defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define MULTI2_SIMD_NEON
#endif
#endif

class CMulti2Decoder
{
public:
	CMulti2Decoder(void);
	~CMulti2Decoder(void);
	
	void Initialize(const BYTE *pSystemKey, const BYTE *pInitialCbc);
	const bool SetScrambleKey(const BYTE *pScrambleKey);
	// dwExtraSizeはdwSizeを越えてオーバーラン可能なサイズ。最適化のため16以上を推奨
	const bool Decode(BYTE *pData, const DWORD dwSize, const BYTE byScrCtrl, const DWORD dwExtraSize = 0) const;

private:
	class DATKEY	// Data Key(Dk) 64bit
	{
	public:
		inline void SetHexData(const BYTE *pHexData);
		inline void GetHexData(BYTE *pHexData) const;
		
		DWORD dwRight, dwLeft;
	};
	
	class SYSKEY	// System Key(Sk), Expanded Key(Wk) 256bit
	{
	public:
		void SetHexData(const BYTE *pHexData);
		void SetKey(int nIndex, DWORD dwKey);
	
		DWORD dwKeys[8];
#if defined(MULTI2_SIMD_SSE2) || defined(MULTI2_SIMD_NEON)
		// アラインのために16バイトだけ余分にとる
		BYTE byKeys[16 * 8 + 16];
		void *pdw4Keys;
#endif
	};

	static inline void DecryptBlock(DATKEY &Block, const SYSKEY &WorkKey);
	static inline void DecryptBlockX2(DATKEY &Block1, DATKEY &Block2, const SYSKEY &WorkKey);
	static inline void EncryptBlock(DATKEY &Block, const SYSKEY &WorkKey);

	static void KeySchedule(SYSKEY &WorkKey, const SYSKEY &SysKey, DATKEY &DataKey);

	static inline void RoundFuncPi1(DATKEY &Block);
	static inline void RoundFuncPi1X2(DATKEY &Block1, DATKEY &Block2);
	static inline void RoundFuncPi2(DATKEY &Block, const DWORD dwK1);
	static inline void RoundFuncPi2X2(DATKEY &Block1, DATKEY &Block2, const DWORD dwK1);
	static inline void RoundFuncPi3(DATKEY &Block, const DWORD dwK2, const DWORD dwK3);
	static inline void RoundFuncPi3X2(DATKEY &Block1, DATKEY &Block2, const DWORD dwK2, const DWORD dwK3);
	static inline void RoundFuncPi4(DATKEY &Block, const DWORD dwK4);
	static inline void RoundFuncPi4X2(DATKEY &Block1, DATKEY &Block2, const DWORD dwK4);

	DATKEY m_InitialCbc;
	SYSKEY m_SystemKey;
	SYSKEY m_WorkKeyOdd, m_WorkKeyEven;
	
	bool m_bIsSysKeyValid;
	bool m_bIsWorkKeyValid;
};
