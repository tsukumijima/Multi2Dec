// MediaGrabber.cpp: ���f�B�A�O���o�f�R�[�_
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "MediaGrabber.h"


/////////////////////////////////////////////////////////////////////////////
// ���f�B�A�O���o�f�R�[�_
/////////////////////////////////////////////////////////////////////////////

const bool CMediaGrabber::InputMedia(IMediaData *pMediaData, const DWORD dwInputIndex)
{
	// �C�x���g�ʒm
	SendDecoderEvent(IMediaGrabber::EID_ON_MEDIADATA, static_cast<PVOID>(pMediaData));

	return true;
}

CMediaGrabber::CMediaGrabber(IBonObject *pOwner)
	: CMediaDecoder(pOwner, 1UL, 1UL)
{
	// �������Ȃ�
}

CMediaGrabber::~CMediaGrabber(void)
{
	// �������Ȃ�
}

const bool CMediaGrabber::OnPlay(void)
{
	// �C�x���g�ʒm
	SendDecoderEvent(IMediaGrabber::EID_ON_PLAY);

	return true;
}

const bool CMediaGrabber::OnStop(void)
{
	// �C�x���g�ʒm
	SendDecoderEvent(IMediaGrabber::EID_ON_STOP);

	return true;
}

const bool CMediaGrabber::OnReset(void)
{
	// �C�x���g�ʒm
	SendDecoderEvent(IMediaGrabber::EID_ON_RESET);

	return true;
}
