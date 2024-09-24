// SyncObject.h: �����I�u�W�F�N�g���b�p�[�N���X
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// �N���e�B�J���Z�N�V�������b�p�[�N���X
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
// �u���b�N�X�R�[�v���b�N�N���X
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
// �C�x���g���b�p�[�N���X
/////////////////////////////////////////////////////////////////////////////

class CSmartEvent
{
public:
	enum
	{
		WE_TRIGGER,		// �C�x���g���V�O�i����ԂɂȂ���
		WE_TIMEOUT,		// �^�C���A�E�g����
		WE_CLOSED		// �C�x���g�͊��ɊJ������Ă���
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
// �~���[�e�b�N�X���b�p�[�N���X
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
