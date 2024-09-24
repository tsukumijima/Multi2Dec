// MediaBase.h: Bonメディア基底クラス
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// IMediaData基底クラス
/////////////////////////////////////////////////////////////////////////////

class CMediaData :	public CBonObject,
					public IMediaData
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CMediaData)

// IMediaData
	virtual BYTE * GetData(void) const;
	virtual const DWORD GetSize(void) const;

	virtual void SetAt(const DWORD dwPos, const BYTE byData);
	virtual const BYTE GetAt(const DWORD dwPos) const;

	virtual const DWORD SetData(const BYTE *pData, const DWORD dwDataSize);
	virtual const DWORD AddData(const BYTE *pData, const DWORD dwDataSize);
	virtual const DWORD AddData(const IMediaData *pData);
	virtual const DWORD AddByte(const BYTE byData);
	virtual const DWORD TrimHead(const DWORD dwTrimSize = 1UL);
	virtual const DWORD TrimTail(const DWORD dwTrimSize = 1UL);

	virtual const DWORD CopyData(const IMediaData *pSrc);
	virtual const DWORD GetBuffer(const DWORD dwGetSize);

	virtual const DWORD SetSize(const DWORD dwSetSize);
	virtual const DWORD SetSize(const DWORD dwSetSize, const BYTE byFiller);
	
	virtual void ClearSize(void);

// CMediaData
	CMediaData(IBonObject *pOwner);
	CMediaData(const CMediaData &Operand);
	CMediaData(const DWORD dwBuffSize);
	CMediaData(const BYTE *pData, const DWORD dwDataSize);
	CMediaData(const BYTE byFiller, const DWORD dwDataSize);
	virtual ~CMediaData(void);

	CMediaData & operator = (const CMediaData &Operand);

protected:
	DWORD m_dwDataSize;
	DWORD m_dwBuffSize;
	BYTE *m_pData;
};


/////////////////////////////////////////////////////////////////////////////
// IMediaDecoder基底クラス
/////////////////////////////////////////////////////////////////////////////

class CMediaDecoder :	public CBonObject,
						public IMediaDecoder
{
public:
// IMediaDecoder
	virtual const DWORD GetInputNum(void) const;
	virtual const DWORD GetOutputNum(void) const;

	virtual const bool PlayDecoder(void);
	virtual const bool StopDecoder(void);
	virtual const bool ResetDecoder(void);

	virtual const bool SetOutputDecoder(IMediaDecoder *pDecoder, const DWORD dwOutputIndex = 0UL, const DWORD dwInputIndex = 0UL);
	virtual const bool InputMedia(IMediaData *pMediaData, const DWORD dwInputIndex = 0UL);

// CMediaDecoder
	CMediaDecoder(IBonObject *pOwner, const DWORD dwInputNum = 1UL, const DWORD dwOutputNum = 1UL, CSmartLock *pLock = NULL);
	virtual ~CMediaDecoder(void);

protected:
	virtual const bool OnPlay(void);
	virtual const bool OnStop(void);
	virtual const bool OnReset(void);

	virtual const bool OutputMedia(IMediaData *pMediaData, const DWORD dwOutptIndex = 0UL);
	virtual const DWORD SendDecoderEvent(const DWORD dwEventID, PVOID pParam = NULL);

	// 出力ピンデータベース
	struct TAG_OUTPUTDECODER
	{
		IMediaDecoder *pDecoder;
		DWORD dwInputIndex;
	} m_aOutputDecoder[16];

	IMediaDecoderHandler *m_pEventHandler;

	CSmartLock *m_pLock;
	const DWORD m_dwInputNum;
	const DWORD m_dwOutputNum;
};
