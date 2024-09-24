// TsDataEncode.h: TS基本データクラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// ARIB STD-B24 Part1文字列処理クラス
/////////////////////////////////////////////////////////////////////////////

class CAribString
{
public:
	static const DWORD AribToString(TCHAR *lpszDst, const BYTE *pSrcData, const DWORD dwSrcLen);
	
private:
	enum CODE_SET
	{
		CODE_UNKNOWN,				// 不明なグラフィックセット(非対応)
		CODE_KANJI,					// Kanji
		CODE_ALPHANUMERIC,			// Alphanumeric
		CODE_HIRAGANA,				// Hiragana
		CODE_KATAKANA,				// Katakana
		CODE_MOSAIC_A,				// Mosaic A
		CODE_MOSAIC_B,				// Mosaic B
		CODE_MOSAIC_C,				// Mosaic C
		CODE_MOSAIC_D,				// Mosaic D
		CODE_PROP_ALPHANUMERIC,		// Proportional Alphanumeric
		CODE_PROP_HIRAGANA,			// Proportional Hiragana
		CODE_PROP_KATAKANA,			// Proportional Katakana
		CODE_JIS_X0201_KATAKANA,	// JIS X 0201 Katakana
		CODE_JIS_KANJI_PLANE_1,		// JIS compatible Kanji Plane 1
		CODE_JIS_KANJI_PLANE_2,		// JIS compatible Kanji Plane 2
		CODE_ADDITIONAL_SYMBOLS		// Additional symbols
	};

	enum FONT_SIZE
	{
		SIZE_SMALL,					// SSZ	非対応
		SIZE_MEDIUM,				// MSZ	半角
		SIZE_NORMAL,				// NSZ	全角
		SIZE_UNSUPPORTED			// SZX	非対応
	};

	CODE_SET m_CodeG[4];
	CODE_SET *m_pLockingGL;
	CODE_SET *m_pLockingGR;
	CODE_SET *m_pSingleGL;
	
	FONT_SIZE m_FontSize;
	
	BYTE m_byEscSeqCount;
	BYTE m_byEscSeqIndex;
	bool m_bIsEscSeqDrcs;
	
	CAribString(void);
	
	const DWORD AribToStringInternal(TCHAR *lpszDst, const BYTE *pSrcData, const DWORD dwSrcLen);
	inline const DWORD ProcessCharCode(TCHAR *lpszDst, const WORD wCode, const CODE_SET CodeSet);

	inline const DWORD PutKanjiChar(TCHAR *lpszDst, const WORD wCode);
	inline const DWORD PutAlphanumericChar(TCHAR *lpszDst, const WORD wCode);
	inline const DWORD PutHiraganaChar(TCHAR *lpszDst, const WORD wCode);
	inline const DWORD PutKatakanaChar(TCHAR *lpszDst, const WORD wCode);
	inline const DWORD PutJisKatakanaChar(TCHAR *lpszDst, const WORD wCode);
	inline const DWORD PutSymbolsChar(TCHAR *lpszDst, const WORD wCode);

	inline const DWORD ProcessCtrlC0(TCHAR *lpszDst, const BYTE *pCode, DWORD &dwSrcPos);
	inline const DWORD ProcessCtrlC1(TCHAR *lpszDst, const BYTE *pCode, DWORD &dwSrcPos);

	inline void ProcessEscapeSeq(const BYTE byCode);
	
	inline void LockingShiftGL(const BYTE byIndexG);
	inline void LockingShiftGR(const BYTE byIndexG);
	inline void SingleShiftGL(const BYTE byIndexG);

	inline const bool DesignationGSET(const BYTE byIndexG, const BYTE byCode);
	inline const bool DesignationDRCS(const BYTE byIndexG, const BYTE byCode);
};


/////////////////////////////////////////////////////////////////////////////
// ARIB STD-B10 Part2 Annex C MJD+JTC 処理クラス
/////////////////////////////////////////////////////////////////////////////

class CAribTime
{
public:
	static void AribToSystemTime(SYSTEMTIME *pSysTime, const BYTE *pHexData);
	static void SplitAribMjd(const WORD wAribMjd, WORD *pwYear, WORD *pwMonth, WORD *pwDay, WORD *pwDayOfWeek = NULL);
	static void SplitAribBcd(const BYTE *pAribBcd, WORD *pwHour, WORD *pwMinute, WORD *pwSecond = NULL);
	static const DWORD AribBcdToSecond(const BYTE *pAribBcd);
};


/////////////////////////////////////////////////////////////////////////////
// SYSTEMTIME ラップクラス
/////////////////////////////////////////////////////////////////////////////

class CTsTime : public SYSTEMTIME
{
public:
	static const int ARIB_TIMEZONE_SEC = 9 * 3600;

	CTsTime();
	CTsTime(const SYSTEMTIME &SystemTime);
	CTsTime(const ULONGLONG llFileTime);
	CTsTime(const BYTE *pHexData);
	CTsTime(const WORD wYear, const WORD wMonth, const WORD wDay, const WORD wHour = 0U, const WORD wMinute = 0U, const WORD wSecond = 0U);
	
	void SetNowTime(void);
	void SetAribTime(const BYTE *pHexData);
	void SetTime(const WORD wYear, const WORD wMonth, const WORD wDay, const WORD wHour = 0U, const WORD wMinute = 0U, const WORD wSecond = 0U);

	void ClearTime(void);
	const bool IsEmpty(void) const;

	operator const ULONGLONG () const;

	const CTsTime & operator = (const SYSTEMTIME &SystemTime);
	const CTsTime & operator = (const ULONGLONG llFileTime);

	const CTsTime operator + (const DWORD dwSecond) const;
	const CTsTime operator - (const DWORD dwSecond) const;
	const CTsTime & operator += (const DWORD dwSecond);
	const CTsTime & operator -= (const DWORD dwSecond);

	const bool operator == (const CTsTime &Operand) const;
	const bool operator != (const CTsTime &Operand) const;
	const bool operator < (const CTsTime &Operand) const;
	const bool operator > (const CTsTime &Operand) const;
	const bool operator <= (const CTsTime &Operand) const;
	const bool operator >= (const CTsTime &Operand) const;
};
