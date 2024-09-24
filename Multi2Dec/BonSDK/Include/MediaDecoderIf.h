// MediaDecoderIf.h: ���f�B�A�f�R�[�_�C���^�t�F�[�X��`
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


#include "TsStreamIf.h"
#include "TsEpgIf.h"


/////////////////////////////////////////////////////////////////////////////
// TS�\�[�X�`���[�i�f�R�[�_�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class ITsSourceTuner :			public IBonObject
{
public:
	enum	// OpenTuner() �G���[�R�[�h
	{
		EC_OPEN_SUCCESS	= 0x00000000UL,		// �I�[�v������
		EC_DEVICE_NONE	= 0x00000001UL,		// �`���[�i��������Ȃ�
		EC_DEVICE_FULL	= 0x00000002UL,		// �g�p�\�ȃ`���[�i���Ȃ�
		EC_OPEN_FAILED	= 0x00000003UL,		// �I�[�v�����s
		EC_INTERNAL_ERR	= 0x00000004UL		// �����G���[
	};
	
	virtual const DWORD OpenTuner(LPCTSTR lpszBCId) = 0;
	virtual const bool CloseTuner(void) = 0;

	virtual const DWORD GetDeviceName(LPTSTR lpszName) = 0;

	virtual const DWORD GetTuningSpaceName(const DWORD dwSpace, LPTSTR lpszName) = 0;
	virtual const DWORD GetChannelName(const DWORD dwSpace, const DWORD dwChannel, LPTSTR lpszName) = 0;
	
	virtual const DWORD GetCurSpace(void) = 0;
	virtual const DWORD GetCurChannel(void) = 0;

	virtual const bool SetChannel(const DWORD dwSpace, const DWORD dwChannel) = 0;
	virtual const float GetSignalLevel(void) = 0;
	
	virtual IHalTsTuner * GetHalTsTuner(void) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// TS�p�P�b�g�����f�R�[�_�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class ITsPacketSync :			public IBonObject
{
public:
	enum	// �f�R�[�_�C�x���gID				// [�C�x���g���e]				[pParam]
	{
		EID_SYNC_ERR		= 0x00000000UL,		// �����G���[���o				(DWORD)�G���[��
		EID_FORMAT_ERR		= 0x00000001UL,		// �t�H�[�}�b�g�G���[���o		(DWORD)�G���[��
		EID_TRANSPORT_ERR	= 0x00000002UL,		// �g�����X�|�[�g�G���[���o		(DWORD)�G���[��
		EID_CONTINUITY_ERR	= 0x00000003UL		// �A����(�h���b�v)�G���[���o	(DWORD)�G���[��
	};

	virtual void DiscardNullPacket(const bool bEnable = true) = 0;

	virtual void ResetStatistics(void) = 0;

	virtual const DWORD GetPacketStride(void) = 0;

	virtual const DWORD GetInputPacketNum(const WORD wPID = TS_INVALID_PID) = 0;
	virtual const DWORD GetOutputPacketNum(const WORD wPID = TS_INVALID_PID) = 0;

	virtual const DWORD GetInputBitrate(void) = 0;
	virtual const DWORD GetOutputBitrate(void) = 0;

	virtual const DWORD GetSyncErrNum(void) = 0;
	virtual const DWORD GetFormatErrNum(void) = 0;
	virtual const DWORD GetTransportErrNum(void) = 0;
	virtual const DWORD GetContinuityErrNum(const WORD wPID = TS_INVALID_PID) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// TS�f�X�N�����u���f�R�[�_�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class ITsDescrambler :			public IBonObject
{
public:
	enum	// �f�R�[�_�C�x���gID				// [�C�x���g���e]				[pParam]
	{
		EID_CANT_DESCRAMBLE	= 0x00000000UL,		// �����R��p�P�b�g���o			(DWORD)�����R��p�P�b�g��
		EID_ECM_PROCESSED	= 0x00000001UL,		// ECM��������					(DWORD)GetDescramblerState() ���^�[���R�[�h
		EID_EMM_PROCESSED	= 0x00000002UL		// EMM��������					(bool)EMM������
	};

	enum	// GetDescramblerState() ���^�[���R�[�h
	{
		DS_NO_ERROR			= 0x00000000UL,		// �G���[�Ȃ�����
		DS_BCAS_ERROR		= 0x00000001UL,		// B-CAS�J�[�h�G���[
		DS_NOT_CONTRACTED	= 0x00000002UL		// �������_��
	};

	virtual const bool OpenDescrambler(LPCTSTR lpszBCId) = 0;
	virtual const bool CloseDescrambler(void) = 0;

	virtual void DiscardScramblePacket(const bool bEnable = true) = 0;
	virtual void EnableEmmProcess(const bool bEnable = true) = 0;
	virtual void EnableBuffering(const bool bEnable = true) = 0;

	virtual void ResetStatistics(void) = 0;

	virtual const DWORD GetDescramblingState(const WORD wProgramID) = 0;

	virtual const DWORD GetInputPacketNum(const WORD wPID = TS_INVALID_PID) = 0;
	virtual const DWORD GetOutputPacketNum(const WORD wPID = TS_INVALID_PID) = 0;
	virtual const DWORD GetScramblePacketNum(const WORD wPID = TS_INVALID_PID) = 0;
	virtual const DWORD GetEcmProcessNum(void) = 0;
	virtual const DWORD GetEmmProcessNum(void) = 0;
	
	virtual IHalBcasCard * GetHalBcasCard(void) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// �v���O�����A�i���C�U�f�R�[�_�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IProgAnalyzer :			public IBonObject
{
	virtual const WORD GetNetworkID(void) = 0;
	virtual const WORD GetTsID(void) = 0;

	virtual const DWORD GetProgramNum(void) = 0;
	virtual const WORD GetProgramID(const DWORD dwIndex = 0UL) = 0;
	virtual const WORD GetProgramVideoPID(const WORD wProgramID = 0x0000U) = 0;
	virtual const WORD GetProgramAudioPID(const WORD wProgramID = 0x0000U) = 0;
	virtual const WORD GetProgramPcrPID(const WORD wProgramID = 0x0000U) = 0;

	virtual const DWORD GetNetworkName(LPTSTR lpszNetworkName) = 0;
	virtual const DWORD GetServiceName(LPTSTR lpszServiceName, const WORD wProgramID = 0x0000U) = 0;
	virtual const bool GetStreamTime(SYSTEMTIME *pStreamTime) = 0;

	virtual const WORD GetCurrentProgramID(void) = 0;
	virtual const bool SetCurrentProgramID(const WORD wProgramID = 0x0000U) = 0;

	virtual const IEpgItem * GetCurrentEpgInfo(const WORD wProgramID = 0x0000U) = 0;
	virtual const IEpgItem * GetNextEpgInfo(const WORD wProgramID = 0x0000U) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// �t�@�C�����C�^�f�R�[�_�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IFileWriter :				public IBonObject
{
public:
	enum	// �f�R�[�_�C�x���gID				// [�C�x���g���e]				[pParam]
	{
		EID_BUFF_OVERFLOW	= 0x00000000UL		// �o�b�t�@�I�[�o�t���[			N/A
	};
	
	virtual const bool OpenFile(LPCTSTR lpszFileName, const bool bAsyncWrite = false, const DWORD dwBuffSize = 0UL) = 0;
	virtual bool CloseFile(void) = 0;
	
	virtual const ULONGLONG GetFileLength(void) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// �t�@�C�����[�_�f�R�[�_�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IFileReader :				public IBonObject
{
public:
	enum	// �f�R�[�_�C�x���gID				// [�C�x���g���e]				[pParam]
	{
		EID_READ_START	= 0x00000000UL,			// ���[�h�J�n					N/A
		EID_READ_END	= 0x00000001UL,			// ���[�h�I��					N/A
		EID_PRE_READ	= 0x00000002UL,			// ���[�h�O						N/A
		EID_POST_READ	= 0x00000003UL			// ���[�h��						N/A
	};
	
	virtual const bool OpenFile(LPCTSTR lpszFileName, const bool bAsyncRead = false, const DWORD dwBuffSize = 0UL) = 0;
	virtual bool CloseFile(void) = 0;

	virtual const DWORD ReadSync(const DWORD dwReadSize) = 0;
	
	virtual const bool SetReadPos(const ULONGLONG llPos) = 0;
	virtual const ULONGLONG GetReadPos(void) = 0;
	
	virtual const ULONGLONG GetFileLength(void) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// ���f�B�A�O���o�f�R�[�_�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IMediaGrabber :			public IBonObject
{
public:
	enum	// �f�R�[�_�C�x���gID				// [�C�x���g���e]				[pParam]
	{
		EID_ON_PLAY			= 0x00000000UL,		// PlayDecoder()				N/A
		EID_ON_STOP			= 0x00000001UL,		// StopDecoder()				N/A
		EID_ON_RESET		= 0x00000002UL,		// ResetDecoder()				N/A
		EID_ON_MEDIADATA	= 0x00000003UL		// InputMedia()					(IMediaData *)���̓f�[�^
	};
};
