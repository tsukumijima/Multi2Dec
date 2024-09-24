// TsEpgCore.cpp: TS EPG コアクラス
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "TsEpgCore.h"
#include <TChar.h>
#include <Vector>


/////////////////////////////////////////////////////////////////////////////
// IEitItem基底クラス
/////////////////////////////////////////////////////////////////////////////

const bool CEitItem::ParseItem(void)
{
	// サイズをチェック
	if(m_dwDataSize < 12UL)return false;

	// データを解析する
	m_wEventID = ((WORD)m_pData[0] << 8) | (WORD)m_pData[1];
	m_StartTime.SetAribTime(&m_pData[2]);
	m_dwDuration = CAribTime::AribBcdToSecond(&m_pData[7]);
	m_byRunningStatus = m_pData[10] >> 5;
	m_bIsScrambled = (m_pData[10] & 0x10U)? true : false;
	
	const WORD wDescLen = ((WORD)(m_pData[10] & 0x0FU) << 8) | (WORD)m_pData[11];
	m_EventDesc.SetData(&m_pData[12], wDescLen);
	m_EventDesc.ParseDescs();

	return true;
}

const WORD CEitItem::GetEventID(void) const
{
	// イベントIDを返す
	return m_wEventID;
}

const CTsTime & CEitItem::GetStartTime(void) const
{
	// 開始時間を返す
	return m_StartTime;
}

const DWORD CEitItem::GetDuration(void) const
{
	// 番組長を返す
	return m_dwDuration;
}

const BYTE CEitItem::GetRunningStatus(void) const
{
	// 進行状態を返す
	return m_byRunningStatus;
}

const bool CEitItem::IsScrambled(void) const
{
	// スクランブルフラグを返す
	return m_bIsScrambled;
}

const IDescBlock * CEitItem::GetEventDesc(void) const
{
	// 記述子ブロックを返す
	return &m_EventDesc;
}

const bool CEitItem::CopyItem(const IEitItem *pSrc)
{
	// IMediaDataインタフェースを取得
	const IMediaData * const pMediaData = dynamic_cast<const IMediaData *>(pSrc);
	if(!pMediaData)return false;

	// アイテムデータコピー
	if(CMediaData::CopyData(pMediaData) != GetSize())return false;
	
	// データ解析
	return ParseItem();
}

const DWORD CEitItem::CopyData(const IMediaData *pSrc)
{
	// IPsiSectionインタフェース取得
	const IEitItem * const pEitItem = dynamic_cast<const IEitItem *>(pSrc);

	if(pEitItem){
		// IEitItemとしてコピー
		return (CopyItem(pEitItem))? GetSize() : 0UL;
		}
	else{
		// IMediaDataとしてコピー
		return CMediaData::CopyData(pSrc);
		}
}

CEitItem::CEitItem(IBonObject *pOwner)
	: CMediaData(25UL)
	, m_wEventID(0x0000U)
	, m_dwDuration(0x00000000UL)
	, m_byRunningStatus(0x00U)
	, m_bIsScrambled(false)
	, m_EventDesc(NULL)
{
	// 何もしない
}

CEitItem::~CEitItem(void)
{
	// 何もしない
}


/////////////////////////////////////////////////////////////////////////////
// IEpgItem基底クラス
/////////////////////////////////////////////////////////////////////////////

const BYTE CEpgItem::StoreEitItem(const IEitItem *pEitItem)
{
	if(!pEitItem)return SST_EMPTY;

	// 基本情報を解析
	BYTE byLastState = m_byStoreState;
	m_byStoreState |= StoreEventID(pEitItem);

	// 記述子を解析
	const IDescBlock *pDescBlock = pEitItem->GetEventDesc();





//	for(DWORD dwIndex = 0UL ; dwIndex < pDescBlock->GetDescNum() ; dwIndex++){
//		const IDescBase *pDesc = pDescBlock->GetDescByIndex(dwIndex);
//		::BON_TRACE(TEXT("[%lu] TAG = %02X, LEN = %u\n"), dwIndex, pDesc->GetTag(), pDesc->GetLength());		
//		}
//
//	::BON_TRACE(TEXT("\n"));







	if(pDescBlock){
		m_byStoreState |= StoreShortDesc(pDescBlock);
		m_byStoreState |= StoreExtendDesc(pDescBlock);
		m_byStoreState |= StoreComponentDesc(pDescBlock);
		m_byStoreState |= StoreAudioComponentDesc(pDescBlock);
		m_byStoreState |= StoreContentDesc(pDescBlock);
		}

	// 更新された情報を返す
	return m_byStoreState & (~byLastState);
}

void CEpgItem::Reset(void)
{
	// 状態をリセットする
	m_byStoreState = SST_EMPTY;
	m_wEventID = 0x0000U;
	m_StartTime.ClearTime();
	m_dwDuration = 0x00000000UL;
	m_bIsScrambled = false;

	m_strEventTitle = TEXT("");
	m_strEventInfo = TEXT("");
	m_strEventDetail = TEXT("");

	m_wVideoResolution = 0U;
	m_bIsVideoProgressive = false;
	m_bIsVideoAspectWide = true;
	m_bIsVideoPanAndScan = false;
	
	m_byAudioMode = ADM_UNDEFINED;
	
	m_GenreArray.clear();
}

const BYTE CEpgItem::GetStoreState(void) const
{
	// データのストア進捗状態を返す
	return m_byStoreState;
}

const WORD CEpgItem::GetEventID(void) const
{
	// イベントIDを返す
	return m_wEventID;
}

const CTsTime & CEpgItem::GetStartTime(void) const
{
	// 開始時間を返す
	return m_StartTime;
}

const DWORD CEpgItem::GetDuration(void) const
{
	// 番組長を返す
	return m_dwDuration;
}

const bool CEpgItem::IsScrambled(void) const
{
	// スクランブルフラグを返す
	return m_bIsScrambled;
}

const DWORD CEpgItem::GetEventTitle(LPTSTR lpszDst) const
{
	// 番組のタイトルを返す
	if(lpszDst)::_tcscpy(lpszDst, m_strEventTitle.c_str());

	return m_strEventTitle.length();
}

const DWORD CEpgItem::GetEventInfo(LPTSTR lpszDst) const
{
	// 番組の情報を返す
	if(lpszDst)::_tcscpy(lpszDst, m_strEventInfo.c_str());

	return m_strEventInfo.length();
}

const DWORD CEpgItem::GetEventDetail(LPTSTR lpszDst) const
{
	// 番組の詳細を返す
	if(lpszDst)::_tcscpy(lpszDst, m_strEventDetail.c_str());

	return m_strEventDetail.length();
}

const WORD CEpgItem::GetVideoResolution(void) const
{
	// 解像度(横方向のピクセル数)を返す
	return m_wVideoResolution;
}

const bool CEpgItem::IsVideoProgressive(void) const
{
	// プログレッシブの有無を返す
	return m_bIsVideoProgressive;
}

const bool CEpgItem::IsVideoAspectWide(void) const
{
	// アスペクト比16:9の有無を返す
	return m_bIsVideoAspectWide;
}

const bool CEpgItem::IsVideoPanAndScan(void) const
{
	// パンスキャンの有無を返す
	return m_bIsVideoPanAndScan;
}

const BYTE CEpgItem::GetAudioMode(void) const
{
	// 音声モードを返す
	return m_byAudioMode;
}

const DWORD CEpgItem::GetGenreNum(void) const
{
	// ジャンル数を返す
	return m_GenreArray.size();
}

const BYTE CEpgItem::GetGenreLevel1(const DWORD dwIndex) const
{
	// ジャンル1を返す
	return (dwIndex < m_GenreArray.size())? (m_GenreArray[dwIndex] >> 4) : 0x0FU;
}

const BYTE CEpgItem::GetGenreLevel2(const DWORD dwIndex) const
{
	// ジャンル2を返す
	return (dwIndex < m_GenreArray.size())? (m_GenreArray[dwIndex] & 0x0F) : 0x0FU;
}

const bool CEpgItem::CopyItem(const IEpgItem *pSrc)
{
	if(!pSrc)return false;

	// インスタンスをコピーする
	m_byStoreState = pSrc->GetStoreState();
	m_wEventID = pSrc->GetEventID();
	m_StartTime = pSrc->GetStartTime();
	m_dwDuration = pSrc->GetDuration();
	m_bIsScrambled = pSrc->IsScrambled();

	m_wVideoResolution = pSrc->GetVideoResolution();
	m_bIsVideoProgressive = pSrc->IsVideoProgressive();
	m_bIsVideoAspectWide = pSrc->IsVideoAspectWide();
	m_bIsVideoPanAndScan = pSrc->IsVideoPanAndScan();

	m_byAudioMode = pSrc->GetAudioMode();

	// 面倒だが実装クラスに依存しないようにするにはこうするしかない
	std::vector<TCHAR> szTempString;
	DWORD dwLength;

	// 番組のタイトルをコピー
	if(dwLength = pSrc->GetEventTitle(NULL)){
		szTempString.resize(dwLength + 1UL);
		pSrc->GetEventTitle(&szTempString[0]);
		m_strEventTitle = &szTempString[0];
		}
	else{
		m_strEventTitle = TEXT("");
		}
		
	// 番組の情報をコピー
	if(dwLength = pSrc->GetEventInfo(NULL)){
		szTempString.resize(dwLength + 1UL);
		pSrc->GetEventInfo(&szTempString[0]);
		m_strEventInfo = &szTempString[0];
		}
	else{
		m_strEventInfo = TEXT("");
		}

	// 番組の詳細をコピー				
	if(dwLength = pSrc->GetEventDetail(NULL)){
		szTempString.resize(dwLength + 1UL);
		pSrc->GetEventDetail(&szTempString[0]);
		m_strEventDetail = &szTempString[0];
		}
	else{
		m_strEventDetail = TEXT("");
		}

	// ジャンルをコピー
	const DWORD dwGenreNum = pSrc->GetGenreNum();
	m_GenreArray.resize(dwGenreNum);
	
	for(DWORD dwIndex = 0UL ; dwIndex < dwGenreNum ; dwIndex++){
		m_GenreArray[dwIndex] = (pSrc->GetGenreLevel1() << 4) | (pSrc->GetGenreLevel2() & 0x0FU);
		}

	return true;
}

CEpgItem::CEpgItem(IBonObject *pOwner)
	: CBonObject(pOwner)
{
	Reset();
}

CEpgItem::~CEpgItem(void)
{
	// 何もしない
}

const BYTE CEpgItem::StoreEventID(const IEitItem *pEitItem)
{
	if((m_byStoreState == SST_EMPTY) || (pEitItem->GetEventID() != m_wEventID)){
		// イベントID未確定 or イベントID変化時
		Reset();

		m_wEventID = pEitItem->GetEventID();
		m_StartTime = pEitItem->GetStartTime();
		m_dwDuration = pEitItem->GetDuration();
		m_bIsScrambled = pEitItem->IsScrambled();

		return SST_EVENT_ID;
		}
	else if((pEitItem->GetStartTime() != m_StartTime) || (pEitItem->GetDuration() != m_dwDuration)){
		// 放送スケジュール変化時
		m_StartTime = pEitItem->GetStartTime();
		m_dwDuration = pEitItem->GetDuration();

		return SST_SCHEDULE_CHANGE;
		}
	else{
		// 変化なし、なにもしない
		return SST_EMPTY;
		}
}

const BYTE CEpgItem::StoreShortDesc(const IDescBlock *pDescBlock)
{
	if(m_byStoreState & SST_SHORT_INFO)return SST_EMPTY;

	// 短形式イベント記述子をストア
	const IShortEventDesc *pShortEventDesc = dynamic_cast<const IShortEventDesc *>(pDescBlock->GetDescByTag(IShortEventDesc::DESC_TAG));
	if(!pShortEventDesc)return SST_EMPTY;

	std::vector<TCHAR> szTempString;
	DWORD dwLength;

	// 番組のタイトルをストア
	if(dwLength = pShortEventDesc->GetEventName(NULL)){
		szTempString.resize(dwLength + 1UL);
		pShortEventDesc->GetEventName(&szTempString[0]);
		m_strEventTitle = &szTempString[0];
		}

	// 番組の情報をストア
	if(dwLength = pShortEventDesc->GetEventDesc(NULL)){
		szTempString.resize(dwLength + 1UL);
		pShortEventDesc->GetEventDesc(&szTempString[0]);
		m_strEventInfo = &szTempString[0];
		}

	return SST_SHORT_INFO;
}

const BYTE CEpgItem::StoreExtendDesc(const IDescBlock *pDescBlock)
{
	if(m_byStoreState & SST_EXTEND_INFO)return SST_EMPTY;

	// 拡張形式イベント記述子をストア
	DWORD dwIndex = 0UL;
	const IExtendEventDesc *pExtendEventDesc;

	TCHAR szTempString[256 * 16];	// 考えうる最大長(マルチバイト文字ではさらに2倍必要だが) 
	BYTE AribString[256 * 16];		// 考えうる最大長
	DWORD dwLength = 0UL;

	while(pExtendEventDesc = dynamic_cast<const IExtendEventDesc *>(pDescBlock->GetDescByTag(IExtendEventDesc::DESC_TAG, dwIndex++))){
		if(dwIndex == 1UL)m_strEventDetail = TEXT("");

		for(DWORD dwItem = 0UL ; dwItem < pExtendEventDesc->GetItemNum() ; dwItem++){
			if(pExtendEventDesc->GetItemName(NULL, dwItem)){
				// 前アイテムのアイテム内容ストア
				if(dwLength){
					if(CAribString::AribToString(szTempString, AribString, dwLength)){
						m_strEventDetail += szTempString;
						m_strEventDetail += TEXT("\r\n\r\n");
						}
						
					dwLength = 0UL;
					}

				// アイテム名
				pExtendEventDesc->GetItemName(szTempString, dwItem);
				m_strEventDetail += TEXT("－");
				m_strEventDetail += szTempString;
				m_strEventDetail += TEXT("－\r\n");
				}

			// アイテム内容
			dwLength += pExtendEventDesc->GetItemText(&AribString[dwLength], dwItem);
			}
		}

	// 最終アイテム内容ストア
	if(dwLength){
		if(CAribString::AribToString(szTempString, AribString, dwLength)){
			m_strEventDetail += szTempString;
			}
		}

	return (dwIndex > 1UL)? SST_EXTEND_INFO : SST_EMPTY;
}

const BYTE CEpgItem::StoreComponentDesc(const IDescBlock *pDescBlock)
{
	if(m_byStoreState & SST_VIDEO_INFO)return SST_EMPTY;

	// コンポーネント記述子をストア
	DWORD dwIndex = 0UL;
	const IComponentDesc *pComponentDesc;

	while(pComponentDesc = dynamic_cast<const IComponentDesc *>(pDescBlock->GetDescByTag(IComponentDesc::DESC_TAG, dwIndex++))){
		if(pComponentDesc->IsVideoComponent()){
			// 映像情報をストア
			m_wVideoResolution = pComponentDesc->GetVideoResolution();
			m_bIsVideoProgressive = pComponentDesc->IsVideoProgressive();
			m_bIsVideoAspectWide = pComponentDesc->IsVideoAspectWide();
			m_bIsVideoPanAndScan = pComponentDesc->IsVideoPanAndScan();
			
			return SST_VIDEO_INFO;
			}
		}

	return SST_EMPTY;
}

const BYTE CEpgItem::StoreAudioComponentDesc(const IDescBlock *pDescBlock)
{
	if(m_byStoreState & SST_AUDIO_INFO)return SST_EMPTY;

	// 音声コンポーネント記述子をストア
	const IAudioComponentDesc *pAudioComponentDesc = dynamic_cast<const IAudioComponentDesc *>(pDescBlock->GetDescByTag(IAudioComponentDesc::DESC_TAG));
	if(!pAudioComponentDesc)return SST_EMPTY;
	
	switch(pAudioComponentDesc->GetComponentType()){
		case 0x01U :	// 1/0モード（シングルモノ）
			m_byAudioMode = ADM_MONO;
			break;
			
		case 0x02U :	// 1/0＋1/0モード（デュアルモノ）
			m_byAudioMode = ADM_DUAL;
			break;

		case 0x03U :	// 2/0モード（ステレオ）
		case 0x05U :	// 3/0モード
			m_byAudioMode = ADM_STEREO;
			break;

		case 0x04U :	// 2/1モード
		case 0x06U :	// 2/2モード
		case 0x07U :	// 3/1モード
		case 0x08U :	// 3/2モード
		case 0x09U :	// 3/2＋LFEモード
			m_byAudioMode = ADM_SURROUND;
			break;

		default :
			m_byAudioMode = ADM_UNDEFINED;
			break;
		}

	return SST_AUDIO_INFO;
}

const BYTE CEpgItem::StoreContentDesc(const IDescBlock *pDescBlock)
{
	if(m_byStoreState & SST_CONTENT_INFO)return SST_EMPTY;

	// コンテント記述子をストア
	const IContentDesc *pContentDesc = dynamic_cast<const IContentDesc *>(pDescBlock->GetDescByTag(IContentDesc::DESC_TAG));
	if(!pContentDesc)return SST_EMPTY;

	const DWORD dwGenreNum = pContentDesc->GetGenreNum();
	m_GenreArray.resize(dwGenreNum);
	
	for(DWORD dwIndex = 0UL ; dwIndex < dwGenreNum ; dwIndex++){
		m_GenreArray[dwIndex] = (pContentDesc->GetGenreLevel1() << 4) | (pContentDesc->GetGenreLevel2() & 0x0FU);
		}

	return SST_CONTENT_INFO;
}
