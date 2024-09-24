// DllMain.cpp: DLLエントリーポイントの定義
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
// ファイルローカル関数プロトタイプ
/////////////////////////////////////////////////////////////////////////////

static void RegisterClassFactory(void);


/////////////////////////////////////////////////////////////////////////////
// DLLエントリー
/////////////////////////////////////////////////////////////////////////////

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch(ul_reason_for_call){
		case DLL_PROCESS_ATTACH :

#ifdef _DEBUG
			// メモリリーク検出有効
			::_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | ::_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG)); 
#endif
			// クラス登録
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
	// クラスファクトリー登録
	::BON_REGISTER_CLASS<TsConverter>();
	::BON_REGISTER_CLASS<CMulti2Converter>();
}


#ifdef _MANAGED
#pragma managed(pop)
#endif
