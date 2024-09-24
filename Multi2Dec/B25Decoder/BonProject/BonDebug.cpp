// BonDebug.cpp: �f�o�b�O�⏕���C�u����
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include <TChar.h>
#include <StdLib.h>


/////////////////////////////////////////////////////////////////////////////
// �ėp��O�N���X
/////////////////////////////////////////////////////////////////////////////

CBonException::CBonException(LPCSTR lpszSourceFile, const DWORD dwSourceLine, const BONGUID ClassId)
	: m_ClassId(ClassId)
	, m_lpszSourceFile(lpszSourceFile)
	, m_dwSourceLine(dwSourceLine)
{
	// ������������
	::_tcscpy(m_szDescription, TEXT("��O�Ɋւ�������͂���܂���"));
}

CBonException::~CBonException(void)
{
	// �������Ȃ�
}

CBonException & CBonException::operator( )(LPCTSTR lpszFormat, ...)
{
	// ������������
	va_list Args;
	va_start(Args ,lpszFormat);
	::_vstprintf(m_szDescription, lpszFormat, Args);
	va_end(Args);
	
	return *this;
}

void CBonException::Notify(const bool bEnable)
{
	// ��O��ʒm����
	TCHAR szOutput[1024] = TEXT("");
	TCHAR szSource[1024] = TEXT("�s���ȃ\�[�X�t�@�C��");
	
	// �\�[�X�t�@�C�������擾
	GetSourceFile(szSource);
	
	if(m_ClassId != ::GetGuid()){
		TCHAR szModuleName[BON_MAX_MODULE_NAME] = TEXT("�s����");
		TCHAR szClassName[BON_MAX_CLASS_NAME]	= TEXT("�s����");
		
		// �N���X�����擾
		::GET_BON_CLASS_NAME(m_ClassId, szClassName);
		
		// ���W���[�������擾
		::GET_BON_MODULE_NAME(::GET_BON_CLASS_MODULE(m_ClassId), szModuleName);
		
		// �f�o�b�O�o�͕����񐶐�
		::_stprintf(szOutput, TEXT("[��O�C�x���g]\r\n�@�E�ꏊ\t�F %s���W���[���@%s�N���X\r\n�@�E�\�[�X\t�F %s�@%lu �s��\r\n�@�E����\t�F %s\r\n"), szModuleName, szClassName, szSource, m_dwSourceLine, m_szDescription);
		}
	else{
		// �f�o�b�O�o�͕����񐶐�
		::_stprintf(szOutput, TEXT("[��O�C�x���g]\r\n�@�E�ꏊ\t�F �O���[�o�����͔�Bon�N���X\r\n�@�E�\�[�X\t�F %s�@%lu �s��\r\n�@�E����\t�F %s\r\n"), szSource, m_dwSourceLine, m_szDescription);
		}

	// �f�o�b�O�o��
	::OutputDebugString(szOutput);

	// �C�x���g���O�ɏo��
		// �܂�������

	// ���[�U�ɒʒm
	if(bEnable){
		::_tcscat(szOutput, TEXT("\r\n\r\n��O�𖳎����ăv���O�����̎��s���p�����܂����H"));

		if(::MessageBox(NULL, szOutput, TEXT("BonSDK ��O�ʒm"), MB_YESNO | MB_ICONERROR | MB_TASKMODAL) == IDYES){
			// ���s���p��
			return;
			}

		// �f�o�b�O�u���[�N
		::DebugBreak();
		}
}

const BONGUID CBonException::GetGuid(void)
{
	// ��O��������BONGUID��Ԃ�
	return m_ClassId;
}

const DWORD CBonException::GetDescription(LPTSTR lpszDescription)
{
	// ��O�������̐�����Ԃ�
	if(lpszDescription)::_tcscpy(lpszDescription, m_szDescription);

	// ��������Ԃ�
	return ::_tcslen(m_szDescription);
}

const DWORD CBonException::GetSourceFile(LPTSTR lpszSourceFile)
{
	// ��O�������̃\�[�X�t�@�C������Ԃ�
	TCHAR szPathName[_MAX_PATH] = TEXT("");
	TCHAR szFileName[_MAX_FNAME] = TEXT("");
	TCHAR szExtName[_MAX_EXT] = TEXT("");

#ifdef _UNICODE
	// UNICODE�ɕϊ�
	::MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, m_lpszSourceFile, -1, szPathName, sizeof(szPathName));
#else
	// ���̂܂܃R�s�[
	::_tcscpy(szPathName, m_lpszSourceFile);
#endif

	// �t�@�C�����𒊏o
	::_tsplitpath(szPathName, NULL, NULL, szFileName, szExtName);
	::_tmakepath(szPathName, NULL, NULL, szFileName, szExtName);	

	if(lpszSourceFile)::_tcscpy(lpszSourceFile, szPathName);

	// ��������Ԃ�
	return ::_tcslen(szPathName);
}

const DWORD CBonException::GetSourceLine(void)
{
	// ��O�������̃\�[�X�R�[�h�s��Ԃ�
	return m_dwSourceLine;
}


/////////////////////////////////////////////////////////////////////////////
// �ėp�ƍ��N���X
/////////////////////////////////////////////////////////////////////////////

CBonAssert::CBonAssert(LPCSTR lpszSourceFile, const DWORD dwSourceLine, const BONGUID ClassId)
	: m_lpszSourceFile(lpszSourceFile)
	, m_dwSourceLine(dwSourceLine)
	, m_ClassId(ClassId)
{
	// �������Ȃ�
}

void CBonAssert::operator( )(const bool bSuccess, LPCTSTR lpszFormat, ...)
{
	// ��������
	if(!bSuccess){
		// ��O�I�u�W�F�N�g����
		CBonException Exception(m_lpszSourceFile, m_dwSourceLine, m_ClassId);
		
		if(lpszFormat){
			TCHAR szDescription[1024] = TEXT("");

			// �t�H�[�}�b�g��������
			va_list Args;
			va_start(Args , lpszFormat);
			::_vstprintf(szDescription, lpszFormat, Args);
			va_end(Args);
			Exception(szDescription);
			}

		// ��O�𔭐�������
		Exception.Notify();
		}
}

void CBonAssert::operator( )(const void *pPointer, LPCTSTR lpszFormat, ...)
{
	// �|�C���^��
	if(!pPointer){
		// ��O�I�u�W�F�N�g����
		CBonException Exception(m_lpszSourceFile, m_dwSourceLine, m_ClassId);
		
		if(lpszFormat){
			TCHAR szDescription[1024] = TEXT("");

			// �t�H�[�}�b�g��������
			va_list Args;
			va_start(Args , lpszFormat);
			::_vstprintf(szDescription, lpszFormat, Args);
			va_end(Args);
			Exception(szDescription);
			}

		// ��O�𔭐�������
		Exception.Notify();
		}
}


/////////////////////////////////////////////////////////////////////////////
// �f�o�b�O�⏕�֐�
/////////////////////////////////////////////////////////////////////////////

const BONGUID GetGuid(void)
{
	// �O���[�o���X�R�[�v�̏ꍇ��BCID_NULL��Ԃ�
	return BCID_NULL;
}

void BON_TRACE(LPCTSTR lpszFormat, ...)
{
	TCHAR szOutput[1024] = TEXT("");

	// ���b�Z�[�W��������
	va_list Args;
	va_start(Args , lpszFormat);
	::_vstprintf(szOutput, lpszFormat, Args);
	va_end(Args);

	// �f�o�b�O�o�͂ɏ�������
	::OutputDebugString(szOutput);
}
