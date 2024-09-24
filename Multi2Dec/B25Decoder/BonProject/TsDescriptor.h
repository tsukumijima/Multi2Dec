// TsDescriptor.h: �f�X�N���v�^�N���X
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


#include "TsDescBase.h"


/////////////////////////////////////////////////////////////////////////////
// [0x09] Conditional Access Method �L�q�q�N���X
/////////////////////////////////////////////////////////////////////////////

class CCaMethodDesc :	public CDescBase,
						public ICaMethodDesc
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CCaMethodDesc)

// ICaMethodDesc
	virtual const WORD GetCaMethodID(void) const;
	virtual const WORD GetCaPID(void) const;
	virtual const BYTE * GetPrivateData(void) const;
	virtual const BYTE GetPrivateDataSize(void) const;

// CCaMethodDesc
	CCaMethodDesc(IBonObject *pOwner);
	virtual ~CCaMethodDesc(void);

protected:
// CDescBase
	virtual const bool ParseData(void);
};


/////////////////////////////////////////////////////////////////////////////
// [0x40] �l�b�g���[�N�� �L�q�q�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class CNetworkNameDesc :	public CDescBase,
							public INetworkNameDesc
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CNetworkNameDesc)

// INetworkNameDesc
	virtual const DWORD GetNetworkName(LPTSTR lpszDst) const;

// CNetworkNameDesc
	CNetworkNameDesc(IBonObject *pOwner);
	virtual ~CNetworkNameDesc(void);

protected:
// CDescBase
	virtual const bool ParseData(void);
};


/////////////////////////////////////////////////////////////////////////////
// [0x41] �T�[�r�X���X�g �L�q�q�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class CServiceListDesc :	public CDescBase,
							public IServiceListDesc
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CServiceListDesc)

// IServiceListDesc
	virtual const DWORD GetServiceNum(void) const;
	virtual const WORD GetServiceID(const DWORD dwIndex) const;
	virtual const BYTE GetServiceType(const DWORD dwIndex) const;
	
// CServiceListDesc
	CServiceListDesc(IBonObject *pOwner);
	virtual ~CServiceListDesc(void);

protected:
// CDescBase
	virtual const bool ParseData(void);
};


/////////////////////////////////////////////////////////////////////////////
// [0x48] Service �L�q�q�N���X
/////////////////////////////////////////////////////////////////////////////

class CServiceDesc :	public CDescBase,
						public IServiceDesc
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CServiceDesc)

// IServiceDesc
	virtual const BYTE GetServiceType(void) const;
	virtual const DWORD GetProviderName(LPTSTR lpszDst) const;
	virtual const DWORD GetServiceName(LPTSTR lpszDst) const;

// CServiceDesc
	CServiceDesc(IBonObject *pOwner);
	virtual ~CServiceDesc(void);

protected:
// CDescBase
	virtual const bool ParseData(void);
};


/////////////////////////////////////////////////////////////////////////////
// [0x4D] �Z�`���C�x���g �L�q�q�N���X
/////////////////////////////////////////////////////////////////////////////

class CShortEventDesc :	public CDescBase,
						public IShortEventDesc
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CShortEventDesc)

// IShortEventDesc
	virtual const DWORD GetLanguageCode(void) const;
	virtual const DWORD GetEventName(LPTSTR lpszDst) const;
	virtual const DWORD GetEventDesc(LPTSTR lpszDst) const;

// CShortEventDesc
	CShortEventDesc(IBonObject *pOwner);
	virtual ~CShortEventDesc(void);

protected:
// CDescBase
	virtual const bool ParseData(void);
};


/////////////////////////////////////////////////////////////////////////////
// [0x4E] �g���`���C�x���g �L�q�q�N���X
/////////////////////////////////////////////////////////////////////////////

class CExtendEventDesc :	public CDescBase,
							public IExtendEventDesc
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CExtendEventDesc)

// IExtendEventDesc
	virtual const BYTE GetDescNumber(void) const;
	virtual const BYTE GetLastDescNumber(void) const;
	virtual const DWORD GetLanguageCode(void) const;
	virtual const DWORD GetItemNum(void) const;
	virtual const DWORD GetItemName(LPTSTR lpszDst, const DWORD dwIndex = 0UL) const;
	virtual const DWORD GetItemText(BYTE *pDst, const DWORD dwIndex = 0UL) const;
	virtual const DWORD GetExtDesc(LPTSTR lpszDst) const;

// CExtendEventDesc
	CExtendEventDesc(IBonObject *pOwner);
	virtual ~CExtendEventDesc(void);

protected:
// CDescBase
	virtual const bool ParseData(void);
	
// CExtendedEventDesc
	virtual const BYTE * GetItemPointer(const DWORD dwIndex) const;
};


/////////////////////////////////////////////////////////////////////////////
// [0x50] Component �L�q�q�N���X
/////////////////////////////////////////////////////////////////////////////

class CComponentDesc :	public CDescBase,
						public IComponentDesc
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CComponentDesc)

// IComponentDesc
	virtual const BYTE GetStreamContent(void) const;
	virtual const BYTE GetComponentType(void) const;
	virtual const BYTE GetComponentTag(void) const;
	virtual const DWORD GetLanguageCode(void) const;
	virtual const DWORD GetComponentText(LPTSTR lpszDst) const;

	virtual const bool IsVideoComponent(void) const;
	virtual const WORD GetVideoResolution(void) const;
	virtual const bool IsVideoProgressive(void) const;
	virtual const bool IsVideoAspectWide(void) const;
	virtual const bool IsVideoPanAndScan(void) const;

// CComponentDesc
	CComponentDesc(IBonObject *pOwner);
	virtual ~CComponentDesc(void);

protected:
// CDescBase
	virtual const bool ParseData(void);
};


/////////////////////////////////////////////////////////////////////////////
// [0x52] Stream Identifier �L�q�q�N���X
/////////////////////////////////////////////////////////////////////////////

class CStreamIdDesc :	public CDescBase,
						public IStreamIdDesc
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CStreamIdDesc)

// IStreamIdDesc
	virtual const BYTE GetComponentTag(void) const;

// CStreamIdDesc
	CStreamIdDesc(IBonObject *pOwner);
	virtual ~CStreamIdDesc(void);

protected:
// CDescBase
	virtual const bool ParseData(void);
};


/////////////////////////////////////////////////////////////////////////////
// [0x54] Content �L�q�q�N���X
/////////////////////////////////////////////////////////////////////////////

class CContentDesc :	public CDescBase,
						public IContentDesc
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CContentDesc)

// IContentDesc
	virtual const DWORD GetGenreNum(void) const;
	virtual const BYTE GetGenreLevel1(const DWORD dwIndex = 0UL) const;
	virtual const BYTE GetGenreLevel2(const DWORD dwIndex = 0UL) const;
	virtual const BYTE GetUserGenre1(const DWORD dwIndex = 0UL) const;
	virtual const BYTE GetUserGenre2(const DWORD dwIndex = 0UL) const;
	
// CContentDesc
	CContentDesc(IBonObject *pOwner);
	virtual ~CContentDesc(void);

protected:
// CDescBase
	virtual const bool ParseData(void);
};


/////////////////////////////////////////////////////////////////////////////
// [0xC4] ���� Component �L�q�q�N���X
/////////////////////////////////////////////////////////////////////////////

class CAudioComponentDesc :	public CDescBase,
							public IAudioComponentDesc
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CAudioComponentDesc)

// ISoundComponentDesc
	virtual const BYTE GetComponentType(void) const;
	virtual const BYTE GetComponentTag(void) const;
	virtual const BYTE GetStreamType(void) const;
	virtual const BYTE GetSimulcastGroupTag(void) const;
	virtual const bool IsEsMultiLanguage(void) const;
	virtual const bool IsMainComponent(void) const;
	virtual const BYTE GetQualityIndicator(void) const;
	virtual const BYTE GetSamplingRate(void) const;
	virtual const DWORD GetMainLanguageCode(void) const;
	virtual const DWORD GetSubLanguageCode(void) const;
	virtual const DWORD GetComponentText(LPTSTR lpszDst) const;

	virtual const bool IsDualMonoMode(void) const;
	virtual const bool IsEnableCenter(void) const;
	virtual const bool IsEnableFront(void) const;
	virtual const bool IsEnableSurround(void) const;
	virtual const bool IsEnableLFE(void) const;

// CAudioComponentDesc
	CAudioComponentDesc(IBonObject *pOwner);
	virtual ~CAudioComponentDesc(void);

protected:
// CDescBase
	virtual const bool ParseData(void);
};
