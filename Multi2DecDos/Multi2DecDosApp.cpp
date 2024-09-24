// Multi2DecDosApp.cpp: Multi2DecDos�A�v���P�[�V�����N���X
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "Multi2DecDosApp.h"
#include <Share.h >


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// Multi2DecDos�A�v���P�[�V�����N���X
/////////////////////////////////////////////////////////////////////////////

CMulti2DecDosApp::CMulti2DecDosApp(IBonObject * const pOwner)
	: CBonObject(pOwner)
	, m_pMulti2Converter(NULL)
{
	// �C���X�^���X����
	m_pMulti2Converter = ::BON_SAFE_CREATE<IMulti2Converter *>(TEXT("CMulti2Converter"), dynamic_cast<IMulti2Converter::IHandler *>(this));
}

CMulti2DecDosApp::~CMulti2DecDosApp(void)
{
	// �C���X�^���X�J��
	BON_SAFE_RELEASE(m_pMulti2Converter);
}

const bool CMulti2DecDosApp::Run(const CONFIGSET *pConfigSet)
{
	// �ݒ��ۑ�
	m_ConfigSet = *pConfigSet;

	if(!m_pMulti2Converter){
		// �C���X�^���X�������s
		::printf("�G���[: bon_TsConverter.dll�����[�h�ł��܂���B\n");
		return false;
		}

	// �R���o�[�g�J�n
	const DWORD dwErrorCode = m_pMulti2Converter->StartConvert(pConfigSet->szInputPath, (pConfigSet->bOutput)? pConfigSet->szOutputPath : NULL, pConfigSet->bB25, pConfigSet->bEMM, pConfigSet->bNULL, pConfigSet->bPURE);

	switch(dwErrorCode){
		case IMulti2Converter::EC_FILE_ERROR :
			::printf("�G���[: �t�@�C�����I�[�v���ł��܂���B\n");
			return false;

		case IMulti2Converter::EC_BCAS_ERROR :
			::printf("�G���[: B-CAS�J�[�h���I�[�v���ł��܂���B\n");
			return false;

		case IMulti2Converter::EC_CLASS_ERROR :
			::printf("�G���[: ����ɕK�v�ȃ��W���[����������܂���B\n");
			return false;

		case IMulti2Converter::EC_INTERNAL_ERROR :
			::printf("�G���[: �����G���[�B\n");
			return false;
		}

	// �����C�x���g��҂�
	m_ConvertEvent.WaitEvent();

	// �R���o�[�g�I��
	m_pMulti2Converter->EndConvert();

	return true;
}

void CMulti2DecDosApp::OnMulti2ConverterEvent(IMulti2Converter *pMulti2Converter, const DWORD dwEventID, PVOID pParam)
{
	switch(dwEventID){
		case IMulti2Converter::EID_CONV_START:
			// �R���o�[�g�J�n
			m_dwLastProgress = 0UL;

			::printf("Now Processing...\n\n");
			::printf("0%%                                50%%                                100%%\n");
			::printf("|-----+------+------+------+------|------+------+------+------+------|\n");
			break;
		
		case IMulti2Converter::EID_CONV_END:
			// ���O�o��
			::printf("\n\n");
			PrintLogFile(stdout);

			// ���O�t�@�C���o��
			if(!m_ConfigSet.bNoLog){					
				OutputLogFile();
				}
			
			// �����C�x���g�Z�b�g
			m_ConvertEvent.SetEvent();
			break;

		case IMulti2Converter::EID_CONV_PROGRESS:
			// �i���󋵍X�V
			UpdateProgress((DWORD)pParam);
			break;

		case IMulti2Converter::EID_CONV_ERROR:
			// �G���[����
			::printf("\n\n�G���[: �����G���[�B\n");

			// �����C�x���g�Z�b�g
			m_ConvertEvent.SetEvent();
			break;
		}
}

void CMulti2DecDosApp::UpdateProgress(const DWORD dwProgress)
{
	// �i���󋵂��X�V
	const DWORD dwCurProgress = (70UL * dwProgress) / 1000UL;

	for( ; m_dwLastProgress < dwCurProgress ; m_dwLastProgress++){
		::printf("|");
		}
}

void CMulti2DecDosApp::OutputLogFile(void)
{
	// ���O�t�@�C�������\�z
	TCHAR szPath[_MAX_PATH + 1] = {TEXT('\0')};
	TCHAR szDrive[_MAX_DRIVE + 1] = {TEXT('\0')};
	TCHAR szDir[_MAX_DIR + 1] = {TEXT('\0')};
	TCHAR szFile[_MAX_FNAME + 1] = {TEXT('\0')};
	::_tsplitpath(m_ConfigSet.szOutputPath, szDrive, szDir, szFile, NULL);
	::_tmakepath(szPath, szDrive, szDir, szFile, TEXT("txt"));

	// ���O�t�@�C���I�[�v��
	FILE *pLogFile = ::_tfsopen(szPath, TEXT("wt"), _SH_DENYWR);
	if(!pLogFile)return;

	// ���O�o��
	PrintLogFile(pLogFile);

	// ���O�t�@�C���N���[�Y
	::fclose(pLogFile);
}

void CMulti2DecDosApp::PrintLogFile(FILE * const pStream)
{
	// ���O�o��
	TCHAR szText[1024];
	char szMbText[_MAX_PATH];
		
	// ���̓t�@�C�����
	::ZeroMemory(szMbText, sizeof(szMbText));
	::WideCharToMultiByte(CP_OEMCP, WC_COMPOSITECHECK, m_ConfigSet.szInputPath, ::lstrlen(m_ConfigSet.szInputPath), szMbText, sizeof(szMbText), NULL, NULL);
	::_ftprintf(pStream, TEXT("Input File: %S\n"), szMbText);
	::_ftprintf(pStream, TEXT("Length: %12s (%s Byte)\n\n"),
		ToUnitedText(m_pMulti2Converter->GetSrcFileLength()),
		ToCommaText(m_pMulti2Converter->GetSrcFileLength())
		);		

	// �o�̓t�@�C�����
	if(m_ConfigSet.bOutput){
		::ZeroMemory(szMbText, sizeof(szMbText));
		::WideCharToMultiByte(CP_OEMCP, WC_COMPOSITECHECK, m_ConfigSet.szOutputPath, ::lstrlen(m_ConfigSet.szOutputPath), szMbText, sizeof(szMbText), NULL, NULL);
		::_ftprintf(pStream, TEXT("Output File: %s\n"), szMbText);
		::_ftprintf(pStream, TEXT("Length: %12s (%s Byte)\n\n"),
			ToUnitedText(m_pMulti2Converter->GetDstFileLength()),
			ToCommaText(m_pMulti2Converter->GetDstFileLength())
			);
		}

	// PID���̃G���[���̍ő啝�����߂�
	int iInWidth = 0, iOutWidth = 0, iDropWidth = 0, iScramblingWidth = 0;

	for(WORD wPID = 0x0000U ; wPID < 0x2000U ; wPID++){
		if(m_pMulti2Converter->GetInputPacketNum(wPID)){
			::_stprintf(szText, TEXT("%lu"), m_pMulti2Converter->GetInputPacketNum(wPID));
			if(::lstrlen(szText) > iInWidth)iInWidth = ::lstrlen(szText);

			::_stprintf(szText, TEXT("%lu"), m_pMulti2Converter->GetOutputPacketNum(wPID));
			if(::lstrlen(szText) > iOutWidth)iOutWidth = ::lstrlen(szText);

			::_stprintf(szText, TEXT("%lu"), m_pMulti2Converter->GetContinuityErrNum(wPID));
			if(::lstrlen(szText) > iDropWidth)iDropWidth = ::lstrlen(szText);

			::_stprintf(szText, TEXT("%lu"), m_pMulti2Converter->GetScramblePacketNum(wPID));
			if(::lstrlen(szText) > iScramblingWidth)iScramblingWidth = ::lstrlen(szText);
			}
		}

	// PID���̃G���[���
	for(WORD wPID = 0x0000U ; wPID < 0x2000U ; wPID++){
		if(m_pMulti2Converter->GetInputPacketNum(wPID)){
			if(m_ConfigSet.bOutput){
				::_ftprintf(pStream, TEXT("[PID: 0x%04X]  In: %*lu,  Out: %*lu,  Drop: %*lu,  Scrambling: %*lu\n"),
					wPID,
					iInWidth,			m_pMulti2Converter->GetInputPacketNum(wPID),
					iOutWidth,			m_pMulti2Converter->GetOutputPacketNum(wPID),
					iDropWidth,			m_pMulti2Converter->GetContinuityErrNum(wPID),
					iScramblingWidth,	m_pMulti2Converter->GetScramblePacketNum(wPID)
					);
				}
			else{
				::_ftprintf(pStream, TEXT("[PID: 0x%04X]  In: %*lu,  Drop: %*lu,  Scrambling: %*lu\n"),
					wPID,
					iInWidth,			m_pMulti2Converter->GetInputPacketNum(wPID),
					iDropWidth,			m_pMulti2Converter->GetContinuityErrNum(wPID),
					iScramblingWidth,	m_pMulti2Converter->GetScramblePacketNum(wPID)
					);
				}
			}
		}

	::_ftprintf(pStream, TEXT("\nSync Error       : %12s\n"), ToCommaText(m_pMulti2Converter->GetSyncErrNum()));
	::_ftprintf(pStream, TEXT("Format Error     : %12s\n"), ToCommaText(m_pMulti2Converter->GetFormatErrNum()));
	::_ftprintf(pStream, TEXT("Transport Error  : %12s Packet\n"), ToCommaText(m_pMulti2Converter->GetTransportErrNum()));
	::_ftprintf(pStream, TEXT("Total Drop Error : %12s Packet\n"), ToCommaText(m_pMulti2Converter->GetContinuityErrNum()));
	::_ftprintf(pStream, TEXT("Total Scrambling : %12s Packet\n"), ToCommaText(m_pMulti2Converter->GetScramblePacketNum()));
	::_ftprintf(pStream, TEXT("\nTotal Input      : %12s Packet\n"), ToCommaText(m_pMulti2Converter->GetInputPacketNum()));

	if(m_ConfigSet.bOutput){
		::_ftprintf(pStream, TEXT("Total Output     : %12s Packet\n"), ToCommaText(m_pMulti2Converter->GetOutputPacketNum()));
		}

	::_ftprintf(pStream, TEXT("Packet Stride    : %12lu Byte\n"), m_pMulti2Converter->GetPacketStride());

	if(m_ConfigSet.bB25){
		::_ftprintf(pStream, TEXT("\nECM Process      : %12s\n"), ToCommaText(m_pMulti2Converter->GetEcmProcessNum()));

		if(m_ConfigSet.bEMM){
			::_ftprintf(pStream, TEXT("EMM Process      : %12s\n"), ToCommaText(m_pMulti2Converter->GetEmmProcessNum()));
			}
		}
}

LPCTSTR CMulti2DecDosApp::ToUnitedText(const ULONGLONG llValue)
{
	static TCHAR szBuffer[256];

	DWORD dwValue;
	double lfValue;
	TCHAR cUnit;

	// �P�ʔ���A�X�P�[�����O
	if(llValue < 0x400ULL){
		// �P�ʂȂ�
		::_stprintf(szBuffer, TEXT("%lu B"), (DWORD)llValue);
		return szBuffer;
		}
	else if(llValue < 0x100000ULL){
		// K
		cUnit = TEXT('K');
		dwValue = (DWORD)(llValue >> 10);
		lfValue = (double)llValue / (double)(1ULL << 10);
		}
	else if(llValue < 0x40000000ULL){
		// M
		cUnit = TEXT('M');
		dwValue = (DWORD)(llValue >> 20);
		lfValue = (double)llValue / (double)(1ULL << 20);
		}
	else if(llValue < 0x10000000000ULL){
		// G
		cUnit = TEXT('G');
		dwValue = (DWORD)(llValue >> 30);
		lfValue = (double)llValue / (double)(1ULL << 30);
		}
	else{
		// T
		cUnit = TEXT('T');
		dwValue = (DWORD)(llValue >> 40);
		lfValue = (double)llValue / (double)(1ULL << 40);
		}

	// �����킹
	if(dwValue < 10UL){
		// X.XX
		::_stprintf(szBuffer, TEXT("%.2f %CB"), lfValue, cUnit);
		}
	else if(dwValue < 100UL){
		// XX.X
		::_stprintf(szBuffer, TEXT("%.1f %CB"), lfValue, cUnit);
		}
	else{
		// XXX
		::_stprintf(szBuffer, TEXT("%.0f %CB"), lfValue, cUnit);
		}

	return szBuffer;
}

LPCTSTR CMulti2DecDosApp::ToCommaText(const ULONGLONG llValue)
{
	static TCHAR szBuffer[256];
	TCHAR szValue[256];
	TCHAR szTemp[256];

	// ������ɕϊ�
	::_stprintf(szValue, TEXT("%llu"), llValue);
	const int iValueLen = ::lstrlen(szValue);

	szBuffer[0] = TEXT('\0');

	// �R���}�ŋ�؂�
	for(int iPos = 0 ; iPos < iValueLen ; iPos++){
		::lstrcpy(szTemp, szBuffer);

		if(!(iPos % 3) && iPos){
			::_stprintf(szBuffer, TEXT("%c,%s"), szValue[iValueLen - iPos - 1], szTemp);
			}
		else{
			::_stprintf(szBuffer, TEXT("%c%s"), szValue[iValueLen - iPos - 1], szTemp);
			}
		}

	return szBuffer;
}
