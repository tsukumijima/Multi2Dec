// TsTableBase.h: PSIテーブル基底クラス
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


#include "TsSection.h"


/////////////////////////////////////////////////////////////////////////////
// PSIテーブル基底クラス
/////////////////////////////////////////////////////////////////////////////

class CPsiTableBase :	public CBonObject,
						public ITsPidMapTarget,
						protected IPsiSectionParser::IHandler
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CPsiTableBase)

// ITsPidMapTarget
	virtual const bool StorePacket(const ITsPacket *pTsPacket);
	virtual void OnPidReset(ITsPidMapper *pTsPidMapper, const WORD wPID);
	virtual void OnPidMapped(ITsPidMapper *pTsPidMapper, const WORD wPID);
	virtual void OnPidUnmapped(ITsPidMapper *pTsPidMapper, const WORD wPID);

// CPsiTableBase
	CPsiTableBase(IBonObject *pOwner);
	virtual ~CPsiTableBase(void);

	virtual void Reset(void);
	const DWORD GetCrcErrorCount(void) const;

protected:
// IPsiSectionParser::IHandler
	virtual void OnPsiSection(const IPsiSectionParser *pPsiSectionParser, const IPsiSection *pPsiSection);

// CPsiTableBase
	virtual const bool OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection);

	CPsiSectionParser m_SectionParser;
	CPsiSection m_LastSection;

	DWORD m_dwRefCount;
};
