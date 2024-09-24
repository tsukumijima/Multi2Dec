// TsDescrambler.h: TSデスクランブルデコーダ
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


#include "TsPidMapper.h"
#include "TsTableBase.h"
#include "Multi2Decoder.h"
#include <Vector>


/////////////////////////////////////////////////////////////////////////////
// TSデスクランブルデコーダ
/////////////////////////////////////////////////////////////////////////////
// Input	#0	: ITsPacket		TSパケット
// Output	#0	: ITsPacket		TSパケット
/////////////////////////////////////////////////////////////////////////////

class CTsDescrambler :	public CMediaDecoder,
						public ITsDescrambler,
						protected IPatTable::IHandler,
						protected ICatTable::IHandler,
						protected IPmtTable::IHandler,
						protected ITotTable::IHandler
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CTsDescrambler)

// CMediaDecoder
	virtual const bool InputMedia(IMediaData *pMediaData, const DWORD dwInputIndex = 0UL);
	
// ITsDescrambler
	virtual const bool OpenDescrambler(LPCTSTR lpszBCId);
	virtual const bool CloseDescrambler(void);
	virtual void DiscardScramblePacket(const bool bEnable = true);
	virtual void EnableEmmProcess(const bool bEnable = true);
	virtual void EnableBuffering(const bool bEnable = true);
	virtual void ResetStatistics(void);
	virtual const DWORD GetDescramblingState(const WORD wProgramID);
	virtual const DWORD GetInputPacketNum(const WORD wPID = TS_INVALID_PID);
	virtual const DWORD GetOutputPacketNum(const WORD wPID = TS_INVALID_PID);
	virtual const DWORD GetScramblePacketNum(const WORD wPID = TS_INVALID_PID);
	virtual const DWORD GetEcmProcessNum(void);
	virtual const DWORD GetEmmProcessNum(void);
	virtual IHalBcasCard * GetHalBcasCard(void);

// CTsDescrambler
	CTsDescrambler(IBonObject *pOwner);
	virtual ~CTsDescrambler(void);
	
protected:
	class CEcmProcessor;
	class CEmmProcessor;
	class CEsProcessor;

// CMediaDecoder
	virtual const bool OnPlay(void);
	virtual const bool OnStop(void);
	virtual const bool OnReset(void);

// IPatTable::IHandler
	virtual void OnPatTable(const IPatTable *pPatTable);

// ICatTable::IHandler
	virtual void OnCatTable(const ICatTable *pCatTable);

// IPmtTable::IHandler
	virtual void OnPmtTable(const IPmtTable *pPmtTable);

// ITotTable::IHandler
	virtual void OnTotTable(const ITotTable *pTotTable);

// CTsDescrambler
	void InputPacket(ITsPacket * const pPacket);
	void BufManagement(ITsPacket * const pPacket);
	void OutputPacket(ITsPacket * const pPacket);

	void ResetPidMap(void);
	void FlushUnprocPacket(void);
	void PushUnprocPacket(const ITsPacket *pPacket);
	void ClearUnprocPacket(void);

	CTsPidMapper m_TsInputMap;
	CTsPidMapper m_TsOutputMap;
	
	IHalBcasCard *m_pHalBcasCard;

	bool m_bDiscardScramblePacket;
	bool m_bEnableEmmProcess;
	bool m_bEnableBuffering;

	DWORD m_dwInputPacketNum;
	DWORD m_dwOutputPacketNum;
	DWORD m_dwScramblePacketNum;

	DWORD m_adwInputPacketNum[0x2000];
	DWORD m_adwOutputPacketNum[0x2000];
	DWORD m_adwScramblePacketNum[0x2000];

	DWORD m_dwEcmProcessNum;
	DWORD m_dwEmmProcessNum;

	WORD m_wLastTsID;
	
	enum
	{
		BDS_INITIAL,
		BDS_STORING_PMT,
		BDS_STORING_ECM,
		BDS_RUNNING
	} m_DecBufState;
	
	std::vector<ITsPacket *> m_PacketBuf;
	DWORD m_dwPmtWaitCount;
};


/////////////////////////////////////////////////////////////////////////////
// ECM処理内部クラス
/////////////////////////////////////////////////////////////////////////////

class CTsDescrambler::CEcmProcessor :	public CPsiTableBase
{
	friend CTsDescrambler;

public:
	enum {TABLE_ID = 0x82U};

// CBonObject
	DECLARE_IBONOBJECT(CEcmProcessor)

// CPsiTableBase
	virtual void OnPidReset(ITsPidMapper *pTsPidMapper, const WORD wPID);
	virtual void Reset(void);

// CEcmProcessor
	CEcmProcessor(IBonObject *pOwner);
	virtual ~CEcmProcessor(void);

	virtual const bool IsEcmReceived(void) const;
	virtual const bool DescramblePacket(ITsPacket * const pTsPacket);

protected:
// CPsiTableBase
	virtual const bool OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection);

	CTsDescrambler * const m_pTsDescrambler;
	IHalBcasCard * const m_pHalBcasCard;

	CMulti2Decoder m_Multi2Decoder;
	DWORD m_dwLastRetryTime;
	DWORD m_dwDescramblingState;
	bool m_bIsEcmReceived;
};


/////////////////////////////////////////////////////////////////////////////
// EMM処理内部クラス
/////////////////////////////////////////////////////////////////////////////

class CTsDescrambler::CEmmProcessor :	public CPsiTableBase
{
public:
	enum
	{
		SECTION_TABLE_ID = 0x84U,	// EMMセクション
		MESSAGE_TABLE_ID = 0x85U,	// EMM共通/個別メッセージ
	};

// CBonObject
	DECLARE_IBONOBJECT(CEmmProcessor)

// CEmmProcessor
	CEmmProcessor(IBonObject *pOwner);
	virtual ~CEmmProcessor(void);

protected:
// CPsiTableBase
	virtual const bool OnTableUpdate(const IPsiSection *pNewSection, const IPsiSection *pLastSection);

// CEmmProcessor
	virtual const bool OnEmmBody(const BYTE *pEmmData, const WORD wEmmSize);

	CTsDescrambler * const m_pTsDescrambler;
	IHalBcasCard * const m_pHalBcasCard;
};


/////////////////////////////////////////////////////////////////////////////
// ES処理内部クラス
/////////////////////////////////////////////////////////////////////////////

class CTsDescrambler::CEsProcessor :	public CBonObject,
										public ITsPidMapTarget
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CEsProcessor)

// CEsProcessor
	CEsProcessor(IBonObject *pOwner);
	virtual ~CEsProcessor(void);

// ITsPidMapTarget
	virtual const bool StorePacket(const ITsPacket *pTsPacket);
	virtual void OnPidReset(ITsPidMapper *pTsPidMapper, const WORD wPID);
	virtual void OnPidMapped(ITsPidMapper *pTsPidMapper, const WORD wPID);
	virtual void OnPidUnmapped(ITsPidMapper *pTsPidMapper, const WORD wPID);

	CEcmProcessor * const m_pEcmProcessor;
};
