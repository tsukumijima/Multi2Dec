// TsPidMapper.cpp: PID�}�b�p���N���X
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "TsPidMapper.h"


/////////////////////////////////////////////////////////////////////////////
// ITsPidMapper���N���X
/////////////////////////////////////////////////////////////////////////////

const bool CTsPidMapper::StorePacket(const ITsPacket *pTsPacket)
{
	if(!pTsPacket)return false;

	const WORD wPID = pTsPacket->GetPID();
	
	if(wPID >= 0x2000U)return false;				// PID�͈͊O
	if(!m_apMapTarget[wPID])return false;			// PID�}�b�v�^�[�Q�b�g�Ȃ�

	// PID�}�b�v�^�[�Q�b�g�Ƀp�P�b�g����
	return m_apMapTarget[wPID]->StorePacket(pTsPacket);
}

void CTsPidMapper::ResetPid(const WORD wPID)
{
	if(wPID < 0x2000U){
		// �w��^�[�Q�b�g�����Z�b�g
		if(m_apMapTarget[wPID]){
			m_apMapTarget[wPID]->OnPidReset(this, wPID);
			}
		}
	else{
		// �S�^�[�Q�b�g�����Z�b�g
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

	// ���݂̃^�[�Q�b�g���A���}�b�v
	UnmapPid(wPID);

	// �V�����^�[�Q�b�g���}�b�v
	m_apMapTarget[wPID] = pTsPidMapTarget;
	m_wMapTargetNum++;
	
	// �n���h���Ăяo��
	pTsPidMapTarget->OnPidMapped(this, wPID);

	return true;
}

const bool CTsPidMapper::UnmapPid(const WORD wPID)
{
	if(wPID >= 0x2000U)return false;

	if(!m_apMapTarget[wPID])return false;

	// ���݂̃^�[�Q�b�g���A���}�b�v
	ITsPidMapTarget * const pOldTarget = m_apMapTarget[wPID];
	m_apMapTarget[wPID] = NULL;
	m_wMapTargetNum--;

	// �n���h���Ăяo��
	pOldTarget->OnPidUnmapped(this, wPID);

	return true;
}

void CTsPidMapper::ClearPid(void)
{
	// �S�^�[�Q�b�g���A���}�b�v
	for(WORD wPID = 0x0000U ; wPID < 0x2000U ; wPID++){
		UnmapPid(wPID);
		}
}

ITsPidMapTarget * CTsPidMapper::GetMapTarget(const WORD wPID) const
{
	// �}�b�v����Ă���^�[�Q�b�g��Ԃ�
	return (wPID <= 0x1FFFU)? m_apMapTarget[wPID] : NULL;
}

const WORD CTsPidMapper::GetMapNum(void) const
{
	// �}�b�v�ς�PID����Ԃ�
	return m_wMapTargetNum;
}

CTsPidMapper::CTsPidMapper(IBonObject *pOwner)
	: CBonObject(pOwner)
	, m_wMapTargetNum(0U)
{
	// PID�}�b�v������
	::ZeroMemory(m_apMapTarget, sizeof(m_apMapTarget));
}

CTsPidMapper::~CTsPidMapper(void)
{
	ClearPid();
}
