// Multi2Converter.h: Multi2復号コンバータクラス
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


#include "TsConverterIf.h"


/////////////////////////////////////////////////////////////////////////////
// Multi2復号コンバータクラス
/////////////////////////////////////////////////////////////////////////////

class CMulti2Converter :	public CBonObject,
							public IMulti2Converter,
							protected IMediaDecoderHandler
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CMulti2Converter)

// IMulti2Converter
	virtual const DWORD StartConvert(LPCTSTR lpszSrcFile, LPCTSTR lpszDstFile, const bool bB25 = true, const bool bEMM = false, const bool bNULL = false, const bool bDiscard = false);
	virtual void EndConvert(void);
	virtual const DWORD GetPacketStride(void) const;
	virtual const DWORD GetInputPacketNum(const WORD wPID = TS_INVALID_PID) const;
	virtual const DWORD GetOutputPacketNum(const WORD wPID = TS_INVALID_PID) const;
	virtual const DWORD GetSyncErrNum(void) const;
	virtual const DWORD GetFormatErrNum(void) const;
	virtual const DWORD GetTransportErrNum(void) const;
	virtual const DWORD GetContinuityErrNum(const WORD wPID = TS_INVALID_PID) const;
	virtual const DWORD GetScramblePacketNum(const WORD wPID = TS_INVALID_PID) const;
	virtual const DWORD GetEcmProcessNum(void) const;
	virtual const DWORD GetEmmProcessNum(void) const;
	virtual const ULONGLONG GetSrcFileLength(void) const;
	virtual const ULONGLONG GetDstFileLength(void) const;

// CMulti2Converter
	CMulti2Converter(IBonObject * const pOwner);
	virtual ~CMulti2Converter(void);
	
protected:
	virtual const DWORD OnDecoderEvent(IMediaDecoder *pDecoder, const DWORD dwEventID, PVOID pParam);
	virtual void CloseDecoder(void);
	virtual void SendConverterEvent(const DWORD dwEventID, PVOID pParam = NULL);

	IFileReader *m_pFileReader;
	ITsPacketSync *m_pTsPacketSync;
	ITsDescrambler *m_pTsDescrambler;
	IFileWriter *m_pFileWriter;

	IMulti2Converter::IHandler *m_pHandler;
};
