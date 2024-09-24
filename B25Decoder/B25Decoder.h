// B25Decoder.h: CB25Decoder クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#pragma once


#define B25SDK_IMPLEMENT


#include "BonSDK.h"
#include "IB25Decoder.h"
#include "TsPacketSync.h"
#include "TsDescrambler.h"
#include "MediaGrabber.h"


class CB25Decoder : public IB25Decoder2,
					protected IMediaDecoderHandler,
					public CBonObject
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CB25Decoder)

// IB25Decoder
	virtual const BOOL Initialize(DWORD dwRound = 4);
	virtual const BOOL Decode(BYTE *pSrcBuf, const DWORD dwSrcSize, BYTE **ppDstBuf, DWORD *pdwDstSize);
	virtual const BOOL Flush(BYTE **ppDstBuf, DWORD *pdwDstSize);
	virtual const BOOL Reset(void);

// IB25Decoder2
	virtual void DiscardNullPacket(const bool bEnable = true);
	virtual void DiscardScramblePacket(const bool bEnable = true);
	virtual void EnableEmmProcess(const bool bEnable = true);
	virtual const DWORD GetDescramblingState(const WORD wProgramID);
	virtual void ResetStatistics(void);
	virtual const DWORD GetPacketStride(void);
	virtual const DWORD GetInputPacketNum(const WORD wPID = TS_INVALID_PID);
	virtual const DWORD GetOutputPacketNum(const WORD wPID = TS_INVALID_PID);
	virtual const DWORD GetSyncErrNum(void);
	virtual const DWORD GetFormatErrNum(void);
	virtual const DWORD GetTransportErrNum(void);
	virtual const DWORD GetContinuityErrNum(const WORD wPID = TS_INVALID_PID);
	virtual const DWORD GetScramblePacketNum(const WORD wPID = TS_INVALID_PID);
	virtual const DWORD GetEcmProcessNum(void);
	virtual const DWORD GetEmmProcessNum(void);

// CB25Decoder
	CB25Decoder(IBonObject * const pOwner);
	virtual ~CB25Decoder(void);

protected:
	virtual const DWORD OnDecoderEvent(IMediaDecoder *pDecoder, const DWORD dwEventID, PVOID pParam);

	CTsPacketSync m_TsPacketSync;
	CTsDescrambler m_TsDescrambler;
	CMediaGrabber m_MediaGrabber;

	CMediaData m_InputBuff;
	CMediaData m_OutputBuff;

	CSmartLock m_GraphLock;
};
