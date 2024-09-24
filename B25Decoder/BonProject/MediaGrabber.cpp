// MediaGrabber.cpp: メディアグラバデコーダ
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BonSDK.h"
#include "MediaGrabber.h"


/////////////////////////////////////////////////////////////////////////////
// メディアグラバデコーダ
/////////////////////////////////////////////////////////////////////////////

const bool CMediaGrabber::InputMedia(IMediaData *pMediaData, const DWORD dwInputIndex)
{
	// イベント通知
	SendDecoderEvent(IMediaGrabber::EID_ON_MEDIADATA, static_cast<PVOID>(pMediaData));

	return true;
}

CMediaGrabber::CMediaGrabber(IBonObject *pOwner)
	: CMediaDecoder(pOwner, 1UL, 1UL)
{
	// 何もしない
}

CMediaGrabber::~CMediaGrabber(void)
{
	// 何もしない
}

const bool CMediaGrabber::OnPlay(void)
{
	// イベント通知
	SendDecoderEvent(IMediaGrabber::EID_ON_PLAY);

	return true;
}

const bool CMediaGrabber::OnStop(void)
{
	// イベント通知
	SendDecoderEvent(IMediaGrabber::EID_ON_STOP);

	return true;
}

const bool CMediaGrabber::OnReset(void)
{
	// イベント通知
	SendDecoderEvent(IMediaGrabber::EID_ON_RESET);

	return true;
}
