// BonObject.cpp: �x�[�X�C���^�t�F�[�X���N���X
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "BonObject.h"
#include <TChar.h>


/////////////////////////////////////////////////////////////////////////////
// Bon�I�u�W�F�N�g���C���^�t�F�[�X���N���X
/////////////////////////////////////////////////////////////////////////////

void CBonObject::Release(void)
{
	// �C���X�^���X�J��
	delete this;
}

const BONGUID CBonObject::GetGuid(void)
{
	// �N���X����Ԃ�
	return TypeToGuid(typeid(CBonObject));
}

CBonObject::CBonObject(IBonObject *pOwner)
	: m_pOwner(pOwner)
{
	// �������Ȃ�
}

CBonObject::~CBonObject(void)
{
	// �������Ȃ�
}

IBonObject * CBonObject::CreateInstance(IBonObject *pOwner)
{
	// �C���X�^���X����
	return reinterpret_cast<IBonObject *>(new CBonObject(pOwner));
}

const BONGUID CBonObject::TypeToGuid(const type_info &TypeID)
{
	TCHAR szClassName[BON_MAX_CLASS_NAME] = TEXT("");

	// GetGuid()�̃C���v�������g��񋟂���
#ifdef _UNICODE
	// �N���X����UNICODE�ɕϊ�
	::MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, &(TypeID.name()[6]), -1, szClassName, sizeof(szClassName));
#else
	// �N���X�����R�s�[
	::_tcscpy(szClassName, &(TypeID.name()[6]));
#endif

	// GUID��Ԃ�
	return ::BON_NAME_TO_GUID(szClassName);
}
