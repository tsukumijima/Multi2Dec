// BonObject.h: �x�[�X�C���^�t�F�[�X���N���X
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// Bon�I�u�W�F�N�g���C���^�t�F�[�X���N���X
/////////////////////////////////////////////////////////////////////////////

class CBonObject	: public IBonObject
{
public:
// IBonObject
	virtual void Release(void);
	virtual const BONGUID GetGuid(void);

// CBonObject
	CBonObject(IBonObject *pOwner = NULL);
	virtual ~CBonObject(void);

	static IBonObject * CreateInstance(IBonObject *pOwner);

protected:
	static const BONGUID TypeToGuid(const type_info &TypeID);

	IBonObject *m_pOwner;
};


/////////////////////////////////////////////////////////////////////////////
// IBonObject�C���v�������g�}�N��
/////////////////////////////////////////////////////////////////////////////

#define DECLARE_IBONOBJECT(C)	\
	static IBonObject * CreateInstance(IBonObject *pOwner){return reinterpret_cast<IBonObject *>(new C(pOwner));}\
	virtual void Release(void){CBonObject::Release();}\
	virtual const BONGUID GetGuid(void){return CBonObject::TypeToGuid(typeid(C));}