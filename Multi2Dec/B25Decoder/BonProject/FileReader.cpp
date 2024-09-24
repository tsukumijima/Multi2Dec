// FileReader.cpp: �t�@�C�����[�_�f�R�[�_
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "FileReader.h"


/////////////////////////////////////////////////////////////////////////////
// �t�@�C�����[�J���萔�ݒ�
/////////////////////////////////////////////////////////////////////////////

// �f�t�H���g�̃o�b�t�@�T�C�Y
#define DEF_BUFFSIZE	0x100000UL							// 1MB
#define DEF_BUFFNUM		8UL									// 250ms  ����


/////////////////////////////////////////////////////////////////////////////
// �t�@�C�����[�_�f�R�[�_
/////////////////////////////////////////////////////////////////////////////

const bool CFileReader::OpenFile(LPCTSTR lpszFileName, const bool bAsyncRead, const DWORD dwBuffSize)
{
	CloseFile();

	try{
		// �o�b�t�@�T�C�Y�v�Z
		m_dwBuffSize = (dwBuffSize)? dwBuffSize : DEF_BUFFSIZE;
		const DWORD dwReadNum = (bAsyncRead)? DEF_BUFFNUM : 1UL;

		// �X�}�[�g�t�@�C���C���X�^���X����
		if(!(m_pFile = ::BON_SAFE_CREATE<ISmartFile *>(TEXT("CSmartFile"))))throw __LINE__;

		// �t�@�C���I�[�v��
		if(!m_pFile->Open(lpszFileName, ISmartFile::MF_READ, m_dwBuffSize))throw __LINE__;

		// �t�@�C���T�C�Y�`�F�b�N
		if(!m_pFile->GetLength())throw __LINE__;

		// ���f�B�A�v�[���T�C�Y���T�C�Y
		m_MediaPool.resize(dwReadNum);
		if(m_MediaPool.size() < dwReadNum)throw __LINE__;
		::ZeroMemory(&m_MediaPool[0], sizeof(IMediaData *) * dwReadNum);
			
		// ���f�B�A�f�[�^�C���X�^���X�m��
		for(DWORD dwIndex = 0UL ; dwIndex < dwReadNum ; dwIndex++){
			m_MediaPool[dwIndex] = ::BON_SAFE_CREATE<IMediaData *>(TEXT("CMediaData"));
			if(!m_MediaPool[dwIndex])throw __LINE__;
				
			// �o�b�t�@�m��
			m_MediaPool[dwIndex]->SetSize(m_dwBuffSize);
			}
			
		// ���f�B�A�v�[���擪�C�e���[�^�Z�b�g
		m_itFreeMedia = m_MediaPool.begin();			
		}
	catch(...){
		// �G���[����
		CloseFile();
		return false;
		}
		
	return true;
}

bool CFileReader::CloseFile(void)
{
	// �f�R�[�_�O���t���~
	StopDecoder();

	// �t�@�C���N���[�Y
	if(m_pFile)m_pFile->Close();

	// �X�}�[�g�t�@�C���C���X�^���X�J��
	BON_SAFE_RELEASE(m_pFile);

	// ���f�B�A�L���[�N���A
	while(!m_MediaQue.empty())m_MediaQue.pop();

	// ���f�B�A�f�[�^�C���X�^���X�J��
	for(DWORD dwIndex = 0UL ; dwIndex < m_MediaPool.size() ; dwIndex++){
		BON_SAFE_RELEASE(m_MediaPool[dwIndex]);		
		}
	
	// ���f�B�A�v�[���J��
	m_MediaPool.clear();

	return true;
}

const DWORD CFileReader::ReadSync(const DWORD dwReadSize)
{
	if(!m_pFile || !dwReadSize)return 0UL;
	
	// �v���T�C�Y�m��
	if((*m_itFreeMedia)->SetSize(dwReadSize) != dwReadSize)return 0UL;

	// �f�[�^�ǂݍ���
	const DWORD dwActualSize = m_pFile->ReadData((*m_itFreeMedia)->GetData(), dwReadSize);
	if(!dwActualSize)return 0UL;

	// �L���T�C�Y�ݒ�
	if((*m_itFreeMedia)->SetSize(dwActualSize) != dwActualSize)return 0UL;

	// ���f�B�A�f�[�^�o��
	return OutputMedia(*m_itFreeMedia)? dwActualSize : 0UL;
}

const bool CFileReader::SetReadPos(const ULONGLONG llPos)
{
	CBlockLock AutoLock(m_pLock);
	
	// �t�@�C���V�[�N
	return (m_pFile)? m_pFile->SeekPos(llPos) : false;
}

const ULONGLONG CFileReader::GetReadPos(void)
{
	CBlockLock AutoLock(m_pLock);
	
	// �t�@�C���|�W�V������Ԃ�
	return (m_pFile)? m_pFile->GetPos() : 0ULL;
}

const ULONGLONG CFileReader::GetFileLength(void)
{
	CBlockLock AutoLock(m_pLock);
	
	// �t�@�C���T�C�Y��Ԃ�
	return (m_pFile)? m_pFile->GetLength() : 0ULL;
}

CFileReader::CFileReader(IBonObject *pOwner)
	: CMediaDecoder(pOwner, 0UL, 1UL, new CSmartLock)
	, m_pFile(NULL)
	, m_dwBuffSize(DEF_BUFFSIZE)
{
	// �������Ȃ�
}

CFileReader::~CFileReader(void)
{
	// �t�@�C�������
	CloseFile();
}

const bool CFileReader::OnPlay(void)
{
	OnStop();
	
	// ���f�B�A�L���[�N���A
	while(!m_MediaQue.empty())m_MediaQue.pop();
	
	// ���f�B�A�v�[���擪�C�e���[�^�Z�b�g
	m_itFreeMedia = m_MediaPool.begin();

	// �X���b�h�J�n
	if(m_MediaPool.size()){
		// �o�̓X���b�h�J�n
		if(!m_OutputThread.StartThread(this, &CFileReader::OutputThread, NULL, false))return false;
		
		// �ǂݍ��݃X���b�h�J�n
		if(!m_ReadThread.StartThread(this, &CFileReader::ReadThread)){
			m_OutputThread.EndThread(true);
			}
		}
		
	return true;
}

const bool CFileReader::OnStop(void)
{
	// �ǂݍ��݃X���b�h�I��
	m_ReadThread.EndThread(true);

	// �o�̓X���b�h�I��
	m_OutputThread.EndThread(true);
	
	return true;
}

void CFileReader::ReadThread(CSmartThread<CFileReader> *pThread, bool &bKillSignal, PVOID pParam)
{
	DWORD dwIdleNum;

	// �J�n�C�x���g�ʒm
	SendDecoderEvent(IFileReader::EID_READ_START);

	while(!bKillSignal){
		// �L���[�ɋ󂫂��������t�@�C����ǂݍ���
		while(!bKillSignal){
			// �A�C�h���L���[���擾
			m_pLock->Lock();
			dwIdleNum = m_MediaPool.size() - m_MediaQue.size();
			m_pLock->Unlock();
			if(!dwIdleNum)break;

			// �ǂݍ��ݑO�C�x���g�ʒm
			SendDecoderEvent(IFileReader::EID_PRE_READ);

			// �t�@�C���ǂݍ���
			if(!ReadAsync(m_dwBuffSize)){
			
				// �t�@�C���I�[�A�o�̓L���[�̃f�[�^���S�ď��������܂ő҂�
				while(m_OutputThread.IsRunning()){
					// �������L���[���擾
					m_pLock->Lock();
					dwIdleNum = m_MediaQue.size();
					m_pLock->Unlock();
					if(!dwIdleNum)break;
					
					// �X���b�h�T�X�y���h�A�L���[�ɋ󂫂��o����܂ő҂�
					pThread->SuspendThread();
					}

				bKillSignal = true;
				break;
				}

			// �ǂݍ��݌�C�x���g�ʒm
			SendDecoderEvent(IFileReader::EID_POST_READ);

			// �L���[�ɒǉ�
			m_pLock->Lock();
			m_MediaQue.push(*m_itFreeMedia);
			if(++m_itFreeMedia == m_MediaPool.end())m_itFreeMedia = m_MediaPool.begin();
			m_pLock->Unlock();
			
			// �o�̓X���b�h���W���[��
			m_OutputThread.ResumeThread();
			}

		// �X���b�h�T�X�y���h�A�L���[�ɋ󂫂��o����܂ő҂�
		if(!bKillSignal)pThread->SuspendThread();
		}

	// �I���C�x���g�ʒm
	SendDecoderEvent(IFileReader::EID_READ_END);
}

void CFileReader::OutputThread(CSmartThread<CFileReader> *pThread, bool &bKillSignal, PVOID pParam)
{
	IMediaData *pMediaData;
	
	while(!bKillSignal){
		while(!bKillSignal){
			// �o�͑҂��L���[���擾
			m_pLock->Lock();
			pMediaData = (!m_MediaQue.empty())? m_MediaQue.front() : NULL;
			m_pLock->Unlock();
			if(!pMediaData)break;
			
			// ���f�B�A�f�[�^�o��
			OutputMedia(pMediaData);

			// �L���[���o��
			m_pLock->Lock();
			m_MediaQue.pop();
			m_pLock->Unlock();
			
			// �ǂݍ��݃X���b�h���W���[��
			m_ReadThread.ResumeThread();
			}
		
		// �X���b�h�T�X�y���h�A�L���[�Ƀf�[�^������܂ő҂�
		if(!bKillSignal)pThread->SuspendThread();
		}
}

const DWORD CFileReader::ReadAsync(const DWORD dwReadSize)
{
	if(!m_pFile)return 0UL;
	
	// �v���T�C�Y�m��
	if((*m_itFreeMedia)->SetSize(dwReadSize) != dwReadSize)return 0UL;

	// �f�[�^�ǂݍ���
	const DWORD dwActualSize = m_pFile->ReadData((*m_itFreeMedia)->GetData(), m_dwBuffSize);
	if(!dwActualSize)return 0UL;

	// �L���T�C�Y�ݒ�
	if((*m_itFreeMedia)->SetSize(dwActualSize) != dwActualSize)return 0UL;

	return dwActualSize;
}
