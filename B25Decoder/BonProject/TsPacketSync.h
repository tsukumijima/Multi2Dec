// TsPacketSync.h: TSパケット同期デコーダ
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


#include "TsPacket.h"


/////////////////////////////////////////////////////////////////////////////
// TSパケット同期デコーダ
/////////////////////////////////////////////////////////////////////////////
// Input	#0	: IMediaData	TSストリーム
// Output	#0	: ITsPacket		TSパケット
/////////////////////////////////////////////////////////////////////////////

class CTsPacketSync :	public CMediaDecoder,
						public ITsPacketSync
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CTsPacketSync)

// CMediaDecoder
	virtual const bool InputMedia(IMediaData *pMediaData, const DWORD dwInputIndex = 0UL);
	
// ITspacketSync
	virtual void DiscardNullPacket(const bool bEnable = true);
	virtual void ResetStatistics(void);
	virtual const DWORD GetPacketStride(void);
	virtual const DWORD GetInputPacketNum(const WORD wPID = TS_INVALID_PID);
	virtual const DWORD GetOutputPacketNum(const WORD wPID = TS_INVALID_PID);
	virtual const DWORD GetInputBitrate(void);
	virtual const DWORD GetOutputBitrate(void);
	virtual const DWORD GetSyncErrNum(void);
	virtual const DWORD GetFormatErrNum(void);
	virtual const DWORD GetTransportErrNum(void);
	virtual const DWORD GetContinuityErrNum(const WORD wPID = TS_INVALID_PID);
	
// CTsPacketSync
	CTsPacketSync(IBonObject *pOwner);
	virtual ~CTsPacketSync(void);
	
protected:
// CMediaDecoder
	virtual const bool OnPlay(void);
	virtual const bool OnReset(void);

// CTsPacketSync
	virtual void SyncPacket(const BYTE *pStream, const DWORD dwSize);
	virtual const bool ParseTsPacket(void);
	virtual void OnTsPacket(void);
	virtual void UpdateBitrate(void);
	
	CTsPacket m_TsPacket;

	BYTE m_abyContCounter[0x2000];

	DWORD m_dwCurStrideSize;
	DWORD m_dwPrvStrideSize;
	DWORD m_dwFixStrideSize;
	DWORD m_dwFixStrideCount;

	bool m_bDiscardNullPacket;

	DWORD m_dwInputPacketNum;
	DWORD m_dwOutputPacketNum;

	DWORD m_dwSyncErrNum;
	DWORD m_dwFormatErrNum;
	DWORD m_dwTransportErrNum;
	DWORD m_dwContinuityErrNum;

	DWORD m_adwInputPacketNum[0x2000];
	DWORD m_adwOutputPacketNum[0x2000];
	DWORD m_adwContinuityErrNum[0x2000];

	DWORD m_dwLastUpdateTime;
	DWORD m_dwLastInputPacket;
	DWORD m_dwLastOutputPacket;
	DWORD m_dwCurInputPacket;
	DWORD m_dwCurOutputPacket;
};
