// SyncObject.cpp: 同期オブジェクトラッパークラス
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BonSDK.h"
#include "SyncObject.h"


//////////////////////////////////////////////////////////////////////
// クリティカルセクションラッパークラス
//////////////////////////////////////////////////////////////////////

CSmartLock::CSmartLock()
{
#ifdef _WIN32
	// クリティカルセクション初期化
	::InitializeCriticalSection(&m_CriticalSection);
#endif
}

CSmartLock::~CSmartLock()
{
#ifdef _WIN32
	// クリティカルセクション削除
	::DeleteCriticalSection(&m_CriticalSection);
#endif
}

void CSmartLock::Lock(void)
{
	// クリティカルセクション取得
#ifdef _WIN32
	::EnterCriticalSection(&m_CriticalSection);
#else
	m_mutex.lock();
#endif
}

void CSmartLock::Unlock(void)
{
	// クリティカルセクション開放
#ifdef _WIN32
	::LeaveCriticalSection(&m_CriticalSection);
#else
	m_mutex.unlock();
#endif
}


//////////////////////////////////////////////////////////////////////
// ブロックスコープロッククラス
//////////////////////////////////////////////////////////////////////

CBlockLock::CBlockLock(CSmartLock *pSmartLock)
	: m_pSmartLock(pSmartLock)
{
	// ロック取得
	if(m_pSmartLock)m_pSmartLock->Lock();
}

CBlockLock::~CBlockLock()
{
	// ロック開放
	if(m_pSmartLock)m_pSmartLock->Unlock();
}


#ifdef _WIN32
//////////////////////////////////////////////////////////////////////
// イベントラッパークラス
//////////////////////////////////////////////////////////////////////

CSmartEvent::CSmartEvent(const bool bManualReset, const bool bInitialState)
	: m_hEvent(NULL)
{
	// イベント作成
	m_hEvent = ::CreateEvent(NULL, (bManualReset)? TRUE : FALSE, (bInitialState)? TRUE : FALSE, NULL);
}

CSmartEvent::~CSmartEvent(void)
{
	// イベント開放
	if(m_hEvent)::CloseHandle(m_hEvent);
}

const bool CSmartEvent::SetEvent(void)
{
	// イベントセット
	return (::SetEvent(m_hEvent))? true : false;
}

const bool CSmartEvent::PulseEvent(void)
{
	// イベントセット(1つのスレッドのみ開放)
	return (::PulseEvent(m_hEvent))? true : false;
}
const bool CSmartEvent::ResetEvent(void)
{
	// イベントリセット
	return (::ResetEvent(m_hEvent))? true : false;
}

const DWORD CSmartEvent::WaitEvent(const DWORD dwTimeout)
{
	// イベント待ち
	switch(::WaitForSingleObject(m_hEvent, (dwTimeout)? dwTimeout : INFINITE)){
		case WAIT_ABANDONED :	return WE_CLOSED;
		case WAIT_OBJECT_0	:	return WE_TRIGGER;
		case WAIT_TIMEOUT	:	return WE_TIMEOUT;
		default				:	return WE_CLOSED;
		}
}


/////////////////////////////////////////////////////////////////////////////
// ミューテックスラッパークラス
/////////////////////////////////////////////////////////////////////////////

CSmartMutex::CSmartMutex(void)
	: m_hMutex(NULL)
{

}

CSmartMutex::~CSmartMutex(void)
{
	Close();
}

const bool CSmartMutex::Create(LPCTSTR lpszName)
{
	// 無名には非対応(プロセス間の排他制御のみ)
	if(!lpszName)return false;
	if(!::_tcslen(lpszName))return false;

	// 既にオープン済みの場合はエラー
	if(m_hMutex)return false;

	// Mutex作成
	if(!(m_hMutex = ::CreateMutex(NULL, TRUE, lpszName)))return false;

	// 他のプロセスでオープンされている場合はエラー
	if(::GetLastError() == ERROR_ALREADY_EXISTS){
		Close();
		return false;
		}

	return true;
}

const bool CSmartMutex::Close(void)
{
	if(!m_hMutex)return false;

	// Mutexを閉じる
	const BOOL bReturn = ::CloseHandle(m_hMutex);
	m_hMutex = NULL;

	return (bReturn)? true : false;
}

const bool CSmartMutex::IsExist(LPCTSTR lpszName)
{
	// 指定した名前のMutexをオープンする
	HANDLE hMutex = ::OpenMutex(MUTEX_ALL_ACCESS, FALSE, lpszName);

	if(hMutex){
		// 既にMutexは存在する
		::CloseHandle(hMutex);
		return true;
		}
	else{
		// Mutexは存在しない
		return false;
		}
}
#endif
