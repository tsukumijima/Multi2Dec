// TsSection.cpp: PSIセクション基底クラス
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "TsSection.h"


/////////////////////////////////////////////////////////////////////////////
// PSIセクション基底クラス
/////////////////////////////////////////////////////////////////////////////

const bool CPsiSection::ParseHeader(const bool bIsExtended)
{
	const DWORD dwHeaderSize = (bIsExtended)? 8UL : 3UL;

	// ヘッダサイズチェック
	if(m_dwDataSize < dwHeaderSize)return false;

	// 標準形式ヘッダ解析
	m_Header.byTableID					= m_pData[0];									// +0
	m_Header.bSectionSyntaxIndicator	= (m_pData[1] & 0x80U)? true : false;			// +1 bit7
	m_Header.bPrivateIndicator			= (m_pData[1] & 0x40U)? true : false;			// +1 bit6
	m_Header.wSectionLength = ((WORD)(m_pData[1] & 0x0FU) << 8) | (WORD)m_pData[2];		// +1 bit5-0, +2

	if(m_Header.bSectionSyntaxIndicator && bIsExtended){
		// 拡張形式のヘッダ解析
		m_Header.wTableIdExtension		= (WORD)m_pData[3] << 8 | (WORD)m_pData[4];		// +3, +4
		m_Header.byVersionNo			= (m_pData[5] & 0x3EU) >> 1;					// +5 bit5-1
		m_Header.bCurrentNextIndicator	= (m_pData[5] & 0x01U)? true : false;			// +5 bit0
		m_Header.bySectionNumber		= m_pData[6];									// +6
		m_Header.byLastSectionNumber	= m_pData[7];									// +7
		}

	// ヘッダのフォーマット適合性をチェックする
	if((m_pData[1] & 0x30U) != 0x30U)return false;										// 固定ビット異常
	else if(m_Header.wSectionLength > 4093U)return false;								// セクション長異常
	else if(m_Header.bSectionSyntaxIndicator){
		// 拡張形式のエラーチェック
		if(!bIsExtended)return false;													// 目的のヘッダではない
		else if((m_pData[5] & 0xC0U) != 0xC0U)return false;								// 固定ビット異常
		else if(m_Header.bySectionNumber > m_Header.byLastSectionNumber)return false;	// セクション番号異常
		else if(m_Header.wSectionLength < 9U)return false;								// セクション長異常
		}
	else{
		// 標準形式のエラーチェック	
		if(bIsExtended)return false;													// 目的のヘッダではない
		else if(m_Header.wSectionLength < 4U)return false;								// セクション長異常
		}

	return true;
}

void CPsiSection::Reset(void)
{
	// データをリセットする
	ClearSize();	
	::ZeroMemory(&m_Header, sizeof(m_Header));
}

BYTE * CPsiSection::GetPayloadData(void) const
{
	// ペイロードポインタを返す
	const DWORD dwHeaderSize = (m_Header.bSectionSyntaxIndicator)? 8UL : 3UL;

	return (m_dwDataSize > dwHeaderSize)? &m_pData[dwHeaderSize] : NULL;
}

const WORD CPsiSection::GetPayloadSize(void) const
{
	// 実際に保持しているペイロードサイズを返す(※ヘッダのセクション長ではない)
	const DWORD dwHeaderSize = (m_Header.bSectionSyntaxIndicator)? 8UL : 3UL;

	if(m_dwDataSize <= dwHeaderSize){
		return 0U;
		}
	else if(m_Header.bSectionSyntaxIndicator){
		// 拡張セクション
		return (m_dwDataSize >= (m_Header.wSectionLength + 3UL))? (m_Header.wSectionLength - 9U) : (WORD)(m_dwDataSize - dwHeaderSize);
		}
	else{
		// 標準セクション
		return (m_dwDataSize >= (m_Header.wSectionLength + 3UL))? m_Header.wSectionLength : (WORD)(m_dwDataSize - dwHeaderSize);
		}
}

const BYTE CPsiSection::GetTableID(void) const
{
	// テーブルIDを返す
	return m_Header.byTableID;
}

const bool CPsiSection::IsExtendedSection(void) const
{
	// セクションシンタックスインジケータを返す
	return m_Header.bSectionSyntaxIndicator;
}

const bool CPsiSection::IsPrivate(void) const
{
	// プライベートインジケータを返す
	return m_Header.bPrivateIndicator;
}

const WORD CPsiSection::GetSectionLength(void) const
{
	// セクション長を返す
	return m_Header.wSectionLength;
}

const WORD CPsiSection::GetTableIdExtension(void) const
{
	// テーブルID拡張を返す
	return m_Header.wTableIdExtension;
}

const BYTE CPsiSection::GetVersionNo(void) const
{
	// バージョン番号を返す
	return m_Header.byVersionNo;
}

const bool CPsiSection::IsCurrentNext(void) const
{
	// カレントネクストインジケータを返す
	return m_Header.bCurrentNextIndicator;
}

const BYTE CPsiSection::GetSectionNumber(void) const
{
	// セクション番号を返す
	return m_Header.bySectionNumber;
}

const BYTE CPsiSection::GetLastSectionNumber(void) const
{
	// ラストセクション番号を返す
	return m_Header.byLastSectionNumber;
}

const DWORD CPsiSection::GetSectionCrc(void) const
{
	// CRCを返す
	if(!m_Header.bSectionSyntaxIndicator)return 0UL;
	else if(m_dwDataSize <= 12UL)return 0U;
	else if(m_dwDataSize < (m_Header.wSectionLength + 3UL))return 0U;
	else{
		const DWORD dwCrcPos = m_Header.wSectionLength - 1UL;
		return ((DWORD)m_pData[dwCrcPos] << 24) | ((DWORD)m_pData[dwCrcPos + 1UL] << 16) | ((DWORD)m_pData[dwCrcPos + 2UL] << 8) | (DWORD)m_pData[dwCrcPos + 3UL];
		}
}

const bool CPsiSection::CopySection(const IPsiSection *pSrc)
{
	// IMediaDataインタフェースを取得
	const IMediaData * const pMediaData = dynamic_cast<const IMediaData *>(pSrc);
	if(!pMediaData)return false;

	// セクションデータコピー
	if(CMediaData::CopyData(pMediaData) != GetSize())return false;
	
	// ヘッダコピー
	m_Header.byTableID					= pSrc->GetTableID();
	m_Header.bSectionSyntaxIndicator	= pSrc->IsExtendedSection();
	m_Header.bPrivateIndicator			= pSrc->IsPrivate();
	m_Header.wSectionLength				= pSrc->GetSectionLength();
	m_Header.wTableIdExtension			= pSrc->GetTableIdExtension();
	m_Header.byVersionNo				= pSrc->GetVersionNo();
	m_Header.bCurrentNextIndicator		= pSrc->IsCurrentNext();
	m_Header.bySectionNumber			= pSrc->GetSectionNumber();
	m_Header.byLastSectionNumber		= pSrc->GetLastSectionNumber();

	return true;
}

const DWORD CPsiSection::CompareSection(const IPsiSection *pSrc) const
{
	if(!pSrc)return CR_IDENTICAL;

	DWORD dwReturn = CR_IDENTICAL;

	// テーブルIDをチェック
	if(GetTableID() != pSrc->GetTableID())dwReturn |= CR_TABLE_ID;

	// テーブルID拡張をチェック
	if(GetTableIdExtension() != pSrc->GetTableIdExtension())dwReturn |= CR_EXTENSION;

	// セクション番号
	if(GetSectionNumber() != pSrc->GetSectionNumber())dwReturn |= CR_EXTENSION;

	// ラストセクション番号
	if(GetLastSectionNumber() != pSrc->GetLastSectionNumber())dwReturn |= CR_EXTENSION;

	// セクションの内容を比較する
	if(GetPayloadSize() != pSrc->GetPayloadSize())dwReturn |= CR_PAYLOAD;

	const BYTE *pSrcData = GetPayloadData();
	const BYTE *pDstData = pSrc->GetPayloadData();
	
	// ペイロードバイナリ比較
	for(DWORD dwPos = 0 ; dwPos < GetPayloadSize() ; dwPos++){
		if(pSrcData[dwPos] != pDstData[dwPos]){
			dwReturn |= CR_PAYLOAD;
			break;
			}
		}
		
	// CRC比較
	if(GetSectionCrc() != pSrc->GetSectionCrc())dwReturn |= CR_OTHER_ALL;
	
	
	// 一致する
	return dwReturn;
}

const DWORD CPsiSection::CopyData(const IMediaData *pSrc)
{
	// IPsiSectionインタフェース取得
	const IPsiSection * const pPsiSection = dynamic_cast<const IPsiSection *>(pSrc);

	if(pPsiSection){
		// ITsPacketとしてコピー
		return (CopySection(pPsiSection))? GetSize() : 0UL;
		}
	else{
		// IMediaDataとしてコピー
		return CMediaData::CopyData(pSrc);
		}
}

CPsiSection::CPsiSection(IBonObject *pOwner)
	: CMediaData(pOwner)
{
	// データ初期化
	Reset();
}

CPsiSection::~CPsiSection(void)
{

}


/////////////////////////////////////////////////////////////////////////////
// IPsiSectionParser基底クラス
/////////////////////////////////////////////////////////////////////////////

void CPsiSectionParser::SetExtSection(const bool bExtSection)
{
	// 標準セクションと拡張セクションを設定する
	if(m_bTargetExt != bExtSection){
		m_bTargetExt = bExtSection;
		Reset();
		}
}

const bool CPsiSectionParser::StorePacket(const ITsPacket *pTsPacket)
{
	const BYTE * const pData = pTsPacket->GetPayloadData();
	const BYTE bySize = pTsPacket->GetPayloadSize();

	if(!bySize || !pData)return false;

	const BYTE byUnitStartPos = (pTsPacket->m_Header.bPayloadUnitStartIndicator)? (pData[0] + 1U) : 0U;

	if(byUnitStartPos){
		// [ヘッダ断片 | ペイロード断片] + [スタッフィングバイト] + ヘッダ先頭 + [ヘッダ断片] + [ペイロード断片] + [スタッフィングバイト]
		BYTE byPos = 1U;
		
		if(byUnitStartPos > 1U){
			// ユニット開始位置が先頭ではない場合(断片がある場合)
			byPos += StoreHeader(&pData[byPos], bySize - byPos);
			byPos += StorePayload(&pData[byPos], bySize - byPos);
			}
		
		// ユニット開始位置から新規セクションのストアを開始する
		m_bIsStoring = false;
		m_PsiSection.ClearSize();

		byPos = byUnitStartPos;
		byPos += StoreHeader(&pData[byPos], bySize - byPos);
		byPos += StorePayload(&pData[byPos], bySize - byPos);
		}
	else{
		// [ヘッダ断片] + ペイロード + [スタッフィングバイト]
		BYTE byPos = 0U;
		byPos += StoreHeader(&pData[byPos], bySize - byPos);
		byPos += StorePayload(&pData[byPos], bySize - byPos);
		}

	return true;
}

void CPsiSectionParser::Reset(void)
{
	// 状態をリセットする
	m_bIsStoring = false;
	m_wStoreSize = 0U;
	m_dwCrcErrorNum = 0UL;
	m_PsiSection.Reset();
}

const DWORD CPsiSectionParser::GetCrcErrorNum(void) const
{
	// CRCエラー数を返す
	return m_dwCrcErrorNum;
}

CPsiSectionParser::CPsiSectionParser(IBonObject *pOwner)
	: m_pHandler(dynamic_cast<IHandler *>(pOwner))
	, m_PsiSection(NULL)
	, m_bTargetExt(true)
	, m_bIsStoring(false)
	, m_dwCrcErrorNum(0UL)
{
	if(!m_pHandler)::DebugBreak();
}

CPsiSectionParser::~CPsiSectionParser(void)
{

}

const BYTE CPsiSectionParser::StoreHeader(const BYTE *pPayload, const BYTE byRemain)
{
	// ヘッダを解析してセクションのストアを開始する
	if(m_bIsStoring)return 0U;

	const BYTE byHeaderSize = (m_bTargetExt)? 8U : 3U;
	const BYTE byHeaderRemain = byHeaderSize - (BYTE)m_PsiSection.GetSize();

	if(byRemain >= byHeaderRemain){
		// ヘッダストア完了、ヘッダを解析してペイロードのストアを開始する
		m_PsiSection.AddData(pPayload, byHeaderRemain);
		if(m_PsiSection.ParseHeader(m_bTargetExt)){
			// ヘッダフォーマットOK、ヘッダのみのCRCを計算する
			m_wStoreSize = m_PsiSection.GetSectionLength() + 3U;
			m_dwStoreCrc = CalcCrc(pPayload, byHeaderSize);
			m_bIsStoring = true;
			return byHeaderRemain;
			}
		else{
			// ヘッダエラー
			m_PsiSection.Reset();
			return byRemain;
			}
		}
	else{
		// ヘッダストア未完了、次のデータを待つ
		m_PsiSection.AddData(pPayload, byRemain);
		return byRemain;
		}
}

const BYTE CPsiSectionParser::StorePayload(const BYTE *pPayload, const BYTE byRemain)
{
	// セクションのストアを完了する
	if(!m_bIsStoring)return 0U;
	
	const WORD wStoreRemain = m_wStoreSize - (WORD)m_PsiSection.GetSize();

	if(wStoreRemain <= (WORD)byRemain){
		// ストア完了
		m_PsiSection.AddData(pPayload, wStoreRemain);
				
		if(!CalcCrc(pPayload, wStoreRemain, m_dwStoreCrc)){
			// CRC正常、ハンドラにセクションを渡す
			if(m_pHandler)m_pHandler->OnPsiSection(this, &m_PsiSection);
			}
		else{
			// CRC異常
			if(m_dwCrcErrorNum < 0xFFFFFFFFUL)m_dwCrcErrorNum++;
			}
		
		// 状態を初期化し、次のセクション受信に備える
		m_PsiSection.Reset();
		m_bIsStoring = false;

		return (BYTE)wStoreRemain;
		}
	else{
		// ストア未完了、次のペイロードを待つ
		m_PsiSection.AddData(pPayload, byRemain);
		m_dwStoreCrc = CalcCrc(pPayload, byRemain, m_dwStoreCrc);
		return byRemain;
		}
}

const DWORD CPsiSectionParser::CalcCrc(const BYTE *pData, const WORD wDataSize, DWORD dwCurCrc)
{
	// CRC32計算
	static const DWORD CrcTable[256] = {
		0x00000000UL, 0x04C11DB7UL, 0x09823B6EUL, 0x0D4326D9UL,	0x130476DCUL, 0x17C56B6BUL, 0x1A864DB2UL, 0x1E475005UL,	0x2608EDB8UL, 0x22C9F00FUL, 0x2F8AD6D6UL, 0x2B4BCB61UL,	0x350C9B64UL, 0x31CD86D3UL, 0x3C8EA00AUL, 0x384FBDBDUL,
		0x4C11DB70UL, 0x48D0C6C7UL, 0x4593E01EUL, 0x4152FDA9UL,	0x5F15ADACUL, 0x5BD4B01BUL, 0x569796C2UL, 0x52568B75UL,	0x6A1936C8UL, 0x6ED82B7FUL, 0x639B0DA6UL, 0x675A1011UL,	0x791D4014UL, 0x7DDC5DA3UL, 0x709F7B7AUL, 0x745E66CDUL,
		0x9823B6E0UL, 0x9CE2AB57UL, 0x91A18D8EUL, 0x95609039UL,	0x8B27C03CUL, 0x8FE6DD8BUL, 0x82A5FB52UL, 0x8664E6E5UL,	0xBE2B5B58UL, 0xBAEA46EFUL, 0xB7A96036UL, 0xB3687D81UL,	0xAD2F2D84UL, 0xA9EE3033UL, 0xA4AD16EAUL, 0xA06C0B5DUL,
		0xD4326D90UL, 0xD0F37027UL, 0xDDB056FEUL, 0xD9714B49UL,	0xC7361B4CUL, 0xC3F706FBUL, 0xCEB42022UL, 0xCA753D95UL,	0xF23A8028UL, 0xF6FB9D9FUL, 0xFBB8BB46UL, 0xFF79A6F1UL,	0xE13EF6F4UL, 0xE5FFEB43UL, 0xE8BCCD9AUL, 0xEC7DD02DUL,
		0x34867077UL, 0x30476DC0UL, 0x3D044B19UL, 0x39C556AEUL,	0x278206ABUL, 0x23431B1CUL, 0x2E003DC5UL, 0x2AC12072UL,	0x128E9DCFUL, 0x164F8078UL, 0x1B0CA6A1UL, 0x1FCDBB16UL,	0x018AEB13UL, 0x054BF6A4UL, 0x0808D07DUL, 0x0CC9CDCAUL,
		0x7897AB07UL, 0x7C56B6B0UL, 0x71159069UL, 0x75D48DDEUL,	0x6B93DDDBUL, 0x6F52C06CUL, 0x6211E6B5UL, 0x66D0FB02UL,	0x5E9F46BFUL, 0x5A5E5B08UL, 0x571D7DD1UL, 0x53DC6066UL,	0x4D9B3063UL, 0x495A2DD4UL, 0x44190B0DUL, 0x40D816BAUL,
		0xACA5C697UL, 0xA864DB20UL, 0xA527FDF9UL, 0xA1E6E04EUL,	0xBFA1B04BUL, 0xBB60ADFCUL, 0xB6238B25UL, 0xB2E29692UL,	0x8AAD2B2FUL, 0x8E6C3698UL, 0x832F1041UL, 0x87EE0DF6UL,	0x99A95DF3UL, 0x9D684044UL, 0x902B669DUL, 0x94EA7B2AUL,
		0xE0B41DE7UL, 0xE4750050UL, 0xE9362689UL, 0xEDF73B3EUL,	0xF3B06B3BUL, 0xF771768CUL, 0xFA325055UL, 0xFEF34DE2UL,	0xC6BCF05FUL, 0xC27DEDE8UL, 0xCF3ECB31UL, 0xCBFFD686UL,	0xD5B88683UL, 0xD1799B34UL, 0xDC3ABDEDUL, 0xD8FBA05AUL,
		0x690CE0EEUL, 0x6DCDFD59UL, 0x608EDB80UL, 0x644FC637UL,	0x7A089632UL, 0x7EC98B85UL, 0x738AAD5CUL, 0x774BB0EBUL,	0x4F040D56UL, 0x4BC510E1UL, 0x46863638UL, 0x42472B8FUL,	0x5C007B8AUL, 0x58C1663DUL, 0x558240E4UL, 0x51435D53UL,
		0x251D3B9EUL, 0x21DC2629UL, 0x2C9F00F0UL, 0x285E1D47UL,	0x36194D42UL, 0x32D850F5UL, 0x3F9B762CUL, 0x3B5A6B9BUL,	0x0315D626UL, 0x07D4CB91UL, 0x0A97ED48UL, 0x0E56F0FFUL,	0x1011A0FAUL, 0x14D0BD4DUL, 0x19939B94UL, 0x1D528623UL,
		0xF12F560EUL, 0xF5EE4BB9UL, 0xF8AD6D60UL, 0xFC6C70D7UL,	0xE22B20D2UL, 0xE6EA3D65UL, 0xEBA91BBCUL, 0xEF68060BUL,	0xD727BBB6UL, 0xD3E6A601UL, 0xDEA580D8UL, 0xDA649D6FUL,	0xC423CD6AUL, 0xC0E2D0DDUL, 0xCDA1F604UL, 0xC960EBB3UL,
		0xBD3E8D7EUL, 0xB9FF90C9UL, 0xB4BCB610UL, 0xB07DABA7UL,	0xAE3AFBA2UL, 0xAAFBE615UL, 0xA7B8C0CCUL, 0xA379DD7BUL,	0x9B3660C6UL, 0x9FF77D71UL, 0x92B45BA8UL, 0x9675461FUL,	0x8832161AUL, 0x8CF30BADUL, 0x81B02D74UL, 0x857130C3UL,
		0x5D8A9099UL, 0x594B8D2EUL, 0x5408ABF7UL, 0x50C9B640UL,	0x4E8EE645UL, 0x4A4FFBF2UL, 0x470CDD2BUL, 0x43CDC09CUL,	0x7B827D21UL, 0x7F436096UL, 0x7200464FUL, 0x76C15BF8UL,	0x68860BFDUL, 0x6C47164AUL, 0x61043093UL, 0x65C52D24UL,
		0x119B4BE9UL, 0x155A565EUL, 0x18197087UL, 0x1CD86D30UL,	0x029F3D35UL, 0x065E2082UL, 0x0B1D065BUL, 0x0FDC1BECUL,	0x3793A651UL, 0x3352BBE6UL, 0x3E119D3FUL, 0x3AD08088UL,	0x2497D08DUL, 0x2056CD3AUL, 0x2D15EBE3UL, 0x29D4F654UL,
		0xC5A92679UL, 0xC1683BCEUL, 0xCC2B1D17UL, 0xC8EA00A0UL,	0xD6AD50A5UL, 0xD26C4D12UL, 0xDF2F6BCBUL, 0xDBEE767CUL,	0xE3A1CBC1UL, 0xE760D676UL, 0xEA23F0AFUL, 0xEEE2ED18UL,	0xF0A5BD1DUL, 0xF464A0AAUL, 0xF9278673UL, 0xFDE69BC4UL,
		0x89B8FD09UL, 0x8D79E0BEUL, 0x803AC667UL, 0x84FBDBD0UL,	0x9ABC8BD5UL, 0x9E7D9662UL, 0x933EB0BBUL, 0x97FFAD0CUL,	0xAFB010B1UL, 0xAB710D06UL, 0xA6322BDFUL, 0xA2F33668UL,	0xBCB4666DUL, 0xB8757BDAUL, 0xB5365D03UL, 0xB1F740B4UL
		};		

	for(register WORD wPos = 0U ; wPos < wDataSize ; wPos++){
		dwCurCrc = (dwCurCrc << 8) ^ CrcTable[ (dwCurCrc >> 24) ^ pData[wPos] ];
		}

	return dwCurCrc;
}
