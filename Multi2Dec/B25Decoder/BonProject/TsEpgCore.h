// TsEpgCore.h: TS EPG コアクラス
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


#include "TsDescBase.h"
#include <String>
#include <Vector>


/////////////////////////////////////////////////////////////////////////////
// マクロ定義
/////////////////////////////////////////////////////////////////////////////

#ifdef _UNICODE
	#define EpgString std::wstring		// UNICODE
#else
	#define EpgString std::string		// char
#endif


/////////////////////////////////////////////////////////////////////////////
// IEitItem基底クラス
/////////////////////////////////////////////////////////////////////////////

class CEitItem :	public CMediaData,
					public IEitItem
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CEitItem)

// IEitItem
	virtual const bool ParseItem(void);
	virtual const WORD GetEventID(void) const;
	virtual const CTsTime & GetStartTime(void) const;
	virtual const DWORD GetDuration(void) const;
	virtual const BYTE GetRunningStatus(void) const;
	virtual const bool IsScrambled(void) const;
	virtual const IDescBlock * GetEventDesc(void) const;
	virtual const bool CopyItem(const IEitItem *pSrc);

// CMediaData
	virtual const DWORD CopyData(const IMediaData *pSrc);

// CEitItem
	CEitItem(IBonObject *pOwner);
	virtual ~CEitItem(void);
	
protected:
	WORD m_wEventID;
	CTsTime m_StartTime;
	DWORD m_dwDuration;
	BYTE m_byRunningStatus;
	bool m_bIsScrambled;
	CDescBlock m_EventDesc;
};


/////////////////////////////////////////////////////////////////////////////
// IEpgItem基底クラス
/////////////////////////////////////////////////////////////////////////////

class CEpgItem :	public CBonObject,
					public IEpgItem
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CEpgItem)

// IEpgItem
	virtual const BYTE StoreEitItem(const IEitItem *pEitItem);
	virtual void Reset(void);
	virtual const BYTE GetStoreState(void) const;
	virtual const WORD GetEventID(void) const;
	virtual const CTsTime & GetStartTime(void) const;
	virtual const DWORD GetDuration(void) const;
	virtual const bool IsScrambled(void) const;
	virtual const DWORD GetEventTitle(LPTSTR lpszDst) const;
	virtual const DWORD GetEventInfo(LPTSTR lpszDst) const;
	virtual const DWORD GetEventDetail(LPTSTR lpszDst) const;
	virtual const WORD GetVideoResolution(void) const;
	virtual const bool IsVideoProgressive(void) const;
	virtual const bool IsVideoAspectWide(void) const;
	virtual const bool IsVideoPanAndScan(void) const;
	virtual const BYTE GetAudioMode(void) const;
	virtual const DWORD GetGenreNum(void) const;
	virtual const BYTE GetGenreLevel1(const DWORD dwIndex = 0UL) const;
	virtual const BYTE GetGenreLevel2(const DWORD dwIndex = 0UL) const;
	virtual const bool CopyItem(const IEpgItem *pSrc);

// CEpgItem
	CEpgItem(IBonObject *pOwner);
	virtual ~CEpgItem(void);

protected:
	virtual const BYTE StoreEventID(const IEitItem *pEitItem);

	virtual const BYTE StoreShortDesc(const IDescBlock *pDescBlock);
	virtual const BYTE StoreExtendDesc(const IDescBlock *pDescBlock);
	virtual const BYTE StoreComponentDesc(const IDescBlock *pDescBlock);
	virtual const BYTE StoreAudioComponentDesc(const IDescBlock *pDescBlock);
	virtual const BYTE StoreContentDesc(const IDescBlock *pDescBlock);

	BYTE m_byStoreState;

	WORD m_wEventID;
	CTsTime m_StartTime;
	DWORD m_dwDuration;
	bool m_bIsScrambled;
	
	EpgString m_strEventTitle;
	EpgString m_strEventInfo;
	EpgString m_strEventDetail;
	
	WORD m_wVideoResolution;
	bool m_bIsVideoProgressive;
	bool m_bIsVideoAspectWide;
	bool m_bIsVideoPanAndScan;

	BYTE m_byAudioMode;
	
	std::vector<BYTE> m_GenreArray;
};