// SmartFile.cpp: �ėp�t�@�C���X�g���[�W�N���X
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "SmartFile.h"
#include <StdIo.h>
#include <Share.h >


/////////////////////////////////////////////////////////////////////////////
// �ėp�t�@�C���X�g���[�W�N���X
/////////////////////////////////////////////////////////////////////////////

// �f�t�H���g�̃o�b�t�@�T�C�Y
#define DEF_BUFFSIZE	0x100000UL	// 1MB


const bool CSmartFile::Open(LPCTSTR lpszFileName, const DWORD dwModeFlags, const DWORD dwBuffSize)
{
	Close();
	
	TCHAR szOpenMode[16] = {TEXT('\0')};
	int iShareFlags = 0;

	// ���[�h�̃`�F�b�N(�ǂݏ��������ɂ͑Ή����Ă��Ȃ�)
	switch(dwModeFlags){
		case MF_READ :
			::_tcscpy(szOpenMode, TEXT("rb"));
			iShareFlags = _SH_DENYNO;
			break;
			
		case MF_WRITE :
			::_tcscpy(szOpenMode, TEXT("wb"));
			iShareFlags = _SH_DENYWR;		
			break;
		
		default :
			return false;
		}

	// �t�@�C���X�g���[���I�[�v��
	if(!(m_pFileStream = ::_tfsopen(lpszFileName, szOpenMode, iShareFlags)))return false;
	
	// �o�b�t�@�T�C�Y�ݒ�
	if(::setvbuf(m_pFileStream, NULL, _IOFBF, (dwBuffSize)? dwBuffSize : DEF_BUFFSIZE)){
		Close();
		return false;
		}

	return true;
}

void CSmartFile::Close(void)
{
	// �f�[�^�t���b�V��
	FlushData();
	
	// �t�@�C���N���[�Y
	if(m_pFileStream){
		::fclose(m_pFileStream);
		m_pFileStream = NULL;
		}
}

const DWORD CSmartFile::ReadData(BYTE *pBuff, const DWORD dwLen)
{
	if(!m_pFileStream || !pBuff || !dwLen)return 0UL;
	
	// �t�@�C���ǂݎ��
	const DWORD dwReturn = ::_fread_nolock(pBuff, 1UL, dwLen, m_pFileStream);
	
	// �G���[or�t�@�C���I�[�`�F�b�N
	if(dwReturn != dwLen){
		if(::ferror(m_pFileStream)){
			::clearerr(m_pFileStream);
			return 0UL;
			}
		}

	// �ǂݎ�����o�C�g����Ԃ�
	return dwReturn;
}

const DWORD CSmartFile::ReadData(BYTE *pBuff, const DWORD dwLen, const ULONGLONG llPos)
{
	// �t�@�C���|�W�V�����V�[�N
	if(!SeekPos(llPos))return 0UL;

	// �t�@�C���ǂݎ��
	return ReadData(pBuff, dwLen);
}

const DWORD CSmartFile::WriteData(const BYTE *pBuff, const DWORD dwLen)
{
	if(!m_pFileStream || !pBuff || !dwLen)return 0UL;

	// �t�@�C����������
	const DWORD dwReturn = ::_fwrite_nolock(pBuff, 1UL, dwLen, m_pFileStream);

	// �G���[�`�F�b�N
	if(dwReturn != dwLen){
		::clearerr(m_pFileStream);
		return 0UL;
		}

	// �������񂾃o�C�g����Ԃ�
	return dwReturn;
}

const DWORD CSmartFile::WriteData(const BYTE *pBuff, const DWORD dwLen, const ULONGLONG llPos)
{
	// �t�@�C���|�W�V�����V�[�N
	if(!SeekPos(llPos))return 0UL;

	// �t�@�C����������
	return WriteData(pBuff, dwLen);
}

const bool CSmartFile::FlushData(void)
{
	if(!m_pFileStream)return false;

	// �t�@�C���t���b�V��
	return (!::fflush(m_pFileStream))? true : false;
}

const bool CSmartFile::SeekPos(const ULONGLONG llPos)
{
	if(!m_pFileStream)return false;

	// �t�@�C���|�W�V�����V�[�N
	return (!::_fseeki64(m_pFileStream, llPos, SEEK_SET))? true : false;
}

const ULONGLONG CSmartFile::GetPos(void)
{
	if(!m_pFileStream)return 0ULL;

	// �t�@�C���|�W�V�����擾
	const __int64 iReturn = ::_ftelli64(m_pFileStream);

	return (iReturn != -1LL)? (ULONGLONG)iReturn : 0ULL;
}

const ULONGLONG CSmartFile::GetLength(void)
{
	if(!m_pFileStream)return 0ULL;
	
	// ���݂̃t�@�C���|�W�V�����擾
	const __int64 iCurPos = ::_ftelli64(m_pFileStream);
	if(iCurPos == -1LL)return 0ULL;

	// �t�@�C���|�W�V�������I�[�Ɉړ�
	if(::_fseeki64(m_pFileStream, 0LL, SEEK_END))return 0ULL;
	
	// �I�[�̃t�@�C���|�W�V�����擾
	const __int64 iEndPos = ::_ftelli64(m_pFileStream);

	// �t�@�C���|�W�V���������ɖ߂�
	if(::_fseeki64(m_pFileStream, iCurPos, SEEK_SET))return 0ULL;

	// �t�@�C���T�C�Y��Ԃ�
	return (iEndPos != -1LL)? (ULONGLONG)iEndPos : 0ULL;
}

CSmartFile::CSmartFile(IBonObject *pOwner)
	: CBonObject(pOwner)
	, m_pFileStream(NULL)
{
	// �������Ȃ�
}

CSmartFile::~CSmartFile(void)
{
	// �t�@�C�������
	Close();
}
