// BonMediaIf.h: Bon���f�B�A�C���^�t�F�[�X��`
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// ���f�B�A�f�[�^�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IMediaData :				public IBonObject
{
public:
	virtual BYTE * GetData(void) const = 0;
	virtual const DWORD GetSize(void) const = 0;

	virtual void SetAt(const DWORD dwPos, const BYTE byData) = 0;
	virtual const BYTE GetAt(const DWORD dwPos) const = 0;

	virtual const DWORD SetData(const BYTE *pData, const DWORD dwDataSize) = 0;
	virtual const DWORD AddData(const BYTE *pData, const DWORD dwDataSize) = 0;
	virtual const DWORD AddData(const IMediaData *pData) = 0;
	virtual const DWORD AddByte(const BYTE byData) = 0;
	virtual const DWORD TrimHead(const DWORD dwTrimSize = 1UL) = 0;
	virtual const DWORD TrimTail(const DWORD dwTrimSize = 1UL) = 0;

	virtual const DWORD CopyData(const IMediaData *pSrc) = 0;
	virtual const DWORD GetBuffer(const DWORD dwGetSize) = 0;

	virtual const DWORD SetSize(const DWORD dwSetSize) = 0;
	virtual const DWORD SetSize(const DWORD dwSetSize, const BYTE byFiller) = 0;
	
	virtual void ClearSize(void) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// ���f�B�A�f�R�[�_�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IMediaDecoder :			public IBonObject
{
public:
	virtual const DWORD GetInputNum(void) const = 0;
	virtual const DWORD GetOutputNum(void) const = 0;

	virtual const bool PlayDecoder(void) = 0;
	virtual const bool StopDecoder(void) = 0;
	virtual const bool ResetDecoder(void) = 0;

	virtual const bool SetOutputDecoder(IMediaDecoder *pDecoder, const DWORD dwOutputIndex = 0UL, const DWORD dwInputIndex = 0UL) = 0;
	virtual const bool InputMedia(IMediaData *pMediaData, const DWORD dwInputIndex = 0UL) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// ���f�B�A�f�R�[�_�C�x���g�n���h���C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IMediaDecoderHandler :	public IBonObject
{
public:
	virtual const DWORD OnDecoderEvent(IMediaDecoder *pDecoder, const DWORD dwEventID, PVOID pParam) = 0;
};
