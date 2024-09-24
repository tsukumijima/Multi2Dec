// TsTableBase.cpp: TSテーブル基底クラス
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "TsTableBase.h"


/////////////////////////////////////////////////////////////////////////////
// PSIテーブル基底クラス
/////////////////////////////////////////////////////////////////////////////

const bool CPsiTableBase::StorePacket(const ITsPacket *pTsPacket)
{
	// パケットをパーサに入力
	return m_SectionParser.StorePacket(pTsPacket);
}

void CPsiTableBase::OnPidReset(ITsPidMapper *pTsPidMapper, const WORD wPID)
{
	// 状態をリセット
	Reset();
}

void CPsiTableBase::OnPidMapped(ITsPidMapper *pTsPidMapper, const WORD wPID)
{
	// 参照カウンタインクリメント
	m_dwRefCount++;
	
	// リセット
	Reset();
}

void CPsiTableBase::OnPidUnmapped(ITsPidMapper *pTsPidMapper, const WORD wPID)
{
	// 参照カウンタデクリメント
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
	// 状態をリセット
	m_SectionParser.Reset();
	m_LastSection.Reset();
}

const DWORD CPsiTableBase::GetCrcErrorCount(void) const
{
	// CRCエラー数を返す
	return m_SectionParser.GetCrcErrorNum();
}

void CPsiTableBase::OnPsiSection(const IPsiSectionParser *pPsiSectionParser, const IPsiSection *pPsiSection)
{
	// デフォルトの実装ではセクションペイロード更新時に仮想関数に通知する
	if(m_LastSection.CompareSection(pPsiSection)){
		// セクションが更新された
		if(OnTableUpdate(pPsiSection, &m_LastSection)){
			// セクションストア
			m_LastSection.CopySection(pPsiSection);
			}
		}
}

const bool CPsiTableBase::OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection)
{
	// デフォルトの実装では何もしない
	return true;
}
