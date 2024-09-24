// SmartThread.h: スマートスレッドクラス
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// スマートスレッドクラス
/////////////////////////////////////////////////////////////////////////////

template <class T> class CSmartThread
{
public:
	// スレッドメソッド型
	typedef void (T::*TAGET_FUNC)(CSmartThread *pThread, bool &bKillSignal, PVOID pParam);

	CSmartThread(void)
		: m_pThreadOwner(NULL)
		, m_pTargetFunc(NULL)
		, m_pThreadParam(NULL)
		, m_hThread(NULL)
		, m_dwThreadID(0UL)
		, m_bKillSignal(true)
		, m_bSyncSignal(false)
	{
	
	}
	
	~CSmartThread(void)
	{
		EndThread();
	}

	const bool StartThread(T *pOwner, TAGET_FUNC pTargetFunc, PVOID pThreadParam = NULL, const bool bStart = true)
	{
		if(!pOwner || !pTargetFunc)return false;
		
		// 開放されていない場合は開放する
		if(m_hThread)::CloseHandle(m_hThread);
	
		// 関数ポインタ、パラメータ保存
		m_pThreadOwner = pOwner;
		m_pTargetFunc = pTargetFunc;
		m_pThreadParam = pThreadParam;
		
		// シグナルクリア
		m_bSyncSignal = false;
		m_bKillSignal = false;
		
		// スレッド起動
		if(!(m_hThread = ::CreateThread(NULL, 0UL, CSmartThread::ThreadFunc, static_cast<LPVOID>(this), (bStart)? 0UL : CREATE_SUSPENDED, &m_dwThreadID))){
			// スレッド起動失敗
			return false;
			}

		// スレッド起動完了待ち
		if(bStart){
			while(!m_bSyncSignal)::Sleep(0);
			}	

		return true;
	}

	const bool EndThread(const bool bPumpMessage = false)
	{
		if(!m_hThread)return false;
	
		// 終了シグナルセット
		m_bKillSignal = true;

		// スレッドの中から呼び出した場合は開放しない
		if(::GetCurrentThreadId() == m_dwThreadID)return true;
	
		// サスペンド状態のときはレジューム
		::ResumeThread(m_hThread);

		// 終了を待つ
		while(m_bSyncSignal){
			if(!bPumpMessage)::Sleep(0);
			else PumpMessage();
			}

		// スレッド開放
		::CloseHandle(m_hThread);
		m_hThread = NULL;

		return true;	
	}
	
	const bool SuspendThread(void)
	{
		if(!m_hThread)false;

		// スレッドサスペンド
		::SuspendThread(m_hThread);
	
		return true;	
	}
	
	const bool ResumeThread(void)
	{
		if(!m_hThread)false;

		// スレッドレジューム
		::ResumeThread(m_hThread);
	
		return true;	
	}

	const HANDLE GetThreadHandle(void) const
	{
		// スレッドハンドルを返す
		return m_hThread;	
	}
	
	const DWORD GetThreadID(void) const
	{
		// スレッドIDを返す
		return m_dwThreadID;
	}

	const bool IsRunning(void) const
	{
		// スレッド実行中有無を返す
		return m_bSyncSignal;
	}
	
protected:
	static DWORD WINAPI ThreadFunc(LPVOID pParam)
	{
		CSmartThread *pThis = static_cast<CSmartThread *>(pParam);

		// サスペンド状態で終了
		if(pThis->m_bKillSignal){
			pThis->m_bSyncSignal = false;
			return 0UL;	
			}

		// 起動シグナルセット
		pThis->m_bSyncSignal = true;

		// スレッドメソッド実行
		((pThis->m_pThreadOwner)->*(pThis->m_pTargetFunc))(pThis, pThis->m_bKillSignal, pThis->m_pThreadParam);

		// 起動シグナルクリア
		pThis->m_bSyncSignal = false;

		return 0UL;	
	}

	static void PumpMessage(void)
	{
		MSG Msg;

		// メッセージポンプを駆動する
		while(::PeekMessage(&Msg, NULL, 0UL, 0UL, PM_REMOVE)){
			::TranslateMessage(&Msg);
			::DispatchMessage(&Msg);
			}
	}

	T *m_pThreadOwner;
	TAGET_FUNC m_pTargetFunc;
	PVOID m_pThreadParam;

	HANDLE m_hThread;
	DWORD m_dwThreadID;

	bool m_bSyncSignal;
	bool m_bKillSignal;
};
