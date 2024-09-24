// B25Decoder.cpp: CCB25Decoder �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "B25Decoder.h"


//////////////////////////////////////////////////////////////////////
// �C���X�^���X�������\�b�h
//////////////////////////////////////////////////////////////////////

extern "C"
{

B25DECAPI IB25Decoder * CreateB25Decoder()
{
	// �C���X�^���X����
	return dynamic_cast<IB25Decoder *>(new CB25Decoder(NULL));
}

B25DECAPI IB25Decoder2 * CreateB25Decoder2()
{
	// �C���X�^���X����
	return dynamic_cast<IB25Decoder2 *>(new CB25Decoder(NULL));
}

}


//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

CB25Decoder::CB25Decoder(IBonObject * const pOwner)
	: CBonObject(pOwner)
	, m_TsPacketSync(NULL)
	, m_TsDescrambler(NULL)
	, m_MediaGrabber(dynamic_cast<IMediaDecoderHandler *>(this))
	, m_InputBuff((BYTE)0x00U, 0x100000UL)	// 1MB�̃o�b�t�@���m��
	, m_OutputBuff((BYTE)0x00U, 0x100000UL)	// 1MB�̃o�b�t�@���m��
{
	// �f�R�[�_�O���t�\�z
	m_TsPacketSync.SetOutputDecoder(&m_TsDescrambler);
	m_TsDescrambler.SetOutputDecoder(&m_MediaGrabber);
}

CB25Decoder::~CB25Decoder(void)
{
	// �������Ȃ�
}

const BOOL CB25Decoder::Initialize(DWORD dwRound)
{
	// �f�R�[�_�I�[�v���A�f�R�[�_�O���t�Đ�
	return Reset();
}

const BOOL CB25Decoder::Decode(BYTE *pSrcBuf, const DWORD dwSrcSize, BYTE **ppDstBuf, DWORD *pdwDstSize)
{
	CBlockLock AutoLock(&m_GraphLock);

	if(!pSrcBuf || !pdwDstSize || !ppDstBuf || !pdwDstSize){
		// �������s��
		return FALSE;
		}

	if(!m_TsDescrambler.GetHalBcasCard()){
		// B-CAS�J�[�h���I�[�v������Ă��Ȃ�
		return FALSE;
		}

	// �o�̓o�b�t�@�N���A
	m_OutputBuff.ClearSize();

	// ���f�B�A�f�[�^�Z�b�g
	m_InputBuff.SetData(pSrcBuf, dwSrcSize);
	
	// �f�R�[�_�ɓ��͂���
	if(!m_TsPacketSync.InputMedia(&m_InputBuff)){
		// �f�R�[�_�O���t�ŃG���[����
		return FALSE;
		}

	// �o�̓f�[�^�Z�b�g
	*ppDstBuf = m_OutputBuff.GetData();
	*pdwDstSize = m_OutputBuff.GetSize();

	return TRUE;
}

const BOOL CB25Decoder::Flush(BYTE **ppDstBuf, DWORD *pdwDstSize)
{
	// ���̃��\�b�h�͌��ʂ��Ȃ�(�݊����̂��ߎ���)
	if(!ppDstBuf || !pdwDstSize){
		// �������s��
		return FALSE;
		}

	// �o�̓f�[�^�Z�b�g(�f�[�^�͎c��Ȃ����ߏ�ɋ�)
	*ppDstBuf = NULL;
	*pdwDstSize = 0;

	return TRUE;
}

const BOOL CB25Decoder::Reset(void)
{
	CBlockLock AutoLock(&m_GraphLock);

	// ��U�f�R�[�_�O���t��~
	m_TsPacketSync.StopDecoder();
	
	// �p�P�b�g�����f�R�[�_�ݒ�
	m_TsPacketSync.DiscardNullPacket(false);
	
	// �f�X�N�����u���f�R�[�_�I�[�v��
	if(!m_TsDescrambler.OpenDescrambler(TEXT("CBcasCardReader")))return FALSE;

	// �f�X�N�����u���f�R�[�_�ݒ�
	m_TsDescrambler.EnableEmmProcess(false);
	m_TsDescrambler.DiscardScramblePacket(false);

	// �o�̓o�b�t�@�N���A
	m_OutputBuff.ClearSize();

	// ���v��񃊃Z�b�g
	ResetStatistics();

	// �f�R�[�_�O���t�Đ�
	return (m_TsPacketSync.PlayDecoder())? TRUE : FALSE;
}

void CB25Decoder::DiscardNullPacket(const bool bEnable)
{
	// NULL�p�P�b�g�j���̗L����ݒ�
	m_TsPacketSync.DiscardNullPacket(bEnable);
}

void CB25Decoder::DiscardScramblePacket(const bool bEnable)
{
	// �����R��p�P�b�g�j���̗L����ݒ�
	m_TsDescrambler.DiscardScramblePacket(bEnable);
}

void CB25Decoder::EnableEmmProcess(const bool bEnable)
{
	// EMM�����̗L��/������ݒ�
	m_TsDescrambler.EnableEmmProcess(bEnable);
}

const DWORD CB25Decoder::GetDescramblingState(const WORD wProgramID)
{
	// ���{���͕K�v
	// CBlockLock AutoLock(&m_GraphLock);

	// �w�肵���v���O����ID�̕�����Ԃ�Ԃ�
	return m_TsDescrambler.GetDescramblingState(wProgramID);
}

void CB25Decoder::ResetStatistics(void)
{
	// ���v�������Z�b�g����
	m_TsPacketSync.ResetStatistics();
	m_TsDescrambler.ResetStatistics();
}

const DWORD CB25Decoder::GetPacketStride(void)
{
	// �p�P�b�g������Ԃ�
	return m_TsPacketSync.GetPacketStride();
}

const DWORD CB25Decoder::GetInputPacketNum(const WORD wPID)
{
	// ���̓p�P�b�g����Ԃ��@��TS_INVALID_PID�w�莞�͑SPID�̍��v��Ԃ�
	return m_TsPacketSync.GetInputPacketNum(wPID);
}

const DWORD CB25Decoder::GetOutputPacketNum(const WORD wPID)
{
	// �o�̓p�P�b�g����Ԃ��@��TS_INVALID_PID�w�莞�͑SPID�̍��v��Ԃ�
	return m_TsPacketSync.GetOutputPacketNum(wPID);
}

const DWORD CB25Decoder::GetSyncErrNum(void)
{
	// �����G���[����Ԃ�
	return m_TsPacketSync.GetSyncErrNum();
}

const DWORD CB25Decoder::GetFormatErrNum(void)
{
	// �t�H�[�}�b�g�G���[����Ԃ�
	return m_TsPacketSync.GetFormatErrNum();
}

const DWORD CB25Decoder::GetTransportErrNum(void)
{
	// �r�b�g�G���[����Ԃ�
	return m_TsPacketSync.GetTransportErrNum();
}

const DWORD CB25Decoder::GetContinuityErrNum(const WORD wPID)
{
	// �h���b�v�p�P�b�g����Ԃ�
	return m_TsPacketSync.GetContinuityErrNum(wPID);
}

const DWORD CB25Decoder::GetScramblePacketNum(const WORD wPID)
{
	// �����R��p�P�b�g����Ԃ�
	return m_TsDescrambler.GetScramblePacketNum(wPID);
}

const DWORD CB25Decoder::GetEcmProcessNum(void)
{
	// ECM��������Ԃ�(B-CAS�J�[�h�A�N�Z�X��)
	return m_TsDescrambler.GetEcmProcessNum();
}

const DWORD CB25Decoder::GetEmmProcessNum(void)
{
	// EMM��������Ԃ�(B-CAS�J�[�h�A�N�Z�X��)
	return m_TsDescrambler.GetEmmProcessNum();
}

const DWORD CB25Decoder::OnDecoderEvent(IMediaDecoder *pDecoder, const DWORD dwEventID, PVOID pParam)
{
	if(pDecoder == dynamic_cast<IMediaDecoder *>(&m_MediaGrabber)){
		if(dwEventID == IMediaGrabber::EID_ON_MEDIADATA){	
			// �o�̓o�b�t�@�Ƀf�[�^��ǉ�
			m_OutputBuff.AddData(reinterpret_cast<IMediaData *>(pParam));		
			}
		}

	return 0UL;
}


/////////////////////////////////////////////////////////////////////////////
// For Delphi and General Non C++ Laguages
// From MobileHackerz (http://mobilehackerz.jp/contents/Software/cap_hdus)
/////////////////////////////////////////////////////////////////////////////

extern "C"
{

B25DECAPI const BOOL B25Decoder_Initialize(IB25Decoder *pB25, DWORD dwRound)
{
	return pB25->Initialize(dwRound);
}

B25DECAPI void B25Decoder_Release(IB25Decoder *pB25)
{
	pB25->Release();
}

B25DECAPI const BOOL B25Decoder_Decode(IB25Decoder *pB25, BYTE *pSrcBuf, const DWORD dwSrcSize, BYTE **ppDstBuf, DWORD *pdwDstSize)
{
	return pB25->Decode(pSrcBuf, dwSrcSize, ppDstBuf, pdwDstSize);
}

B25DECAPI const BOOL B25Decoder_Flush(IB25Decoder *pB25, BYTE **ppDstBuf, DWORD *pdwDstSize)
{
	return pB25->Flush(ppDstBuf, pdwDstSize);
}

B25DECAPI const BOOL B25Decoder_Reset(IB25Decoder *pB25)
{
	return pB25->Reset();
}

B25DECAPI const BOOL B25Decoder2_Initialize(IB25Decoder2 *pB25, DWORD dwRound)
{
	return pB25->Initialize(dwRound);
}

B25DECAPI void B25Decoder2_Release(IB25Decoder2 *pB25)
{
	pB25->Release();
}

B25DECAPI const BOOL B25Decoder2_Decode(IB25Decoder2 *pB25, BYTE *pSrcBuf, const DWORD dwSrcSize, BYTE **ppDstBuf, DWORD *pdwDstSize)
{
	return pB25->Decode(pSrcBuf, dwSrcSize, ppDstBuf, pdwDstSize);
}

B25DECAPI const BOOL B25Decoder2_Flush(IB25Decoder2 *pB25, BYTE **ppDstBuf, DWORD *pdwDstSize)
{
	return pB25->Flush(ppDstBuf, pdwDstSize);
}

B25DECAPI const BOOL B25Decoder2_Reset(IB25Decoder2 *pB25)
{
	return pB25->Reset();
}

B25DECAPI void B25Decoder2_DiscardNullPacket(IB25Decoder2 *pB25, const bool bEnable)
{
	pB25->DiscardNullPacket(bEnable);
}

B25DECAPI void B25Decoder2_DiscardScramblePacket(IB25Decoder2 *pB25, const bool bEnable)
{
	pB25->DiscardScramblePacket(bEnable);
}

B25DECAPI void B25Decoder2_EnableEmmProcess(IB25Decoder2 *pB25, const bool bEnable)
{
	pB25->EnableEmmProcess(bEnable);
}

B25DECAPI const DWORD B25Decoder2_GetDescramblingState(IB25Decoder2 *pB25, const WORD wProgramID)
{
	return pB25->GetDescramblingState(wProgramID);
}

B25DECAPI void B25Decoder2_ResetStatistics(IB25Decoder2 *pB25)
{
	pB25->ResetStatistics();
}

B25DECAPI const DWORD B25Decoder2_GetPacketStride(IB25Decoder2 *pB25)
{
	return pB25->GetPacketStride();
}

B25DECAPI const DWORD B25Decoder2_GetInputPacketNum(IB25Decoder2 *pB25, const WORD wPID)
{
	return pB25->GetInputPacketNum(wPID);
}

B25DECAPI const DWORD B25Decoder2_GetOutputPacketNum(IB25Decoder2 *pB25, const WORD wPID)
{
	return pB25->GetOutputPacketNum(wPID);
}

B25DECAPI const DWORD B25Decoder2_GetSyncErrNum(IB25Decoder2 *pB25)
{
	return pB25->GetSyncErrNum();
}

B25DECAPI const DWORD B25Decoder2_GetFormatErrNum(IB25Decoder2 *pB25)
{
	return pB25->GetFormatErrNum();
}

B25DECAPI const DWORD B25Decoder2_GetTransportErrNum(IB25Decoder2 *pB25)
{
	return pB25->GetTransportErrNum();
}

B25DECAPI const DWORD B25Decoder2_GetContinuityErrNum(IB25Decoder2 *pB25, const WORD wPID)
{
	return pB25->GetContinuityErrNum(wPID);
}

B25DECAPI const DWORD B25Decoder2_GetScramblePacketNum(IB25Decoder2 *pB25, const WORD wPID)
{
	return pB25->GetScramblePacketNum(wPID);
}

B25DECAPI const DWORD B25Decoder2_GetEcmProcessNum(IB25Decoder2 *pB25)
{
	return pB25->GetEcmProcessNum();
}

B25DECAPI const DWORD B25Decoder2_GetEmmProcessNum(IB25Decoder2 *pB25)
{
	return pB25->GetEmmProcessNum();
}

}