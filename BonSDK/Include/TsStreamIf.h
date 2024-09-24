// TsStreamIf.h: TSストリームインタフェース定義
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


//#include "TsDataEncode.h"


/////////////////////////////////////////////////////////////////////////////
// 定数定義
/////////////////////////////////////////////////////////////////////////////

#define	TS_PACKET_SIZE	188UL		// TSパケットサイズ

#define TS_INVALID_PID	0xFFFFU		// 無効PID


/////////////////////////////////////////////////////////////////////////////
// TSパケットインタフェース
/////////////////////////////////////////////////////////////////////////////

class ITsPacket :	public IBonObject
{
public:
	enum	// ParsePacket() エラーコード
	{
		EC_VALID		= 0x00000000UL,		// 正常パケット
		EC_FORMAT		= 0x00000001UL,		// フォーマットエラー
		EC_TRANSPORT	= 0x00000002UL,		// トランスポートエラー(ビットエラー)
		EC_CONTINUITY	= 0x00000003UL		// 連続性カウンタエラー(ドロップ)
	};

	virtual const DWORD ParsePacket(BYTE *pContinuityCounter = NULL) = 0;

	virtual BYTE * GetRawData(void) const = 0;

	virtual const WORD GetPID(void) const = 0;
	virtual const bool IsScrambled(void) const = 0;
	virtual const bool HaveAdaptationField(void) const = 0;

	virtual BYTE * GetPayloadData(void) const = 0;
	virtual const BYTE GetPayloadSize(void) const = 0;

	virtual const bool CopyPacket(const ITsPacket *pSrc) = 0;
	virtual const bool ComparePacket(const ITsPacket *pSrc) const = 0;

	struct TAG_TSPACKETHEADER
	{
		BYTE bySyncByte;					// Sync Byte
		bool bTransportErrorIndicator;		// Transport Error Indicator
		bool bPayloadUnitStartIndicator;	// Payload Unit Start Indicator
		bool bTransportPriority;			// Transport Priority
		WORD wPID;							// PID
		BYTE byTransportScramblingCtrl;		// Transport Scrambling Control
		BYTE byAdaptationFieldCtrl;			// Adaptation Field Control
		BYTE byContinuityCounter;			// Continuity Counter
	} m_Header;

	struct TAG_ADAPTFIELDHEADER
	{
		BYTE byAdaptationFieldLength;		// Adaptation Field Length
		bool bDiscontinuityIndicator;		// Discontinuity Indicator
		bool bRamdomAccessIndicator;		// Random Access Indicator
		bool bEsPriorityIndicator;			// Elementary Stream Priority Indicator
		bool bPcrFlag;						// PCR Flag
		bool bOpcrFlag;						// OPCR Flag
		bool bSplicingPointFlag;			// Splicing Point Flag
		bool bTransportPrivateDataFlag;		// Transport Private Data Flag
		bool bAdaptationFieldExtFlag;		// Adaptation Field Extension Flag
		const BYTE *pOptionData;			// オプションフィールドデータ
		BYTE byOptionSize;					// オプションフィールド長
	} m_AdaptationField;
};


/////////////////////////////////////////////////////////////////////////////
// PSIセクションインタフェース
/////////////////////////////////////////////////////////////////////////////

class IPsiSection :	public IBonObject
{
public:
	enum	// CompareSection() 結果フラグ
	{
		CR_IDENTICAL	= 0x00000000UL,		// 一致
		CR_TABLE_ID		= 0x00000001UL,		// テーブルID
		CR_EXTENSION	= 0x00000002UL,		// テーブルID拡張
		CR_SECT_NO		= 0x00000004UL,		// セクション番号
		CR_LAST_SECT_NO	= 0x00000008UL,		// ラストセクション番号
		CR_PAYLOAD		= 0x00000010UL,		// セクションデータ
		CR_OTHER_ALL	= 0x00000020UL		// その他全て(CRCが異なる)
	};

	virtual const bool ParseHeader(const bool bExtSection = true) = 0;
	virtual void Reset(void) = 0;

	virtual BYTE * GetPayloadData(void) const = 0;
	virtual const WORD GetPayloadSize(void) const = 0;

	virtual const BYTE GetTableID(void) const = 0;
	virtual const bool IsExtendedSection(void) const = 0;
	virtual const bool IsPrivate(void) const = 0;
	virtual const WORD GetSectionLength(void) const = 0;
	virtual const WORD GetTableIdExtension(void) const = 0;
	virtual const BYTE GetVersionNo(void) const = 0;
	virtual const bool IsCurrentNext(void) const = 0;
	virtual const BYTE GetSectionNumber(void) const = 0;
	virtual const BYTE GetLastSectionNumber(void) const = 0;
	virtual const DWORD GetSectionCrc(void) const = 0;

	virtual const bool CopySection(const IPsiSection *pSrc) = 0;
	virtual const DWORD CompareSection(const IPsiSection *pSrc) const = 0;
};


/////////////////////////////////////////////////////////////////////////////
// PSIセクションパーサインタフェース
/////////////////////////////////////////////////////////////////////////////

class IPsiSectionParser :	public IBonObject
{
public:
	// PSIセクションハンドラインタフェース
	class IHandler :	public IBonObject
	{
	public:
		virtual void OnPsiSection(const IPsiSectionParser *pPsiSectionParser, const IPsiSection *pPsiSection) = 0;
	};

	virtual void SetExtSection(const bool bExtSection = true) = 0;
	virtual const bool StorePacket(const ITsPacket *pTsPacket) = 0;
	virtual void Reset(void) = 0;
	virtual const DWORD GetCrcErrorNum(void) const = 0;
};


/////////////////////////////////////////////////////////////////////////////
// デスクリプタ共通インタフェース
/////////////////////////////////////////////////////////////////////////////

class IDescBase :	public IBonObject
{
public:
	virtual const DWORD ParseDesc(const BYTE *pData, const DWORD dwDataSize) = 0;

	virtual const BYTE GetTag(void) const = 0;
	virtual const BYTE GetLength(void) const = 0;
	virtual const BYTE * GetData(void) const = 0;
};


/////////////////////////////////////////////////////////////////////////////
// デスクリプタ集合インタフェース
/////////////////////////////////////////////////////////////////////////////

class IDescBlock :	public IBonObject
{
public:
	virtual const DWORD ParseDescs(void) = 0;
	virtual void Reset(void) = 0;

	virtual const DWORD GetDescNum(void) const = 0;
	virtual const IDescBase * GetDescByTag(const BYTE byTag, const DWORD dwIndex = 0UL) const = 0;
	virtual const IDescBase * GetDescByIndex(const DWORD dwIndex = 0UL) const = 0;

	virtual const bool CopyDescBlock(const IDescBlock *pSrc) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// PIDマップターゲットインタフェース
/////////////////////////////////////////////////////////////////////////////

class ITsPidMapper;

class ITsPidMapTarget :	public IBonObject
{
public:
	virtual const bool StorePacket(const ITsPacket *pTsPacket) = 0;
	virtual void OnPidReset(ITsPidMapper *pTsPidMapper, const WORD wPID) = 0;
	virtual void OnPidMapped(ITsPidMapper *pTsPidMapper, const WORD wPID) = 0;
	virtual void OnPidUnmapped(ITsPidMapper *pTsPidMapper, const WORD wPID) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// PIDマッパインタフェース
/////////////////////////////////////////////////////////////////////////////

class ITsPidMapper :	public IBonObject
{
public:
	virtual const bool StorePacket(const ITsPacket *pTsPacket) = 0;

	virtual void ResetPid(const WORD wPID = TS_INVALID_PID) = 0;
	virtual const bool MapPid(const WORD wPID, ITsPidMapTarget *pTsPidMapTarget) = 0;
	virtual const bool UnmapPid(const WORD wPID) = 0;
	virtual void ClearPid(void) = 0;

	virtual ITsPidMapTarget * GetMapTarget(const WORD wPID) const = 0;
	virtual const WORD GetMapNum(void) const = 0;
};
