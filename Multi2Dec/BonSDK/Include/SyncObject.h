// SyncObject.h: 同期オブジェクトラッパークラス
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// クリティカルセクションラッパークラス
/////////////////////////////////////////////////////////////////////////////

class CSmartLock
{
public:
	CSmartLock();
	virtual ~CSmartLock();
	
	void Lock(void);
	void Unlock(void);
	
protected:
	CRITICAL_SECTION m_CriticalSection;
};


/////////////////////////////////////////////////////////////////////////////
// ブロックスコープロッククラス
/////////////////////////////////////////////////////////////////////////////

class CBlockLock
{
public:
	CBlockLock(CSmartLock *pSmartLock);
	virtual ~CBlockLock();
		
protected:
	CSmartLock *m_pSmartLock;
};


/////////////////////////////////////////////////////////////////////////////
// イベントラッパークラス
/////////////////////////////////////////////////////////////////////////////

class CSmartEvent
{
public:
	enum
	{
		WE_TRIGGER,		// イベントがシグナル状態になった
		WE_TIMEOUT,		// タイムアウトした
		WE_CLOSED		// イベントは既に開放されている
	};

	CSmartEvent(const bool bManualReset = false, const bool bInitialState = false);
	virtual ~CSmartEvent(void);

	const bool SetEvent(void);
	const bool PulseEvent(void);
	const bool ResetEvent(void);

	const DWORD WaitEvent(const DWORD dwTimeout = 0UL);

protected:
	HANDLE m_hEvent;

};


/////////////////////////////////////////////////////////////////////////////
// ミューテックスラッパークラス
/////////////////////////////////////////////////////////////////////////////

class CSmartMutex
{
public:
	CSmartMutex(void);
	virtual ~CSmartMutex(void);

	const bool Create(LPCTSTR lpszName);
	const bool Close(void);

	static const bool IsExist(LPCTSTR lpszName);

protected:
	HANDLE m_hMutex;
};
