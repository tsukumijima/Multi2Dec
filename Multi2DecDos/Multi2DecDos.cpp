// Multi2DecDos.cpp : �R���\�[�� �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "stdafx.h"
#include "Multi2DecDosApp.h"




// �t�@�C�����[�J���֐��v���g�^�C�v
static void PrintUsage(void);
static const bool ParseParam(CMulti2DecDosApp::CONFIGSET * const pConfigSet, const int iParamNum, LPTSTR aszParam[]);
static LPCTSTR FindParam(const int iParamNum, LPTSTR aszParam[], LPCTSTR lpszFind);
static const bool ParsePath(CMulti2DecDosApp::CONFIGSET * const pConfigSet);




// �G���g���[�|�C���g
int _tmain(int argc, _TCHAR* argv[])
{
	CMulti2DecDosApp::CONFIGSET ConfigSet;
	CMulti2DecDosApp App(NULL);

	// �o�[�V�������
	::printf("\n- Multi2DecDos Ver.2.10 - http://2sen.dip.jp/dtv/\n\n");

	// �p�����[�^�����
	if(!ParseParam(&ConfigSet, argc, argv)){
		// �p�����[�^���s��
		PrintUsage();
		return -1;	
		}

	// �t�@�C�����𐶐�
	if(!ParsePath(&ConfigSet)){
		// �t�@�C�������s��
		return -1;
		}

	// �R���o�[�g���s
	if(!App.Run(&ConfigSet)){
		return -1;
		}

	return 0;
}

static void PrintUsage(void)
{
	// �w���v��\��
	::printf("\nMulti2DecDos [[/D | /C | /U] | [/OUT] [/B25] [/EMM] [/PURE] [/NULL]] [/N]\n");
	::printf("             ���̓t�@�C�� [�o�̓t�@�C��]\n\n");
	::printf("  /D            �X�N�����u���������s���܂��B(�f�t�H���g����)\n");
	::printf("  /C            TS�t�@�C���̃G���[�`�F�b�N���s���܂��B\n");
	::printf("  /U            B-CAS�J�[�h�̌_����̍X�V���s���܂��B(EMM����)\n");
	::printf("  /OUT          �t�@�C�����o�͂��܂��B\n");
	::printf("  /B25          Multi2�������s���܂��B\n");
	::printf("  /EMM          EMM�������s���܂��B\n");
	::printf("  /PURE         �����R��p�P�b�g���o�͂��܂���B\n");
	::printf("  /NULL         NULL�p�P�b�g���o�͂��܂���B\n");
	::printf("  /N            ���O�t�@�C�����o�͂��܂���B\n");
	::printf("  ���̓t�@�C��  ����TS�t�@�C���̃p�X��\n");
	::printf("  �o�̓t�@�C��  ����TS�t�@�C���̃p�X��\n\n");
}

static const bool ParseParam(CMulti2DecDosApp::CONFIGSET * const pConfigSet, const int iParamNum, LPTSTR aszParam[])
{
	// �f�t�H���g�p�����[�^(/D ����)
	pConfigSet->bOutput = true;
	pConfigSet->bB25	= true;
	pConfigSet->bEMM	= false;
	pConfigSet->bPURE	= false;
	pConfigSet->bNULL	= true;
	pConfigSet->bNoLog	= false;
	pConfigSet->szInputPath[0] = TEXT('\0');
	pConfigSet->szOutputPath[0] = TEXT('\0');

	// �t�@�C���������
	if(iParamNum >= 3){
		if((aszParam[iParamNum - 2][0] != TEXT('/')) && (aszParam[iParamNum - 1][0] != TEXT('/'))){
			// ����/�o�̓t�@�C������ۑ�
			::lstrcpy(pConfigSet->szInputPath,  aszParam[iParamNum - 2]);
			::lstrcpy(pConfigSet->szOutputPath, aszParam[iParamNum - 1]);
			}
		else if(aszParam[iParamNum - 1][0] != TEXT('/')){
			// ���̓t�@�C������ۑ�
			::lstrcpy(pConfigSet->szInputPath, aszParam[iParamNum - 1]);
			}
		else{
			// �t�@�C�������Ȃ�
			::printf("�G���[: ���̓t�@�C�����w�肳��Ă��܂���B\n");
			return false;
			}		
		}
	else if(iParamNum >= 2){
		if(aszParam[1][0] != TEXT('/')){
			// ���̓t�@�C������ۑ�
			::lstrcpy(pConfigSet->szInputPath, aszParam[1]);
			return true;
			}
		else{
			// �t�@�C�������Ȃ�
			::printf("�G���[: ���̓t�@�C�������w�肳��Ă��܂���B\n");
			return false;
			}		
		}
	else{
		// �p�����[�^��1���Ȃ�
		::printf("�G���[: �p�����[�^���w�肳��Ă��܂���B\n");
		return false;
		}

	// ���O�o�͐���p�����[�^�����
	pConfigSet->bNoLog = (FindParam(iParamNum, aszParam, TEXT("/N")))? true : false;

	// �v���Z�b�g�p�����[�^�����
	int iPresetNum = 0;
		
	if(FindParam(iParamNum, aszParam, TEXT("/D"))){
		// /D
		iPresetNum++;
		pConfigSet->bOutput = true;
		pConfigSet->bB25	= true;
		pConfigSet->bEMM	= false;
		pConfigSet->bPURE	= false;
		pConfigSet->bNULL	= true;
		}

	if(FindParam(iParamNum, aszParam, TEXT("/C"))){
		// /C
		iPresetNum++;
		pConfigSet->bOutput = false;
		pConfigSet->bB25	= false;
		pConfigSet->bEMM	= false;
		pConfigSet->bPURE	= false;
		pConfigSet->bNULL	= true;
		}

	if(FindParam(iParamNum, aszParam, TEXT("/U"))){
		// /U
		iPresetNum++;
		pConfigSet->bOutput = false;
		pConfigSet->bB25	= true;
		pConfigSet->bEMM	= true;
		pConfigSet->bPURE	= false;
		pConfigSet->bNULL	= true;
		}

	if(iPresetNum == 1){
		// �v���Z�b�g���[�h
		return true;
		}
	else if(iPresetNum > 1){
		::printf("�G���[: /D /C /U �p�����[�^�͓�����1�����w��ł��܂���B\n");
		return false;
		}

	// �J�X�^���p�����[�^���
	pConfigSet->bOutput = (FindParam(iParamNum, aszParam, TEXT("/OUT")))?  true : false;
	pConfigSet->bB25	= (FindParam(iParamNum, aszParam, TEXT("/B25")))?  true : false;
	pConfigSet->bEMM	= (FindParam(iParamNum, aszParam, TEXT("/EMM")))?  true : false;
	pConfigSet->bPURE	= (FindParam(iParamNum, aszParam, TEXT("/PURE")))? true : false;
	pConfigSet->bNULL	= (FindParam(iParamNum, aszParam, TEXT("/NULL")))? true : false;

	return true;
}

static LPCTSTR FindParam(const int iParamNum, LPTSTR aszParam[], LPCTSTR lpszFind)
{
	// �p�����[�^���X�g����w�肵�����������������
	for(int i = 1 ; i < iParamNum ; i++){
		if(!::lstrcmpi(aszParam[i], lpszFind)){
			return aszParam[i];
			}		
		}

	return NULL;
}

static const bool ParsePath(CMulti2DecDosApp::CONFIGSET * const pConfigSet)
{
	const int iInputLen = ::lstrlen(pConfigSet->szInputPath);

	// �t�@�C�������`�F�b�N����
	if(iInputLen < 4){
		::printf("�G���[: ���̓t�@�C�������Z�����܂��B\n");
		return false;
		}
		
	// �t�@�C���p�X���`�F�b�N����
	if(::lstrcmpi(&pConfigSet->szInputPath[iInputLen - 3], TEXT(".ts"))){
		::printf("�G���[: ���̓t�@�C���̊g���q��TS�t�@�C���ł͂���܂���B\n");
		return false;
		}

	// �o�̓t�@�C���p�X�𐶐�
	if(pConfigSet->szOutputPath[0] == TEXT('\0')){
		::lstrcpy(pConfigSet->szOutputPath, pConfigSet->szInputPath);
		pConfigSet->szOutputPath[iInputLen - 3] = TEXT('\0');
		::lstrcat(pConfigSet->szOutputPath, TEXT("_dec.ts"));
		}

	return true;
}
