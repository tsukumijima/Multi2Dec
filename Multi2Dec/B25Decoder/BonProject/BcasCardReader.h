// BcasCardReader.h: B-CASカードリーダクラス
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Vector>
#include <String>
#include <WinScard.h>


/////////////////////////////////////////////////////////////////////////////
// 汎用スマートカードリーダ用 B-CASカードインタフェース実装クラス
/////////////////////////////////////////////////////////////////////////////

class CBcasCardReader :	public CBonObject,
						public IHalDevice,
						public IHalBcasCard
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CBcasCardReader)

// IHalDevice
	virtual const BONGUID GetDeviceType(void);
	virtual const DWORD GetDeviceName(LPTSTR lpszName);
	virtual const DWORD GetTotalDeviceNum(void);
	virtual const DWORD GetActiveDeviceNum(void);

// IHalBcasCard
	virtual const bool OpenCard(void);
	virtual void CloseCard(void);
	virtual const bool IsInteractive(void);
	virtual const BYTE * GetBcasCardID(void);
	virtual const BYTE * GetInitialCbc(void);
	virtual const BYTE * GetSystemKey(void);
	virtual const BYTE * GetKsFromEcm(const BYTE *pEcmData, const DWORD dwEcmSize);
	virtual const DWORD GetEcmError(void);
	virtual const bool SendEmmSection(const BYTE *pEmmData, const DWORD dwEmmSize);
	virtual const WORD GetPowerCtrlInfo(POWERCTRLINFO *pPowerCtrlInfo);

// CBcasCardReader
	CBcasCardReader(IBonObject *pOwner);
	virtual ~CBcasCardReader(void);
	
protected:
	const bool EnumBcasCardReader(void);
	const bool TransmitCommand(const SCARDHANDLE hBcasCard, const BYTE *pSendData, const DWORD dwSendSize, BYTE *pRecvData, const DWORD dwMaxRecv, DWORD *pdwRecvSize = NULL);
	const bool InitialSetting(const SCARDHANDLE hBcasCard);

	SCARDCONTEXT m_ScardContext;
	SCARDHANDLE m_hBcasCard;

#ifdef _UNICODE	
	std::vector<std::wstring> m_CardReaderArray;
#else
	std::vector<std::string> m_CardReaderArray;
#endif

	struct TAG_BCASCARDINFO
	{
		BYTE BcasCardID[6];		// Card ID
		BYTE SystemKey[32];		// Descrambling system key
		BYTE InitialCbc[8];		// Descrambler CBC initial value
	} m_BcasCardInfo;
	
	struct TAG_ECMSTATUS
	{
		BYTE KsData[16];		// Ks Odd + Even	
	} m_EcmStatus;
	
	DWORD m_dwEcmError;
};
