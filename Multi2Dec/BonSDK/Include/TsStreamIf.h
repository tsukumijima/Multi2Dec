// TsStreamIf.h: TS�X�g���[���C���^�t�F�[�X��`
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


//#include "TsDataEncode.h"


/////////////////////////////////////////////////////////////////////////////
// �萔��`
/////////////////////////////////////////////////////////////////////////////

#define	TS_PACKET_SIZE	188UL		// TS�p�P�b�g�T�C�Y

#define TS_INVALID_PID	0xFFFFU		// ����PID


/////////////////////////////////////////////////////////////////////////////
// TS�p�P�b�g�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class ITsPacket :	public IBonObject
{
public:
	enum	// ParsePacket() �G���[�R�[�h
	{
		EC_VALID		= 0x00000000UL,		// ����p�P�b�g
		EC_FORMAT		= 0x00000001UL,		// �t�H�[�}�b�g�G���[
		EC_TRANSPORT	= 0x00000002UL,		// �g�����X�|�[�g�G���[(�r�b�g�G���[)
		EC_CONTINUITY	= 0x00000003UL		// �A�����J�E���^�G���[(�h���b�v)
	};

	virtual const DWORD ParsePacket(BYTE *pContinuityCounter = NULL) = 0;

	virtual BYTE * GetRawData(void) const = 0;

	virtual const WORD GetPID(void) const = 0;
	virtual const bool IsScrambled(void) const = 0;
	virtual const bool HaveAdaptationField(void) const = 0;

	virtual BYTE * GetPayloadData(void) const = 0;
	virtual const BYTE GetPayloadSize(void) const = 0;

	virtual const bool CopyPacket(const ITsPacket *pSrc) = 0;
	virtual const bool ComparePacket(const ITsPacket *pSrc) const = 0;

	struct TAG_TSPACKETHEADER
	{
		BYTE bySyncByte;					// Sync Byte
		bool bTransportErrorIndicator;		// Transport Error Indicator
		bool bPayloadUnitStartIndicator;	// Payload Unit Start Indicator
		bool bTransportPriority;			// Transport Priority
		WORD wPID;							// PID
		BYTE byTransportScramblingCtrl;		// Transport Scrambling Control
		BYTE byAdaptationFieldCtrl;			// Adaptation Field Control
		BYTE byContinuityCounter;			// Continuity Counter
	} m_Header;

	struct TAG_ADAPTFIELDHEADER
	{
		BYTE byAdaptationFieldLength;		// Adaptation Field Length
		bool bDiscontinuityIndicator;		// Discontinuity Indicator
		bool bRamdomAccessIndicator;		// Random Access Indicator
		bool bEsPriorityIndicator;			// Elementary Stream Priority Indicator
		bool bPcrFlag;						// PCR Flag
		bool bOpcrFlag;						// OPCR Flag
		bool bSplicingPointFlag;			// Splicing Point Flag
		bool bTransportPrivateDataFlag;		// Transport Private Data Flag
		bool bAdaptationFieldExtFlag;		// Adaptation Field Extension Flag
		const BYTE *pOptionData;			// �I�v�V�����t�B�[���h�f�[�^
		BYTE byOptionSize;					// �I�v�V�����t�B�[���h��
	} m_AdaptationField;
};


/////////////////////////////////////////////////////////////////////////////
// PSI�Z�N�V�����C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IPsiSection :	public IBonObject
{
public:
	enum	// CompareSection() ���ʃt���O
	{
		CR_IDENTICAL	= 0x00000000UL,		// ��v
		CR_TABLE_ID		= 0x00000001UL,		// �e�[�u��ID
		CR_EXTENSION	= 0x00000002UL,		// �e�[�u��ID�g��
		CR_SECT_NO		= 0x00000004UL,		// �Z�N�V�����ԍ�
		CR_LAST_SECT_NO	= 0x00000008UL,		// ���X�g�Z�N�V�����ԍ�
		CR_PAYLOAD		= 0x00000010UL,		// �Z�N�V�����f�[�^
		CR_OTHER_ALL	= 0x00000020UL		// ���̑��S��(CRC���قȂ�)
	};

	virtual const bool ParseHeader(const bool bExtSection = true) = 0;
	virtual void Reset(void) = 0;

	virtual BYTE * GetPayloadData(void) const = 0;
	virtual const WORD GetPayloadSize(void) const = 0;

	virtual const BYTE GetTableID(void) const = 0;
	virtual const bool IsExtendedSection(void) const = 0;
	virtual const bool IsPrivate(void) const = 0;
	virtual const WORD GetSectionLength(void) const = 0;
	virtual const WORD GetTableIdExtension(void) const = 0;
	virtual const BYTE GetVersionNo(void) const = 0;
	virtual const bool IsCurrentNext(void) const = 0;
	virtual const BYTE GetSectionNumber(void) const = 0;
	virtual const BYTE GetLastSectionNumber(void) const = 0;
	virtual const DWORD GetSectionCrc(void) const = 0;
	
	virtual const bool CopySection(const IPsiSection *pSrc) = 0;
	virtual const DWORD CompareSection(const IPsiSection *pSrc) const = 0;
};


/////////////////////////////////////////////////////////////////////////////
// PSI�Z�N�V�����p�[�T�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IPsiSectionParser :	public IBonObject
{
public:
	// PSI�Z�N�V�����n���h���C���^�t�F�[�X
	class IHandler :	public IBonObject
	{
	public:
		virtual void OnPsiSection(const IPsiSectionParser *pPsiSectionParser, const IPsiSection *pPsiSection) = 0;
	};

	virtual void SetExtSection(const bool bExtSection = true) = 0;
	virtual const bool StorePacket(const ITsPacket *pTsPacket) = 0;
	virtual void Reset(void) = 0;
	virtual const DWORD GetCrcErrorNum(void) const = 0;
};


/////////////////////////////////////////////////////////////////////////////
// �f�X�N���v�^���ʃC���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IDescBase :	public IBonObject
{
public:
	virtual const DWORD ParseDesc(const BYTE *pData, const DWORD dwDataSize) = 0;

	virtual const BYTE GetTag(void) const = 0;
	virtual const BYTE GetLength(void) const = 0;
	virtual const BYTE * GetData(void) const = 0;
};


/////////////////////////////////////////////////////////////////////////////
// �f�X�N���v�^�W���C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IDescBlock :	public IBonObject
{
public:
	virtual const DWORD ParseDescs(void) = 0;
	virtual void Reset(void) = 0;
	
	virtual const DWORD GetDescNum(void) const = 0;
	virtual const IDescBase * GetDescByTag(const BYTE byTag, const DWORD dwIndex = 0UL) const = 0;
	virtual const IDescBase * GetDescByIndex(const DWORD dwIndex = 0UL) const = 0;
	
	virtual const bool CopyDescBlock(const IDescBlock *pSrc) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// PID�}�b�v�^�[�Q�b�g�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class ITsPidMapper;

class ITsPidMapTarget :	public IBonObject
{
public:
	virtual const bool StorePacket(const ITsPacket *pTsPacket) = 0;
	virtual void OnPidReset(ITsPidMapper *pTsPidMapper, const WORD wPID) = 0;
	virtual void OnPidMapped(ITsPidMapper *pTsPidMapper, const WORD wPID) = 0;
	virtual void OnPidUnmapped(ITsPidMapper *pTsPidMapper, const WORD wPID) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// PID�}�b�p�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class ITsPidMapper :	public IBonObject
{
public:
	virtual const bool StorePacket(const ITsPacket *pTsPacket) = 0;

	virtual void ResetPid(const WORD wPID = TS_INVALID_PID) = 0;
	virtual const bool MapPid(const WORD wPID, ITsPidMapTarget *pTsPidMapTarget) = 0;
	virtual const bool UnmapPid(const WORD wPID) = 0;
	virtual void ClearPid(void) = 0;

	virtual ITsPidMapTarget * GetMapTarget(const WORD wPID) const = 0;
	virtual const WORD GetMapNum(void) const = 0;
};
