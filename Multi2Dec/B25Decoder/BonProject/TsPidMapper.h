// TsPidMapper.h: PIDマッパ基底クラス
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// ITsPidMapper基底クラス
/////////////////////////////////////////////////////////////////////////////

class CTsPidMapper :	public CBonObject,
						public ITsPidMapper
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CTsPidMapper)

// ITsPidMapper
	virtual const bool StorePacket(const ITsPacket *pTsPacket);
	virtual void ResetPid(const WORD wPID = TS_INVALID_PID);
	virtual const bool MapPid(const WORD wPID, ITsPidMapTarget *pTsPidMapTarget);
	virtual const bool UnmapPid(const WORD wPID);
	virtual void ClearPid(void);
	virtual ITsPidMapTarget * GetMapTarget(const WORD wPID) const;
	virtual const WORD GetMapNum(void) const;

// CTsPidMapper
	CTsPidMapper(IBonObject *pOwner);
	virtual ~CTsPidMapper(void);
	
protected:
	ITsPidMapTarget * m_apMapTarget[0x2000];
	WORD m_wMapTargetNum;
};
