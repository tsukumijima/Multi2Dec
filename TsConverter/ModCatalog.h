// ModCatalog.h: モジュールカタログ実装クラス
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

/////////////////////////////////////////////////////////////////////////////
// モジュールカタログ実装クラス
/////////////////////////////////////////////////////////////////////////////

class TsConverter	:	public CModCatalogBase
{
public:
// CBonObject
	DECLARE_IBONOBJECT(TsConverter)

// TsConverter
	TsConverter(IBonObject *pOwner);
	virtual ~TsConverter(void);
};


/////////////////////////////////////////////////////////////////////////////
// グローバル変数
/////////////////////////////////////////////////////////////////////////////

extern HMODULE g_hModule;
