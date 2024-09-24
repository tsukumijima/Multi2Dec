// SyncObject.cpp: �����I�u�W�F�N�g���b�p�[�N���X
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "SyncObject.h"
#include <TChar.h>


//////////////////////////////////////////////////////////////////////
// �N���e�B�J���Z�N�V�������b�p�[�N���X
//////////////////////////////////////////////////////////////////////

CSmartLock::CSmartLock()
{
	// �N���e�B�J���Z�N�V����������
	::InitializeCriticalSection(&m_CriticalSection);
}

CSmartLock::~CSmartLock()
{
	// �N���e�B�J���Z�N�V�����폜
	::DeleteCriticalSection(&m_CriticalSection);
}

void CSmartLock::Lock(void)
{
	// �N���e�B�J���Z�N�V�����擾
	::EnterCriticalSection(&m_CriticalSection);
}

void CSmartLock::Unlock(void)
{
	// �N���e�B�J���Z�N�V�����J��
	::LeaveCriticalSection(&m_CriticalSection);
}
	
	
//////////////////////////////////////////////////////////////////////
// �u���b�N�X�R�[�v���b�N�N���X
//////////////////////////////////////////////////////////////////////
	
CBlockLock::CBlockLock(CSmartLock *pSmartLock)
	: m_pSmartLock(pSmartLock)
{
	// ���b�N�擾
	if(m_pSmartLock)m_pSmartLock->Lock();
}

CBlockLock::~CBlockLock()
{
	// ���b�N�J��
	if(m_pSmartLock)m_pSmartLock->Unlock();
}


//////////////////////////////////////////////////////////////////////
// �C�x���g���b�p�[�N���X
//////////////////////////////////////////////////////////////////////

CSmartEvent::CSmartEvent(const bool bManualReset, const bool bInitialState)
	: m_hEvent(NULL)
{
	// �C�x���g�쐬
	m_hEvent = ::CreateEvent(NULL, (bManualReset)? TRUE : FALSE, (bInitialState)? TRUE : FALSE, NULL);
}

CSmartEvent::~CSmartEvent(void)
{
	// �C�x���g�J��
	if(m_hEvent)::CloseHandle(m_hEvent);
}

const bool CSmartEvent::SetEvent(void)
{
	// �C�x���g�Z�b�g
	return (::SetEvent(m_hEvent))? true : false;
}

const bool CSmartEvent::PulseEvent(void)
{
	// �C�x���g�Z�b�g(1�̃X���b�h�̂݊J��)
	return (::PulseEvent(m_hEvent))? true : false;
}
const bool CSmartEvent::ResetEvent(void)
{
	// �C�x���g���Z�b�g
	return (::ResetEvent(m_hEvent))? true : false;
}

const DWORD CSmartEvent::WaitEvent(const DWORD dwTimeout)
{
	// �C�x���g�҂�
	switch(::WaitForSingleObject(m_hEvent, (dwTimeout)? dwTimeout : INFINITE)){
		case WAIT_ABANDONED :	return WE_CLOSED;
		case WAIT_OBJECT_0	:	return WE_TRIGGER;
		case WAIT_TIMEOUT	:	return WE_TIMEOUT;
		default				:	return WE_CLOSED;
		}
}


/////////////////////////////////////////////////////////////////////////////
// �~���[�e�b�N�X���b�p�[�N���X
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
	// �����ɂ͔�Ή�(�v���Z�X�Ԃ̔r������̂�)
	if(!lpszName)return false;
	if(!::_tcslen(lpszName))return false;

	// ���ɃI�[�v���ς݂̏ꍇ�̓G���[
	if(m_hMutex)return false;
	
	// Mutex�쐬
	if(!(m_hMutex = ::CreateMutex(NULL, TRUE, lpszName)))return false;
	
	// ���̃v���Z�X�ŃI�[�v������Ă���ꍇ�̓G���[
	if(::GetLastError() == ERROR_ALREADY_EXISTS){
		Close();
		return false;
		}
	
	return true;
}

const bool CSmartMutex::Close(void)
{
	if(!m_hMutex)return false;
	
	// Mutex�����
	const BOOL bReturn = ::CloseHandle(m_hMutex);
	m_hMutex = NULL;
	
	return (bReturn)? true : false;
}

const bool CSmartMutex::IsExist(LPCTSTR lpszName)
{
	// �w�肵�����O��Mutex���I�[�v������
	HANDLE hMutex = ::OpenMutex(MUTEX_ALL_ACCESS, FALSE, lpszName);
	
	if(hMutex){
		// ����Mutex�͑��݂���
		::CloseHandle(hMutex);
		return true;
		}
	else{
		// Mutex�͑��݂��Ȃ�
		return false;
		}	
}
