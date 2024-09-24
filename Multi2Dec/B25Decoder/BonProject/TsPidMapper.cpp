// TsPidMapper.cpp: PIDマッパ基底クラス
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "TsPidMapper.h"


/////////////////////////////////////////////////////////////////////////////
// ITsPidMapper基底クラス
/////////////////////////////////////////////////////////////////////////////

const bool CTsPidMapper::StorePacket(const ITsPacket *pTsPacket)
{
	if(!pTsPacket)return false;

	const WORD wPID = pTsPacket->GetPID();
	
	if(wPID >= 0x2000U)return false;				// PID範囲外
	if(!m_apMapTarget[wPID])return false;			// PIDマップターゲットなし

	// PIDマップターゲットにパケット入力
	return m_apMapTarget[wPID]->StorePacket(pTsPacket);
}

void CTsPidMapper::ResetPid(const WORD wPID)
{
	if(wPID < 0x2000U){
		// 指定ターゲットをリセット
		if(m_apMapTarget[wPID]){
			m_apMapTarget[wPID]->OnPidReset(this, wPID);
			}
		}
	else{
		// 全ターゲットをリセット
		for(WORD wPID = 0x0000U ; wPID < 0x2000U ; wPID++){
			if(m_apMapTarget[wPID]){
				m_apMapTarget[wPID]->OnPidReset(this, wPID);
				}
			}
		}
}

const bool CTsPidMapper::MapPid(const WORD wPID, ITsPidMapTarget *pTsPidMapTarget)
{
	if((wPID >= 0x2000U) || !pTsPidMapTarget)return false;

	// 現在のターゲットをアンマップ
	UnmapPid(wPID);

	// 新しいターゲットをマップ
	m_apMapTarget[wPID] = pTsPidMapTarget;
	m_wMapTargetNum++;
	
	// ハンドラ呼び出し
	pTsPidMapTarget->OnPidMapped(this, wPID);

	return true;
}

const bool CTsPidMapper::UnmapPid(const WORD wPID)
{
	if(wPID >= 0x2000U)return false;

	if(!m_apMapTarget[wPID])return false;

	// 現在のターゲットをアンマップ
	ITsPidMapTarget * const pOldTarget = m_apMapTarget[wPID];
	m_apMapTarget[wPID] = NULL;
	m_wMapTargetNum--;

	// ハンドラ呼び出し
	pOldTarget->OnPidUnmapped(this, wPID);

	return true;
}

void CTsPidMapper::ClearPid(void)
{
	// 全ターゲットをアンマップ
	for(WORD wPID = 0x0000U ; wPID < 0x2000U ; wPID++){
		UnmapPid(wPID);
		}
}

ITsPidMapTarget * CTsPidMapper::GetMapTarget(const WORD wPID) const
{
	// マップされているターゲットを返す
	return (wPID <= 0x1FFFU)? m_apMapTarget[wPID] : NULL;
}

const WORD CTsPidMapper::GetMapNum(void) const
{
	// マップ済みPID数を返す
	return m_wMapTargetNum;
}

CTsPidMapper::CTsPidMapper(IBonObject *pOwner)
	: CBonObject(pOwner)
	, m_wMapTargetNum(0U)
{
	// PIDマップ初期化
	::ZeroMemory(m_apMapTarget, sizeof(m_apMapTarget));
}

CTsPidMapper::~CTsPidMapper(void)
{
	ClearPid();
}
