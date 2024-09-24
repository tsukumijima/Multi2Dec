// BonHalIf.h: Bon HALインタフェース定義
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// HAL基本インタフェース
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
// TSチューナ HALインタフェース
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
// B-CASカード HALインタフェース
/////////////////////////////////////////////////////////////////////////////

class IHalBcasCard :			public IBonObject
{
public:
	enum	// GetEcmError() リターンコード
	{
		EC_NO_ERROR			= 0x00000000UL,		// エラーなし正常
		EC_NOT_OPEN			= 0x00000001UL,		// カードをオープンしていない
		EC_TRANSMIT_ERROR	= 0x00000002UL,		// カード通信エラー
		EC_NOT_CONTRACTED	= 0x00000003UL,		// 視聴未契約
		EC_BADARGUMENT		= 0x00000004UL		// 引数が不正
	};

	typedef struct		// 通電制御情報構造体
	{
		SYSTEMTIME stStartTime;	// 通電開始日
		SYSTEMTIME stEndTime;	// 通電終了日
		BYTE byDurTime;			// 電源OFF後の通電時間[h]
		WORD wNtID;				// 受信ネットワーク
		WORD wTsID;				// 受信TS
	} POWERCTRLINFO;
	
	virtual const bool OpenCard(void) = 0;
	virtual void CloseCard(void) = 0;
	
	virtual const bool IsInteractive(void) = 0;
	
	virtual const BYTE * GetBcasCardID(void) = 0;	// 6バイト
	virtual const BYTE * GetInitialCbc(void) = 0;	// 8バイト
	virtual const BYTE * GetSystemKey(void) = 0;	// 32バイト
	
	virtual const BYTE * GetKsFromEcm(const BYTE *pEcmData, const DWORD dwEcmSize) = 0;
	virtual const DWORD GetEcmError(void) = 0;

	virtual const bool SendEmmSection(const BYTE *pEmmData, const DWORD dwEmmSize) = 0;
	
	virtual const WORD GetPowerCtrlInfo(POWERCTRLINFO *pPowerCtrlInfo) = 0;
};
