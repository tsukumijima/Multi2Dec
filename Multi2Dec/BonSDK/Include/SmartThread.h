// SmartThread.h: �X�}�[�g�X���b�h�N���X
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// �X�}�[�g�X���b�h�N���X
/////////////////////////////////////////////////////////////////////////////

template <class T> class CSmartThread
{
public:
	// �X���b�h���\�b�h�^
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
		
		// �J������Ă��Ȃ��ꍇ�͊J������
		if(m_hThread)::CloseHandle(m_hThread);
	
		// �֐��|�C���^�A�p�����[�^�ۑ�
		m_pThreadOwner = pOwner;
		m_pTargetFunc = pTargetFunc;
		m_pThreadParam = pThreadParam;
		
		// �V�O�i���N���A
		m_bSyncSignal = false;
		m_bKillSignal = false;
		
		// �X���b�h�N��
		if(!(m_hThread = ::CreateThread(NULL, 0UL, CSmartThread::ThreadFunc, static_cast<LPVOID>(this), (bStart)? 0UL : CREATE_SUSPENDED, &m_dwThreadID))){
			// �X���b�h�N�����s
			return false;
			}

		// �X���b�h�N�������҂�
		if(bStart){
			while(!m_bSyncSignal)::Sleep(0);
			}	

		return true;
	}

	const bool EndThread(const bool bPumpMessage = false)
	{
		if(!m_hThread)return false;
	
		// �I���V�O�i���Z�b�g
		m_bKillSignal = true;

		// �X���b�h�̒�����Ăяo�����ꍇ�͊J�����Ȃ�
		if(::GetCurrentThreadId() == m_dwThreadID)return true;
	
		// �T�X�y���h��Ԃ̂Ƃ��̓��W���[��
		::ResumeThread(m_hThread);

		// �I����҂�
		while(m_bSyncSignal){
			if(!bPumpMessage)::Sleep(0);
			else PumpMessage();
			}

		// �X���b�h�J��
		::CloseHandle(m_hThread);
		m_hThread = NULL;

		return true;	
	}
	
	const bool SuspendThread(void)
	{
		if(!m_hThread)false;

		// �X���b�h�T�X�y���h
		::SuspendThread(m_hThread);
	
		return true;	
	}
	
	const bool ResumeThread(void)
	{
		if(!m_hThread)false;

		// �X���b�h���W���[��
		::ResumeThread(m_hThread);
	
		return true;	
	}

	const HANDLE GetThreadHandle(void) const
	{
		// �X���b�h�n���h����Ԃ�
		return m_hThread;	
	}
	
	const DWORD GetThreadID(void) const
	{
		// �X���b�hID��Ԃ�
		return m_dwThreadID;
	}

	const bool IsRunning(void) const
	{
		// �X���b�h���s���L����Ԃ�
		return m_bSyncSignal;
	}
	
protected:
	static DWORD WINAPI ThreadFunc(LPVOID pParam)
	{
		CSmartThread *pThis = static_cast<CSmartThread *>(pParam);

		// �T�X�y���h��ԂŏI��
		if(pThis->m_bKillSignal){
			pThis->m_bSyncSignal = false;
			return 0UL;	
			}

		// �N���V�O�i���Z�b�g
		pThis->m_bSyncSignal = true;

		// �X���b�h���\�b�h���s
		((pThis->m_pThreadOwner)->*(pThis->m_pTargetFunc))(pThis, pThis->m_bKillSignal, pThis->m_pThreadParam);

		// �N���V�O�i���N���A
		pThis->m_bSyncSignal = false;

		return 0UL;	
	}

	static void PumpMessage(void)
	{
		MSG Msg;

		// ���b�Z�[�W�|���v���쓮����
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
