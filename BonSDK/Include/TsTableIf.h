// TsTableIf.h: TSテーブルインタフェース定義
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// [0x00] PATテーブルインタフェース
/////////////////////////////////////////////////////////////////////////////

class IPatTable :	public IBonObject
{
public:
	// PATテーブルハンドラインタフェース
	class IHandler :	public IBonObject
	{
	public:
		virtual void OnPatTable(const IPatTable *pPatTable) = 0;
	};

	enum {TABLE_ID = 0x00U};

	virtual const WORD GetTsID(void) const = 0;

	virtual const WORD GetNitPID(const DWORD dwIndex = 0UL) const = 0;
	virtual const DWORD GetNitNum(void) const = 0;

	virtual const WORD GetPmtPID(const DWORD dwIndex = 0UL) const = 0;
	virtual const WORD GetProgramID(const DWORD dwIndex = 0UL) const = 0;
	virtual const DWORD GetProgramNum(void) const = 0;
};


/////////////////////////////////////////////////////////////////////////////
// [0x01] CATテーブルインタフェース
/////////////////////////////////////////////////////////////////////////////

class ICatTable :	public IBonObject
{
public:
	// CATテーブルハンドラインタフェース
	class IHandler :	public IBonObject
	{
	public:
		virtual void OnCatTable(const ICatTable *pCatTable) = 0;
	};

	enum {TABLE_ID = 0x01U};

	virtual const IDescBlock * GetCatDesc(void) const = 0;
};


/////////////////////////////////////////////////////////////////////////////
// [0x02] PMTテーブルインタフェース
/////////////////////////////////////////////////////////////////////////////

class IPmtTable :	public IBonObject
{
public:
	// PMTテーブルハンドラインタフェース
	class IHandler :	public IBonObject
	{
	public:
		virtual void OnPmtTable(const IPmtTable *pPmtTable) = 0;
	};

	enum {TABLE_ID = 0x02U};

	virtual const WORD GetProgramID(void) const = 0;
	virtual const WORD GetPcrPID(void) const = 0;
	virtual const IDescBlock * GetPmtDesc(void) const = 0;

	virtual const DWORD GetEsNum(void) const = 0;
	virtual const BYTE GetStreamTypeID(const DWORD dwIndex) const = 0;
	virtual const WORD GetEsPID(const DWORD dwIndex) const = 0;
	virtual const IDescBlock * GetEsDesc(const DWORD dwIndex) const = 0;
};


/////////////////////////////////////////////////////////////////////////////
// [0x40] NITテーブルインタフェース
/////////////////////////////////////////////////////////////////////////////

class INitTable :	public IBonObject
{
public:
	// NITテーブルハンドラインタフェース
	class IHandler :	public IBonObject
	{
	public:
		virtual void OnNitTable(const INitTable *pNitTable) = 0;
	};

	enum {TABLE_ID = 0x40U};

	virtual const WORD GetNetworkID(void) const = 0;
	virtual const IDescBlock * GetNetworkDesc(void) const = 0;

	virtual const DWORD GetTsNum(void) const = 0;
	virtual const BYTE GetTsID(const DWORD dwIndex) const = 0;
	virtual const BYTE GetOrgNetworkID(const DWORD dwIndex) const = 0;
	virtual const IDescBlock * GetTsDesc(const DWORD dwIndex) const = 0;
};


/////////////////////////////////////////////////////////////////////////////
// [0x42] SDTテーブルインタフェース
/////////////////////////////////////////////////////////////////////////////

class ISdtTable :	public IBonObject
{
public:
	// SDTテーブルハンドラインタフェース
	class IHandler :	public IBonObject
	{
	public:
		virtual void OnSdtTable(const ISdtTable *pSdtTable) = 0;
	};

	enum {TABLE_ID = 0x42U};

	enum	// GetRunningStatus() リターンコード
	{
		RST_UNDEFINED	= 0U,	// 未定義
		RST_NOTRUNNING	= 1U,	// 非実行中
		RST_STARTSOON	= 2U,	// 数秒以内に開始
		RST_STOP		= 3U,	// 停止中
		RST_START		= 4U	// 実行中
	};

	virtual const WORD GetTsID(void) const = 0;
	virtual const WORD GetOrgNetworkID(void) const = 0;

	virtual const DWORD GetServiceNum(void) const = 0;
	virtual const WORD GetServiceID(const DWORD dwIndex = 0UL) const = 0;
	virtual const bool IsEitSchedule(const WORD wServiceID) const = 0;
	virtual const bool IsEitPresentFollowing(const WORD wServiceID) const = 0;
	virtual const BYTE GetRunningStatus(const WORD wServiceID) const = 0;
	virtual const bool IsScrambling(const WORD wServiceID) const = 0;
	virtual const IDescBlock * GetServiceDesc(const WORD wServiceID) const = 0;
};


/////////////////////////////////////////////////////////////////////////////
// [0x4E-0x6F] EITテーブルインタフェース
/////////////////////////////////////////////////////////////////////////////

class IEitTable :	public IBonObject
{
public:
	// EITテーブルハンドラインタフェース
	class IHandler :	public IBonObject
	{
	public:
		virtual void OnEitTable(const IEitTable *pEitTable, const IEitItem *pEitItem) = 0;
	};

	enum
	{
		TABLE_ID_CURORNEXT_SELF		= 0x4EU,
		TABLE_ID_CURORNEXT_OTHER	= 0x4FU,
		TABLE_ID_SCHEDULED_SELF_50  = 0x50U,
		TABLE_ID_SCHEDULED_SELF_51  = 0x51U,
		TABLE_ID_SCHEDULED_SELF_52  = 0x52U,
		TABLE_ID_SCHEDULED_SELF_53  = 0x53U,
		TABLE_ID_SCHEDULED_SELF_54  = 0x54U,
		TABLE_ID_SCHEDULED_SELF_55  = 0x55U,
		TABLE_ID_SCHEDULED_SELF_56  = 0x56U,
		TABLE_ID_SCHEDULED_SELF_57  = 0x57U,
		TABLE_ID_SCHEDULED_SELF_58  = 0x58U,
		TABLE_ID_SCHEDULED_SELF_59  = 0x59U,
		TABLE_ID_SCHEDULED_SELF_5A  = 0x5AU,
		TABLE_ID_SCHEDULED_SELF_5B  = 0x5BU,
		TABLE_ID_SCHEDULED_SELF_5C  = 0x5CU,
		TABLE_ID_SCHEDULED_SELF_5D  = 0x5DU,
		TABLE_ID_SCHEDULED_SELF_5E  = 0x5EU,
		TABLE_ID_SCHEDULED_SELF_5F  = 0x5FU,
		TABLE_ID_SCHEDULED_OTHER_60 = 0x60U,
		TABLE_ID_SCHEDULED_OTHER_61 = 0x61U,
		TABLE_ID_SCHEDULED_OTHER_62 = 0x62U,
		TABLE_ID_SCHEDULED_OTHER_63 = 0x63U,
		TABLE_ID_SCHEDULED_OTHER_64 = 0x64U,
		TABLE_ID_SCHEDULED_OTHER_65 = 0x65U,
		TABLE_ID_SCHEDULED_OTHER_66 = 0x66U,
		TABLE_ID_SCHEDULED_OTHER_67 = 0x67U,
		TABLE_ID_SCHEDULED_OTHER_68 = 0x68U,
		TABLE_ID_SCHEDULED_OTHER_69 = 0x69U,
		TABLE_ID_SCHEDULED_OTHER_6A = 0x6AU,
		TABLE_ID_SCHEDULED_OTHER_6B = 0x6BU,
		TABLE_ID_SCHEDULED_OTHER_6C = 0x6CU,
		TABLE_ID_SCHEDULED_OTHER_6D = 0x6DU,
		TABLE_ID_SCHEDULED_OTHER_6E = 0x65U,
		TABLE_ID_SCHEDULED_OTHER_6F = 0x6FU,

		TABLE_ID_START				= TABLE_ID_CURORNEXT_SELF,
		TABLE_ID_END				= TABLE_ID_SCHEDULED_OTHER_6F
	};

	virtual const WORD GetServiceID(void) const = 0;
	virtual const WORD GetTsID(void) const = 0;
	virtual const WORD GetOrgNetworkID(void) const = 0;

	virtual const BYTE GetTableID(void) const = 0;
	virtual const BYTE GetLastTableID(void) const = 0;

	virtual const BYTE GetSectionNumber(void) const = 0;
	virtual const BYTE GetLastSectionNumber(void) const = 0;
	virtual const BYTE GetSegmentLastSectionNumber(void) const = 0;
};


/////////////////////////////////////////////////////////////////////////////
// [0x73] TOTテーブルインタフェース
/////////////////////////////////////////////////////////////////////////////

class ITotTable :	public IBonObject
{
public:
	// TOTテーブルハンドラインタフェース
	class IHandler :	public IBonObject
	{
	public:
		virtual void OnTotTable(const ITotTable *pTotTable) = 0;
	};

	enum {TABLE_ID = 0x73U};

	virtual const SYSTEMTIME & GetDateTime(void) const = 0;
	virtual const IDescBlock * GetTotDesc(void) const = 0;
};
