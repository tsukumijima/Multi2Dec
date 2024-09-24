// TsDescBase.h: デスクリプタ基底クラス
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


#include <vector>


/////////////////////////////////////////////////////////////////////////////
// デスクリプタ共通インタフェース基底クラス
/////////////////////////////////////////////////////////////////////////////

class CDescBase :	public CBonObject,
					public IDescBase
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CDescBase)

// IDescBase
	virtual const DWORD ParseDesc(const BYTE *pData, const DWORD dwDataSize);
	virtual const BYTE GetTag(void) const;
	virtual const BYTE GetLength(void) const;
	virtual const BYTE * GetData(void) const;

// CDescBase
	CDescBase(IBonObject *pOwner);
	virtual ~CDescBase(void);

protected:
	virtual const bool ParseData(void);

	BYTE m_byDescTag;
	BYTE m_byDescLen;
	const BYTE *m_pDescData;
};


/////////////////////////////////////////////////////////////////////////////
// デスクリプタ集合インタフェース基底クラス
/////////////////////////////////////////////////////////////////////////////

class CDescBlock :	public CMediaData,
					public IDescBlock
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CDescBlock)

// IDescBlock
	virtual const DWORD ParseDescs(void);
	virtual void Reset(void);
	virtual const DWORD GetDescNum(void) const;
	virtual const IDescBase * GetDescByTag(const BYTE byTag, const DWORD dwIndex = 0UL) const;
	virtual const IDescBase * GetDescByIndex(const DWORD dwIndex = 0UL) const;
	virtual const bool CopyDescBlock(const IDescBlock *pSrc);

// CMediaData
	virtual const DWORD CopyData(const IMediaData *pSrc);

// CPsiSection
	CDescBlock(IBonObject *pOwner);
	virtual ~CDescBlock(void);

protected:
	static CDescBase * CreateDesc(const BYTE byTag);

	std::vector<CDescBase *> m_DescArray;
};
