// TsDescriptorIf.h: TS�L�q�q�C���^�t�F�[�X��`
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// �萔��`
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// [0x09] Conditional Access Method �L�q�q�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class ICaMethodDesc :	public IBonObject
{
public:
	enum {DESC_TAG = 0x09U};
	
	virtual const WORD GetCaMethodID(void) const = 0;
	virtual const WORD GetCaPID(void) const = 0;
	virtual const BYTE * GetPrivateData(void) const = 0;
	virtual const BYTE GetPrivateDataSize(void) const = 0;
};


/////////////////////////////////////////////////////////////////////////////
// [0x40] �l�b�g���[�N�� �L�q�q�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class INetworkNameDesc :	public IBonObject
{
public:
	enum {DESC_TAG = 0x40U};
	
	virtual const DWORD GetNetworkName(LPTSTR lpszDst) const = 0;
};


/////////////////////////////////////////////////////////////////////////////
// [0x41] �T�[�r�X���X�g �L�q�q�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IServiceListDesc :	public IBonObject
{
public:
	enum {DESC_TAG = 0x41U};
	
	virtual const DWORD GetServiceNum(void) const = 0;
	virtual const WORD GetServiceID(const DWORD dwIndex) const = 0;
	virtual const BYTE GetServiceType(const DWORD dwIndex) const = 0;
};


/////////////////////////////////////////////////////////////////////////////
// [0x48] Service �L�q�q�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IServiceDesc :	public IBonObject
{
public:
	enum {DESC_TAG = 0x48U};
	
	virtual const BYTE GetServiceType(void) const = 0;
	virtual const DWORD GetProviderName(LPTSTR lpszDst) const = 0;
	virtual const DWORD GetServiceName(LPTSTR lpszDst) const = 0;
};


/////////////////////////////////////////////////////////////////////////////
// [0x4D] �Z�`���C�x���g �L�q�q�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IShortEventDesc :	public IBonObject
{
public:
	enum {DESC_TAG = 0x4DU};
	
	virtual const DWORD GetLanguageCode(void) const = 0;
	virtual const DWORD GetEventName(LPTSTR lpszDst) const = 0;
	virtual const DWORD GetEventDesc(LPTSTR lpszDst) const = 0;
};


/////////////////////////////////////////////////////////////////////////////
// [0x4E] �g���`���C�x���g �L�q�q�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IExtendEventDesc :	public IBonObject
{
public:
	enum {DESC_TAG = 0x4EU};
	
	virtual const BYTE GetDescNumber(void) const = 0;
	virtual const BYTE GetLastDescNumber(void) const = 0;
	virtual const DWORD GetLanguageCode(void) const = 0;

	virtual const DWORD GetItemNum(void) const = 0;
	virtual const DWORD GetItemName(LPTSTR lpszDst, const DWORD dwIndex = 0UL) const = 0;
	virtual const DWORD GetItemText(BYTE *pDst, const DWORD dwIndex = 0UL) const = 0;

	virtual const DWORD GetExtDesc(LPTSTR lpszDst) const = 0;
};


/////////////////////////////////////////////////////////////////////////////
// [0x50] Component �L�q�q�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IComponentDesc :	public IBonObject
{
public:
	enum {DESC_TAG = 0x50U};
	
	virtual const BYTE GetStreamContent(void) const = 0;
	virtual const BYTE GetComponentType(void) const = 0;
	virtual const BYTE GetComponentTag(void) const = 0;
	virtual const DWORD GetLanguageCode(void) const = 0;
	virtual const DWORD GetComponentText(LPTSTR lpszDst) const = 0;
	
	virtual const bool IsVideoComponent(void) const = 0;
	virtual const WORD GetVideoResolution(void) const = 0;
	virtual const bool IsVideoProgressive(void) const = 0;
	virtual const bool IsVideoAspectWide(void) const = 0;
	virtual const bool IsVideoPanAndScan(void) const = 0;
};


/////////////////////////////////////////////////////////////////////////////
// [0x52] Stream Identifier �L�q�q�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IStreamIdDesc :	public IBonObject
{
public:
	enum {DESC_TAG = 0x52U};
	
	virtual const BYTE GetComponentTag(void) const = 0;
};


/////////////////////////////////////////////////////////////////////////////
// [0x54] Content �L�q�q�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IContentDesc :	public IBonObject
{
public:
	enum {DESC_TAG = 0x54U};
	
	virtual const DWORD GetGenreNum(void) const = 0;
	virtual const BYTE GetGenreLevel1(const DWORD dwIndex = 0UL) const = 0;
	virtual const BYTE GetGenreLevel2(const DWORD dwIndex = 0UL) const = 0;
	virtual const BYTE GetUserGenre1(const DWORD dwIndex = 0UL) const = 0;
	virtual const BYTE GetUserGenre2(const DWORD dwIndex = 0UL) const = 0;
};


/////////////////////////////////////////////////////////////////////////////
// [0xC4] ���� Component �L�q�q�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IAudioComponentDesc :	public IBonObject
{
public:
	enum {DESC_TAG = 0xC4U};

	virtual const BYTE GetComponentType(void) const = 0;
	virtual const BYTE GetComponentTag(void) const = 0;
	virtual const BYTE GetStreamType(void) const = 0;
	virtual const BYTE GetSimulcastGroupTag(void) const = 0;
	virtual const bool IsEsMultiLanguage(void) const = 0;
	virtual const bool IsMainComponent(void) const = 0;
	virtual const BYTE GetQualityIndicator(void) const = 0;
	virtual const BYTE GetSamplingRate(void) const = 0;
	virtual const DWORD GetMainLanguageCode(void) const = 0;
	virtual const DWORD GetSubLanguageCode(void) const = 0;
	virtual const DWORD GetComponentText(LPTSTR lpszDst) const = 0;

	virtual const bool IsDualMonoMode(void) const = 0;
	virtual const bool IsEnableCenter(void) const = 0;
	virtual const bool IsEnableFront(void) const = 0;
	virtual const bool IsEnableSurround(void) const = 0;
	virtual const bool IsEnableLFE(void) const = 0;
};
