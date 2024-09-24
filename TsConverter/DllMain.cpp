// DllMain.cpp: DLL�G���g���[�|�C���g�̒�`
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "ModCatalog.h"
#include "Multi2Converter.h"


#ifdef _MANAGED
#pragma managed(push, off)
#endif


/////////////////////////////////////////////////////////////////////////////
// �t�@�C�����[�J���֐��v���g�^�C�v
/////////////////////////////////////////////////////////////////////////////

static void RegisterClassFactory(void);


/////////////////////////////////////////////////////////////////////////////
// DLL�G���g���[
/////////////////////////////////////////////////////////////////////////////

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch(ul_reason_for_call){
		case DLL_PROCESS_ATTACH :

#ifdef _DEBUG
			// ���������[�N���o�L��
			::_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | ::_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG)); 
#endif
			// �N���X�o�^
			g_hModule = hModule;
			::RegisterClassFactory();

			break;

		case DLL_PROCESS_DETACH :

			break;
		}

	return TRUE;
}

static void RegisterClassFactory(void)
{
	// �N���X�t�@�N�g���[�o�^
	::BON_REGISTER_CLASS<TsConverter>();
	::BON_REGISTER_CLASS<CMulti2Converter>();
}


#ifdef _MANAGED
#pragma managed(pop)
#endif
