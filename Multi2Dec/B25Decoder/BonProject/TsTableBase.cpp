// TsTableBase.cpp: TS�e�[�u�����N���X
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "TsTableBase.h"


/////////////////////////////////////////////////////////////////////////////
// PSI�e�[�u�����N���X
/////////////////////////////////////////////////////////////////////////////

const bool CPsiTableBase::StorePacket(const ITsPacket *pTsPacket)
{
	// �p�P�b�g���p�[�T�ɓ���
	return m_SectionParser.StorePacket(pTsPacket);
}

void CPsiTableBase::OnPidReset(ITsPidMapper *pTsPidMapper, const WORD wPID)
{
	// ��Ԃ����Z�b�g
	Reset();
}

void CPsiTableBase::OnPidMapped(ITsPidMapper *pTsPidMapper, const WORD wPID)
{
	// �Q�ƃJ�E���^�C���N�������g
	m_dwRefCount++;
	
	// ���Z�b�g
	Reset();
}

void CPsiTableBase::OnPidUnmapped(ITsPidMapper *pTsPidMapper, const WORD wPID)
{
	// �Q�ƃJ�E���^�f�N�������g
	if(!(--m_dwRefCount))Release();
}

CPsiTableBase::CPsiTableBase(IBonObject *pOwner)
	: CBonObject(pOwner)
	, m_SectionParser(dynamic_cast<IPsiSectionParser::IHandler *>(this))
	, m_LastSection(NULL)
	, m_dwRefCount(0UL)
{

}

CPsiTableBase::~CPsiTableBase(void)
{

}

void CPsiTableBase::Reset(void)
{
	// ��Ԃ����Z�b�g
	m_SectionParser.Reset();
	m_LastSection.Reset();
}

const DWORD CPsiTableBase::GetCrcErrorCount(void) const
{
	// CRC�G���[����Ԃ�
	return m_SectionParser.GetCrcErrorNum();
}

void CPsiTableBase::OnPsiSection(const IPsiSectionParser *pPsiSectionParser, const IPsiSection *pPsiSection)
{
	// �f�t�H���g�̎����ł̓Z�N�V�����y�C���[�h�X�V���ɉ��z�֐��ɒʒm����
	if(m_LastSection.CompareSection(pPsiSection)){
		// �Z�N�V�������X�V���ꂽ
		if(OnTableUpdate(pPsiSection, &m_LastSection)){
			// �Z�N�V�����X�g�A
			m_LastSection.CopySection(pPsiSection);
			}
		}
}

const bool CPsiTableBase::OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection)
{
	// �f�t�H���g�̎����ł͉������Ȃ�
	return true;
}
