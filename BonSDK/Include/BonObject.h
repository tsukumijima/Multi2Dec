// BonObject.h: ベースインタフェース基底クラス
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// Bonオブジェクト基底インタフェース基底クラス
/////////////////////////////////////////////////////////////////////////////

class CBonObject	: public IBonObject
{
public:
// IBonObject
	virtual void Release(void);
#ifdef _WIN32
	virtual const BONGUID GetGuid(void);
#endif

// CBonObject
	CBonObject(IBonObject *pOwner = NULL);
	virtual ~CBonObject(void);

	static IBonObject * CreateInstance(IBonObject *pOwner);

protected:
#ifdef _WIN32
	static const BONGUID TypeToGuid(const type_info &TypeID);
#endif

	IBonObject *m_pOwner;
};


/////////////////////////////////////////////////////////////////////////////
// IBonObjectインプリメントマクロ
/////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#define DECLARE_IBONOBJECT(C)	\
	static IBonObject * CreateInstance(IBonObject *pOwner){return reinterpret_cast<IBonObject *>(new C(pOwner));}\
	virtual void Release(void){CBonObject::Release();}\
	virtual const BONGUID GetGuid(void){return CBonObject::TypeToGuid(typeid(C));}
#else
#define DECLARE_IBONOBJECT(C)	\
	static IBonObject * CreateInstance(IBonObject *pOwner){return reinterpret_cast<IBonObject *>(new C(pOwner));}\
	virtual void Release(void){CBonObject::Release();}
#endif
