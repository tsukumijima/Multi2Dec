// BonHalIf.h: Bon HAL�C���^�t�F�[�X��`
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// HAL��{�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IHalDevice :				public IBonObject
{
public:
	virtual const BONGUID GetDeviceType(void) = 0;
	virtual const DWORD GetDeviceName(LPTSTR lpszName) = 0;
	virtual const DWORD GetTotalDeviceNum(void) = 0;
	virtual const DWORD GetActiveDeviceNum(void) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// TS�`���[�i HAL�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IHalTsTuner :				public IBonObject
{
public:
	virtual const bool OpenTuner(void) = 0;
	virtual void CloseTuner(void) = 0;
	
	virtual const DWORD EnumTuningSpace(const DWORD dwSpace, LPTSTR lpszSpace) = 0;
	virtual const DWORD EnumChannelName(const DWORD dwSpace, const DWORD dwChannel, LPTSTR lpszChannel) = 0;
	
	virtual const DWORD GetCurSpace(void) = 0;
	virtual const DWORD GetCurChannel(void) = 0;

	virtual const bool SetChannel(const DWORD dwSpace, const DWORD dwChannel) = 0;
	virtual const bool SetLnbPower(const bool bEnable) = 0;
	virtual const float GetSignalLevel(void) = 0;

	virtual const bool GetStream(BYTE **ppStream, DWORD *pdwSize, DWORD *pdwRemain) = 0;

	virtual void PurgeStream(void) = 0;
	virtual const DWORD WaitStream(const DWORD dwTimeOut = 0UL) = 0;
	virtual const DWORD GetAvailableNum(void) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// B-CAS�J�[�h HAL�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IHalBcasCard :			public IBonObject
{
public:
	enum	// GetEcmError() ���^�[���R�[�h
	{
		EC_NO_ERROR			= 0x00000000UL,		// �G���[�Ȃ�����
		EC_NOT_OPEN			= 0x00000001UL,		// �J�[�h���I�[�v�����Ă��Ȃ�
		EC_TRANSMIT_ERROR	= 0x00000002UL,		// �J�[�h�ʐM�G���[
		EC_NOT_CONTRACTED	= 0x00000003UL,		// �������_��
		EC_BADARGUMENT		= 0x00000004UL		// �������s��
	};

	typedef struct		// �ʓd������\����
	{
		SYSTEMTIME stStartTime;	// �ʓd�J�n��
		SYSTEMTIME stEndTime;	// �ʓd�I����
		BYTE byDurTime;			// �d��OFF��̒ʓd����[h]
		WORD wNtID;				// ��M�l�b�g���[�N
		WORD wTsID;				// ��MTS
	} POWERCTRLINFO;
	
	virtual const bool OpenCard(void) = 0;
	virtual void CloseCard(void) = 0;
	
	virtual const bool IsInteractive(void) = 0;
	
	virtual const BYTE * GetBcasCardID(void) = 0;	// 6�o�C�g
	virtual const BYTE * GetInitialCbc(void) = 0;	// 8�o�C�g
	virtual const BYTE * GetSystemKey(void) = 0;	// 32�o�C�g
	
	virtual const BYTE * GetKsFromEcm(const BYTE *pEcmData, const DWORD dwEcmSize) = 0;
	virtual const DWORD GetEcmError(void) = 0;

	virtual const bool SendEmmSection(const BYTE *pEmmData, const DWORD dwEmmSize) = 0;
	
	virtual const WORD GetPowerCtrlInfo(POWERCTRLINFO *pPowerCtrlInfo) = 0;
};
