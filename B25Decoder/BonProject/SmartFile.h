// SmartFile.h: 汎用ファイルストレージクラス
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


#include <Tchar.h>


/////////////////////////////////////////////////////////////////////////////
// 汎用ファイルストレージクラス
/////////////////////////////////////////////////////////////////////////////

class CSmartFile :	public CBonObject,
					public ISmartFile
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CSmartFile)

// ISmartFile
	virtual const bool Open(LPCTSTR lpszFileName, const DWORD dwModeFlags = MF_READ, const DWORD dwBuffSize = 0UL);
	virtual void Close(void);

	virtual const DWORD ReadData(BYTE *pBuff, const DWORD dwLen);
	virtual const DWORD ReadData(BYTE *pBuff, const DWORD dwLen, const ULONGLONG llPos);

	virtual const DWORD WriteData(const BYTE *pBuff, const DWORD dwLen);
	virtual const DWORD WriteData(const BYTE *pBuff, const DWORD dwLen, const ULONGLONG llPos);

	virtual const bool FlushData(void);

	virtual const bool SeekPos(const ULONGLONG llPos);
	virtual const ULONGLONG GetPos(void);

	virtual const ULONGLONG GetLength(void);

// CSmartFile
	CSmartFile(IBonObject *pOwner);
	virtual ~CSmartFile(void);

protected:
	FILE *m_pFileStream;
};
