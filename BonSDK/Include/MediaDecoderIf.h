// MediaDecoderIf.h: メディアデコーダインタフェース定義
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


#include "TsStreamIf.h"
#include "TsEpgIf.h"


/////////////////////////////////////////////////////////////////////////////
// TSソースチューナデコーダインタフェース
/////////////////////////////////////////////////////////////////////////////

class ITsSourceTuner :			public IBonObject
{
public:
	enum	// OpenTuner() エラーコード
	{
		EC_OPEN_SUCCESS	= 0x00000000UL,		// オープン成功
		EC_DEVICE_NONE	= 0x00000001UL,		// チューナが見つからない
		EC_DEVICE_FULL	= 0x00000002UL,		// 使用可能なチューナがない
		EC_OPEN_FAILED	= 0x00000003UL,		// オープン失敗
		EC_INTERNAL_ERR	= 0x00000004UL		// 内部エラー
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
// TSパケット同期デコーダインタフェース
/////////////////////////////////////////////////////////////////////////////

class ITsPacketSync :			public IBonObject
{
public:
	enum	// デコーダイベントID				// [イベント内容]				[pParam]
	{
		EID_SYNC_ERR		= 0x00000000UL,		// 同期エラー検出				(DWORD)エラー数
		EID_FORMAT_ERR		= 0x00000001UL,		// フォーマットエラー検出		(DWORD)エラー数
		EID_TRANSPORT_ERR	= 0x00000002UL,		// トランスポートエラー検出		(DWORD)エラー数
		EID_CONTINUITY_ERR	= 0x00000003UL		// 連続性(ドロップ)エラー検出	(DWORD)エラー数
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
// TSデスクランブルデコーダインタフェース
/////////////////////////////////////////////////////////////////////////////

class ITsDescrambler :			public IBonObject
{
public:
	enum	// デコーダイベントID				// [イベント内容]				[pParam]
	{
		EID_CANT_DESCRAMBLE	= 0x00000000UL,		// 復号漏れパケット検出			(DWORD)復号漏れパケット数
		EID_ECM_PROCESSED	= 0x00000001UL,		// ECM処理完了					(DWORD)GetDescramblerState() リターンコード
		EID_EMM_PROCESSED	= 0x00000002UL		// EMM処理完了					(bool)EMM成功可否
	};

	enum	// GetDescramblerState() リターンコード
	{
		DS_NO_ERROR			= 0x00000000UL,		// エラーなし正常
		DS_BCAS_ERROR		= 0x00000001UL,		// B-CASカードエラー
		DS_NOT_CONTRACTED	= 0x00000002UL		// 視聴未契約
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
// プログラムアナライザデコーダインタフェース
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
// ファイルライタデコーダインタフェース
/////////////////////////////////////////////////////////////////////////////

class IFileWriter :				public IBonObject
{
public:
	enum	// デコーダイベントID				// [イベント内容]				[pParam]
	{
		EID_BUFF_OVERFLOW	= 0x00000000UL		// バッファオーバフロー			N/A
	};
	
	virtual const bool OpenFile(LPCTSTR lpszFileName, const bool bAsyncWrite = false, const DWORD dwBuffSize = 0UL) = 0;
	virtual bool CloseFile(void) = 0;
	
	virtual const ULONGLONG GetFileLength(void) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// ファイルリーダデコーダインタフェース
/////////////////////////////////////////////////////////////////////////////

class IFileReader :				public IBonObject
{
public:
	enum	// デコーダイベントID				// [イベント内容]				[pParam]
	{
		EID_READ_START	= 0x00000000UL,			// リード開始					N/A
		EID_READ_END	= 0x00000001UL,			// リード終了					N/A
		EID_PRE_READ	= 0x00000002UL,			// リード前						N/A
		EID_POST_READ	= 0x00000003UL			// リード後						N/A
	};
	
	virtual const bool OpenFile(LPCTSTR lpszFileName, const bool bAsyncRead = false, const DWORD dwBuffSize = 0UL) = 0;
	virtual bool CloseFile(void) = 0;

	virtual const DWORD ReadSync(const DWORD dwReadSize) = 0;
	
	virtual const bool SetReadPos(const ULONGLONG llPos) = 0;
	virtual const ULONGLONG GetReadPos(void) = 0;
	
	virtual const ULONGLONG GetFileLength(void) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// メディアグラバデコーダインタフェース
/////////////////////////////////////////////////////////////////////////////

class IMediaGrabber :			public IBonObject
{
public:
	enum	// デコーダイベントID				// [イベント内容]				[pParam]
	{
		EID_ON_PLAY			= 0x00000000UL,		// PlayDecoder()				N/A
		EID_ON_STOP			= 0x00000001UL,		// StopDecoder()				N/A
		EID_ON_RESET		= 0x00000002UL,		// ResetDecoder()				N/A
		EID_ON_MEDIADATA	= 0x00000003UL		// InputMedia()					(IMediaData *)入力データ
	};
};
