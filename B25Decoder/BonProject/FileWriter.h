// FileWriter.h: ファイルライタデコーダ
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


#include <Vector>
#include <Queue>


/////////////////////////////////////////////////////////////////////////////
// ファイルライタデコーダ
/////////////////////////////////////////////////////////////////////////////
// Input	#0	: IMediaData	バイナリストリーム
/////////////////////////////////////////////////////////////////////////////

class CFileWriter :	public CMediaDecoder,
					public IFileWriter
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CFileWriter)

// CMediaDecoder
	virtual const bool InputMedia(IMediaData *pMediaData, const DWORD dwInputIndex = 0UL);
	
// IFileWriter
	virtual const bool OpenFile(LPCTSTR lpszFileName, const bool bAsyncWrite = false, const DWORD dwBuffSize = 0UL);
	virtual bool CloseFile(void);
	virtual const ULONGLONG GetFileLength(void);
	
// CFileWriter
	CFileWriter(IBonObject *pOwner);
	virtual ~CFileWriter(void);
	
protected:
// CMediaDecoder
	virtual const bool OnPlay(void);
	virtual const bool OnStop(void);

// CFileWriter
	virtual const bool WriteSync(const IMediaData *pMediaData);
	virtual const bool WriteAsync(const IMediaData *pMediaData);
	void WriteThread(CSmartThread<CFileWriter> *pThread, bool &bKillSignal, PVOID pParam);

	ISmartFile *m_pFile;
	CSmartThread<CFileWriter> m_WriteThread;

	std::vector<IMediaData *> m_MediaPool;
	std::vector<IMediaData *>::iterator m_itFreeMedia;
	std::queue<IMediaData *> m_MediaQue;
};
