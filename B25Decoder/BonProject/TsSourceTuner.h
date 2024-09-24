// TsSourceTuner.h: TSソースチューナデコーダ
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// TSソースチューナデコーダ
/////////////////////////////////////////////////////////////////////////////
// Output	#0	: IMediaData	TSストリーム
/////////////////////////////////////////////////////////////////////////////

class CTsSourceTuner :	public CMediaDecoder,
						public ITsSourceTuner
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CTsSourceTuner)

// ITsSourceTuner
	virtual const DWORD OpenTuner(LPCTSTR lpszBCId);
	virtual const bool CloseTuner(void);
	virtual const DWORD GetDeviceName(LPTSTR lpszName);
	virtual const DWORD GetTuningSpaceName(const DWORD dwSpace, LPTSTR lpszName);
	virtual const DWORD GetChannelName(const DWORD dwSpace, const DWORD dwChannel, LPTSTR lpszName);
	virtual const DWORD GetCurSpace(void);
	virtual const DWORD GetCurChannel(void);
	virtual const bool SetChannel(const DWORD dwSpace, const DWORD dwChannel);
	virtual const float GetSignalLevel(void);
	virtual IHalTsTuner * GetHalTsTuner(void);

// CTsSourceTuner
	CTsSourceTuner(IBonObject *pOwner);
	virtual ~CTsSourceTuner(void);

protected:
// CMediaDecoder
	virtual const bool OnPlay(void);
	virtual const bool OnStop(void);
	virtual const bool OnReset(void);

// CTsSourceTuner
	void TsRecvThread(CSmartThread<CTsSourceTuner> *pThread, bool &bKillSignal, PVOID pParam);
	const bool PurgeStream(void);
	virtual void OnTsStream(BYTE *pStreamData, DWORD dwStreamSize);

	IHalTsTuner *m_pHalTsTuner;
	IMediaData *m_pTsBuffer;

	CSmartThread<CTsSourceTuner> m_TsRecvThread;
	bool m_bIsPlaying;
};
