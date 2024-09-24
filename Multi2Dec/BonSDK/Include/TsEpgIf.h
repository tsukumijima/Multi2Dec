// TsEpgIf.h: TS EPG �C���^�t�F�[�X��`
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


#include "TsDataEncode.h"


/////////////////////////////////////////////////////////////////////////////
// EIT�A�C�e���C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IEitItem :	public IBonObject
{
public:
	virtual const bool ParseItem(void) = 0;

	virtual const WORD GetEventID(void) const = 0;
	virtual const CTsTime & GetStartTime(void) const = 0;
	virtual const DWORD GetDuration(void) const = 0;
	virtual const BYTE GetRunningStatus(void) const = 0;
	virtual const bool IsScrambled(void) const = 0;
	virtual const IDescBlock * GetEventDesc(void) const = 0;
	
	virtual const bool CopyItem(const IEitItem *pSrc) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// EPG�A�C�e���C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IEpgItem :	public IBonObject
{
public:
	enum	// StoreEitItem()�AGetStoreState() ���^�[���R�[�h(������OR)
	{
		SST_EMPTY			= 0x00U,	// �f�[�^�Ȃ�
		SST_EVENT_ID		= 0x01U,	// EVENT ID�X�g�A�ς�
		SST_SHORT_INFO		= 0x02U,	// �Z�`���C�x���g���X�g�A�ς�
		SST_EXTEND_INFO		= 0x04U,	// �g���`���C�x���g���X�g�A�ς�
		SST_VIDEO_INFO		= 0x08U,	// �f�����X�g�A�ς�
		SST_AUDIO_INFO		= 0x10U,	// �������X�g�A�ς�
		SST_CONTENT_INFO	= 0x20U,	// �R���e���g���X�g�A�ς�
		SST_SCHEDULE_CHANGE	= 0x20U,	// �����X�P�W���[���ω����o
		SST_COMPLETE		= (SST_EVENT_ID | SST_SHORT_INFO | SST_EXTEND_INFO | SST_VIDEO_INFO | SST_AUDIO_INFO)
	};

	enum	// GetAudioMode() ���^�[���R�[�h
	{
		ADM_UNDEFINED		= 0x00U,	// ����`
		ADM_MONO			= 0x01U,	// ���m����
		ADM_DUAL			= 0x02U,	// ��d����
		ADM_STEREO			= 0x03U,	// �X�e���I
		ADM_SURROUND		= 0x04U		// �T���E���h
	};

	virtual const BYTE StoreEitItem(const IEitItem *pEitItem) = 0;
	virtual void Reset(void) = 0;

	virtual const BYTE GetStoreState(void) const = 0;

// SST_EVENT_ID�X�g�A�ς݂ŗL��
	virtual const WORD GetEventID(void) const = 0;
	virtual const CTsTime & GetStartTime(void) const = 0;
	virtual const DWORD GetDuration(void) const = 0;
	virtual const bool IsScrambled(void) const = 0;

// SST_SHORT_INFO�X�g�A�ς݂ŗL��
	virtual const DWORD GetEventTitle(LPTSTR lpszDst) const = 0;
	virtual const DWORD GetEventInfo(LPTSTR lpszDst) const = 0;

// SST_EXTEND_INFO�X�g�A�ς݂ŗL��
	virtual const DWORD GetEventDetail(LPTSTR lpszDst) const = 0;

// SST_VIDEO_INFO�X�g�A�ς݂ŗL��
	virtual const WORD GetVideoResolution(void) const = 0;
	virtual const bool IsVideoProgressive(void) const = 0;
	virtual const bool IsVideoAspectWide(void) const = 0;
	virtual const bool IsVideoPanAndScan(void) const = 0;

// SST_AUDIO_INFO�X�g�A�ς݂ŗL��
	virtual const BYTE GetAudioMode(void) const = 0;

// SST_CONTENT_INFO�X�g�A�ς݂ŗL��
	virtual const DWORD GetGenreNum(void) const = 0;
	virtual const BYTE GetGenreLevel1(const DWORD dwIndex = 0UL) const = 0;
	virtual const BYTE GetGenreLevel2(const DWORD dwIndex = 0UL) const = 0;

	virtual const bool CopyItem(const IEpgItem *pSrc) = 0;
};
