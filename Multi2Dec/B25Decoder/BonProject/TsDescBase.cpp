// TsDescBase.cpp: �f�X�N���v�^���N���X
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "TsDescBase.h"
#include "TsDescriptor.h"


/////////////////////////////////////////////////////////////////////////////
// �f�X�N���v�^���ʃC���^�t�F�[�X���N���X
/////////////////////////////////////////////////////////////////////////////

const DWORD CDescBase::ParseDesc(const BYTE *pData, const DWORD dwDataSize)
{
	// �L�q�q�̃f�t�H���g��͏���
	if(!pData)return 0UL;									// �f�[�^����
	else if(dwDataSize < 2U)return 0UL;						// �Œ�ł��^�O+�L�q�q����2�o�C�g�K�v
	else if(dwDataSize < (WORD)(pData[1] + 2U))return 0UL;	// �f�[�^���L�q�q�̃T�C�Y����������

	// �w�b�_�ۑ�
	m_byDescTag = pData[0];
	m_byDescLen = pData[1];

	// �f�[�^�|�C���^�ۑ�
	m_pDescData = (m_byDescLen)? &pData[2] : NULL;

	if(m_pDescData){
		// �f�[�^���
		if(!ParseData())return 0UL;
		}

	return m_byDescLen + 2UL;
}

const BYTE CDescBase::GetTag(void) const
{
	// �L�q�q�^�O��Ԃ�
	return m_byDescTag;
}

const BYTE CDescBase::GetLength(void) const
{
	// �L�q�q����Ԃ�(�^�O�ƋL�q�q���͏���)
	return m_byDescLen;
}

const BYTE * CDescBase::GetData(void) const
{
	// �L�q�q�f�[�^��Ԃ�
	return m_pDescData;
}

CDescBase::CDescBase(IBonObject *pOwner)
	: CBonObject(pOwner)
	, m_byDescTag(0x00U)
	, m_byDescLen(0x00U)
	, m_pDescData(NULL)
{

}

CDescBase::~CDescBase(void)
{

}

const bool CDescBase::ParseData(void)
{
	// �f�t�H���g�̏����ł͉������Ȃ�(�h���N���X�ŃI�[�o�[���C�h����)
	return true;
}


/////////////////////////////////////////////////////////////////////////////
// �f�X�N���v�^�W���C���^�t�F�[�X���N���X
/////////////////////////////////////////////////////////////////////////////

const DWORD CDescBlock::ParseDescs(void)
{
	// �L�q�q�C���X�^���X�N���A
	for(DWORD dwIndex = 0UL ; dwIndex < m_DescArray.size() ; dwIndex++){
		BON_SAFE_RELEASE(m_DescArray[dwIndex]);
		}
	
	m_DescArray.clear();

	// �f�[�^�|�C���^�擾
	const BYTE *pData = GetData();
	if(!pData || (GetSize() < 2UL))return 0UL;

	// �L�q�q���
	CDescBase *pDescBase;
	DWORD dwPos = 0UL;

	while((GetSize() - dwPos) >= 2UL){
		// �^�O�ɑΉ�����L�q�q�̃C���X�^���X����
		if(!(pDescBase = CreateDesc(pData[dwPos])))break;

		// �L�q�q���
		if(pDescBase->ParseDesc(&pData[dwPos], GetSize() - dwPos)){
			// �L�q�q�ǉ�
			m_DescArray.push_back(pDescBase);
			}
		else{
			// ��Ή� or �G���[
			::BON_ASSERT(false, TEXT("�L�q�q����Ή� or �G���[\r\nTag = %02X"), pData[dwPos]);
			
			pDescBase->Release();
			}

		// ��͈ʒu�X�V
		dwPos += (pData[dwPos + 1UL] + 2UL);
		}
		
	return m_DescArray.size();
}

void CDescBlock::Reset(void)
{
	// �L�q�q�C���X�^���X�N���A
	for(DWORD dwIndex = 0UL ; dwIndex < m_DescArray.size() ; dwIndex++){
		BON_SAFE_RELEASE(m_DescArray[dwIndex]);
		}
	
	m_DescArray.clear();

	// �f�[�^�N���A
	ClearSize();
}

const DWORD CDescBlock::GetDescNum(void) const
{
	// �L�q�q�̐���Ԃ�
	return m_DescArray.size();
}

const IDescBase * CDescBlock::GetDescByTag(const BYTE byTag, const DWORD dwIndex) const
{
	// �w�肳�ꂽ�^�O�̋L�q�q��Ԃ�
	for(DWORD dwPos = 0UL, dwCount = 0UL ; dwPos < m_DescArray.size() ; dwPos++){
		if(m_DescArray[dwPos]){
			if(m_DescArray[dwPos]->GetTag() == byTag){
				if(dwCount++ == dwIndex){
					return m_DescArray[dwPos];	
					}
				}		
			}
		}

	return NULL;
}

const IDescBase * CDescBlock::GetDescByIndex(const DWORD dwIndex) const
{
	// �C���f�b�N�X�Ŏw�肵���L�q�q��Ԃ�
	return (dwIndex < m_DescArray.size())? m_DescArray[dwIndex] : NULL;
}

const bool CDescBlock::CopyDescBlock(const IDescBlock *pSrc)
{
	// IMediaData�C���^�t�F�[�X���擾
	const IMediaData * const pMediaData = dynamic_cast<const IMediaData *>(pSrc);
	if(!pMediaData)return false;	
	
	// �ێ��f�[�^���Z�b�g
	Reset();
	
	// �f�[�^�R�s�[�A���
	if(CMediaData::CopyData(pMediaData)){
		return (ParseDescs())? true : false;		
		}

	return false;
}

const DWORD CDescBlock::CopyData(const IMediaData *pSrc)
{
	// IPsiSection�C���^�t�F�[�X�擾
	const IDescBlock * const pDescBlock = dynamic_cast<const IDescBlock *>(pSrc);

	if(pDescBlock){
		// IDescBlock�Ƃ��ăR�s�[
		return (CopyDescBlock(pDescBlock))? GetSize() : 0UL;
		}
	else{
		// IMediaData�Ƃ��ăR�s�[
		return CMediaData::CopyData(pSrc);
		}
}

CDescBlock::CDescBlock(IBonObject *pOwner)
	: CMediaData(pOwner)
{

}

CDescBlock::~CDescBlock(void)
{
	// �L�q�q�C���X�^���X�J��
	Reset();
}

CDescBase * CDescBlock::CreateDesc(const BYTE byTag)
{
	// �^�O�ɑΉ������L�q�q�̃C���X�^���X�𐶐�����
	switch(byTag){
		case CCaMethodDesc::DESC_TAG		: return dynamic_cast<CDescBase *>(CCaMethodDesc::CreateInstance(NULL));
		case CNetworkNameDesc::DESC_TAG		: return dynamic_cast<CDescBase *>(CNetworkNameDesc::CreateInstance(NULL));
		case CServiceListDesc::DESC_TAG		: return dynamic_cast<CDescBase *>(CServiceListDesc::CreateInstance(NULL));
		case CServiceDesc::DESC_TAG			: return dynamic_cast<CDescBase *>(CServiceDesc::CreateInstance(NULL));
		case CShortEventDesc::DESC_TAG		: return dynamic_cast<CDescBase *>(CShortEventDesc::CreateInstance(NULL));
		case CExtendEventDesc::DESC_TAG		: return dynamic_cast<CDescBase *>(CExtendEventDesc::CreateInstance(NULL));
		case CComponentDesc::DESC_TAG		: return dynamic_cast<CDescBase *>(CComponentDesc::CreateInstance(NULL));
		case CStreamIdDesc::DESC_TAG		: return dynamic_cast<CDescBase *>(CStreamIdDesc::CreateInstance(NULL));
		case CContentDesc::DESC_TAG			: return dynamic_cast<CDescBase *>(CContentDesc::CreateInstance(NULL));
		case CAudioComponentDesc::DESC_TAG	: return dynamic_cast<CDescBase *>(CAudioComponentDesc::CreateInstance(NULL));
		default :							  return dynamic_cast<CDescBase *>(CDescBase::CreateInstance(NULL));
		}
}
