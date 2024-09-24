// TsEncode.cpp: TSエンコードクラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include <MbString.h>
#include "TsDataEncode.h"


//////////////////////////////////////////////////////////////////////
// CAribString クラスの構築/消滅
//////////////////////////////////////////////////////////////////////

static const bool abCharSizeTable[] =
{
	false,	// CODE_UNKNOWN					不明なグラフィックセット(非対応)
	true,	// CODE_KANJI					Kanji
	false,	// CODE_ALPHANUMERIC			Alphanumeric
	false,	// CODE_HIRAGANA				Hiragana
	false,	// CODE_KATAKANA				Katakana
	false,	// CODE_MOSAIC_A				Mosaic A
	false,	// CODE_MOSAIC_B				Mosaic B
	false,	// CODE_MOSAIC_C				Mosaic C
	false,	// CODE_MOSAIC_D				Mosaic D
	false,	// CODE_PROP_ALPHANUMERIC		Proportional Alphanumeric
	false,	// CODE_PROP_HIRAGANA			Proportional Hiragana
	false,	// CODE_PROP_KATAKANA			Proportional Katakana
	false,	// CODE_JIS_X0201_KATAKANA		JIS X 0201 Katakana
	true,	// CODE_JIS_KANJI_PLANE_1		JIS compatible Kanji Plane 1
	true,	// CODE_JIS_KANJI_PLANE_2		JIS compatible Kanji Plane 2
	true	// CODE_ADDITIONAL_SYMBOLS		Additional symbols
};

const DWORD CAribString::AribToString(TCHAR *lpszDst, const BYTE *pSrcData, const DWORD dwSrcLen)
{
	// ARIB STD-B24 Part1 → Shift-JIS / Unicode変換
	CAribString WorkObject;
	
	return WorkObject.AribToStringInternal(lpszDst, pSrcData, dwSrcLen);
}

CAribString::CAribString(void)
	: m_byEscSeqCount(0U)
	, m_byEscSeqIndex(0U)
	, m_bIsEscSeqDrcs(false)
	, m_FontSize(SIZE_NORMAL)
	, m_pSingleGL(NULL)
{
	// 状態初期設定
	m_CodeG[0] = CODE_KANJI;
	m_CodeG[1] = CODE_ALPHANUMERIC;
	m_CodeG[2] = CODE_HIRAGANA;
	m_CodeG[3] = CODE_KATAKANA;

	m_pLockingGL = &m_CodeG[0];
	m_pLockingGR = &m_CodeG[2];
}

const DWORD CAribString::AribToStringInternal(TCHAR *lpszDst, const BYTE *pSrcData, const DWORD dwSrcLen)
{
	if(!pSrcData || !dwSrcLen || !lpszDst)return 0UL;

	DWORD dwSrcPos = 0UL;
	DWORD dwDstLen = 0UL;

	while(dwSrcPos < dwSrcLen){
		if(!m_byEscSeqCount){
			if(pSrcData[dwSrcPos] <= 0x20){
				// C0領域
				dwDstLen += ProcessCtrlC0(&lpszDst[dwDstLen], &pSrcData[dwSrcPos], dwSrcPos);
				}
			else if(pSrcData[dwSrcPos] <= 0x7EU){
				// GL領域
				const CODE_SET CurCodeSet = (m_pSingleGL)? *m_pSingleGL : *m_pLockingGL;
				m_pSingleGL = NULL;
				
				if(abCharSizeTable[CurCodeSet]){
					// 2バイトコード
					if((dwSrcLen - dwSrcPos) < 2UL)break;
					
					dwDstLen += ProcessCharCode(&lpszDst[dwDstLen], ((WORD)pSrcData[dwSrcPos + 0] << 8) | (WORD)pSrcData[dwSrcPos + 1], CurCodeSet);
					dwSrcPos += 2;
					}
				else{
					// 1バイトコード
					dwDstLen += ProcessCharCode(&lpszDst[dwDstLen], (WORD)pSrcData[dwSrcPos], CurCodeSet);
					dwSrcPos++;
					}
				}
			else if(pSrcData[dwSrcPos] <= 0xA0){
				// C1領域
				dwDstLen += ProcessCtrlC1(&lpszDst[dwDstLen], &pSrcData[dwSrcPos], dwSrcPos);
				}
			else if(pSrcData[dwSrcPos] <= 0xFEU){
				// GR領域
				const CODE_SET CurCodeSet = *m_pLockingGR;
				
				if(abCharSizeTable[CurCodeSet]){
					// 2バイトコード
					if((dwSrcLen - dwSrcPos) < 2UL)break;
					
					dwDstLen += ProcessCharCode(&lpszDst[dwDstLen], ((WORD)(pSrcData[dwSrcPos + 0] & 0x7FU) << 8) | (WORD)(pSrcData[dwSrcPos + 1] & 0x7FU), CurCodeSet);
					dwSrcPos += 2;
					}
				else{
					// 1バイトコード
					dwDstLen += ProcessCharCode(&lpszDst[dwDstLen], (WORD)(pSrcData[dwSrcPos] & 0x7FU), CurCodeSet);
					dwSrcPos++;
					}
				}
			else{
				// 何もしない
				}
			}
		else{
			// エスケープシーケンス処理
			ProcessEscapeSeq(pSrcData[dwSrcPos++]);
			}
		}

	// 終端文字
	lpszDst[dwDstLen] = TEXT('\0');

	return dwDstLen;
}

inline const DWORD CAribString::ProcessCharCode(TCHAR *lpszDst, const WORD wCode, const CODE_SET CodeSet)
{
	switch(CodeSet){
		case CODE_KANJI	:
		case CODE_JIS_KANJI_PLANE_1 :
		case CODE_JIS_KANJI_PLANE_2 :
			// 漢字コード出力
			return PutKanjiChar(lpszDst, wCode);

		case CODE_ALPHANUMERIC :
		case CODE_PROP_ALPHANUMERIC :
			// 英数字コード出力
			return PutAlphanumericChar(lpszDst, wCode);

		case CODE_HIRAGANA :
		case CODE_PROP_HIRAGANA :
			// ひらがなコード出力
			return PutHiraganaChar(lpszDst, wCode);

		case CODE_PROP_KATAKANA :
		case CODE_KATAKANA :
			// カタカナコード出力
			return PutKatakanaChar(lpszDst, wCode);

		case CODE_JIS_X0201_KATAKANA :
			// JISカタカナコード出力
			return PutJisKatakanaChar(lpszDst, wCode);

		case CODE_ADDITIONAL_SYMBOLS :
			// 追加シンボルコード出力
			return PutSymbolsChar(lpszDst, wCode);

		default :
			return 0UL;
		}
}

inline const DWORD CAribString::PutKanjiChar(TCHAR *lpszDst, const WORD wCode)
{
	// JIS→Shift-JIS漢字コード変換
	const WORD wShiftJIS = ::_mbcjistojms(wCode);

#ifdef _UNICODE
	// Shift-JIS → UNICODE
	const char szShiftJIS[3] = {(char)(wShiftJIS >> 8), (char)(wShiftJIS & 0x00FFU), '\0'};
	::MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, szShiftJIS, 2, lpszDst, 2);

	return 1UL;
#else
	// Shift-JIS → Shift-JIS
	lpszDst[0] = (char)(wShiftJIS >> 8);
	lpszDst[1] = (char)(wShiftJIS & 0x00FFU);

	return 2UL;
#endif
}

inline const DWORD CAribString::PutAlphanumericChar(TCHAR *lpszDst, const WORD wCode)
{
	// 英数字文字コード変換
	static const TCHAR *acAlphanumericFullTable = 
		TEXT("　　　　　　　　　　　　　　　　")
		TEXT("　　　　　　　　　　　　　　　　")
		TEXT("　！”＃＄％＆’（）＊＋，－．／")
		TEXT("０１２３４５６７８９：；＜＝＞？")
		TEXT("＠ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯ")
		TEXT("ＰＱＲＳＴＵＶＷＸＹＺ［￥］＾＿")
		TEXT("　ａｂｃｄｅｆｇｈｉｌｋｌｍｎｏ")
		TEXT("ｐｑｒｓｔｕｖｗｘｙｚ｛｜｝￣　");

	static const TCHAR *acAlphanumericHalfTable = 
		TEXT("                ")
		TEXT("                ")
		TEXT(" !\"#$%&'()*+,-./")
		TEXT("0123456789:;<=>?")
		TEXT("@ABCDEFGHIJKLMNO")
		TEXT("PQRSTUVWXYZ[\\]^_")
		TEXT(" abcdefghilklmno")
		TEXT("pqrstuvwxyz{|}~ ");

#ifdef _UNICODE
	lpszDst[0] = (m_FontSize == SIZE_SMALL)? acAlphanumericHalfTable[wCode] : acAlphanumericFullTable[wCode];
	return 1UL;
#else
	if(m_FontSize == SIZE_SMALL){
		lpszDst[0] = acAlphanumericHalfTable[wCode];
		}
	else{
		lpszDst[0] = acAlphanumericFullTable[wCode * 2U + 0U];
		lpszDst[1] = acAlphanumericFullTable[wCode * 2U + 1U];

		return 2UL;
		}
#endif
}

inline const DWORD CAribString::PutHiraganaChar(TCHAR *lpszDst, const WORD wCode)
{
	// ひらがな文字コード変換
	static const TCHAR *acHiraganaTable = 
		TEXT("　　　　　　　　　　　　　　　　")
		TEXT("　　　　　　　　　　　　　　　　")
		TEXT("　ぁあぃいぅうぇえぉおかがきぎく")
		TEXT("ぐけげこごさざしじすずせぜそぞた")
		TEXT("だちぢっつづてでとどなにぬねのは")
		TEXT("ばぱひびぴふぶぷへべぺほぼぽまみ")
		TEXT("むめもゃやゅゆょよらりるれろゎわ")
		TEXT("ゐゑをん　　　ゝゞー。「」、・　");
	
#ifdef _UNICODE
	lpszDst[0] = acHiraganaTable[wCode];

	return 1UL;
#else
	lpszDst[0] = acHiraganaTable[wCode * 2U + 0U];
	lpszDst[1] = acHiraganaTable[wCode * 2U + 1U];

	return 2UL;
#endif
}

inline const DWORD CAribString::PutKatakanaChar(TCHAR *lpszDst, const WORD wCode)
{
	// カタカナ英数字文字コード変換
	static const TCHAR *acKatakanaTable = 
		TEXT("　　　　　　　　　　　　　　　　")
		TEXT("　　　　　　　　　　　　　　　　")
		TEXT("　ァアィイゥウェエォオカガキギク")
		TEXT("グケゲコゴサザシジスズセゼソゾタ")
		TEXT("ダチヂッツヅテデトドナニヌネノハ")
		TEXT("バパヒビピフブプヘベペホボポマミ")
		TEXT("ムメモャヤュユョヨラリルレロヮワ")
		TEXT("ヰヱヲンヴヵヶヽヾー。「」、・　");
	
#ifdef _UNICODE
	lpszDst[0] = acKatakanaTable[wCode];

	return 1UL;
#else
	lpszDst[0] = acKatakanaTable[wCode * 2U + 0U];
	lpszDst[1] = acKatakanaTable[wCode * 2U + 1U];

	return 2UL;
#endif
}

inline const DWORD CAribString::PutJisKatakanaChar(TCHAR *lpszDst, const WORD wCode)
{
	// JISカタカナ文字コード変換
	static const TCHAR *acJisKatakanaFullTable = 
		TEXT("　　　　　　　　　　　　　　　　")
		TEXT("　　　　　　　　　　　　　　　　")
		TEXT("　。「」、・ヲァィゥェォャュョッ")
		TEXT("ーアイウエオカキクケコサシスセソ")
		TEXT("タチツテトナニヌネノハヒフヘホマ")
		TEXT("ミムメモヤユヨラリルレロワン゛゜")
		TEXT("　　　　　　　　　　　　　　　　")
		TEXT("　　　　　　　　　　　　　　　　");

	static const TCHAR *acJisKatakanaHalfTable = 
		TEXT("                ")
		TEXT("                ")
		TEXT(" ｡｢｣､･ｦｧｨｩｪｫｬｭｮｯ")
		TEXT("ｰｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿ")
		TEXT("ﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏ")
		TEXT("ﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜﾝﾞﾟ")
		TEXT("                ")
		TEXT("                ");
	
#ifdef _UNICODE
	lpszDst[0] = (m_FontSize == SIZE_SMALL)? acJisKatakanaHalfTable[wCode] : acJisKatakanaFullTable[wCode];
	return 1UL;
#else
	if(m_FontSize == SIZE_SMALL){
		lpszDst[0] = acJisKatakanaHalfTable[wCode];
		}
	else{
		lpszDst[0] = acJisKatakanaFullTable[wCode * 2U + 0U];
		lpszDst[1] = acJisKatakanaFullTable[wCode * 2U + 1U];

		return 2UL;
		}
#endif
}

inline const DWORD CAribString::PutSymbolsChar(TCHAR *lpszDst, const WORD wCode)
{
	// 追加シンボル文字コード変換(とりあえず必要そうなものだけ)
	static const TCHAR *aszSymbolsTable1[] =
	{
		_T("[HV]"),		_T("[SD]"),		_T("[Ｐ]"),		_T("[Ｗ]"),		_T("[MV]"),		_T("[手]"),		_T("[字]"),		_T("[双]"),			// 0x7A50 - 0x7A57	90/48 - 90/55
		_T("[デ]"),		_T("[Ｓ]"),		_T("[二]"),		_T("[多]"),		_T("[解]"),		_T("[SS]"),		_T("[Ｂ]"),		_T("[Ｎ]"),			// 0x7A58 - 0x7A5F	90/56 - 90/63
		_T("■"),		_T("●"),		_T("[天]"),		_T("[交]"),		_T("[映]"),		_T("[無]"),		_T("[料]"),		_T("[年齢制限]"),	// 0x7A60 - 0x7A67	90/64 - 90/71
		_T("[前]"),		_T("[後]"),		_T("[再]"),		_T("[新]"),		_T("[初]"),		_T("[終]"),		_T("[生]"),		_T("[販]"),			// 0x7A68 - 0x7A6F	90/72 - 90/79
		_T("[声]"),		_T("[吹]"),		_T("[PPV]"),	_T("(秘)"),		_T("ほか")															// 0x7A70 - 0x7A74	90/80 - 90/84
	};

	static const TCHAR *aszSymbolsTable2[] =
	{
		_T("→"),		_T("←"),		_T("↑"),		_T("↓"),		_T("●"),		_T("○"),		_T("年"),		_T("月"),			// 0x7C21 - 0x7C28	92/01 - 92/08
		_T("日"),		_T("円"),		_T("㎡"),		_T("立方ｍ"),	_T("㎝"),		_T("平方㎝"),	_T("立方㎝"),	_T("０."),			// 0x7C29 - 0x7C30	92/09 - 92/16
		_T("１."),		_T("２."),		_T("３."),		_T("４."),		_T("５."),		_T("６."),		_T("７."),		_T("８."),			// 0x7C31 - 0x7C38	92/17 - 92/24
		_T("９."),		_T("氏"),		_T("副"),		_T("元"),		_T("故"),		_T("前"),		_T("新"),		_T("０,"),			// 0x7C39 - 0x7C40	92/25 - 92/32
		_T("１,"),		_T("２,"),		_T("３,"),		_T("４,"),		_T("５,"),		_T("６,"),		_T("７,"),		_T("８,"),			// 0x7C41 - 0x7C48	92/33 - 92/40
		_T("９,"),		_T("(社)"),		_T("(財)"),		_T("(有)"),		_T("(株)"),		_T("(代)"),		_T("(問)"),		_T("＞"),			// 0x7C49 - 0x7C50	92/41 - 92/48
		_T("＜"),		_T("【"),		_T("】"),		_T("◇"),		_T("^2"),		_T("^3"),		_T("(CD)"),		_T("(vn)"),			// 0x7C51 - 0x7C58	92/49 - 92/56
		_T("(ob)"),		_T("(cb)"),		_T("(ce"),		_T("mb)"),		_T("(hp)"),		_T("(br)"),		_T("(p)"),		_T("(s)"),			// 0x7C59 - 0x7C60	92/57 - 92/64
		_T("(ms)"),		_T("(t)"),		_T("(bs)"),		_T("(b)"),		_T("(tb)"),		_T("(tp)"),		_T("(ds)"),		_T("(ag)"),			// 0x7C61 - 0x7C68	92/65 - 92/72
		_T("(eg)"),		_T("(vo)"),		_T("(fl)"),		_T("(ke"),		_T("y)"),		_T("(sa"),		_T("x)"),		_T("(sy"),			// 0x7C69 - 0x7C70	92/73 - 92/80
		_T("n)"),		_T("(or"),		_T("g)"),		_T("(pe"),		_T("r)"),		_T("(R)"),		_T("(C)"),		_T("(箏)"),			// 0x7C71 - 0x7C78	92/81 - 92/88
		_T("DJ"),		_T("[演]"),		_T("Fax")																							// 0x7C79 - 0x7C7B	92/89 - 92/91
	};

	static const TCHAR *aszSymbolsTable3[] =
	{
		_T("(月)"),		_T("(火)"),		_T("(水)"),		_T("(木)"),		_T("(金)"),		_T("(土)"),		_T("(日)"),		_T("(祝)"),			// 0x7D21 - 0x7D28	93/01 - 93/08
		_T("㍾"),		_T("㍽"),		_T("㍼"),		_T("㍻"),		_T("№"),		_T("℡"),		_T("(〒)"),		_T("○"),			// 0x7D29 - 0x7D30	93/09 - 93/16
		_T("〔本〕"),	_T("〔三〕"),	_T("〔二〕"),	_T("〔安〕"),	_T("〔点〕"),	_T("〔打〕"),	_T("〔盗〕"),	_T("〔勝〕"),		// 0x7D31 - 0x7D38	93/17 - 93/24
		_T("〔敗〕"),	_T("〔Ｓ〕"),	_T("［投］"),	_T("［捕］"),	_T("［一］"),	_T("［二］"),	_T("［三］"),	_T("［遊］"),		// 0x7D39 - 0x7D40	93/25 - 93/32
		_T("［左］"),	_T("［中］"),	_T("［右］"),	_T("［指］"),	_T("［走］"),	_T("［打］"),	_T("㍑"),		_T("㎏"),			// 0x7D41 - 0x7D48	93/33 - 93/40
		_T("Hz"),		_T("ha"),		_T("km"),		_T("平方km"),	_T("hPa"),		_T("・"),		_T("・"),		_T("1/2"),			// 0x7D49 - 0x7D50	93/41 - 93/48
		_T("0/3"),		_T("1/3"),		_T("2/3"),		_T("1/4"),		_T("3/4"),		_T("1/5"),		_T("2/5"),		_T("3/5"),			// 0x7D51 - 0x7D58	93/49 - 93/56
		_T("4/5"),		_T("1/6"),		_T("5/6"),		_T("1/7"),		_T("1/8"),		_T("1/9"),		_T("1/10"),		_T("晴れ"),			// 0x7D59 - 0x7D60	93/57 - 93/64
		_T("曇り"),		_T("雨"),		_T("雪"),		_T("△"),		_T("▲"),		_T("▽"),		_T("▼"),		_T("◆"),			// 0x7D61 - 0x7D68	93/65 - 93/72
		_T("・"),		_T("・"),		_T("・"),		_T("◇"),		_T("◎"),		_T("!!"),		_T("!?"),		_T("曇/晴"),		// 0x7D69 - 0x7D70	93/73 - 93/80
		_T("雨"),		_T("雨"),		_T("雪"),		_T("大雪"),		_T("雷"),		_T("雷雨"),		_T("　"),		_T("・"),			// 0x7D71 - 0x7D78	93/81 - 93/88
		_T("・"),		_T("♪"),		_T("℡")																							// 0x7D79 - 0x7D7B	93/89 - 93/91
	};

	static const TCHAR *aszSymbolsTable4[] =
	{
		_T("Ⅰ"),		_T("Ⅱ"),		_T("Ⅲ"),		_T("Ⅳ"),		_T("Ⅴ"),		_T("Ⅵ"),		_T("Ⅶ"),		_T("Ⅷ"),			// 0x7E21 - 0x7E28	94/01 - 94/08
		_T("Ⅸ"),		_T("Ⅹ"),		_T("XI"),		_T("XⅡ"),		_T("⑰"),		_T("⑱"),		_T("⑲"),		_T("⑳"),			// 0x7E29 - 0x7E30	94/09 - 94/16
		_T("(1)"),		_T("(2)"),		_T("(3)"),		_T("(4)"),		_T("(5)"),		_T("(6)"),		_T("(7)"),		_T("(8)"),			// 0x7E31 - 0x7E38	94/17 - 94/24
		_T("(9)"),		_T("(10)"),		_T("(11)"),		_T("(12)"),		_T("(21)"),		_T("(22)"),		_T("(23)"),		_T("(24)"),			// 0x7E39 - 0x7E40	94/25 - 94/32
		_T("(A)"),		_T("(B)"),		_T("(C)"),		_T("(D)"),		_T("(E)"),		_T("(F)"),		_T("(G)"),		_T("(H)"),			// 0x7E41 - 0x7E48	94/33 - 94/40
		_T("(I)"),		_T("(J)"),		_T("(K)"),		_T("(L)"),		_T("(M)"),		_T("(N)"),		_T("(O)"),		_T("(P)"),			// 0x7E49 - 0x7E50	94/41 - 94/48
		_T("(Q)"),		_T("(R)"),		_T("(S)"),		_T("(T)"),		_T("(U)"),		_T("(V)"),		_T("(W)"),		_T("(X)"),			// 0x7E51 - 0x7E58	94/49 - 94/56
		_T("(Y)"),		_T("(Z)"),		_T("(25)"),		_T("(26)"),		_T("(27)"),		_T("(28)"),		_T("(29)"),		_T("(30)"),			// 0x7E59 - 0x7E60	94/57 - 94/64
		_T("①"),		_T("②"),		_T("③"),		_T("④"),		_T("⑤"),		_T("⑥"),		_T("⑦"),		_T("⑧"),			// 0x7E61 - 0x7E68	94/65 - 94/72
		_T("⑨"),		_T("⑩"),		_T("⑪"),		_T("⑫"),		_T("⑬"),		_T("⑭"),		_T("⑮"),		_T("⑯"),			// 0x7E69 - 0x7E70	94/73 - 94/80
		_T("①"),		_T("②"),		_T("③"),		_T("④"),		_T("⑤"),		_T("⑥"),		_T("⑦"),		_T("⑧"),			// 0x7E71 - 0x7E78	94/81 - 94/88
		_T("⑨"),		_T("⑩"),		_T("⑪"),		_T("⑫"),		_T("(31)")															// 0x7E79 - 0x7E7D	94/89 - 94/93
	};

	// シンボルを変換する
	if((wCode >= 0x7A50U) && (wCode <= 0x7A74U)){
		::_tcscpy(lpszDst, aszSymbolsTable1[wCode - 0x7A50U]);
		}
	else if((wCode >= 0x7C21U) && (wCode <= 0x7C7BU)){
		::_tcscpy(lpszDst, aszSymbolsTable2[wCode - 0x7C21U]);
		}
	else if((wCode >= 0x7D21U) && (wCode <= 0x7D7BU)){
		::_tcscpy(lpszDst, aszSymbolsTable3[wCode - 0x7D21U]);
		}
	else if((wCode >= 0x7E21U) && (wCode <= 0x7E7DU)){
		::_tcscpy(lpszDst, aszSymbolsTable4[wCode - 0x7E21U]);
		}
	else{
		::_tcscpy(lpszDst, TEXT("・"));
		}

	return ::_tcslen(lpszDst);
}

inline const DWORD CAribString::ProcessCtrlC0(TCHAR *lpszDst, const BYTE *pCode, DWORD &dwSrcPos)
{
	// C0制御コード処理
	switch(pCode[0]){
		case 0x0DU	:	// APR
			lpszDst[0] = TEXT('\r');
			lpszDst[1] = TEXT('\n');
			dwSrcPos++;
			return 2UL;
			
		case 0x0EU	:	// LS1
			LockingShiftGL(1U);
			dwSrcPos++;
			return 0UL;
			
		case 0x0FU	:	// LS0
			LockingShiftGL(0U);
			dwSrcPos++;
			return 0UL;

		case 0x16U	:	// PAPF　※未対応のため読み飛ばすだけ
			dwSrcPos += 2UL;
			return 0UL;
			
		case 0x19U	:	// SS2
			SingleShiftGL(2U);
			dwSrcPos++;
			return 0UL;
			
		case 0x1BU	:	// ESC
			m_byEscSeqCount = 1U;
			dwSrcPos++;
			return 0UL;

		case 0x1CU	:	// APS　※未対応のため読み飛ばすだけ
			dwSrcPos += 3UL;
			return 0UL;
			
		case 0x1DU	:	// SS3
			SingleShiftGL(3U);
			dwSrcPos++;
			return 0UL;
			
		case 0x20U	:	// SP
			dwSrcPos++;
			if(m_FontSize == SIZE_SMALL){
				// 半角
				lpszDst[0] = TEXT(' ');
				return 1UL;
				}
			else{
				// 全角
#ifdef _UNICODE
				lpszDst[0] = TEXT('　');
				return 1UL;
#else
				::_tcscpy(lpszDst, TEXT("　"));
				return 2UL;
#endif
				}
			
		default		:	// 非対応の制御コード
			dwSrcPos++;
			return 0UL;
		}
}

inline const DWORD CAribString::ProcessCtrlC1(TCHAR *lpszDst, const BYTE *pCode, DWORD &dwSrcPos)
{
	// C1制御コード処理
	switch(pCode[0]){
		case 0x89U	:	// MSZ
			m_FontSize = SIZE_SMALL;
			dwSrcPos++;
			return 0UL;

		case 0x8AU	:	// NSZ
			m_FontSize = SIZE_NORMAL;
			dwSrcPos++;
			return 0UL;

		case 0x8BU	:	// SZX　※未対応のため読み飛ばすだけ
			m_FontSize = SIZE_NORMAL;
			dwSrcPos += 2UL;
			return 0UL;

		case 0x90U	:	// COL　※未対応のため読み飛ばすだけ
			dwSrcPos += ((pCode[1] == 0x20U)? 3UL : 2UL);
			return 0UL;

		case 0x91U	:	// FLC　※未対応のため読み飛ばすだけ
			dwSrcPos += 2UL;
			return 0UL;

		case 0x92U	:	// CDC　※未対応のため読み飛ばすだけ
			dwSrcPos += ((pCode[1] == 0x20U)? 3UL : 2UL);
			return 0UL;

		case 0x93U	:	// POL　※未対応のため読み飛ばすだけ
			dwSrcPos += 2UL;
			return 0UL;

		case 0x94U	:	// WMM　※未対応のため読み飛ばすだけ
			dwSrcPos += 2UL;
			return 0UL;

		case 0x95U	:	// MACRO ※未対応のため読み飛ばすだけ
			while(*(pCode++) != 0x4FU)dwSrcPos++;
			return 0UL;

		case 0x97U	:	// HLC　※未対応のため読み飛ばすだけ
			dwSrcPos += 2UL;
			return 0UL;

		case 0x98U	:	// RPC　※未対応のため読み飛ばすだけ
			dwSrcPos += 2UL;
			return 0UL;

		case 0x9DU	:	// TIME	※未対応のため読み飛ばすだけ
			dwSrcPos += 3UL;
			return 0UL;
			
		default		:	// 非対応の制御コード
			dwSrcPos++;
			return 0UL;
		}
}

inline void CAribString::ProcessEscapeSeq(const BYTE byCode)
{
	// エスケープシーケンス処理
	switch(m_byEscSeqCount){
		// 1バイト目
		case 1U	:
			switch(byCode){
				// Invocation of code elements
				case 0x6EU	: LockingShiftGL(2U);	m_byEscSeqCount = 0U;	return;		// LS2
				case 0x6FU	: LockingShiftGL(3U);	m_byEscSeqCount = 0U;	return;		// LS3
				case 0x7EU	: LockingShiftGR(1U);	m_byEscSeqCount = 0U;	return;		// LS1R
				case 0x7DU	: LockingShiftGR(2U);	m_byEscSeqCount = 0U;	return;		// LS2R
				case 0x7CU	: LockingShiftGR(3U);	m_byEscSeqCount = 0U;	return;		// LS3R

				// Designation of graphic sets
				case 0x24U	:	
				case 0x28U	: m_byEscSeqIndex = 0U;		break;
				case 0x29U	: m_byEscSeqIndex = 1U;		break;
				case 0x2AU	: m_byEscSeqIndex = 2U;		break;
				case 0x2BU	: m_byEscSeqIndex = 3U;		break;
				default		: m_byEscSeqCount = 0U;		return;		// エラー
				}
			break;

		// 2バイト目
		case 2U	:
			if(DesignationGSET(m_byEscSeqIndex, byCode)){
				m_byEscSeqCount = 0U;
				return;
				}
			
			switch(byCode){
				case 0x20	: m_bIsEscSeqDrcs = true;	break;
				case 0x28	: m_bIsEscSeqDrcs = true;	m_byEscSeqIndex = 0U;	break;
				case 0x29	: m_bIsEscSeqDrcs = false;	m_byEscSeqIndex = 1U;	break;
				case 0x2A	: m_bIsEscSeqDrcs = false;	m_byEscSeqIndex = 2U;	break;
				case 0x2B	: m_bIsEscSeqDrcs = false;	m_byEscSeqIndex = 3U;	break;
				default		: m_byEscSeqCount = 0U;		return;		// エラー
				}
			break;

		// 3バイト目
		case 3U	:
			if(!m_bIsEscSeqDrcs){
				if(DesignationGSET(m_byEscSeqIndex, byCode)){
					m_byEscSeqCount = 0U;
					return;
					}
				}
			else{
				if(DesignationDRCS(m_byEscSeqIndex, byCode)){
					m_byEscSeqCount = 0U;
					return;
					}
				}

			if(byCode == 0x20U){
				m_bIsEscSeqDrcs = true;
				}
			else{
				// エラー
				m_byEscSeqCount = 0U;
				return;
				}
			break;

		// 4バイト目
		case 4U	:
			DesignationDRCS(m_byEscSeqIndex, byCode);
			m_byEscSeqCount = 0U;
			return;
		}

	m_byEscSeqCount++;
}

inline void CAribString::LockingShiftGL(const BYTE byIndexG)
{
	// LSx
	m_pLockingGL = &m_CodeG[byIndexG];
}

inline void CAribString::LockingShiftGR(const BYTE byIndexG)
{
	// LSxR
	m_pLockingGR = &m_CodeG[byIndexG];
}

inline void CAribString::SingleShiftGL(const BYTE byIndexG)
{
	// SSx
	m_pSingleGL  = &m_CodeG[byIndexG];
}

inline const bool CAribString::DesignationGSET(const BYTE byIndexG, const BYTE byCode)
{
	// Gのグラフィックセットを割り当てる
	switch(byCode){
		case 0x42U	: m_CodeG[byIndexG] = CODE_KANJI;				return true;	// Kanji
		case 0x4AU	: m_CodeG[byIndexG] = CODE_ALPHANUMERIC;		return true;	// Alphanumeric
		case 0x30U	: m_CodeG[byIndexG] = CODE_HIRAGANA;			return true;	// Hiragana
		case 0x31U	: m_CodeG[byIndexG] = CODE_KATAKANA;			return true;	// Katakana
		case 0x32U	: m_CodeG[byIndexG] = CODE_MOSAIC_A;			return true;	// Mosaic A
		case 0x33U	: m_CodeG[byIndexG] = CODE_MOSAIC_B;			return true;	// Mosaic B
		case 0x34U	: m_CodeG[byIndexG] = CODE_MOSAIC_C;			return true;	// Mosaic C
		case 0x35U	: m_CodeG[byIndexG] = CODE_MOSAIC_D;			return true;	// Mosaic D
		case 0x36U	: m_CodeG[byIndexG] = CODE_PROP_ALPHANUMERIC;	return true;	// Proportional Alphanumeric
		case 0x37U	: m_CodeG[byIndexG] = CODE_PROP_HIRAGANA;		return true;	// Proportional Hiragana
		case 0x38U	: m_CodeG[byIndexG] = CODE_PROP_KATAKANA;		return true;	// Proportional Katakana
		case 0x49U	: m_CodeG[byIndexG] = CODE_JIS_X0201_KATAKANA;	return true;	// JIS X 0201 Katakana
		case 0x39U	: m_CodeG[byIndexG] = CODE_JIS_KANJI_PLANE_1;	return true;	// JIS compatible Kanji Plane 1
		case 0x3AU	: m_CodeG[byIndexG] = CODE_JIS_KANJI_PLANE_2;	return true;	// JIS compatible Kanji Plane 2
		case 0x3BU	: m_CodeG[byIndexG] = CODE_ADDITIONAL_SYMBOLS;	return true;	// Additional symbols
		default		: return false;		// 不明なグラフィックセット
		}
}

inline const bool CAribString::DesignationDRCS(const BYTE byIndexG, const BYTE byCode)
{
	// DRCSのグラフィックセットを割り当てる
	switch(byCode){
		case 0x40U	: m_CodeG[byIndexG] = CODE_UNKNOWN;				return true;	// DRCS-0
		case 0x41U	: m_CodeG[byIndexG] = CODE_UNKNOWN;				return true;	// DRCS-1
		case 0x42U	: m_CodeG[byIndexG] = CODE_UNKNOWN;				return true;	// DRCS-2
		case 0x43U	: m_CodeG[byIndexG] = CODE_UNKNOWN;				return true;	// DRCS-3
		case 0x44U	: m_CodeG[byIndexG] = CODE_UNKNOWN;				return true;	// DRCS-4
		case 0x45U	: m_CodeG[byIndexG] = CODE_UNKNOWN;				return true;	// DRCS-5
		case 0x46U	: m_CodeG[byIndexG] = CODE_UNKNOWN;				return true;	// DRCS-6
		case 0x47U	: m_CodeG[byIndexG] = CODE_UNKNOWN;				return true;	// DRCS-7
		case 0x48U	: m_CodeG[byIndexG] = CODE_UNKNOWN;				return true;	// DRCS-8
		case 0x49U	: m_CodeG[byIndexG] = CODE_UNKNOWN;				return true;	// DRCS-9
		case 0x4AU	: m_CodeG[byIndexG] = CODE_UNKNOWN;				return true;	// DRCS-10
		case 0x4BU	: m_CodeG[byIndexG] = CODE_UNKNOWN;				return true;	// DRCS-11
		case 0x4CU	: m_CodeG[byIndexG] = CODE_UNKNOWN;				return true;	// DRCS-12
		case 0x4DU	: m_CodeG[byIndexG] = CODE_UNKNOWN;				return true;	// DRCS-13
		case 0x4EU	: m_CodeG[byIndexG] = CODE_UNKNOWN;				return true;	// DRCS-14
		case 0x4FU	: m_CodeG[byIndexG] = CODE_UNKNOWN;				return true;	// DRCS-15
		case 0x70U	: m_CodeG[byIndexG] = CODE_UNKNOWN;				return true;	// Macro
		default		: return false;		// 不明なグラフィックセット
		}
}


/////////////////////////////////////////////////////////////////////////////
// ARIB STD-B10 Part2 Annex C MJD+JTC 処理クラス
/////////////////////////////////////////////////////////////////////////////

void CAribTime::AribToSystemTime(SYSTEMTIME *pSysTime, const BYTE *pHexData)
{
	// 全ビットが1のときは未定義
	if((*((DWORD *)pHexData) == 0xFFFFFFFFUL) && (pHexData[4] == 0xFFU)){
		::ZeroMemory(pSysTime, sizeof(SYSTEMTIME));
		return;
		}

	// MJD形式の日付を解析
	SplitAribMjd(((WORD)pHexData[0] << 8) | (WORD)pHexData[1], &pSysTime->wYear, &pSysTime->wMonth, &pSysTime->wDay, &pSysTime->wDayOfWeek);

	// BCD形式の時刻を解析
	SplitAribBcd(&pHexData[2], &pSysTime->wHour, &pSysTime->wMinute, &pSysTime->wSecond);

	// ミリ秒は常に0
	pSysTime->wMilliseconds = 0U;
}

void CAribTime::SplitAribMjd(const WORD wAribMjd, WORD *pwYear, WORD *pwMonth, WORD *pwDay, WORD *pwDayOfWeek)
{
	// MJD形式の日付を解析する
	const DWORD dwYd = (DWORD)(((double)wAribMjd - 15078.2) / 365.25);
	const DWORD dwMd = (DWORD)(((double)wAribMjd - 14956.1 - (double)((int)((double)dwYd * 365.25))) / 30.6001);
	const DWORD dwK = ((dwMd == 14UL) || (dwMd == 15UL))? 1U : 0U;

	if(pwDay)*pwDay = wAribMjd - 14956U - (WORD)((double)dwYd * 365.25) - (WORD)((double)dwMd * 30.6001);
	if(pwYear)*pwYear = (WORD)(dwYd + dwK) + 1900U;
	if(pwMonth)*pwMonth	= (WORD)(dwMd - 1UL - dwK * 12UL);
	if(pwDayOfWeek)*pwDayOfWeek = (wAribMjd + 2U) % 7U;
}

void CAribTime::SplitAribBcd(const BYTE *pAribBcd, WORD *pwHour, WORD *pwMinute, WORD *pwSecond)
{
	// BCD形式の時刻を解析する
	if((pAribBcd[0] == 0xFFU) && (pAribBcd[1] == 0xFFU) && (pAribBcd[2] == 0xFFU)){
		// 時間未定
		if(pwHour)*pwHour		= 0U;
		if(pwMinute)*pwMinute	= 0U;
		if(pwSecond)*pwSecond	= 0U;
		}
	else{
		// 時間未定以外
		if(pwHour)*pwHour		= (WORD)(pAribBcd[0] >> 4) * 10U + (WORD)(pAribBcd[0] & 0x0FU);
		if(pwMinute)*pwMinute	= (WORD)(pAribBcd[1] >> 4) * 10U + (WORD)(pAribBcd[1] & 0x0FU);
		if(pwSecond)*pwSecond	= (WORD)(pAribBcd[2] >> 4) * 10U + (WORD)(pAribBcd[2] & 0x0FU);
		}
}

const DWORD CAribTime::AribBcdToSecond(const BYTE *pAribBcd)
{
	// BCD形式の時刻を秒に変換する
	if((pAribBcd[0] == 0xFFU) && (pAribBcd[1] == 0xFFU) && (pAribBcd[2] == 0xFFU)){
		// 時間未定
		return 0UL;
		}
	else{
		// 時間未定以外
		return (((DWORD)(pAribBcd[0] >> 4) * 10U + (DWORD)(pAribBcd[0] & 0x0FU)) * 3600UL)
							 + (((DWORD)(pAribBcd[1] >> 4) * 10U + (DWORD)(pAribBcd[1] & 0x0FU)) * 60UL)
							 + ((DWORD)(pAribBcd[2] >> 4) * 10U + (DWORD)(pAribBcd[2] & 0x0FU));
		}
}


/////////////////////////////////////////////////////////////////////////////
// SYSTEMTIME ラップクラス
/////////////////////////////////////////////////////////////////////////////

CTsTime::CTsTime()
{
	// 空の時間を生成
	ClearTime();
}

CTsTime::CTsTime(const SYSTEMTIME &SystemTime)
{
	// SYSTEMTIME形式から時間を生成
	ClearTime();
	*this = SystemTime;
}

CTsTime::CTsTime(const ULONGLONG llFileTime)
{
	// FILETIME形式から時間を生成(1601/1/1からの100ns単位の経過時間)
	::FileTimeToSystemTime((FILETIME *)&llFileTime, this);
}

CTsTime::CTsTime(const BYTE *pHexData)
{
	// ARIB MJD+JTC 形式から生成
	SetAribTime(pHexData);
}

CTsTime::CTsTime(const WORD wYear, const WORD wMonth, const WORD wDay, const WORD wHour, const WORD wMinute, const WORD wSecond)
{
	// 各要素を指定して生成
	ClearTime();
	SetTime(wYear, wMonth, wDay, wHour, wMinute, wSecond);
}

void CTsTime::SetNowTime(void)
{
	// 現在時刻をセットする
	::GetLocalTime(this);
}

void CTsTime::SetAribTime(const BYTE *pHexData)
{
	// ARIB MJD+JTC 形式から生成
	::ZeroMemory(this, sizeof(CTsTime));
	CAribTime::AribToSystemTime(this, pHexData);	
}

void CTsTime::SetTime(const WORD wYear, const WORD wMonth, const WORD wDay, const WORD wHour, const WORD wMinute, const WORD wSecond)
{
	// 各要素を設定
	this->wYear = wYear;
	this->wMonth = wMonth;
	this->wDay = wDay;
	this->wHour = wHour;
	this->wMinute = wMinute;
	this->wSecond = wSecond;

	// 曜日を求める
	FILETIME FileTime;
	::SystemTimeToFileTime(this, &FileTime);
	::FileTimeToSystemTime(&FileTime, this);
}

void  CTsTime::ClearTime(void)
{
	// 時間をクリアする
	::ZeroMemory(this, sizeof(CTsTime));
}

const bool CTsTime::IsEmpty(void) const
{
	// 時間が有効かどうか返す(とりあえず年だけを調べる)
	return (!wYear)? true : false;
}

CTsTime::operator const ULONGLONG () const
{
	// FILETIME形式へのキャスト演算子
	ULONGLONG llFileTime;
	::SystemTimeToFileTime(this, (FILETIME *)&llFileTime);
	
	return llFileTime;
}

const CTsTime & CTsTime::operator = (const SYSTEMTIME &SystemTime)
{
	// SYSTEM構造体から代入
	SetTime(SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond);
	
	return *this;
}

const CTsTime & CTsTime::operator = (const ULONGLONG llFileTime)
{
	// FILETIME形式から代入
	::FileTimeToSystemTime((FILETIME *)&llFileTime, this);
	
	return *this;
}

const CTsTime CTsTime::operator + (const DWORD dwSecond) const
{
	// 指定秒数を加算する
	return CTsTime((ULONGLONG)*this + (ULONGLONG)dwSecond * 10000000ULL);
}

const CTsTime CTsTime::operator - (const DWORD dwSecond) const
{
	// 指定秒数を減算する
	return CTsTime((ULONGLONG)*this - (ULONGLONG)dwSecond * 10000000ULL);
}

const CTsTime & CTsTime::operator += (const DWORD dwSecond)
{
	// 指定秒数を加算する
	return (*this = (*this + dwSecond));
}

const CTsTime & CTsTime::operator -= (const DWORD dwSecond)
{
	// 指定秒数を減算する
	return (*this = (*this - dwSecond));
}

const bool CTsTime::operator == (const CTsTime &Operand) const
{
	// 比較演算子
	return ((const ULONGLONG)*this == (const ULONGLONG)Operand)? true : false;
}

const bool CTsTime::operator != (const CTsTime &Operand) const
{
	// 比較演算子
	return ((const ULONGLONG)*this != (const ULONGLONG)Operand)? true : false;
}

const bool CTsTime::operator < (const CTsTime &Operand) const
{
	// 比較演算子
	return ((const ULONGLONG)*this < (const ULONGLONG)Operand)? true : false;
}

const bool CTsTime::operator > (const CTsTime &Operand) const
{
	// 比較演算子
	return ((const ULONGLONG)*this > (const ULONGLONG)Operand)? true : false;
}

const bool CTsTime::operator <= (const CTsTime &Operand) const
{
	// 比較演算子
	return ((const ULONGLONG)*this <= (const ULONGLONG)Operand)? true : false;
}

const bool CTsTime::operator >= (const CTsTime &Operand) const
{
	// 比較演算子
	return ((const ULONGLONG)*this >= (const ULONGLONG)Operand)? true : false;
}
