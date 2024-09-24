// FileWriter.cpp: �t�@�C�����C�^�f�R�[�_
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "FileWriter.h"


/////////////////////////////////////////////////////////////////////////////
// �t�@�C�����[�J���萔�ݒ�
/////////////////////////////////////////////////////////////////////////////

// �f�t�H���g�̃o�b�t�@�T�C�Y
#define DEF_BUFFSIZE	0x100000UL							// 1MB
#define DEF_BUFFNUM		(DEF_BUFFSIZE / TS_PACKET_SIZE)		// 1MB ����


/////////////////////////////////////////////////////////////////////////////
// �t�@�C�����C�^�f�R�[�_
/////////////////////////////////////////////////////////////////////////////

const bool CFileWriter::InputMedia(IMediaData *pMediaData, const DWORD dwInputIndex)
{
	if(!pMediaData || (dwInputIndex >= GetInputNum()) || !m_pFile)return false;

	// �t�@�C����������
	return (m_WriteThread.IsRunning())? WriteAsync(pMediaData) : WriteSync(pMediaData);
}

const bool CFileWriter::OpenFile(LPCTSTR lpszFileName, const bool bAsyncWrite, const DWORD dwBuffSize)
{
	CloseFile();

	try{
		CBlockLock AutoLock(m_pLock);

		// �o�b�t�@�T�C�Y�v�Z
		DWORD dwWriteSize;
		DWORD dwWriteNum;
		
		if(!bAsyncWrite){
			// ������������
			dwWriteSize = (dwBuffSize)? dwBuffSize : DEF_BUFFSIZE;
			dwWriteNum	= 0UL;
			}
		else{
			// �񓯊���������
			dwWriteSize = DEF_BUFFSIZE;
			dwWriteNum = (dwBuffSize)? dwBuffSize : DEF_BUFFNUM;
			}
	
		// �X�}�[�g�t�@�C���C���X�^���X����
		if(!(m_pFile = ::BON_SAFE_CREATE<ISmartFile *>(TEXT("CSmartFile"))))throw __LINE__;

		// �t�@�C���I�[�v��
		if(!m_pFile->Open(lpszFileName, ISmartFile::MF_WRITE, dwWriteSize))throw __LINE__;

		// ���f�B�A�v�[���m��
		if(dwWriteNum){
			// ���f�B�A�v�[���T�C�Y���T�C�Y
			m_MediaPool.resize(dwWriteNum);
			if(m_MediaPool.size() < dwWriteNum)throw __LINE__;
			::ZeroMemory(&m_MediaPool[0], sizeof(IMediaData *) * dwWriteNum);
			
			// ���f�B�A�f�[�^�C���X�^���X�m��
			for(DWORD dwIndex = 0UL ; dwIndex < dwWriteNum ; dwIndex++){
				m_MediaPool[dwIndex] = ::BON_SAFE_CREATE<IMediaData *>(TEXT("CMediaData"));
				if(!m_MediaPool[dwIndex])throw __LINE__;
				}
			
			// ���f�B�A�v�[���擪�C�e���[�^�Z�b�g
			m_itFreeMedia = m_MediaPool.begin();
			}
		}
	catch(...){
		// �G���[����
		CloseFile();
		return false;
		}
		
	return true;
}

bool CFileWriter::CloseFile(void)
{
	CBlockLock AutoLock(m_pLock);

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

const ULONGLONG CFileWriter::GetFileLength(void)
{
	// �t�@�C���T�C�Y��Ԃ�
	return (m_pFile)? m_pFile->GetLength() : 0ULL;
}

CFileWriter::CFileWriter(IBonObject *pOwner)
	: CMediaDecoder(pOwner, 1UL, 0UL, new CSmartLock)
	, m_pFile(NULL)
{
	// �������Ȃ�
}

CFileWriter::~CFileWriter(void)
{
	// �t�@�C�������
	CloseFile();
}

const bool CFileWriter::OnPlay(void)
{
	OnStop();

	// ���f�B�A�L���[�N���A
	while(!m_MediaQue.empty())m_MediaQue.pop();
	
	// ���f�B�A�v�[���擪�C�e���[�^�Z�b�g
	m_itFreeMedia = m_MediaPool.begin();

	// �X���b�h�J�n(�T�X�y���h��ԂŊJ�n)
	return (m_MediaPool.size())? m_WriteThread.StartThread(this, &CFileWriter::WriteThread, NULL, false) : true;
}

const bool CFileWriter::OnStop(void)
{
	// �X���b�h�I��
	m_WriteThread.EndThread(true);

	return true;
}

const bool CFileWriter::WriteSync(const IMediaData *pMediaData)
{
	// �f�[�^�擾
	const BYTE * const pData = pMediaData->GetData();
	const DWORD dwSize = pMediaData->GetSize();
	if(!pData || !dwSize)return true;

	// �t�@�C����������
	if(m_pFile->WriteData(pData, dwSize) != dwSize){
		CloseFile();
		return false;
		}
	
	return true;
}

const bool CFileWriter::WriteAsync(const IMediaData *pMediaData)
{
	DWORD dwIdleNum;

	// �f�[�^�`�F�b�N
	const BYTE * const pData = pMediaData->GetData();
	const DWORD dwSize = pMediaData->GetSize();
	if(!pData || !dwSize)return true;

	// �A�C�h���L���[���擾
	m_pLock->Lock();
	dwIdleNum = m_MediaPool.size() - m_MediaQue.size();
	m_pLock->Unlock();	

	// �L���[���t���̏ꍇ�̏���
	if(!dwIdleNum){
		// �C�x���g�ʒm
		if(SendDecoderEvent(EID_BUFF_OVERFLOW)){
			// �L���[���󂭂܂Ńu���b�N����
			do{
				::Sleep(1UL);

				// �|�[�����O
				m_pLock->Lock();
				dwIdleNum = m_MediaPool.size() - m_MediaQue.size();
				m_pLock->Unlock();	
				}
			while(!dwIdleNum);			
			}
		else{
			// �f�t�H���g�̏����A�Â��f�[�^���̂Ă�
			m_pLock->Lock();
			m_MediaQue.pop();		
			m_pLock->Unlock();
			}		
		}

	// �f�[�^�R�s�[
	if((*m_itFreeMedia)->CopyData(pMediaData) != dwSize)return false;

	// ���f�B�A�v�[���ɋ󂫂�����Ƃ��A�L���[�ɓ���
	m_pLock->Lock();
	m_MediaQue.push(*m_itFreeMedia);
	if(++m_itFreeMedia == m_MediaPool.end())m_itFreeMedia = m_MediaPool.begin();
	m_pLock->Unlock();	

	// �������݃L���[���W���[��
	m_WriteThread.ResumeThread();

	return true;
}

void CFileWriter::WriteThread(CSmartThread<CFileWriter> *pThread, bool &bKillSignal, PVOID pParam)
{
	const IMediaData *pMediaData;

	while(!bKillSignal){
		// �t�@�C����������
		while(1){
			// �L���[�`�F�b�N
			m_pLock->Lock();
			pMediaData = (!m_MediaQue.empty())? m_MediaQue.front() : NULL;
			m_pLock->Unlock();
			if(!pMediaData)break;

			// �t�@�C����������
			WriteSync(pMediaData);

			// �L���[���o��
			m_pLock->Lock();
			m_MediaQue.pop();
			m_pLock->Unlock();	
			}

		// �L���[�Ƀf�[�^����������܂ŃT�X�y���h
		if(!bKillSignal)pThread->SuspendThread();
		}
}
