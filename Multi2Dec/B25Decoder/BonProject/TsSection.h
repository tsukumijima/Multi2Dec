// TsSection.h: PSI�Z�N�V�������N���X
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// IPsiSection���N���X
/////////////////////////////////////////////////////////////////////////////

class CPsiSection :	public CMediaData,
					public IPsiSection
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CPsiSection)

// IPsiSection
	virtual const bool ParseHeader(const bool bExtSection = true);
	virtual void Reset(void);
	virtual BYTE * GetPayloadData(void) const;
	virtual const WORD GetPayloadSize(void) const;
	virtual const BYTE GetTableID(void) const;
	virtual const bool IsExtendedSection(void) const;
	virtual const bool IsPrivate(void) const;
	virtual const WORD GetSectionLength(void) const;
	virtual const WORD GetTableIdExtension(void) const;
	virtual const BYTE GetVersionNo(void) const;
	virtual const bool IsCurrentNext(void) const;
	virtual const BYTE GetSectionNumber(void) const;
	virtual const BYTE GetLastSectionNumber(void) const;
	virtual const DWORD GetSectionCrc(void) const;
	virtual const bool CopySection(const IPsiSection *pSrc);
	virtual const DWORD CompareSection(const IPsiSection *pSrc) const;

// CMediaData
	virtual const DWORD CopyData(const IMediaData *pSrc);

// CPsiSection
	CPsiSection(IBonObject *pOwner);
	virtual ~CPsiSection(void);
	
protected:
	struct TAG_PSIHEADER
	{
		BYTE byTableID;					// �e�[�u��ID
		bool bSectionSyntaxIndicator;	// �Z�N�V�����V���^�b�N�X�C���W�P�[�^
		bool bPrivateIndicator;			// �v���C�x�[�g�C���W�P�[�^
		WORD wSectionLength;			// �Z�N�V������
		WORD wTableIdExtension;			// �e�[�u��ID�g��
		BYTE byVersionNo;				// �o�[�W�����ԍ�
		bool bCurrentNextIndicator;		// �J�����g�l�N�X�g�C���W�P�[�^
		BYTE bySectionNumber;			// �Z�N�V�����ԍ�
		BYTE byLastSectionNumber;		// ���X�g�Z�N�V�����ԍ�
	} m_Header;
};


/////////////////////////////////////////////////////////////////////////////
// IPsiSectionParser���N���X
/////////////////////////////////////////////////////////////////////////////

class CPsiSectionParser :	public CBonObject,
							public IPsiSectionParser
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CPsiSectionParser)

// IPsiSectionParser
	virtual void SetExtSection(const bool bExtSection = true);
	virtual const bool StorePacket(const ITsPacket *pTsPacket);
	virtual void Reset(void);
	virtual const DWORD GetCrcErrorNum(void) const;

// CPsiSectionParser
	CPsiSectionParser(IBonObject *pOwner);
	virtual ~CPsiSectionParser(void);
	
protected:
	const BYTE StoreHeader(const BYTE *pPayload, const BYTE byRemain);
	const BYTE StorePayload(const BYTE *pPayload, const BYTE byRemain);
	static const DWORD CalcCrc(const BYTE *pData, const WORD wDataSize, DWORD dwCurCrc = 0xFFFFFFFFUL);

	IHandler *m_pHandler;
	CPsiSection m_PsiSection;

	bool m_bTargetExt;

	bool m_bIsStoring;
	DWORD m_dwStoreCrc;
	WORD m_wStoreSize;
	DWORD m_dwCrcErrorNum;
};
