// DllMain.cpp: DLLエントリーポイントの定義
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
// 従来のB25Decoder.dllと互換性を維持するため、BonSDK及び必要なモジュールを
// あえて単一のバイナリに組み込んでいる。
// 
// これは特例の処置でありBonSDKの設計思想とは異なる。このためこのような手法
// を用いてソースコードを流用することは許可しない。
// 
// 拡張ツール中の人　http://2sen.dip.jp/dtv/
// 
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// ファイルローカル変数
/////////////////////////////////////////////////////////////////////////////

static CBonCoreEngine BonEngine(NULL);


/////////////////////////////////////////////////////////////////////////////
// ファイルローカル関数プロトタイプ
/////////////////////////////////////////////////////////////////////////////

static void RegisterClass(void);


/////////////////////////////////////////////////////////////////////////////
// DLLエントリー
/////////////////////////////////////////////////////////////////////////////

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch(ul_reason_for_call){
		case DLL_PROCESS_ATTACH :

			// メモリリーク検出有効
			_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF); 

			// コアエンジンスタートアップ
			BonEngine.Startup(hModule);

			// 標準クラス登録
			::RegisterClass();

			// Bonモジュールロード
			BonEngine.RegisterBonModule();

			break;

		case DLL_PROCESS_DETACH :

			// コアエンジンシャットダウン
			BonEngine.Shutdown();

			break;
		}

	return TRUE;
}

static void RegisterClass(void)
{
	// クラスファクトリー登録(標準組み込みクラス)

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
// エクスポートAPI
/////////////////////////////////////////////////////////////////////////////

BONAPI const BONGUID BON_NAME_TO_GUID(LPCTSTR lpszName)
{
	// モジュール名/クラス名/インタフェース名をGUIDに変換
	return BonEngine.BonNameToGuid(lpszName);
}

BONAPI IBonObject * BON_CREATE_INSTANCE(LPCTSTR lpszClassName, IBonObject *pOwner)
{
	// クラスインスタンス生成
	return BonEngine.BonCreateInstance(lpszClassName, pOwner);
}

BONAPI const bool QUERY_BON_MODULE(LPCTSTR lpszModuleName)
{
	// モジュールの有無を返す
	return BonEngine.QueryBonModule(lpszModuleName);
}

BONAPI const DWORD GET_BON_MODULE_NAME(const BONGUID ModuleId, LPTSTR lpszModuleName)
{
	// モジュールのGUIDを名前に変換
	return BonEngine.GetBonModuleName(ModuleId, lpszModuleName);
}

BONAPI const bool REGISTER_BON_CLASS(LPCTSTR lpszClassName, const CLASSFACTORYMETHOD pfnClassFactory, const DWORD dwPriority)
{
	// クラスを登録
	return BonEngine.RegisterBonClass(lpszClassName, pfnClassFactory, dwPriority);
}

BONAPI const bool QUERY_BON_CLASS(LPCTSTR lpszClassName)
{
	// クラスの有無を返す
	return BonEngine.QueryBonClass(lpszClassName);
}

BONAPI const DWORD GET_BON_CLASS_NAME(const BONGUID ClassId, LPTSTR lpszClassName)
{
	// クラスのGUIDを名前に変換
	return BonEngine.GetBonClassName(ClassId, lpszClassName);
}

BONAPI const BONGUID GET_BON_CLASS_MODULE(const BONGUID ClassId)
{
	// クラスを提供するモジュールのGUIDを返す
	return BonEngine.GetBonClassModule(ClassId);
}

BONAPI IBonObject * GET_STOCK_INSTANCE(LPCTSTR lpszClassName)
{
	// ストックインスタンスを返す
	return BonEngine.GetStockInstance(lpszClassName);
}

BONAPI const bool REGISTER_STOCK_INSTANCE(LPCTSTR lpszClassName, IBonObject *pInstance)
{
	// ストックインスタンス登録
	return BonEngine.RegisterStockInstance(lpszClassName, pInstance);
}

BONAPI const bool UNREGISTER_STOCK_INSTANCE(LPCTSTR lpszClassName)
{
	// ストックインスタンス削除
	return BonEngine.UnregisterStockInstance(lpszClassName);
}
