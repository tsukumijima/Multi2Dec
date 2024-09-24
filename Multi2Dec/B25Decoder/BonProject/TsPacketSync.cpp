// TsPacketSync.cpp: TS�p�P�b�g�����f�R�[�_
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "TsPacketSync.h"


/////////////////////////////////////////////////////////////////////////////
// �t�@�C�����[�J���萔�ݒ�
/////////////////////////////////////////////////////////////////////////////

#define STRIDE_FILTER	10UL		// �p�P�b�g��������t�B���^��
#define BITRATE_PERIOD	1000UL		// �r�b�g���[�g�X�V����(1000ms)

// �N���b�v�t���C���N�������g�}�N��
#define CLIPED_INCREMENT(V)			{if((V) < 0xFFFFFFFFUL)(V)++;}


/////////////////////////////////////////////////////////////////////////////
// TS�p�P�b�g�����f�R�[�_
/////////////////////////////////////////////////////////////////////////////

const bool CTsPacketSync::InputMedia(IMediaData *pMediaData, const DWORD dwInputIndex)
{
	// ���̓p�����[�^�`�F�b�N
	if(!pMediaData || (dwInputIndex >= GetInputNum()))return false;

	// �p�P�b�g����
	SyncPacket(pMediaData->GetData(), pMediaData->GetSize());

	return true;
}

void CTsPacketSync::DiscardNullPacket(const bool bEnable)
{
	// NULL�p�P�b�g�̔j����L��/�������ݒ�
	m_bDiscardNullPacket = (bEnable)? true : false;
}

void CTsPacketSync::ResetStatistics(void)
{
	// �p�P�b�g�����J�E���^�����Z�b�g
	m_dwCurStrideSize = 0UL;
	m_dwPrvStrideSize = 0UL;
	m_dwFixStrideSize = 0UL;
	m_dwFixStrideCount = 0UL;

	// ���v�f�[�^�����Z�b�g����
	m_dwInputPacketNum = 0UL;
	m_dwOutputPacketNum = 0UL;
	m_dwSyncErrNum = 0UL;
	m_dwFormatErrNum = 0UL;
	m_dwTransportErrNum = 0UL;
	m_dwContinuityErrNum = 0UL;
	m_dwLastUpdateTime = ::GetTickCount();
	m_dwLastInputPacket = 0UL;
	m_dwLastOutputPacket = 0UL;
	m_dwCurInputPacket = 0UL;
	m_dwCurOutputPacket = 0UL;

	::ZeroMemory(m_adwInputPacketNum, sizeof(m_adwInputPacketNum));
	::ZeroMemory(m_adwOutputPacketNum, sizeof(m_adwOutputPacketNum));
	::ZeroMemory(m_adwContinuityErrNum, sizeof(m_adwContinuityErrNum));

	// �p�P�b�g�A�����J�E���^������������
	::FillMemory(m_abyContCounter, sizeof(m_abyContCounter), 0x10UL);
}

const DWORD CTsPacketSync::GetPacketStride(void)
{
	// �p�P�b�g������Ԃ�
	return m_dwFixStrideSize;
}

const DWORD CTsPacketSync::GetInputPacketNum(const WORD wPID)
{
	// ���̓p�P�b�g����Ԃ�(�G���[�p�P�b�g�����ANULL�p�P�b�g�܂�)
	return (wPID < 0x2000U)? m_adwInputPacketNum[wPID] : m_dwInputPacketNum;
}

const DWORD CTsPacketSync::GetOutputPacketNum(const WORD wPID)
{
	// �o�̓p�P�b�g����Ԃ�(NULL�p�P�b�g����)
	return (wPID < 0x2000U)? m_adwOutputPacketNum[wPID] : m_dwOutputPacketNum;
}

const DWORD CTsPacketSync::GetInputBitrate(void)
{
	// ���̓r�b�g���[�g��Ԃ�(�p�P�b�g�T�C�Y�~1�b�Ԃ̓��̓p�P�b�g���~8bit)
	return m_dwLastInputPacket * TS_PACKET_SIZE * 8UL;
}

const DWORD CTsPacketSync::GetOutputBitrate(void)
{
	// �o�̓r�b�g���[�g��Ԃ�(�p�P�b�g�T�C�Y�~1�b�Ԃ̓��̓p�P�b�g���~8bit)
	return m_dwLastInputPacket * TS_PACKET_SIZE * 8UL;
}

const DWORD CTsPacketSync::GetSyncErrNum(void)
{
	// �����G���[����Ԃ�(�p�P�b�g���E������郂�[�h)
	return m_dwSyncErrNum;
}

const DWORD CTsPacketSync::GetFormatErrNum(void)
{
	// �t�H�[�}�b�g�G���[����Ԃ�(�p�P�b�g�w�b�_���K�i�O�̃��[�h)
	return m_dwFormatErrNum;
}

const DWORD CTsPacketSync::GetTransportErrNum(void)
{
	// �g�����X�|�[�g�G���[����Ԃ�(Transport Error Indicator�������[�h)
	return m_dwTransportErrNum;
}

const DWORD CTsPacketSync::GetContinuityErrNum(const WORD wPID)
{
	// �A�����G���[����Ԃ�(Continuity Counter���s�A���ɂȂ郂�[�h)
	return (wPID < 0x2000U)? m_adwContinuityErrNum[wPID] : m_dwContinuityErrNum;
}

CTsPacketSync::CTsPacketSync(IBonObject *pOwner)
	: CMediaDecoder(pOwner, 1UL, 1UL)
	, m_TsPacket(NULL)
	, m_dwCurStrideSize(0UL)
	, m_dwPrvStrideSize(0UL)
	, m_dwFixStrideSize(0UL)
	, m_dwFixStrideCount(0UL)
	, m_bDiscardNullPacket(true)
	, m_dwInputPacketNum(0UL)
	, m_dwOutputPacketNum(0UL)
	, m_dwSyncErrNum(0UL)
	, m_dwFormatErrNum(0UL)
	, m_dwTransportErrNum(0UL)
	, m_dwContinuityErrNum(0UL)
	, m_dwLastUpdateTime(0UL)
	, m_dwLastInputPacket(0UL)
	, m_dwLastOutputPacket(0UL)
	, m_dwCurInputPacket(0UL)
	, m_dwCurOutputPacket(0UL)
{
	// ���v�f�[�^������
	ResetStatistics();
}

CTsPacketSync::~CTsPacketSync(void)
{
	// �������Ȃ�
}

const bool CTsPacketSync::OnPlay(void)
{
	// ������Ԃ�����������
	return OnReset();
}

const bool CTsPacketSync::OnReset(void)
{
	// �p�P�b�g�T�C�Y�N���A
	m_TsPacket.ClearSize();

	// ���v�f�[�^���Z�b�g
	ResetStatistics();
	
	return true;
}

void CTsPacketSync::SyncPacket(const BYTE *pStream, const DWORD dwSize)
{
	DWORD dwCurPos = 0UL;
	DWORD dwCurSize;

	while(dwCurPos < dwSize){
		dwCurSize = m_TsPacket.GetSize();

		if(!dwCurSize){
			// �����o�C�g�҂���
			do{
				m_dwCurStrideSize++;
				
				if(pStream[dwCurPos++] == 0x47U){
					// �����o�C�g����
					m_TsPacket.AddByte(0x47U);
					break;
					}
				}
			while(dwCurPos < dwSize);
			}
		else{
			// �f�[�^�҂�
			if((dwSize - dwCurPos) >= (TS_PACKET_SIZE - dwCurSize)){
				// �p�P�b�g�T�C�Y���f�[�^���������
				m_TsPacket.AddData(&pStream[dwCurPos], TS_PACKET_SIZE - dwCurSize);
				m_dwCurStrideSize += (TS_PACKET_SIZE - dwCurSize);
				dwCurPos += (TS_PACKET_SIZE - dwCurSize);
				
				// �w�b�_���
				if(ParseTsPacket()){
					// �p�P�b�g����

					// �����G���[����
					if((m_dwFixStrideSize != m_dwCurStrideSize) && m_dwFixStrideSize){
						// �p�P�b�g�����ُ�
						CLIPED_INCREMENT(m_dwSyncErrNum);
						SendDecoderEvent(EID_SYNC_ERR, reinterpret_cast<PVOID>(m_dwSyncErrNum));
						}

					// �p�P�b�g�����t�B���^�����O
					if(m_dwCurStrideSize == m_dwPrvStrideSize){
						if(++m_dwFixStrideCount >= STRIDE_FILTER){
							// �p�P�b�g�����m��
							m_dwFixStrideSize = m_dwCurStrideSize;
							m_dwFixStrideCount = STRIDE_FILTER;
							}
						}
					else{
						m_dwPrvStrideSize = m_dwCurStrideSize;
						m_dwFixStrideCount = 0UL;
						}

					// �p�P�b�g�o��
					OnTsPacket();

					// �r�b�g���[�g�X�V
					UpdateBitrate();
					}

				// �p�P�b�g�T�C�Y�N���A
				m_TsPacket.ClearSize();
				m_dwCurStrideSize = 0UL;			
				}	
			else{
				// �c��̃f�[�^�̓p�P�b�g�T�C�Y�ɖ����Ȃ�
				m_TsPacket.AddData(&pStream[dwCurPos], dwSize - dwCurPos);
				m_dwCurStrideSize += (dwSize - dwCurPos);
				dwCurPos += (dwSize - dwCurPos);
				}			
			}
		}
}

const bool CTsPacketSync::ParseTsPacket(void)
{
	// �p�P�b�g����͂���
	switch(m_TsPacket.ParsePacket(m_abyContCounter)){
		case ITsPacket::EC_VALID :
			// �p�P�b�g����
			CLIPED_INCREMENT(m_dwInputPacketNum);
			CLIPED_INCREMENT(m_adwInputPacketNum[m_TsPacket.GetPID()]);
			CLIPED_INCREMENT(m_dwCurInputPacket);
			return true;

		case ITsPacket::EC_FORMAT :
			// �t�H�[�}�b�g�ُ�
			CLIPED_INCREMENT(m_dwFormatErrNum);
			SendDecoderEvent(EID_FORMAT_ERR, reinterpret_cast<PVOID>(m_dwFormatErrNum));
			return false;
		
		case ITsPacket::EC_TRANSPORT :
			// �g�����X�|�[�g�G���[�ُ�
			CLIPED_INCREMENT(m_dwTransportErrNum);
			SendDecoderEvent(EID_TRANSPORT_ERR, reinterpret_cast<PVOID>(m_dwTransportErrNum));
			return false;
	
		case ITsPacket::EC_CONTINUITY :
			// �A�����ُ�(�h���b�v)�@���p�P�b�g�̔j���͂��Ȃ�
			CLIPED_INCREMENT(m_dwInputPacketNum);
			CLIPED_INCREMENT(m_adwInputPacketNum[m_TsPacket.GetPID()]);
			CLIPED_INCREMENT(m_dwContinuityErrNum);
			CLIPED_INCREMENT(m_adwContinuityErrNum[m_TsPacket.GetPID()]);
			CLIPED_INCREMENT(m_dwCurInputPacket);
			SendDecoderEvent(EID_CONTINUITY_ERR, reinterpret_cast<PVOID>(m_dwContinuityErrNum));
			return true;
		
		default :
			// ��O
			::DebugBreak();
			return false;
		}
}

void CTsPacketSync::OnTsPacket(void)
{
	// ����p�P�b�g�o��
	const WORD wPID = m_TsPacket.GetPID();

	// NULL�p�P�b�g�͔j������
	if(m_bDiscardNullPacket && (wPID == 0x1FFFU))return;

	// �o�̓p�P�b�g���C���N�������g
	CLIPED_INCREMENT(m_dwOutputPacketNum);
	CLIPED_INCREMENT(m_adwOutputPacketNum[wPID]);
	CLIPED_INCREMENT(m_dwCurOutputPacket);
	
	// ���ʃf�R�[�_�Ƀf�[�^�o��
	OutputMedia(&m_TsPacket);
}

void CTsPacketSync::UpdateBitrate(void)
{
	// �r�b�g���[�g�X�V
	const DWORD dwCurUpdateTime = ::GetTickCount();
	
	// �X�V�����`�F�b�N
	if((dwCurUpdateTime - m_dwLastUpdateTime) < BITRATE_PERIOD)return;

	// �J�E���^�X�V
	m_dwLastInputPacket = m_dwCurInputPacket;
	m_dwLastOutputPacket = m_dwCurOutputPacket;

	// �J�E���^���Z�b�g
	m_dwCurInputPacket = 0UL;
	m_dwCurOutputPacket = 0UL;
	
	// �����X�V
	m_dwLastUpdateTime = dwCurUpdateTime;
}
