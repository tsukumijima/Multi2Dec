// TsEpgIf.h: TS EPG インタフェース定義
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


#include "TsDataEncode.h"


/////////////////////////////////////////////////////////////////////////////
// EITアイテムインタフェース
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
// EPGアイテムインタフェース
/////////////////////////////////////////////////////////////////////////////

class IEpgItem :	public IBonObject
{
public:
	enum	// StoreEitItem()、GetStoreState() リターンコード(これらのOR)
	{
		SST_EMPTY			= 0x00U,	// データなし
		SST_EVENT_ID		= 0x01U,	// EVENT IDストア済み
		SST_SHORT_INFO		= 0x02U,	// 短形式イベント情報ストア済み
		SST_EXTEND_INFO		= 0x04U,	// 拡張形式イベント情報ストア済み
		SST_VIDEO_INFO		= 0x08U,	// 映像情報ストア済み
		SST_AUDIO_INFO		= 0x10U,	// 音声情報ストア済み
		SST_CONTENT_INFO	= 0x20U,	// コンテント情報ストア済み
		SST_SCHEDULE_CHANGE	= 0x20U,	// 放送スケジュール変化検出
		SST_COMPLETE		= (SST_EVENT_ID | SST_SHORT_INFO | SST_EXTEND_INFO | SST_VIDEO_INFO | SST_AUDIO_INFO)
	};

	enum	// GetAudioMode() リターンコード
	{
		ADM_UNDEFINED		= 0x00U,	// 未定義
		ADM_MONO			= 0x01U,	// モノラル
		ADM_DUAL			= 0x02U,	// 二重音声
		ADM_STEREO			= 0x03U,	// ステレオ
		ADM_SURROUND		= 0x04U		// サラウンド
	};

	virtual const BYTE StoreEitItem(const IEitItem *pEitItem) = 0;
	virtual void Reset(void) = 0;

	virtual const BYTE GetStoreState(void) const = 0;

// SST_EVENT_IDストア済みで有効
	virtual const WORD GetEventID(void) const = 0;
	virtual const CTsTime & GetStartTime(void) const = 0;
	virtual const DWORD GetDuration(void) const = 0;
	virtual const bool IsScrambled(void) const = 0;

// SST_SHORT_INFOストア済みで有効
	virtual const DWORD GetEventTitle(LPTSTR lpszDst) const = 0;
	virtual const DWORD GetEventInfo(LPTSTR lpszDst) const = 0;

// SST_EXTEND_INFOストア済みで有効
	virtual const DWORD GetEventDetail(LPTSTR lpszDst) const = 0;

// SST_VIDEO_INFOストア済みで有効
	virtual const WORD GetVideoResolution(void) const = 0;
	virtual const bool IsVideoProgressive(void) const = 0;
	virtual const bool IsVideoAspectWide(void) const = 0;
	virtual const bool IsVideoPanAndScan(void) const = 0;

// SST_AUDIO_INFOストア済みで有効
	virtual const BYTE GetAudioMode(void) const = 0;

// SST_CONTENT_INFOストア済みで有効
	virtual const DWORD GetGenreNum(void) const = 0;
	virtual const BYTE GetGenreLevel1(const DWORD dwIndex = 0UL) const = 0;
	virtual const BYTE GetGenreLevel2(const DWORD dwIndex = 0UL) const = 0;

	virtual const bool CopyItem(const IEpgItem *pSrc) = 0;
};
