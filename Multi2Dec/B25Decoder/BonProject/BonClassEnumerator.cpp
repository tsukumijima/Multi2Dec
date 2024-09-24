// BonClassEnumerator.cpp: Bonクラス列挙クラス
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "BonClassEnumerator.h"
#include <TChar.h>


/////////////////////////////////////////////////////////////////////////////
// Bonクラス列挙クラス
/////////////////////////////////////////////////////////////////////////////

const DWORD CBonClassEnumerator::EnumBonClass(LPCTSTR lpszBIId)
{
	if(!lpszBIId)return 0UL;
	if(!lpszBIId[0])return 0UL;

	// リストをクリア
	ReleaseCatalog();

	// インタフェース名保存
	::_tcscpy(m_szEnumBIId, lpszBIId);

	// コアエンジンインスタンス取得
	IBonCoreEngine *pCoreEngine = dynamic_cast<IBonCoreEngine *>(::GET_STOCK_INSTANCE(TEXT("CBonCoreEngine")));
	::BON_ASSERT(pCoreEngine != NULL);

	DWORD dwModIndex = 0UL;
	TCHAR szModName[BON_MAX_MODULE_NAME] = TEXT("");
	
	// モジュール列挙
	while(pCoreEngine->GetBonModuleName(pCoreEngine->EnumBonModule(dwModIndex++), szModName)){
		// モジュールカタログインスタンス生成
		IBonModCatalog *pModCatalog = ::BON_SAFE_CREATE<IBonModCatalog *>(szModName);

		// リストに追加
		if(pModCatalog)m_CatalogList.push_back(pModCatalog);
		}

	// クラスを列挙
	for(dwModIndex = 0UL ; dwModIndex < m_CatalogList.size() ; dwModIndex++){
		DWORD dwClassIndex = 0UL;
		
		while(m_CatalogList[dwModIndex]->EnumClass(lpszBIId, dwClassIndex) != BCID_NULL){
			// クラスリストに追加
			CLASS_CAT_INFO NewInfo = {m_CatalogList[dwModIndex], dwClassIndex++};
			m_ClassList.push_back(NewInfo);
			}
		}

	// 指定されたインタフェースのクラス数を返す
	return m_ClassList.size();
}

const DWORD CBonClassEnumerator::GetBonClassNum(void)
{
	// 列挙されたクラス数を返す
	return m_ClassList.size();
}

const DWORD CBonClassEnumerator::GetBonModuleName(DWORD dwIndex, LPTSTR lpszModuleName)
{
	// モジュール名を返す
	return (dwIndex < m_ClassList.size())? ::GET_BON_CLASS_NAME(m_ClassList[dwIndex].pModCatalog->GetGuid(), lpszModuleName) : 0UL;
}

const DWORD CBonClassEnumerator::GetBonClassName(DWORD dwIndex, LPTSTR lpszClassName)
{
	// クラス名を返す
	return (dwIndex < m_ClassList.size())? ::GET_BON_CLASS_NAME(m_ClassList[dwIndex].pModCatalog->EnumClass(m_szEnumBIId, m_ClassList[dwIndex].dwClassIndex), lpszClassName) : 0UL;
}

const DWORD CBonClassEnumerator::GetBonClassDesc(DWORD dwIndex, LPTSTR lpszDescription)
{
	// クラスの説明を返す
	return (dwIndex < m_ClassList.size())? m_ClassList[dwIndex].pModCatalog->GetClassDesc(m_ClassList[dwIndex].pModCatalog->EnumClass(m_szEnumBIId, m_ClassList[dwIndex].dwClassIndex), lpszDescription) : 0UL;
}

const DWORD CBonClassEnumerator::GetBonModuleComment(DWORD dwIndex, LPTSTR lpszComment)
{
	// モジュールの説明を返す
	return (dwIndex < m_ClassList.size())? m_ClassList[dwIndex].pModCatalog->GetComment(lpszComment) : 0UL;
}

const DWORD CBonClassEnumerator::GetBonModuleAuthor(DWORD dwIndex, LPTSTR lpszAuthor)
{
	// モジュールの作者名を返す
	return (dwIndex < m_ClassList.size())? m_ClassList[dwIndex].pModCatalog->GetAuthor(lpszAuthor) : 0UL;
}

const DWORD CBonClassEnumerator::GetBonModuleWebsite(DWORD dwIndex, LPTSTR lpszWebsite)
{
	// モジュールの連絡先を返す
	return (dwIndex < m_ClassList.size())? m_ClassList[dwIndex].pModCatalog->GetWebsite(lpszWebsite) : 0UL;
}

CBonClassEnumerator::CBonClassEnumerator(IBonObject *pOwner)
	: CBonObject(pOwner)
{
	// 何もしない
}

CBonClassEnumerator::~CBonClassEnumerator(void)
{
	// カタログインスタンス開放
	ReleaseCatalog();
}

void CBonClassEnumerator::ReleaseCatalog(void)
{
	// インタフェース名クリア
	m_szEnumBIId[0] = TEXT('\0');

	// クラスリストクリア
	m_ClassList.clear();
	
	// カタログインスタンス開放
	for(DWORD dwIndex = 0UL ; dwIndex < m_CatalogList.size() ; dwIndex++){
		m_CatalogList[dwIndex]->Release();
		}

	m_CatalogList.clear();
}
