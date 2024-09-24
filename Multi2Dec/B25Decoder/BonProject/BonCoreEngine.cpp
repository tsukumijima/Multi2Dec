// BonCoreEngine.cpp: Bonコアエンジンクラス
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "BonCoreEngine.h"
#include <TChar.h>


/////////////////////////////////////////////////////////////////////////////
// CBonCoreEngineクラス
/////////////////////////////////////////////////////////////////////////////

const BONGUID CBonCoreEngine::BonNameToGuid(LPCTSTR lpszName)
{
	// モジュール/クラス名をBONGUIDに変換する
	if(!lpszName)return BCID_NULL;
	
	const DWORD dwNameLen = ::_tcslen(lpszName);
	
	if(!dwNameLen || (dwNameLen >= BON_MAX_CLASS_NAME))return BCID_NULL;

	return CalcCrc(reinterpret_cast<const BYTE *>(lpszName), dwNameLen * sizeof(lpszName[0]));
}

IBonObject * CBonCoreEngine::BonCreateInstance(LPCTSTR lpszClassName, IBonObject *pOwner)
{
	CBlockLock AutoLock(&m_Lock);

	// 要求されたクラスのインスタンスを生成する
	const BonClassMap::iterator itClass = m_ClassMap.find(BonNameToGuid(lpszClassName));
	
	return (itClass != m_ClassMap.end())? (itClass->second.pfnClassFactory)(pOwner) : NULL;
}

const bool CBonCoreEngine::RegisterBonModule(LPCTSTR lpszModulePath)
{
	CBlockLock AutoLock(&m_Lock);

	// 検索ファイルパスを構築する
	TCHAR szPath[_MAX_PATH]	  = TEXT("");
	TCHAR szDrive[_MAX_DRIVE] = TEXT("");
	TCHAR szDir[_MAX_DIR]     = TEXT("");

	if(lpszModulePath){
		// パス指定
		::_tcscpy(szPath, lpszModulePath);
		}
	else{
		// デフォルトパス(アプリケーションフォルダ)
		::GetModuleFileName(NULL, szPath, _MAX_PATH);
		}

	::_tsplitpath(szPath, szDrive, szDir, NULL, NULL);
	::_tmakepath(szPath, szDrive, szDir, TEXT("bon_*"), TEXT("dll"));

	// ファイルを検索してオープンする
	WIN32_FIND_DATA FindData;
	HANDLE hFileFind = ::FindFirstFile(szPath, &FindData);
	if(hFileFind == INVALID_HANDLE_VALUE)return false;

	do{
		// ファイル名の長さをチェック
		if(::_tcslen(FindData.cFileName) < 9)continue;
	
		// モジュール名設定(「bon_」サフィックスを除去したもの)
		::_tsplitpath(&FindData.cFileName[4], NULL, NULL, szPath, NULL);
		m_CurModuleInfo.strModuleName = szPath;

		// 既存のモジュールをチェック
		if(QueryBonModule(m_CurModuleInfo.strModuleName.c_str()))continue;

		// オープン前のファクトリー数を退避
		const DWORD dwLastClassNum = m_ClassMap.size();
		m_CurModuleInfo.ClassList.clear();

		// DLLを開く
		::_tmakepath(szPath, szDrive, szDir, FindData.cFileName, NULL);	
		if(!(m_CurModuleInfo.hModule = ::LoadLibrary(szPath)))continue;
		
		// 登録されたファクトリー数を確認
		if(m_ClassMap.size() == dwLastClassNum){
			// 1つも登録されなかったので開放する
			::FreeLibrary(m_CurModuleInfo.hModule);
			continue;
			}
	
		// マップに追加
		m_ModuleMap[BonNameToGuid(m_CurModuleInfo.strModuleName.c_str())] = m_CurModuleInfo;
		}
	while(::FindNextFile(hFileFind, &FindData));

	// 検索終了
	::FindClose(hFileFind);
	m_CurModuleInfo.hModule = NULL;

	return true;
}

const bool CBonCoreEngine::QueryBonModule(LPCTSTR lpszModuleName)
{
	CBlockLock AutoLock(&m_Lock);

	// GUIDをキーにモジュールの有無を検索する
	return (m_ModuleMap.find(BonNameToGuid(lpszModuleName)) != m_ModuleMap.end())? true : false;
}

const DWORD CBonCoreEngine::GetBonModuleNum(void)
{
	CBlockLock AutoLock(&m_Lock);

	// モジュール数を返す
	return m_ModuleMap.size();
}

const BONGUID CBonCoreEngine::EnumBonModule(const DWORD dwIndex)
{
	CBlockLock AutoLock(&m_Lock);

	if(dwIndex >= m_ModuleMap.size())return BMID_NULL;

	// インデックス位置の要素を取得
	BonModuleMap::iterator itModule = m_ModuleMap.begin();
	for(DWORD dwPos = 0UL ; dwPos < dwIndex ; dwPos++)itModule++;
	
	// モジュールのGUIDを返す
	return itModule->first;
}

const DWORD CBonCoreEngine::GetBonModuleName(const BONGUID ModuleId, LPTSTR lpszModuleName)
{
	CBlockLock AutoLock(&m_Lock);
	
	// モジュールを検索
	const BonModuleMap::iterator itModule = m_ModuleMap.find(ModuleId);
	if(itModule == m_ModuleMap.end())return 0UL;
	
	// モジュール名を格納
	if(lpszModuleName)::_tcscpy(lpszModuleName, itModule->second.strModuleName.c_str());
	
	// モジュール名長を返す
	return itModule->second.strModuleName.length();
}

const BONGUID CBonCoreEngine::EnumBonModuleClass(const BONGUID ModuleId, const DWORD dwIndex)
{
	CBlockLock AutoLock(&m_Lock);
	
	// モジュールを検索
	const BonModuleMap::iterator itModule = m_ModuleMap.find(ModuleId);
	if(itModule == m_ModuleMap.end())return BCID_NULL;
	
	// インデックス位置のクラスのGUIDを返す
	return (dwIndex < itModule->second.ClassList.size())? itModule->second.ClassList[dwIndex] : BCID_NULL;
}

const bool CBonCoreEngine::RegisterBonClass(LPCTSTR lpszClassName, const CLASSFACTORYMETHOD pfnClassFactory, const DWORD dwPriority)
{
	CBlockLock AutoLock(&m_Lock);

	// GUID計算
	const BONGUID ClassId = BonNameToGuid(lpszClassName);
	if(!pfnClassFactory || (ClassId == BCID_NULL) || !m_CurModuleInfo.hModule)return false;

	// 既存のクラスを検索
	if(QueryBonClass(lpszClassName)){
		// 既存のクラスとプライオリティを比較
		if(dwPriority <= m_ClassMap[ClassId].dwPriority)return false;
		}

	// マップに追加
	BON_CLASS_INFO ClassInfo;
	ClassInfo.strClassName = lpszClassName;
	ClassInfo.pfnClassFactory = pfnClassFactory;
	ClassInfo.dwPriority = dwPriority;
	ClassInfo.ModuleId = BonNameToGuid(m_CurModuleInfo.strModuleName.c_str());

	m_ClassMap[ClassId] = ClassInfo;
	m_CurModuleInfo.ClassList.push_back(ClassId);

	return true;
}

const bool CBonCoreEngine::QueryBonClass(LPCTSTR lpszClassName)
{
	CBlockLock AutoLock(&m_Lock);

	// GUIDをキーにクラスの有無を検索する
	return (m_ClassMap.find(BonNameToGuid(lpszClassName)) != m_ClassMap.end())? true : false;
}

const DWORD CBonCoreEngine::GetBonClassNum(void)
{
	CBlockLock AutoLock(&m_Lock);

	// クラス数を返す
	return m_ClassMap.size();
}

const BONGUID CBonCoreEngine::EnumBonClass(const DWORD dwIndex)
{
	CBlockLock AutoLock(&m_Lock);

	if(dwIndex >= m_ClassMap.size())return BCID_NULL;

	// インデックス位置の要素を取得
	BonClassMap::iterator itClass = m_ClassMap.begin();
	for(DWORD dwPos = 0UL ; dwPos < dwIndex ; dwPos++)itClass++;
	
	// クラスのGUIDを返す
	return itClass->first;
}

const DWORD CBonCoreEngine::GetBonClassName(const BONGUID ClassId, LPTSTR lpszClassName)
{
	CBlockLock AutoLock(&m_Lock);
	
	// クラスを検索
	const BonClassMap::iterator itClass = m_ClassMap.find(ClassId);
	if(itClass == m_ClassMap.end())return 0UL;
	
	// クラス名を格納
	if(lpszClassName)::_tcscpy(lpszClassName, itClass->second.strClassName.c_str());
	
	// クラス名長を返す
	return itClass->second.strClassName.length();
}

const DWORD CBonCoreEngine::GetBonClassPriority(const BONGUID ClassId)
{
	CBlockLock AutoLock(&m_Lock);
	
	// クラスを検索
	const BonClassMap::iterator itClass = m_ClassMap.find(ClassId);
	if(itClass == m_ClassMap.end())return 0xFFFFFFFFUL;
	
	// インデックス位置のクラスのGUIDを返す
	return itClass->second.dwPriority;
}

const BONGUID CBonCoreEngine::GetBonClassModule(const BONGUID ClassId)
{
	CBlockLock AutoLock(&m_Lock);
	
	// クラスを検索
	const BonClassMap::iterator itClass = m_ClassMap.find(ClassId);
	if(itClass == m_ClassMap.end())return BMID_NULL;
	
	// インデックス位置のクラスのGUIDを返す
	return itClass->second.ModuleId;
}

IBonObject * CBonCoreEngine::GetStockInstance(LPCTSTR lpszClassName)
{
	CBlockLock AutoLock(&m_Lock);

	// 要求されたクラスのインスタンスを返す
	const BonInstanceMap::iterator itInstance = m_InstanceMap.find(BonNameToGuid(lpszClassName));
	
	return (itInstance != m_InstanceMap.end())? itInstance->second.pInstance : NULL;
}

const bool CBonCoreEngine::RegisterStockInstance(LPCTSTR lpszClassName, IBonObject *pInstance)
{
	CBlockLock AutoLock(&m_Lock);

	// クラス名からGUID計算
	const BONGUID ClassId = BonNameToGuid(lpszClassName);
	if(ClassId == BCID_NULL)return false;
	
	// マップに追加
	BON_INSTANCE_INFO InstanceInfo;
	InstanceInfo.strClassName = lpszClassName;
	InstanceInfo.pInstance = pInstance;
	
	m_InstanceMap[ClassId] = InstanceInfo;

	return true;
}

const bool CBonCoreEngine::UnregisterStockInstance(LPCTSTR lpszClassName)
{
	CBlockLock AutoLock(&m_Lock);

	// インスタンスを検索
	const BonInstanceMap::iterator itInstance = m_InstanceMap.find(BonNameToGuid(lpszClassName));
	if(itInstance == m_InstanceMap.end())return false;

	// マップから削除
	m_InstanceMap.erase(itInstance);

	return true;
}

const DWORD CBonCoreEngine::EnumStockInstance(const DWORD dwIndex, LPTSTR lpszClassName)
{
	CBlockLock AutoLock(&m_Lock);

	if(dwIndex >= m_InstanceMap.size())return NULL;

	// インデックス位置の要素を取得
	BonInstanceMap::iterator itInstance = m_InstanceMap.begin();
	for(DWORD dwPos = 0UL ; dwPos < dwIndex ; dwPos++)itInstance++;

	// クラス名を格納
	if(lpszClassName)::_tcscpy(lpszClassName, itInstance->second.strClassName.c_str());
	
	// クラス名長を返す
	return itInstance->second.strClassName.length();
}

CBonCoreEngine::CBonCoreEngine(IBonObject *pOwner)
	: CBonObject(pOwner)
{
	m_CurModuleInfo.hModule = NULL;
}

CBonCoreEngine::~CBonCoreEngine(void)
{
	Shutdown();
}

void CBonCoreEngine::Startup(const HINSTANCE hInstance)
{
	CBlockLock AutoLock(&m_Lock);

	// 一旦シャットダウン
	Shutdown();

	// インスタンスハンドル保存
	m_hInstance = hInstance;
	m_CurModuleInfo.hModule = hInstance;
	m_CurModuleInfo.strModuleName = TEXT("CoreEngine");
	
	// 自インスタンスをストックマップに登録
	RegisterStockInstance(TEXT("CBonCoreEngine"), static_cast<CBonObject *>(this));
}

void CBonCoreEngine::Shutdown(void)
{
	CBlockLock AutoLock(&m_Lock);

	// インスタンスマップクリア
	for(BonInstanceMap::iterator itInstance = m_InstanceMap.begin() ; itInstance != m_InstanceMap.end() ; itInstance++){
		// 自身のインスタンス以外であればインスタンス開放
		if(itInstance->second.pInstance != static_cast<CBonObject *>(this)){
			// インスタンス開放
			itInstance->second.pInstance->Release();
			}
		}

	m_InstanceMap.clear();
		
	// クラスマップクリア
	m_ClassMap.clear();

	// モジュールマップクリア
	for(BonModuleMap::iterator itModule = m_ModuleMap.begin() ; itModule != m_ModuleMap.end() ; itModule++){
		::FreeLibrary(itModule->second.hModule);
		}
	
	m_ModuleMap.clear();
	
	// 状態初期化
	m_CurModuleInfo.hModule = NULL;
}

const DWORD CBonCoreEngine::CalcCrc(const BYTE *pData, const DWORD dwDataSize)
{
	// CRC32計算
	static const DWORD CrcTable[256] = {
		0x00000000UL, 0x04C11DB7UL, 0x09823B6EUL, 0x0D4326D9UL,	0x130476DCUL, 0x17C56B6BUL, 0x1A864DB2UL, 0x1E475005UL,	0x2608EDB8UL, 0x22C9F00FUL, 0x2F8AD6D6UL, 0x2B4BCB61UL,	0x350C9B64UL, 0x31CD86D3UL, 0x3C8EA00AUL, 0x384FBDBDUL,
		0x4C11DB70UL, 0x48D0C6C7UL, 0x4593E01EUL, 0x4152FDA9UL,	0x5F15ADACUL, 0x5BD4B01BUL, 0x569796C2UL, 0x52568B75UL,	0x6A1936C8UL, 0x6ED82B7FUL, 0x639B0DA6UL, 0x675A1011UL,	0x791D4014UL, 0x7DDC5DA3UL, 0x709F7B7AUL, 0x745E66CDUL,
		0x9823B6E0UL, 0x9CE2AB57UL, 0x91A18D8EUL, 0x95609039UL,	0x8B27C03CUL, 0x8FE6DD8BUL, 0x82A5FB52UL, 0x8664E6E5UL,	0xBE2B5B58UL, 0xBAEA46EFUL, 0xB7A96036UL, 0xB3687D81UL,	0xAD2F2D84UL, 0xA9EE3033UL, 0xA4AD16EAUL, 0xA06C0B5DUL,
		0xD4326D90UL, 0xD0F37027UL, 0xDDB056FEUL, 0xD9714B49UL,	0xC7361B4CUL, 0xC3F706FBUL, 0xCEB42022UL, 0xCA753D95UL,	0xF23A8028UL, 0xF6FB9D9FUL, 0xFBB8BB46UL, 0xFF79A6F1UL,	0xE13EF6F4UL, 0xE5FFEB43UL, 0xE8BCCD9AUL, 0xEC7DD02DUL,
		0x34867077UL, 0x30476DC0UL, 0x3D044B19UL, 0x39C556AEUL,	0x278206ABUL, 0x23431B1CUL, 0x2E003DC5UL, 0x2AC12072UL,	0x128E9DCFUL, 0x164F8078UL, 0x1B0CA6A1UL, 0x1FCDBB16UL,	0x018AEB13UL, 0x054BF6A4UL, 0x0808D07DUL, 0x0CC9CDCAUL,
		0x7897AB07UL, 0x7C56B6B0UL, 0x71159069UL, 0x75D48DDEUL,	0x6B93DDDBUL, 0x6F52C06CUL, 0x6211E6B5UL, 0x66D0FB02UL,	0x5E9F46BFUL, 0x5A5E5B08UL, 0x571D7DD1UL, 0x53DC6066UL,	0x4D9B3063UL, 0x495A2DD4UL, 0x44190B0DUL, 0x40D816BAUL,
		0xACA5C697UL, 0xA864DB20UL, 0xA527FDF9UL, 0xA1E6E04EUL,	0xBFA1B04BUL, 0xBB60ADFCUL, 0xB6238B25UL, 0xB2E29692UL,	0x8AAD2B2FUL, 0x8E6C3698UL, 0x832F1041UL, 0x87EE0DF6UL,	0x99A95DF3UL, 0x9D684044UL, 0x902B669DUL, 0x94EA7B2AUL,
		0xE0B41DE7UL, 0xE4750050UL, 0xE9362689UL, 0xEDF73B3EUL,	0xF3B06B3BUL, 0xF771768CUL, 0xFA325055UL, 0xFEF34DE2UL,	0xC6BCF05FUL, 0xC27DEDE8UL, 0xCF3ECB31UL, 0xCBFFD686UL,	0xD5B88683UL, 0xD1799B34UL, 0xDC3ABDEDUL, 0xD8FBA05AUL,
		0x690CE0EEUL, 0x6DCDFD59UL, 0x608EDB80UL, 0x644FC637UL,	0x7A089632UL, 0x7EC98B85UL, 0x738AAD5CUL, 0x774BB0EBUL,	0x4F040D56UL, 0x4BC510E1UL, 0x46863638UL, 0x42472B8FUL,	0x5C007B8AUL, 0x58C1663DUL, 0x558240E4UL, 0x51435D53UL,
		0x251D3B9EUL, 0x21DC2629UL, 0x2C9F00F0UL, 0x285E1D47UL,	0x36194D42UL, 0x32D850F5UL, 0x3F9B762CUL, 0x3B5A6B9BUL,	0x0315D626UL, 0x07D4CB91UL, 0x0A97ED48UL, 0x0E56F0FFUL,	0x1011A0FAUL, 0x14D0BD4DUL, 0x19939B94UL, 0x1D528623UL,
		0xF12F560EUL, 0xF5EE4BB9UL, 0xF8AD6D60UL, 0xFC6C70D7UL,	0xE22B20D2UL, 0xE6EA3D65UL, 0xEBA91BBCUL, 0xEF68060BUL,	0xD727BBB6UL, 0xD3E6A601UL, 0xDEA580D8UL, 0xDA649D6FUL,	0xC423CD6AUL, 0xC0E2D0DDUL, 0xCDA1F604UL, 0xC960EBB3UL,
		0xBD3E8D7EUL, 0xB9FF90C9UL, 0xB4BCB610UL, 0xB07DABA7UL,	0xAE3AFBA2UL, 0xAAFBE615UL, 0xA7B8C0CCUL, 0xA379DD7BUL,	0x9B3660C6UL, 0x9FF77D71UL, 0x92B45BA8UL, 0x9675461FUL,	0x8832161AUL, 0x8CF30BADUL, 0x81B02D74UL, 0x857130C3UL,
		0x5D8A9099UL, 0x594B8D2EUL, 0x5408ABF7UL, 0x50C9B640UL,	0x4E8EE645UL, 0x4A4FFBF2UL, 0x470CDD2BUL, 0x43CDC09CUL,	0x7B827D21UL, 0x7F436096UL, 0x7200464FUL, 0x76C15BF8UL,	0x68860BFDUL, 0x6C47164AUL, 0x61043093UL, 0x65C52D24UL,
		0x119B4BE9UL, 0x155A565EUL, 0x18197087UL, 0x1CD86D30UL,	0x029F3D35UL, 0x065E2082UL, 0x0B1D065BUL, 0x0FDC1BECUL,	0x3793A651UL, 0x3352BBE6UL, 0x3E119D3FUL, 0x3AD08088UL,	0x2497D08DUL, 0x2056CD3AUL, 0x2D15EBE3UL, 0x29D4F654UL,
		0xC5A92679UL, 0xC1683BCEUL, 0xCC2B1D17UL, 0xC8EA00A0UL,	0xD6AD50A5UL, 0xD26C4D12UL, 0xDF2F6BCBUL, 0xDBEE767CUL,	0xE3A1CBC1UL, 0xE760D676UL, 0xEA23F0AFUL, 0xEEE2ED18UL,	0xF0A5BD1DUL, 0xF464A0AAUL, 0xF9278673UL, 0xFDE69BC4UL,
		0x89B8FD09UL, 0x8D79E0BEUL, 0x803AC667UL, 0x84FBDBD0UL,	0x9ABC8BD5UL, 0x9E7D9662UL, 0x933EB0BBUL, 0x97FFAD0CUL,	0xAFB010B1UL, 0xAB710D06UL, 0xA6322BDFUL, 0xA2F33668UL,	0xBCB4666DUL, 0xB8757BDAUL, 0xB5365D03UL, 0xB1F740B4UL
		};		

	DWORD dwCurCrc = 0xFFFFFFFFUL;

	for(DWORD dwPos = 0U ; dwPos < dwDataSize ; dwPos++){
		dwCurCrc = (dwCurCrc << 8) ^ CrcTable[ (dwCurCrc >> 24) ^ pData[dwPos] ];
		}

	return dwCurCrc;
}
