// TsDescBase.cpp: デスクリプタ基底クラス
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "TsDescBase.h"
#include "TsDescriptor.h"


/////////////////////////////////////////////////////////////////////////////
// デスクリプタ共通インタフェース基底クラス
/////////////////////////////////////////////////////////////////////////////

const DWORD CDescBase::ParseDesc(const BYTE *pData, const DWORD dwDataSize)
{
	// 記述子のデフォルト解析処理
	if(!pData)return 0UL;									// データが空
	else if(dwDataSize < 2U)return 0UL;						// 最低でもタグ+記述子長の2バイト必要
	else if(dwDataSize < (WORD)(pData[1] + 2U))return 0UL;	// データが記述子のサイズよりも小さい

	// ヘッダ保存
	m_byDescTag = pData[0];
	m_byDescLen = pData[1];

	// データポインタ保存
	m_pDescData = (m_byDescLen)? &pData[2] : NULL;

	if(m_pDescData){
		// データ解析
		if(!ParseData())return 0UL;
		}

	return m_byDescLen + 2UL;
}

const BYTE CDescBase::GetTag(void) const
{
	// 記述子タグを返す
	return m_byDescTag;
}

const BYTE CDescBase::GetLength(void) const
{
	// 記述子長を返す(タグと記述子長は除く)
	return m_byDescLen;
}

const BYTE * CDescBase::GetData(void) const
{
	// 記述子データを返す
	return m_pDescData;
}

CDescBase::CDescBase(IBonObject *pOwner)
	: CBonObject(pOwner)
	, m_byDescTag(0x00U)
	, m_byDescLen(0x00U)
	, m_pDescData(NULL)
{

}

CDescBase::~CDescBase(void)
{

}

const bool CDescBase::ParseData(void)
{
	// デフォルトの処理では何もしない(派生クラスでオーバーライドする)
	return true;
}


/////////////////////////////////////////////////////////////////////////////
// デスクリプタ集合インタフェース基底クラス
/////////////////////////////////////////////////////////////////////////////

const DWORD CDescBlock::ParseDescs(void)
{
	// 記述子インスタンスクリア
	for(DWORD dwIndex = 0UL ; dwIndex < m_DescArray.size() ; dwIndex++){
		BON_SAFE_RELEASE(m_DescArray[dwIndex]);
		}
	
	m_DescArray.clear();

	// データポインタ取得
	const BYTE *pData = GetData();
	if(!pData || (GetSize() < 2UL))return 0UL;

	// 記述子解析
	CDescBase *pDescBase;
	DWORD dwPos = 0UL;

	while((GetSize() - dwPos) >= 2UL){
		// タグに対応する記述子のインスタンス生成
		if(!(pDescBase = CreateDesc(pData[dwPos])))break;

		// 記述子解析
		if(pDescBase->ParseDesc(&pData[dwPos], GetSize() - dwPos)){
			// 記述子追加
			m_DescArray.push_back(pDescBase);
			}
		else{
			// 非対応 or エラー
			::BON_ASSERT(false, TEXT("記述子が非対応 or エラー\r\nTag = %02X"), pData[dwPos]);
			
			pDescBase->Release();
			}

		// 解析位置更新
		dwPos += (pData[dwPos + 1UL] + 2UL);
		}
		
	return m_DescArray.size();
}

void CDescBlock::Reset(void)
{
	// 記述子インスタンスクリア
	for(DWORD dwIndex = 0UL ; dwIndex < m_DescArray.size() ; dwIndex++){
		BON_SAFE_RELEASE(m_DescArray[dwIndex]);
		}
	
	m_DescArray.clear();

	// データクリア
	ClearSize();
}

const DWORD CDescBlock::GetDescNum(void) const
{
	// 記述子の数を返す
	return m_DescArray.size();
}

const IDescBase * CDescBlock::GetDescByTag(const BYTE byTag, const DWORD dwIndex) const
{
	// 指定されたタグの記述子を返す
	for(DWORD dwPos = 0UL, dwCount = 0UL ; dwPos < m_DescArray.size() ; dwPos++){
		if(m_DescArray[dwPos]){
			if(m_DescArray[dwPos]->GetTag() == byTag){
				if(dwCount++ == dwIndex){
					return m_DescArray[dwPos];	
					}
				}		
			}
		}

	return NULL;
}

const IDescBase * CDescBlock::GetDescByIndex(const DWORD dwIndex) const
{
	// インデックスで指定した記述子を返す
	return (dwIndex < m_DescArray.size())? m_DescArray[dwIndex] : NULL;
}

const bool CDescBlock::CopyDescBlock(const IDescBlock *pSrc)
{
	// IMediaDataインタフェースを取得
	const IMediaData * const pMediaData = dynamic_cast<const IMediaData *>(pSrc);
	if(!pMediaData)return false;	
	
	// 保持データリセット
	Reset();
	
	// データコピー、解析
	if(CMediaData::CopyData(pMediaData)){
		return (ParseDescs())? true : false;		
		}

	return false;
}

const DWORD CDescBlock::CopyData(const IMediaData *pSrc)
{
	// IPsiSectionインタフェース取得
	const IDescBlock * const pDescBlock = dynamic_cast<const IDescBlock *>(pSrc);

	if(pDescBlock){
		// IDescBlockとしてコピー
		return (CopyDescBlock(pDescBlock))? GetSize() : 0UL;
		}
	else{
		// IMediaDataとしてコピー
		return CMediaData::CopyData(pSrc);
		}
}

CDescBlock::CDescBlock(IBonObject *pOwner)
	: CMediaData(pOwner)
{

}

CDescBlock::~CDescBlock(void)
{
	// 記述子インスタンス開放
	Reset();
}

CDescBase * CDescBlock::CreateDesc(const BYTE byTag)
{
	// タグに対応した記述子のインスタンスを生成する
	switch(byTag){
		case CCaMethodDesc::DESC_TAG		: return dynamic_cast<CDescBase *>(CCaMethodDesc::CreateInstance(NULL));
		case CNetworkNameDesc::DESC_TAG		: return dynamic_cast<CDescBase *>(CNetworkNameDesc::CreateInstance(NULL));
		case CServiceListDesc::DESC_TAG		: return dynamic_cast<CDescBase *>(CServiceListDesc::CreateInstance(NULL));
		case CServiceDesc::DESC_TAG			: return dynamic_cast<CDescBase *>(CServiceDesc::CreateInstance(NULL));
		case CShortEventDesc::DESC_TAG		: return dynamic_cast<CDescBase *>(CShortEventDesc::CreateInstance(NULL));
		case CExtendEventDesc::DESC_TAG		: return dynamic_cast<CDescBase *>(CExtendEventDesc::CreateInstance(NULL));
		case CComponentDesc::DESC_TAG		: return dynamic_cast<CDescBase *>(CComponentDesc::CreateInstance(NULL));
		case CStreamIdDesc::DESC_TAG		: return dynamic_cast<CDescBase *>(CStreamIdDesc::CreateInstance(NULL));
		case CContentDesc::DESC_TAG			: return dynamic_cast<CDescBase *>(CContentDesc::CreateInstance(NULL));
		case CAudioComponentDesc::DESC_TAG	: return dynamic_cast<CDescBase *>(CAudioComponentDesc::CreateInstance(NULL));
		default :							  return dynamic_cast<CDescBase *>(CDescBase::CreateInstance(NULL));
		}
}
