// MediaGrabber.h: ���f�B�A�O���o�f�R�[�_
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// ���f�B�A�O���o�f�R�[�_
/////////////////////////////////////////////////////////////////////////////
// Input	#0	: IMediaData	�o�C�i���X�g���[��
// Output	#0	: IMediaData	�o�C�i���X�g���[��
/////////////////////////////////////////////////////////////////////////////

class CMediaGrabber :	public CMediaDecoder,
						public IMediaGrabber
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CMediaGrabber)

// CMediaDecoder
	virtual const bool InputMedia(IMediaData *pMediaData, const DWORD dwInputIndex = 0UL);
	
// CMediaGrabber
	CMediaGrabber(IBonObject *pOwner);
	virtual ~CMediaGrabber(void);
	
protected:
// CMediaDecoder
	virtual const bool OnPlay(void);
	virtual const bool OnStop(void);
	virtual const bool OnReset(void);
};
