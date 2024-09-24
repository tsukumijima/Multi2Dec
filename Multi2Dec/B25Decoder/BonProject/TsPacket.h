// TsPacket.h: TSパケット基底クラス
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// ITsPacket基底クラス
/////////////////////////////////////////////////////////////////////////////

class CTsPacket :	public CMediaData,
					public ITsPacket
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CTsPacket)

// ITsPacket
	virtual const DWORD ParsePacket(BYTE *pContinuityCounter = NULL);
	virtual BYTE * GetRawData(void) const;
	virtual const WORD GetPID(void) const;
	virtual const bool IsScrambled(void) const;
	virtual const bool HaveAdaptationField(void) const;
	virtual BYTE * GetPayloadData(void) const;
	virtual const BYTE GetPayloadSize(void) const;
	virtual const bool CopyPacket(const ITsPacket *pSrc);
	virtual const bool ComparePacket(const ITsPacket *pSrc) const;

// CMediaData
	virtual const DWORD CopyData(const IMediaData *pSrc);

// CTsPacket
	CTsPacket(IBonObject *pOwner);
	virtual ~CTsPacket(void);
};
