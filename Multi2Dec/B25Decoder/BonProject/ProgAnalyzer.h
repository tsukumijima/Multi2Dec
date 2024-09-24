// ProgAnalyzer.h: プログラムアナライザデコーダ
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


#include "TsTableIf.h"
#include "TsPidMapper.h"
#include <Vector>


/////////////////////////////////////////////////////////////////////////////
// プログラムアナライザデコーダ
/////////////////////////////////////////////////////////////////////////////
// Input	#0	: ITsPacket		TSパケット
// Output	#0	: ITsPacket		TSパケット
/////////////////////////////////////////////////////////////////////////////

class CProgAnalyzer :	public CMediaDecoder,
						public IProgAnalyzer,
						protected IPatTable::IHandler,
						protected IPmtTable::IHandler,
						protected INitTable::IHandler,
						protected ISdtTable::IHandler,
						protected ITotTable::IHandler,
						protected IEitTable::IHandler
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CProgAnalyzer)

// CMediaDecoder
	virtual const bool InputMedia(IMediaData *pMediaData, const DWORD dwInputIndex = 0UL);
	
// IProgAnalyzer
	virtual const WORD GetNetworkID(void);
	virtual const WORD GetTsID(void);
	virtual const DWORD GetProgramNum(void);
	virtual const WORD GetProgramID(const DWORD dwIndex = 0UL);
	virtual const WORD GetProgramVideoPID(const WORD wProgramID = 0x0000U);
	virtual const WORD GetProgramAudioPID(const WORD wProgramID = 0x0000U);
	virtual const WORD GetProgramPcrPID(const WORD wProgramID = 0x0000U);
	virtual const DWORD GetNetworkName(LPTSTR lpszNetworkName);
	virtual const DWORD GetServiceName(LPTSTR lpszServiceName, const WORD wProgramID = 0x0000U);
	virtual const bool GetStreamTime(SYSTEMTIME *pStreamTime);
	virtual const WORD GetCurrentProgramID(void);
	virtual const bool SetCurrentProgramID(const WORD wProgramID = 0x0000U);
	virtual const IEpgItem * GetCurrentEpgInfo(const WORD wProgramID = 0x0000U);
	virtual const IEpgItem * GetNextEpgInfo(const WORD wProgramID = 0x0000U);
	
// CProgAnalyzer
	CProgAnalyzer(IBonObject *pOwner);
	virtual ~CProgAnalyzer(void);
	
protected:
// CMediaDecoder
	virtual const bool OnPlay(void);
	virtual const bool OnReset(void);

// IPatTable::IHandler
	virtual void OnPatTable(const IPatTable *pPatTable);

// IPmtTable::IHandler
	virtual void OnPmtTable(const IPmtTable *pPmtTable);

// NitTable::IHandler
	virtual void OnNitTable(const INitTable *pNitTable);

// ISdtTable::IHandler
	virtual void OnSdtTable(const ISdtTable *pSdtTable);

// ITotTable::IHandler
	virtual void OnTotTable(const ITotTable *pTotTable);

// EitTable::IHandler
	virtual void OnEitTable(const IEitTable *pEitTable, const IEitItem *pEitItem);

// CProgAnalyzer
	struct PROGRAM_ITEM
	{
		bool bAvailable;			// 有効フラグ
		WORD wProgramID;			// プログラムID
		WORD wVideoPID;				// 映像のPID
		WORD wAudioPID;				// 音声のPID
		WORD wPcrPID;				// PCRのPID
		TCHAR szServiceName[256];	// サービス名
		IEpgItem *pCurEpgItem;		// 現在EPG情報
		IEpgItem *pNextEpgItem;		// 次EPG情報
	};

	typedef std::vector<PROGRAM_ITEM> ProgramList;

	struct PROGRAM_CONTEXT
	{
		WORD wNetworkID;			// ネットワークID
		WORD wTsID;					// TS ID
		WORD wCurProgramID;			// 現在プログラムID
		TCHAR szNetworkName[256];	// ネットワーク名
		ProgramList ProgList;
	};

	void ResetPidMap(void);
	PROGRAM_ITEM * FindProgramItem(WORD wProgramID);

	CTsPidMapper m_TsPidMapper;
	PROGRAM_CONTEXT m_ProgContext;
};
