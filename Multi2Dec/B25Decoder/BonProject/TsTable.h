// TsTable.h: TSテーブル基底クラス
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


#include <Vector>
#include "TsSection.h"
#include "TsTableBase.h"
#include "TsDescBase.h"
#include "TsDescriptor.h"
#include "TsEpgCore.h"


/////////////////////////////////////////////////////////////////////////////
// [0x00] PATテーブル基底クラス
/////////////////////////////////////////////////////////////////////////////

class CPatTable :	public CPsiTableBase,
					public IPatTable
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CPatTable)

// CPsiTableBase
	virtual void Reset(void);

// IPatTable
	virtual const WORD GetTsID(void) const;
	virtual const WORD GetNitPID(const DWORD dwIndex = 0UL) const;
	virtual const DWORD GetNitNum(void) const;
	virtual const WORD GetPmtPID(const DWORD dwIndex = 0UL) const;
	virtual const WORD GetProgramID(const DWORD dwIndex = 0UL) const;
	virtual const DWORD GetProgramNum(void) const;

// CPatTable
	CPatTable(IBonObject *pOwner);
	virtual ~CPatTable(void);

protected:
// CPsiTableBase
	virtual const bool OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection);

	struct TAG_PATITEM
	{
		WORD wProgramID;	// 放送番組番号ID
		WORD wPID;			// NIT or PMTのPID
	};

	WORD m_wTsID;			// TS ID

	std::vector<TAG_PATITEM> m_NitPIDArray;
	std::vector<TAG_PATITEM> m_PmtPIDArray;

	IPatTable::IHandler * const m_pHandler;
};


/////////////////////////////////////////////////////////////////////////////
// [0x01] CATテーブル基底クラス
/////////////////////////////////////////////////////////////////////////////

class CCatTable :	public CPsiTableBase,
					public ICatTable
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CCatTable)

// CPsiTableBase
	virtual void Reset(void);

// ICatTable
	virtual const IDescBlock * GetCatDesc(void) const;

// CCatTable
	CCatTable(IBonObject *pOwner);
	virtual ~CCatTable(void);

protected:
// CPsiTableBase
	virtual const bool OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection);

	CDescBlock m_DescBlock;		// 記述子領域

	ICatTable::IHandler * const m_pHandler;
};


/////////////////////////////////////////////////////////////////////////////
// [0x02] PMTテーブル基底クラス
/////////////////////////////////////////////////////////////////////////////

class CPmtTable :	public CPsiTableBase,
					public IPmtTable
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CPmtTable)

// CPsiTableBase
	virtual void Reset(void);

// IPmtTable
	virtual const WORD GetProgramID(void) const;
	virtual const WORD GetPcrPID(void) const;
	virtual const IDescBlock * GetPmtDesc(void) const;
	virtual const DWORD GetEsNum(void) const;
	virtual const BYTE GetStreamTypeID(const DWORD dwIndex) const;
	virtual const WORD GetEsPID(const DWORD dwIndex) const;
	virtual const IDescBlock * GetEsDesc(const DWORD dwIndex) const;

// CPmtTable
	CPmtTable(IBonObject *pOwner);
	virtual ~CPmtTable(void);

protected:
// CPsiTableBase
	virtual const bool OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection);

	struct TAG_PMTITEM
	{
		BYTE byStreamTypeID;		// ストリーム形式ID
		WORD wEsPID;				// エレメンタリPID
		CDescBlock *pDescBlock;		// 記述子領域2
	};

	WORD m_wProgramID;				// 放送番組番号ID
	WORD m_wPcrPID;					// PCR PID
	CDescBlock m_DescBlock;			// 記述子領域1

	std::vector<TAG_PMTITEM> m_EsInfoArray;

	IPmtTable::IHandler * const m_pHandler;
};


/////////////////////////////////////////////////////////////////////////////
// [0x40] NITテーブル基底クラス
/////////////////////////////////////////////////////////////////////////////

class CNitTable :	public CPsiTableBase,
					public INitTable
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CNitTable)

// CPsiTableBase
	virtual void Reset(void);

// INitTable
	virtual const WORD GetNetworkID(void) const;
	virtual const IDescBlock * GetNetworkDesc(void) const;
	virtual const DWORD GetTsNum(void) const;
	virtual const BYTE GetTsID(const DWORD dwIndex) const;
	virtual const BYTE GetOrgNetworkID(const DWORD dwIndex) const;
	virtual const IDescBlock * GetTsDesc(const DWORD dwIndex) const;

// CNitTable
	CNitTable(IBonObject *pOwner);
	virtual ~CNitTable(void);

protected:
// CPsiTableBase
	virtual const bool OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection);

	struct TAG_NITITEM
	{
		WORD wTsID;					// TS ID
		WORD wOrgNetworkID;			// オリジナルネットワークID
		CDescBlock *pDescBlock;		// TS記述子領域
	};

	WORD m_wNetworkID;				// ネットワークID
	CDescBlock m_NwDescBlock;		// ネットワーク記述子領域

	std::vector<TAG_NITITEM> m_TsInfoArray;

	INitTable::IHandler * const m_pHandler;
};



/////////////////////////////////////////////////////////////////////////////
// [0x42] SDTテーブル基底クラス
/////////////////////////////////////////////////////////////////////////////

class CSdtTable :	public CPsiTableBase,
					public ISdtTable
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CSdtTable)

// CPsiTableBase
	virtual void Reset(void);

// ISdtTable
	virtual const WORD GetTsID(void) const;
	virtual const WORD GetOrgNetworkID(void) const;
	virtual const DWORD GetServiceNum(void) const;
	virtual const WORD GetServiceID(const DWORD dwIndex = 0UL) const;
	virtual const bool IsEitSchedule(const WORD wServiceID) const;
	virtual const bool IsEitPresentFollowing(const WORD wServiceID) const;
	virtual const BYTE GetRunningStatus(const WORD wServiceID) const;
	virtual const bool IsScrambling(const WORD wServiceID) const;
	virtual const IDescBlock * GetServiceDesc(const WORD wServiceID) const;

// CSdtTable
	CSdtTable(IBonObject *pOwner);
	virtual ~CSdtTable(void);

protected:
// CPsiTableBase
	virtual const bool OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection);
	const DWORD GetIndexByServiceID(const WORD wServiceID) const;

	struct TAG_SDTITEM
	{
		WORD wServiceID;				// サービスID
		bool bIsEitSchedule;			// EITスケジュールフラグ
		bool IsEitPresentFollowing;		// EIT現在/次フラグ
		BYTE byRunningStatus;			// 進行状態
		bool bIsScrambling;				// スクランブルフラグ
		CDescBlock *pDescBlock;			// 記述子領域
	};

	WORD m_wTsID;						// TS ID
	WORD m_wOrgNetworkID;				// オリジナルネットワークID

	std::vector<TAG_SDTITEM> m_SdtItemArray;

	ISdtTable::IHandler * const m_pHandler;
};


/////////////////////////////////////////////////////////////////////////////
// [0x4E-0x6F] EITテーブル基底クラス
/////////////////////////////////////////////////////////////////////////////

class CEitTable :	public CPsiTableBase,
					public IEitTable
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CEitTable)

// CPsiTableBase
	virtual void Reset(void);

// IEitTable
	virtual const WORD GetServiceID(void) const;
	virtual const WORD GetTsID(void) const;
	virtual const WORD GetOrgNetworkID(void) const;
	virtual const BYTE GetTableID(void) const;
	virtual const BYTE GetLastTableID(void) const;
	virtual const BYTE GetSectionNumber(void) const;
	virtual const BYTE GetLastSectionNumber(void) const;
	virtual const BYTE GetSegmentLastSectionNumber(void) const;

// CEitTable
	CEitTable(IBonObject *pOwner);
	virtual ~CEitTable(void);

protected:
// CPsiTableBase
	virtual void OnPsiSection(const IPsiSectionParser *pPsiSectionParser, const IPsiSection *pPsiSection);
	virtual const bool OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection);

	IEitTable::IHandler * const m_pHandler;
	CEitItem m_EitItem;
};


/////////////////////////////////////////////////////////////////////////////
// [0x73] TOTテーブル基底クラス
/////////////////////////////////////////////////////////////////////////////

class CTotTable :	public CPsiTableBase,
					public ITotTable
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CTotTable)

// CPsiTableBase
	virtual void Reset(void);

// ITotTable
	virtual const SYSTEMTIME & GetDateTime(void) const;
	virtual const IDescBlock * GetTotDesc(void) const;
	
// CTotTable
	CTotTable(IBonObject *pOwner);
	virtual ~CTotTable(void);

protected:
// CPsiTableBase
	virtual const bool OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection);

	CTsTime m_DateTime;			// 現在日付/時刻
	CDescBlock m_DescBlock;		// 記述子領域

	ITotTable::IHandler * const m_pHandler;
};