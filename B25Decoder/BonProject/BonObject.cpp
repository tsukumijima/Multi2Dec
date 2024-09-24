// BonObject.cpp: ベースインタフェース基底クラス
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BonSDK.h"
#include "BonObject.h"


/////////////////////////////////////////////////////////////////////////////
// Bonオブジェクト基底インタフェース基底クラス
/////////////////////////////////////////////////////////////////////////////

void CBonObject::Release(void)
{
	// インスタンス開放
	delete this;
}

#ifdef _WIN32
const BONGUID CBonObject::GetGuid(void)
{
	// クラス名を返す
	return TypeToGuid(typeid(CBonObject));
}
#endif

CBonObject::CBonObject(IBonObject *pOwner)
	: m_pOwner(pOwner)
{
	// 何もしない
}

CBonObject::~CBonObject(void)
{
	// 何もしない
}

IBonObject * CBonObject::CreateInstance(IBonObject *pOwner)
{
	// インスタンス生成
	return reinterpret_cast<IBonObject *>(new CBonObject(pOwner));
}

#ifdef _WIN32
const BONGUID CBonObject::TypeToGuid(const type_info &TypeID)
{
	TCHAR szClassName[BON_MAX_CLASS_NAME] = TEXT("");

	// GetGuid()のインプリメントを提供する
#ifdef _UNICODE
	// クラス名をUNICODEに変換
	::MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, &(TypeID.name()[6]), -1, szClassName, sizeof(szClassName));
#else
	// クラス名をコピー
	::_tcscpy(szClassName, &(TypeID.name()[6]));
#endif

	// GUIDを返す
	return ::BON_NAME_TO_GUID(szClassName);
}
#endif
