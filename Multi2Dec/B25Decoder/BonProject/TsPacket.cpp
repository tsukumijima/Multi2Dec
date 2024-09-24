// TsPacket.cpp: TS�p�P�b�g���N���X
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "TsPacket.h"


/////////////////////////////////////////////////////////////////////////////
// ITsPacket���N���X
/////////////////////////////////////////////////////////////////////////////

const DWORD CTsPacket::ParsePacket(BYTE *pContinuityCounter)
{
	// TS�p�P�b�g�w�b�_���
	m_Header.bySyncByte					= m_pData[0];							// +0
	m_Header.bTransportErrorIndicator	= (m_pData[1] & 0x80U)? true : false;	// +1 bit7
	m_Header.bPayloadUnitStartIndicator	= (m_pData[1] & 0x40U)? true : false;	// +1 bit6
	m_Header.bTransportPriority			= (m_pData[1] & 0x20U)? true : false;	// +1 bit5
	m_Header.wPID = ((WORD)(m_pData[1] & 0x1F) << 8) | (WORD)m_pData[2];		// +1 bit4-0, +2
	m_Header.byTransportScramblingCtrl	= (m_pData[3] & 0xC0U) >> 6;			// +3 bit7-6
	m_Header.byAdaptationFieldCtrl		= (m_pData[3] & 0x30U) >> 4;			// +3 bit5-4
	m_Header.byContinuityCounter		= m_pData[3] & 0x0FU;					// +3 bit3-0
	
	// �A�_�v�e�[�V�����t�B�[���h���
	::ZeroMemory(&m_AdaptationField, sizeof(m_AdaptationField));
	
	if(m_Header.byAdaptationFieldCtrl & 0x02){
		// �A�_�v�e�[�V�����t�B�[���h����
		if(m_AdaptationField.byAdaptationFieldLength = m_pData[4]){								// +4
			// �t�B�[���h���ȍ~����
			m_AdaptationField.bDiscontinuityIndicator	= (m_pData[5] & 0x80U)? true : false;	// +5 bit7
			m_AdaptationField.bRamdomAccessIndicator	= (m_pData[5] & 0x40U)? true : false;	// +5 bit6
			m_AdaptationField.bEsPriorityIndicator		= (m_pData[5] & 0x20U)? true : false;	// +5 bit5
			m_AdaptationField.bPcrFlag					= (m_pData[5] & 0x10U)? true : false;	// +5 bit4
			m_AdaptationField.bOpcrFlag					= (m_pData[5] & 0x08U)? true : false;	// +5 bit3
			m_AdaptationField.bSplicingPointFlag		= (m_pData[5] & 0x04U)? true : false;	// +5 bit2
			m_AdaptationField.bTransportPrivateDataFlag	= (m_pData[5] & 0x02U)? true : false;	// +5 bit1
			m_AdaptationField.bAdaptationFieldExtFlag	= (m_pData[5] & 0x01U)? true : false;	// +5 bit0
			
			if(m_pData[4] > 1U){
				m_AdaptationField.pOptionData			= &m_pData[6];
				m_AdaptationField.byOptionSize			= m_pData[4] - 1U;
				}
			}
		}

	// �p�P�b�g�̃t�H�[�}�b�g�K�������`�F�b�N����
	if(m_Header.bySyncByte != 0x47U)return EC_FORMAT;								// �����o�C�g�s��
	if(m_Header.bTransportErrorIndicator)return EC_TRANSPORT;						// �r�b�g��肠��
	if((m_Header.wPID >= 0x0002U) && (m_Header.wPID <= 0x000FU))return EC_FORMAT;	// ����`PID�͈�
	if(m_Header.byTransportScramblingCtrl == 0x01U)return EC_FORMAT;				// ����`�X�N�����u������l
	if(m_Header.byAdaptationFieldCtrl == 0x00U)return EC_FORMAT;					// ����`�A�_�v�e�[�V�����t�B�[���h����l
	if((m_Header.byAdaptationFieldCtrl == 0x02U) && (m_AdaptationField.byAdaptationFieldLength > 183U))return EC_FORMAT;	// �A�_�v�e�[�V�����t�B�[���h���ُ�
	if((m_Header.byAdaptationFieldCtrl == 0x03U) && (m_AdaptationField.byAdaptationFieldLength > 182U))return EC_FORMAT;	// �A�_�v�e�[�V�����t�B�[���h���ُ�

	if(!pContinuityCounter || m_Header.wPID == 0x1FFFU)return EC_VALID;

	// �A�����`�F�b�N
	const BYTE byOldCounter = pContinuityCounter[m_Header.wPID];
	const BYTE byNewCounter = (m_Header.byAdaptationFieldCtrl & 0x01U)? m_Header.byContinuityCounter : 0x10U;
	pContinuityCounter[m_Header.wPID] = byNewCounter;

	if(!m_AdaptationField.bDiscontinuityIndicator){
		if((byOldCounter < 0x10U) && (byNewCounter < 0x10U)){
			if(((byOldCounter + 1U) & 0x0FU) != byNewCounter){
				return EC_CONTINUITY;
				}
			}
		}

	return EC_VALID;
}

BYTE * CTsPacket::GetRawData(void) const
{
	// �p�P�b�g�̃o�C�i���f�[�^��Ԃ�
	return (m_dwDataSize == TS_PACKET_SIZE)? m_pData : NULL;
}

const WORD CTsPacket::GetPID(void) const
{
	// PID��Ԃ�
	return m_Header.wPID;
}

const bool CTsPacket::IsScrambled(void) const
{
	// �X�N�����u���L����Ԃ�
	return (m_Header.byTransportScramblingCtrl & 0x02U)? true : false;
}

const bool CTsPacket::HaveAdaptationField(void) const
{
	// �A�_�v�e�[�V�����t�B�[���h�̗L����Ԃ�
	return (m_Header.byAdaptationFieldCtrl & 0x02U)? true : false;
}

BYTE * CTsPacket::GetPayloadData(void) const
{
	// �y�C���[�h�|�C���^��Ԃ�
	switch(m_Header.byAdaptationFieldCtrl){
		case 1U :	// �y�C���[�h�̂�
			return &m_pData[4];
		
		case 3U :	// �A�_�v�e�[�V�����t�B�[���h�A�y�C���[�h����
			return &m_pData[m_AdaptationField.byAdaptationFieldLength + 5U];
		
		default :	// �A�_�v�e�[�V�����t�B�[���h�̂� or ��O
			return NULL;
		}
}

const BYTE CTsPacket::GetPayloadSize(void) const
{
	// �y�C���[�h�T�C�Y��Ԃ�
	switch(m_Header.byAdaptationFieldCtrl){
		case 1U :	// �y�C���[�h�̂�
			return (TS_PACKET_SIZE - 4U);
		
		case 3U :	// �A�_�v�e�[�V�����t�B�[���h�A�y�C���[�h����
			return (TS_PACKET_SIZE - m_AdaptationField.byAdaptationFieldLength - 5U);
		
		default :	// �A�_�v�e�[�V�����t�B�[���h�̂� or ��O
			return 0U;
		}
}

const bool CTsPacket::CopyPacket(const ITsPacket *pSrc)
{
	if(!pSrc)return false;
	
	// �o�C�i���f�[�^���擾
	const BYTE *pSrcRawData = pSrc->GetRawData();
	if(!pSrcRawData)return false;

	// �f�[�^���R�s�[
	SetData(pSrcRawData, TS_PACKET_SIZE);
	
	// �p�P�b�g���
	ParsePacket();

	return true;
}

const bool CTsPacket::ComparePacket(const ITsPacket *pSrc) const
{
	// �p�P�b�g��r����
	if(!pSrc)return false;

	const BYTE *pSrcRawData = pSrc->GetRawData();
	
	// �T�C�Y���`�F�b�N
	if(!pSrcRawData || (m_dwDataSize != TS_PACKET_SIZE))return false;

	// �p�P�b�g���r
	return (!::memcmp(pSrcRawData, m_pData, TS_PACKET_SIZE))? true : false;
}

const DWORD CTsPacket::CopyData(const IMediaData *pSrc)
{
	// ITsPacket�C���^�t�F�[�X�擾
	const ITsPacket * const pTsPacket = dynamic_cast<const ITsPacket *>(pSrc);

	if(pTsPacket){
		// ITsPacket�Ƃ��ăR�s�[
		return (CopyPacket(pTsPacket))? TS_PACKET_SIZE : 0UL;
		}
	else{
		// IMediaData�Ƃ��ăR�s�[
		return CMediaData::CopyData(pSrc);
		}
}

CTsPacket::CTsPacket(IBonObject *pOwner)
	: CMediaData(pOwner)
{
	// �o�b�t�@�m��
	GetBuffer(TS_PACKET_SIZE);
	
	// �w�b�_���N���A
	::ZeroMemory(&m_Header, sizeof(m_Header));
	::ZeroMemory(&m_AdaptationField, sizeof(m_AdaptationField));
}

CTsPacket::~CTsPacket(void)
{

}
