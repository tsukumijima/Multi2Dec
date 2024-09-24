// ProgAnalyzer.h: �v���O�����A�i���C�U�f�R�[�_
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


#include "TsTableIf.h"
#include "TsPidMapper.h"
#include <Vector>


/////////////////////////////////////////////////////////////////////////////
// �v���O�����A�i���C�U�f�R�[�_
/////////////////////////////////////////////////////////////////////////////
// Input	#0	: ITsPacket		TS�p�P�b�g
// Output	#0	: ITsPacket		TS�p�P�b�g
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
		bool bAvailable;			// �L���t���O
		WORD wProgramID;			// �v���O����ID
		WORD wVideoPID;				// �f����PID
		WORD wAudioPID;				// ������PID
		WORD wPcrPID;				// PCR��PID
		TCHAR szServiceName[256];	// �T�[�r�X��
		IEpgItem *pCurEpgItem;		// ����EPG���
		IEpgItem *pNextEpgItem;		// ��EPG���
	};

	typedef std::vector<PROGRAM_ITEM> ProgramList;

	struct PROGRAM_CONTEXT
	{
		WORD wNetworkID;			// �l�b�g���[�NID
		WORD wTsID;					// TS ID
		WORD wCurProgramID;			// ���݃v���O����ID
		TCHAR szNetworkName[256];	// �l�b�g���[�N��
		ProgramList ProgList;
	};

	void ResetPidMap(void);
	PROGRAM_ITEM * FindProgramItem(WORD wProgramID);

	CTsPidMapper m_TsPidMapper;
	PROGRAM_CONTEXT m_ProgContext;
};
