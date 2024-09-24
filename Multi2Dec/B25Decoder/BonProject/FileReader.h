// FileReader.h: ファイルリーダデコーダ
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


#include <Vector>
#include <Queue>


/////////////////////////////////////////////////////////////////////////////
// ファイルリーダデコーダ
/////////////////////////////////////////////////////////////////////////////
// Output	#0	: IMediaData	バイナリストリーム
/////////////////////////////////////////////////////////////////////////////

class CFileReader :	public CMediaDecoder,
					public IFileReader
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CFileReader)

// IFileReader
	virtual const bool OpenFile(LPCTSTR lpszFileName, const bool bAsyncRead = false, const DWORD dwBuffSize = 0UL);
	virtual bool CloseFile(void);
	virtual const DWORD ReadSync(const DWORD dwReadSize);
	virtual const bool SetReadPos(const ULONGLONG llPos);
	virtual const ULONGLONG GetReadPos(void);
	virtual const ULONGLONG GetFileLength(void);
	
// CFileReader
	CFileReader(IBonObject *pOwner);
	virtual ~CFileReader(void);
	
protected:
// CMediaDecoder
	virtual const bool OnPlay(void);
	virtual const bool OnStop(void);

// CFileReader
	void ReadThread(CSmartThread<CFileReader> *pThread, bool &bKillSignal, PVOID pParam);
	void OutputThread(CSmartThread<CFileReader> *pThread, bool &bKillSignal, PVOID pParam);
	
	virtual const DWORD ReadAsync(const DWORD dwReadSize);

	ISmartFile *m_pFile;
	CSmartThread<CFileReader> m_ReadThread;
	CSmartThread<CFileReader> m_OutputThread;

	std::vector<IMediaData *> m_MediaPool;
	std::vector<IMediaData *>::iterator m_itFreeMedia;
	std::queue<IMediaData *> m_MediaQue;

	DWORD m_dwBuffSize;
};
