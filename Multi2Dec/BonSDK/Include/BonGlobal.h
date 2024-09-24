// BonGlobal.h: �O���[�o���֐���`
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// �O���[�o���֐��錾 (�I�u�W�F�N�g�������\�b�h)
/////////////////////////////////////////////////////////////////////////////

#ifdef BONSDK_IMPLEMENT
	#define BONAPI	__declspec(dllexport)
#else
	#define BONAPI	__declspec(dllimport)
#endif

#include <typeinfo.h>


// Bon�R�A�G���W��API
extern "C"
{

BONAPI const BONGUID BON_NAME_TO_GUID(LPCTSTR lpszName);
BONAPI IBonObject * BON_CREATE_INSTANCE(LPCTSTR lpszClassName, IBonObject *pOwner);

BONAPI const bool QUERY_BON_MODULE(LPCTSTR lpszModuleName);
BONAPI const DWORD GET_BON_MODULE_NAME(const BONGUID ModuleId, LPTSTR lpszModuleName);

BONAPI const bool REGISTER_BON_CLASS(LPCTSTR lpszClassName, const CLASSFACTORYMETHOD pfnClassFactory, const DWORD dwPriority);
BONAPI const bool QUERY_BON_CLASS(LPCTSTR lpszClassName);
BONAPI const DWORD GET_BON_CLASS_NAME(const BONGUID ClassId, LPTSTR lpszClassName);
BONAPI const BONGUID GET_BON_CLASS_MODULE(const BONGUID ClassId);

BONAPI IBonObject * GET_STOCK_INSTANCE(LPCTSTR lpszClassName);
BONAPI const bool REGISTER_STOCK_INSTANCE(LPCTSTR lpszClassName, IBonObject *pInstance);
BONAPI const bool UNREGISTER_STOCK_INSTANCE(LPCTSTR lpszClassName);

}

// ���[�e�B���e�B�[���\�b�h
template <class T> const bool BON_REGISTER_CLASS(const DWORD dwPriority = 0UL)
{
#ifdef _UNICODE
	WCHAR szClassName[BON_MAX_CLASS_NAME] = TEXT("");

	// �N���X����UNICODE�ɕϊ�
	::MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, &(typeid(T).name()[6]), -1, szClassName, sizeof(szClassName) / sizeof(*szClassName));

	// �N���X�t�@�N�g���[�o�^
	return ::REGISTER_BON_CLASS(szClassName, T::CreateInstance, dwPriority);
#else
	// �N���X�t�@�N�g���[�o�^
	return ::REGISTER_BON_CLASS(&(typeid(T).name()[6]), T::CreateInstance, dwPriority);
#endif
}

template <class T> T BON_SAFE_CREATE(LPCTSTR lpszBCId, IBonObject *pOwner = NULL)
{
	// �C���X�^���X����
	IBonObject *pNewInstance = ::BON_CREATE_INSTANCE(lpszBCId, pOwner);

	// �C���^�t�F�[�X�擾
	T pDestInterface = dynamic_cast<T>(pNewInstance);

	// �ړI�̃C���^�t�F�[�X���������Ă��Ȃ��ꍇ�̓C���X�^���X�J��
	if(pNewInstance && !pDestInterface)pNewInstance->Release();

	return pDestInterface;
}
