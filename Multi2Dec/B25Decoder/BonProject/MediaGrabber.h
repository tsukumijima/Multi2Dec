// MediaGrabber.h: メディアグラバデコーダ
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// メディアグラバデコーダ
/////////////////////////////////////////////////////////////////////////////
// Input	#0	: IMediaData	バイナリストリーム
// Output	#0	: IMediaData	バイナリストリーム
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
