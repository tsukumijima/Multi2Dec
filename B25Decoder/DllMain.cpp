// DllMain.cpp: DLL�G���g���[�|�C���g�̒�`
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"

// bon_CoreEngine
#include "BonCoreEngine.h"
#include "BonClassEnumerator.h"

#include "MediaBase.h"
#ifndef MINIMIZE_FOR_B25DECODER
#include "SmartFile.h"
#include "SmartSocket.h"
#include "RegCfgStorage.h"
#include "IniCfgStorage.h"
#endif

// bon_MediaDecoder
#include "TsPacket.h"
#include "TsSection.h"
#include "TsDescBase.h"
#include "TsPidMapper.h"
#ifndef MINIMIZE_FOR_B25DECODER
#include "TsEpgCore.h"
#endif
#include "TsTable.h"

#ifndef MINIMIZE_FOR_B25DECODER
#include "TsSourceTuner.h"
#endif
#include "TsPacketSync.h"
#include "TsDescrambler.h"
#ifndef MINIMIZE_FOR_B25DECODER
#include "ProgAnalyzer.h"
#include "FileWriter.h"
#include "FileReader.h"
#endif
#include "MediaGrabber.h"

// bon_BcasDriver
#include "BcasCardReader.h"


/////////////////////////////////////////////////////////////////////////////
// 
// �]����B25Decoder.dll�ƌ݊������ێ����邽�߁ABonSDK�y�ѕK�v�ȃ��W���[����
// �����ĒP��̃o�C�i���ɑg�ݍ���ł���B
// 
// ����͓���̏��u�ł���BonSDK�̐݌v�v�z�Ƃ͈قȂ�B���̂��߂��̂悤�Ȏ�@
// ��p���ă\�[�X�R�[�h�𗬗p���邱�Ƃ͋����Ȃ��B
// 
// �g���c�[�����̐l�@http://2sen.dip.jp/dtv/
// 
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// �t�@�C�����[�J���ϐ�
/////////////////////////////////////////////////////////////////////////////

static CBonCoreEngine BonEngine(NULL);


/////////////////////////////////////////////////////////////////////////////
// �t�@�C�����[�J���֐��v���g�^�C�v
/////////////////////////////////////////////////////////////////////////////

static void RegisterClass(void);


/////////////////////////////////////////////////////////////////////////////
// DLL�G���g���[
/////////////////////////////////////////////////////////////////////////////

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch(ul_reason_for_call){
		case DLL_PROCESS_ATTACH :

			// ���������[�N���o�L��
			_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF); 

			// �R�A�G���W���X�^�[�g�A�b�v
			BonEngine.Startup(hModule);

			// �W���N���X�o�^
			::RegisterClass();

			// Bon���W���[�����[�h
			BonEngine.RegisterBonModule();

			break;

		case DLL_PROCESS_DETACH :

			// �R�A�G���W���V���b�g�_�E��
			BonEngine.Shutdown();

			break;
		}

	return TRUE;
}

static void RegisterClass(void)
{
	// �N���X�t�@�N�g���[�o�^(�W���g�ݍ��݃N���X)

	// bon_CoreEngine
	::BON_REGISTER_CLASS<CBonCoreEngine>();
	::BON_REGISTER_CLASS<CBonClassEnumerator>();

	::BON_REGISTER_CLASS<CMediaData>();
#ifndef MINIMIZE_FOR_B25DECODER
	::BON_REGISTER_CLASS<CSmartFile>();
	::BON_REGISTER_CLASS<CSmartSocket>();
	::BON_REGISTER_CLASS<CRegCfgStorage>();
	::BON_REGISTER_CLASS<CIniCfgStorage>();
#endif

	// bon_MediaDecoder
	::BON_REGISTER_CLASS<CTsPacket>();
	::BON_REGISTER_CLASS<CPsiSection>();
	::BON_REGISTER_CLASS<CDescBlock>();
	::BON_REGISTER_CLASS<CPsiSectionParser>();
	::BON_REGISTER_CLASS<CTsPidMapper>();
#ifndef MINIMIZE_FOR_B25DECODER
	::BON_REGISTER_CLASS<CEitItem>();
	::BON_REGISTER_CLASS<CEpgItem>();
#endif

	::BON_REGISTER_CLASS<CPatTable>();
	::BON_REGISTER_CLASS<CCatTable>();
	::BON_REGISTER_CLASS<CPmtTable>();
#ifndef MINIMIZE_FOR_B25DECODER
	::BON_REGISTER_CLASS<CNitTable>();
	::BON_REGISTER_CLASS<CSdtTable>();
	::BON_REGISTER_CLASS<CEitTable>();
#endif
	::BON_REGISTER_CLASS<CTotTable>();
	
#ifndef MINIMIZE_FOR_B25DECODER
	::BON_REGISTER_CLASS<CTsSourceTuner>();
#endif
	::BON_REGISTER_CLASS<CTsPacketSync>();
	::BON_REGISTER_CLASS<CTsDescrambler>();
#ifndef MINIMIZE_FOR_B25DECODER
	::BON_REGISTER_CLASS<CProgAnalyzer>();
	::BON_REGISTER_CLASS<CFileWriter>();
	::BON_REGISTER_CLASS<CFileReader>();
#endif
	::BON_REGISTER_CLASS<CMediaGrabber>();

	// bon_BcasDriver
	::BON_REGISTER_CLASS<CBcasCardReader>();
}


/////////////////////////////////////////////////////////////////////////////
// �G�N�X�|�[�gAPI
/////////////////////////////////////////////////////////////////////////////

BONAPI const BONGUID BON_NAME_TO_GUID(LPCTSTR lpszName)
{
	// ���W���[����/�N���X��/�C���^�t�F�[�X����GUID�ɕϊ�
	return BonEngine.BonNameToGuid(lpszName);
}

BONAPI IBonObject * BON_CREATE_INSTANCE(LPCTSTR lpszClassName, IBonObject *pOwner)
{
	// �N���X�C���X�^���X����
	return BonEngine.BonCreateInstance(lpszClassName, pOwner);
}

BONAPI const bool QUERY_BON_MODULE(LPCTSTR lpszModuleName)
{
	// ���W���[���̗L����Ԃ�
	return BonEngine.QueryBonModule(lpszModuleName);
}

BONAPI const DWORD GET_BON_MODULE_NAME(const BONGUID ModuleId, LPTSTR lpszModuleName)
{
	// ���W���[����GUID�𖼑O�ɕϊ�
	return BonEngine.GetBonModuleName(ModuleId, lpszModuleName);
}

BONAPI const bool REGISTER_BON_CLASS(LPCTSTR lpszClassName, const CLASSFACTORYMETHOD pfnClassFactory, const DWORD dwPriority)
{
	// �N���X��o�^
	return BonEngine.RegisterBonClass(lpszClassName, pfnClassFactory, dwPriority);
}

BONAPI const bool QUERY_BON_CLASS(LPCTSTR lpszClassName)
{
	// �N���X�̗L����Ԃ�
	return BonEngine.QueryBonClass(lpszClassName);
}

BONAPI const DWORD GET_BON_CLASS_NAME(const BONGUID ClassId, LPTSTR lpszClassName)
{
	// �N���X��GUID�𖼑O�ɕϊ�
	return BonEngine.GetBonClassName(ClassId, lpszClassName);
}

BONAPI const BONGUID GET_BON_CLASS_MODULE(const BONGUID ClassId)
{
	// �N���X��񋟂��郂�W���[����GUID��Ԃ�
	return BonEngine.GetBonClassModule(ClassId);
}

BONAPI IBonObject * GET_STOCK_INSTANCE(LPCTSTR lpszClassName)
{
	// �X�g�b�N�C���X�^���X��Ԃ�
	return BonEngine.GetStockInstance(lpszClassName);
}

BONAPI const bool REGISTER_STOCK_INSTANCE(LPCTSTR lpszClassName, IBonObject *pInstance)
{
	// �X�g�b�N�C���X�^���X�o�^
	return BonEngine.RegisterStockInstance(lpszClassName, pInstance);
}

BONAPI const bool UNREGISTER_STOCK_INSTANCE(LPCTSTR lpszClassName)
{
	// �X�g�b�N�C���X�^���X�폜
	return BonEngine.UnregisterStockInstance(lpszClassName);
}
