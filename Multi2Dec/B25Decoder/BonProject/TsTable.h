// TsTable.h: TS�e�[�u�����N���X
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
// [0x00] PAT�e�[�u�����N���X
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
		WORD wProgramID;	// �����ԑg�ԍ�ID
		WORD wPID;			// NIT or PMT��PID
	};

	WORD m_wTsID;			// TS ID

	std::vector<TAG_PATITEM> m_NitPIDArray;
	std::vector<TAG_PATITEM> m_PmtPIDArray;

	IPatTable::IHandler * const m_pHandler;
};


/////////////////////////////////////////////////////////////////////////////
// [0x01] CAT�e�[�u�����N���X
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

	CDescBlock m_DescBlock;		// �L�q�q�̈�

	ICatTable::IHandler * const m_pHandler;
};


/////////////////////////////////////////////////////////////////////////////
// [0x02] PMT�e�[�u�����N���X
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
		BYTE byStreamTypeID;		// �X�g���[���`��ID
		WORD wEsPID;				// �G�������^��PID
		CDescBlock *pDescBlock;		// �L�q�q�̈�2
	};

	WORD m_wProgramID;				// �����ԑg�ԍ�ID
	WORD m_wPcrPID;					// PCR PID
	CDescBlock m_DescBlock;			// �L�q�q�̈�1

	std::vector<TAG_PMTITEM> m_EsInfoArray;

	IPmtTable::IHandler * const m_pHandler;
};


/////////////////////////////////////////////////////////////////////////////
// [0x40] NIT�e�[�u�����N���X
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
		WORD wOrgNetworkID;			// �I���W�i���l�b�g���[�NID
		CDescBlock *pDescBlock;		// TS�L�q�q�̈�
	};

	WORD m_wNetworkID;				// �l�b�g���[�NID
	CDescBlock m_NwDescBlock;		// �l�b�g���[�N�L�q�q�̈�

	std::vector<TAG_NITITEM> m_TsInfoArray;

	INitTable::IHandler * const m_pHandler;
};



/////////////////////////////////////////////////////////////////////////////
// [0x42] SDT�e�[�u�����N���X
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
		WORD wServiceID;				// �T�[�r�XID
		bool bIsEitSchedule;			// EIT�X�P�W���[���t���O
		bool IsEitPresentFollowing;		// EIT����/���t���O
		BYTE byRunningStatus;			// �i�s���
		bool bIsScrambling;				// �X�N�����u���t���O
		CDescBlock *pDescBlock;			// �L�q�q�̈�
	};

	WORD m_wTsID;						// TS ID
	WORD m_wOrgNetworkID;				// �I���W�i���l�b�g���[�NID

	std::vector<TAG_SDTITEM> m_SdtItemArray;

	ISdtTable::IHandler * const m_pHandler;
};


/////////////////////////////////////////////////////////////////////////////
// [0x4E-0x6F] EIT�e�[�u�����N���X
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
// [0x73] TOT�e�[�u�����N���X
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

	CTsTime m_DateTime;			// ���ݓ��t/����
	CDescBlock m_DescBlock;		// �L�q�q�̈�

	ITotTable::IHandler * const m_pHandler;
};