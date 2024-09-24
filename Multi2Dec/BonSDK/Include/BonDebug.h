// BonDebug.h: デバッグ補助ライブラリ
//
/////////////////////////////////////////////////////////////////////////////

#pragma once



#ifdef _DEBUG
	// デバッグビルド
	#define BON_IS_DEBUG	true
#else
	// リリースビルド
	#define BON_IS_DEBUG	false
#endif



/////////////////////////////////////////////////////////////////////////////
// 汎用例外クラス
/////////////////////////////////////////////////////////////////////////////

class CBonException
{
public:
	CBonException(LPCSTR lpszSourceFile, const DWORD dwSourceLine, const BONGUID ClassId);
	virtual ~CBonException(void);

	CBonException & operator( )(LPCTSTR lpszFormat = NULL, ...);

	void Notify(const bool bEnable = BON_IS_DEBUG);

	const BONGUID GetGuid(void);
	const DWORD GetDescription(LPTSTR lpszDescription);
	const DWORD GetSourceFile(LPTSTR lpszSourceFile);
	const DWORD GetSourceLine(void);

protected:
	const BONGUID m_ClassId;
	TCHAR m_szDescription[1024];
	LPCSTR m_lpszSourceFile;
	const DWORD m_dwSourceLine;
};


/////////////////////////////////////////////////////////////////////////////
// 汎用照合クラス
/////////////////////////////////////////////////////////////////////////////

class CBonAssert
{
public:
	CBonAssert(LPCSTR lpszSourceFile, const DWORD dwSourceLine, const BONGUID ClassId);

	void operator( )(const bool bSuccess = false, LPCTSTR lpszFormat = NULL, ...);
	void operator( )(const void *pPointer, LPCTSTR lpszFormat = NULL, ...);
	
protected:
	const BONGUID m_ClassId;
	LPCSTR m_lpszSourceFile;
	const DWORD m_dwSourceLine;
};


/*


class CPerfInspector
{
protected:
	class CPerfInitializer
	{
	public:
		CPerfInitializer()
		{
			::BON_TRACE(TEXT("CPerfInitializer()\n"));
			::QueryPerformanceFrequency((LARGE_INTEGER *)&m_llCounterFreq);
		}
	
		LONGLONG m_llCounterFreq;
	};

public:
	CPerfInspector()
	{
		::QueryPerformanceCounter((LARGE_INTEGER *)&m_llStartTime);
	}
	
	~CPerfInspector()
	{
		LONGLONG m_llEndTime = m_llStartTime;
		::QueryPerformanceCounter((LARGE_INTEGER *)&m_llEndTime);

		m_llEndTime = ((m_llEndTime - m_llStartTime) * 1000000LL) / m_PerfCounter.m_llCounterFreq;
		
		::BON_TRACE(TEXT("Block Process Time = %luus\n"), (DWORD)m_llEndTime);		
	}

protected:
	LONGLONG m_llStartTime;
	static CPerfInitializer m_PerfCounter;
};

CPerfInspector::CPerfInitializer CPerfInspector::m_PerfCounter;




*/







/////////////////////////////////////////////////////////////////////////////
// デバッグ補助関数
/////////////////////////////////////////////////////////////////////////////

const BONGUID GetGuid(void);
void BON_TRACE(LPCTSTR lpszFormat, ...);


/////////////////////////////////////////////////////////////////////////////
// デバッグ補助マクロ
/////////////////////////////////////////////////////////////////////////////

#define BON_EXPECTION	CBonException(__FILE__, __LINE__, GetGuid())
#define BON_ASSERT		CBonAssert(__FILE__, __LINE__, GetGuid())
