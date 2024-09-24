// TsDescriptor.cpp: デスクリプタクラス
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BonSDK.h"
#include "TsDescriptor.h"


/////////////////////////////////////////////////////////////////////////////
// これらの実装クラスは記述子のデータを保持せず、IDescBlockインタフェース
// 実装クラスが保持するメモリブロックを参照する。
// このためこれらのクラスはIDescBlockインタフェースから取得して使用しなければ
// ならない。(単独でインスタンスを生成することはできない)
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// [0x09] Conditional Access Method 記述子クラス
/////////////////////////////////////////////////////////////////////////////

const WORD CCaMethodDesc::GetCaMethodID(void) const
{
	// 限定受信方式IDを返す
	return (WORD)m_pDescData[0] << 8 | (WORD)m_pDescData[1];
}

const WORD CCaMethodDesc::GetCaPID(void) const
{
	// 限定受信PIDを返す
	return (WORD)(m_pDescData[2] & 0x1FU) << 8 | (WORD)m_pDescData[3];
}

const BYTE * CCaMethodDesc::GetPrivateData(void) const
{
	// プライベートデータを返す
	return &m_pDescData[4];
}

const BYTE CCaMethodDesc::GetPrivateDataSize(void) const
{
	// プライベートデータ長を返す
	return m_byDescLen - 4U;
}

CCaMethodDesc::CCaMethodDesc(IBonObject *pOwner)
	: CDescBase(pOwner)
{
	// 何もしない
}

CCaMethodDesc::~CCaMethodDesc(void)
{
	// 何もしない
}

const bool CCaMethodDesc::ParseData(void)
{
	// サイズのみチェックする
	return (m_byDescLen >= 4U)? true : false;
}


/////////////////////////////////////////////////////////////////////////////
// [0x40] ネットワーク名 記述子インタフェース
/////////////////////////////////////////////////////////////////////////////

const DWORD CNetworkNameDesc::GetNetworkName(LPTSTR lpszDst) const
{
	// ネットワーク名長をチェック
	if(!m_byDescLen)return 0UL;

	// ネットワーク名を取得する
	TCHAR szName[256] = TEXT("\0");
	CAribString::AribToString(szName, &m_pDescData[0], m_byDescLen);

	// バッファにコピー
	if(lpszDst)::_tcscpy(lpszDst, szName);

	// バッファに必要な文字数を返す
	return ::_tcslen(szName);
}

CNetworkNameDesc::CNetworkNameDesc(IBonObject *pOwner)
	: CDescBase(pOwner)
{
	// 何もしない
}

CNetworkNameDesc::~CNetworkNameDesc(void)
{
	// 何もしない
}

const bool CNetworkNameDesc::ParseData(void)
{
	// サイズのみチェックする
	return (m_byDescLen >= 1U)? true : false;
}


/////////////////////////////////////////////////////////////////////////////
// [0x41] サービスリスト 記述子インタフェース
/////////////////////////////////////////////////////////////////////////////

const DWORD CServiceListDesc::GetServiceNum(void) const
{
	// サービス数を返す
	return m_byDescLen / 3UL;
}

const WORD CServiceListDesc::GetServiceID(const DWORD dwIndex) const
{
	// サービスIDを返す
	return (dwIndex < GetServiceNum())? ((WORD)m_pDescData[dwIndex * 3] << 8 | (WORD)m_pDescData[dwIndex * 3 + 1]) : 0x0000U;
}

const BYTE CServiceListDesc::GetServiceType(const DWORD dwIndex) const
{
	// サービス形式種別を返す
	return (dwIndex < GetServiceNum())? m_pDescData[dwIndex * 3 + 2] : 0x00U;
}

CServiceListDesc::CServiceListDesc(IBonObject *pOwner)
	: CDescBase(pOwner)
{
	// 何もしない
}

CServiceListDesc::~CServiceListDesc(void)
{
	// 何もしない
}

const bool CServiceListDesc::ParseData(void)
{
	// サイズのみチェックする
	return (m_byDescLen && !(m_byDescLen % 3U))? true : false;
}


/////////////////////////////////////////////////////////////////////////////
// [0x48] Service 記述子クラス
/////////////////////////////////////////////////////////////////////////////

const BYTE CServiceDesc::GetServiceType(void) const
{
	// サービス形式IDを返す
	return m_pDescData[0];
}

const DWORD CServiceDesc::GetProviderName(LPTSTR lpszDst) const
{
	// 事業者名長をチェック
	const DWORD dwPos = 1UL;
	if(!m_pDescData[dwPos])return 0UL;

	// 事業者名を取得する
	TCHAR szName[256] = TEXT("\0");
	CAribString::AribToString(szName, &m_pDescData[dwPos + 1UL], m_pDescData[dwPos]);

	// バッファにコピー
	if(lpszDst)::_tcscpy(lpszDst, szName);

	// バッファに必要な文字数を返す
	return ::_tcslen(szName);
}

const DWORD CServiceDesc::GetServiceName(LPTSTR lpszDst) const
{
	// サービス名長をチェック
	const DWORD dwPos = (DWORD)m_pDescData[1] + 2UL;
	if(!m_pDescData[dwPos])return 0UL;

	// サービス名を取得する
	TCHAR szName[256] = TEXT("\0");
	CAribString::AribToString(szName, &m_pDescData[dwPos + 1UL], m_pDescData[dwPos]);

	// バッファにコピー
	if(lpszDst)::_tcscpy(lpszDst, szName);

	// バッファに必要な文字数を返す
	return ::_tcslen(szName);
}

CServiceDesc::CServiceDesc(IBonObject *pOwner)
	: CDescBase(pOwner)
{
	// 何もしない
}

CServiceDesc::~CServiceDesc(void)
{
	// 何もしない
}

const bool CServiceDesc::ParseData(void)
{
	// サイズのみチェックする
	return (m_byDescLen >= 3U)? true : false;
}


/////////////////////////////////////////////////////////////////////////////
// [0x4D] 短形式イベント 記述子クラス
/////////////////////////////////////////////////////////////////////////////

const DWORD CShortEventDesc::GetLanguageCode(void) const
{
	// 言語コードを返す
	return ((DWORD)m_pDescData[0] << 16) | ((DWORD)m_pDescData[1] << 8) | (DWORD)m_pDescData[2];
}

const DWORD CShortEventDesc::GetEventName(LPTSTR lpszDst) const
{
	// 番組名長をチェック
	const DWORD dwPos = 3UL;
	if(!m_pDescData[dwPos])return 0UL;

	// 番組名を取得する
	TCHAR szName[256] = TEXT("\0");
	CAribString::AribToString(szName, &m_pDescData[dwPos + 1UL], m_pDescData[dwPos]);

	// バッファにコピー
	if(lpszDst)::_tcscpy(lpszDst, szName);

	// バッファに必要な文字数を返す
	return ::_tcslen(szName);
}

const DWORD CShortEventDesc::GetEventDesc(LPTSTR lpszDst) const
{
	// 番組記述長をチェック
	const DWORD dwPos = (DWORD)m_pDescData[3] + 4UL;
	if(!m_pDescData[dwPos])return 0UL;

	// 番組記述を取得する
	TCHAR szName[256] = TEXT("\0");
	CAribString::AribToString(szName, &m_pDescData[dwPos + 1UL], m_pDescData[dwPos]);

	// バッファにコピー
	if(lpszDst)::_tcscpy(lpszDst, szName);

	// バッファに必要な文字数を返す
	return ::_tcslen(szName);
}

CShortEventDesc::CShortEventDesc(IBonObject *pOwner)
	: CDescBase(pOwner)
{
	// 何もしない
}

CShortEventDesc::~CShortEventDesc(void)
{
	// 何もしない
}

const bool CShortEventDesc::ParseData(void)
{
	// サイズのみチェックする
	return (m_byDescLen >= 5U)? true : false;
}


/////////////////////////////////////////////////////////////////////////////
// [0x4E] 拡張形式イベント 記述子クラス
/////////////////////////////////////////////////////////////////////////////

const BYTE CExtendEventDesc::GetDescNumber(void) const
{
	// 記述子番号を返す
	return m_pDescData[0] >> 4;
}

const BYTE CExtendEventDesc::GetLastDescNumber(void) const
{
	// 最終記述子番号を返す
	return m_pDescData[0] & 0x0FU;
}

const DWORD CExtendEventDesc::GetLanguageCode(void) const
{
	// 言語コードを返す
	return ((DWORD)m_pDescData[1] << 16) | ((DWORD)m_pDescData[2] << 8) | (DWORD)m_pDescData[3];
}

const DWORD CExtendEventDesc::GetItemNum(void) const
{
	// アイテム数を返す
	const BYTE *pItem = &m_pDescData[5];
	DWORD dwItem = 0UL;

	for(DWORD dwPos = 0UL ; dwPos < (DWORD)m_pDescData[4] ; dwItem++){
		// ポインタ位置更新
		if((dwPos + (DWORD)pItem[dwPos] + 1U) >= (DWORD)m_pDescData[4])break;
		dwPos += (DWORD)pItem[dwPos] + 1UL;
		dwPos += (DWORD)pItem[dwPos] + 1UL;
		}
		
	return dwItem;
}

const DWORD CExtendEventDesc::GetItemName(LPTSTR lpszDst, const DWORD dwIndex) const
{
	// アイテムポインタを取得
	const BYTE *pItem = GetItemPointer(dwIndex);
	if(!pItem)return 0UL;
	if(!pItem[0])return 0UL;

	// アイテム名を返す
	TCHAR szName[512] = TEXT("\0");
	CAribString::AribToString(szName, &pItem[1], pItem[0]);

	// バッファにコピー
	if(lpszDst)::_tcscpy(lpszDst, szName);

	// バッファに必要な文字数を返す
	return ::_tcslen(szName);	
}

const DWORD CExtendEventDesc::GetItemText(BYTE *pDst, const DWORD dwIndex) const
{
	// アイテムポインタを取得
	const BYTE *pItem = GetItemPointer(dwIndex);
	if(!pItem)return 0UL;
	
	pItem = &pItem[(DWORD)pItem[0] + 1UL];
	
	if(!pItem[0])return 0UL;

	// バッファにコピー
	if(pDst)::CopyMemory(pDst, &pItem[1], pItem[0]);

	// バッファに必要なバイト数を返す
	return pItem[0];	
}

const DWORD CExtendEventDesc::GetExtDesc(LPTSTR lpszDst) const
{
	const BYTE *pItem = &m_pDescData[5];
	DWORD dwPos = 0UL;

	while(true){
		// ポインタ位置更新
		if(dwPos >= (DWORD)m_pDescData[4]){
			// Descriptionなし
			return 0UL;
			}
		else if((dwPos + (DWORD)pItem[dwPos] + 1U) >= (DWORD)m_pDescData[4]){
			// Descriptionあり
			pItem = &pItem[dwPos];
			break;
			}
		else{			
			dwPos += (DWORD)pItem[dwPos] + 1UL;
			dwPos += (DWORD)pItem[dwPos] + 1UL;
			}
		}

	// アイテムのテキストを返す
	TCHAR szDesc[256] = TEXT("\0");
	CAribString::AribToString(szDesc, &pItem[1], pItem[0]);

	// バッファにコピー
	if(lpszDst)::_tcscpy(lpszDst, szDesc);

	// バッファに必要な文字数を返す
	return ::_tcslen(szDesc);	
}

CExtendEventDesc::CExtendEventDesc(IBonObject *pOwner)
	: CDescBase(pOwner)
{
	// 何もしない
}

CExtendEventDesc::~CExtendEventDesc(void)
{
	// 何もしない
}

const bool CExtendEventDesc::ParseData(void)
{
	// サイズのみチェックする
	return (m_byDescLen >= 8U)? true : false;
}

const BYTE * CExtendEventDesc::GetItemPointer(const DWORD dwIndex) const
{
	// 指定したアイテムの先頭ポインタを返す
	const BYTE *pItem = &m_pDescData[5];
	
	for(DWORD dwPos = 0UL, dwItem = 0UL ; dwPos < (DWORD)m_pDescData[4] ; ){
		if((dwPos + (DWORD)pItem[dwPos] + 1U) >= (DWORD)m_pDescData[4])break;

		if(dwItem++ == dwIndex){
			// インデックス到達、ポインタを返す
			return &pItem[dwPos];
			}
		else{
			// ポインタ位置更新
			dwPos += (DWORD)pItem[dwPos] + 1UL;
			dwPos += (DWORD)pItem[dwPos] + 1UL;
			}
		}
		
	return NULL;
}


/////////////////////////////////////////////////////////////////////////////
// [0x50] Component 記述子クラス
/////////////////////////////////////////////////////////////////////////////

const BYTE CComponentDesc::GetStreamContent(void) const
{
	// コンポーネント内容を返す
	return m_pDescData[0] & 0x0FU;
}

const BYTE CComponentDesc::GetComponentType(void) const
{
	// コンポーネントタイプを返す
	return m_pDescData[1];
}

const BYTE CComponentDesc::GetComponentTag(void) const
{
	// コンポーネントタグを返す
	return m_pDescData[2];
}

const DWORD CComponentDesc::GetLanguageCode(void) const
{
	// 言語コードを返す
	return ((WORD)m_pDescData[3] << 16) | ((WORD)m_pDescData[4] << 8) | (WORD)m_pDescData[5];
}

const DWORD CComponentDesc::GetComponentText(LPTSTR lpszDst) const
{
	// コンポーネント記述を返す
	if(m_byDescLen < 7U)return 0UL;
	
	TCHAR szText[256] = TEXT("\0");
	CAribString::AribToString(szText, &m_pDescData[6], m_byDescLen - 6UL);

	// バッファにコピー
	if(lpszDst)::_tcscpy(lpszDst, szText);

	// バッファに必要な文字数を返す
	return ::_tcslen(szText);
}

const bool CComponentDesc::IsVideoComponent(void) const
{
	// コンポーネント内容が映像に関するものかどうかを返す
	return (GetStreamContent() == 0x01U)? true : false;
}

const WORD CComponentDesc::GetVideoResolution(void) const
{
	// 解像度(横方向のピクセル数)を返す
	if(!IsVideoComponent())return 0U;
	
	switch(GetComponentType() & 0x0FU){
		case 0x01U :
		case 0x02U :
		case 0x03U :				
		case 0x04U :
			switch(GetComponentType() & 0xF0U){
				case 0xD0U :	// 240p
					return 240U;

				case 0x00U :	// 480i
				case 0xA0U :	// 480p
					return 480U;
					
				case 0xC0U :	// 720p
					return 720U;
				
				case 0xB0U :	// 1080i
					return 1080U;
				
				default :		// 未定義の解像度
					return 0U;
				}
			break;
		
		default :
			// 未定義の解像度
			return 0U;
		}
}

const bool CComponentDesc::IsVideoProgressive(void) const
{
	// プログレッシブの有無を返す
	if(!IsVideoComponent())return false;
	
	switch(GetComponentType() & 0x0FU){
		case 0x01U :
		case 0x02U :
		case 0x03U :				
		case 0x04U :
			switch(GetComponentType() & 0xF0U){
				case 0x00U :	// 480i
				case 0xB0U :	// 1080i
					return false;

				case 0xD0U :	// 240p
				case 0xA0U :	// 480p
				case 0xC0U :	// 720p
					return true;

				default :		// 未定義の解像度
					return false;
				}
			break;
		
		default :
			// 未定義の解像度
			return false;
		}
}

const bool CComponentDesc::IsVideoAspectWide(void) const
{
	// アスペクト比16:9の有無を返す
	if(!IsVideoComponent())return true;

	switch(GetComponentType() & 0xF0U){
		case 0xD0U :	// 240p
		case 0x00U :	// 480i
		case 0xA0U :	// 480p
		case 0xC0U :	// 720p
		case 0xB0U :	// 1080i
			switch(GetComponentType() & 0x0FU){
				case 0x01U :
					return false;	// 4:3
				
				case 0x02U :
				case 0x03U :				
				case 0x04U :
					return true;	// 16:9
		
				default :
					// 未定義の解像度
					return true;
				}
			break;

		default :	// 未定義の解像度
			return true;
		}
}

const bool CComponentDesc::IsVideoPanAndScan(void) const
{
	// パンスキャンの有無を返す
	if(!IsVideoComponent())return false;

	switch(GetComponentType() & 0xF0U){
		case 0xD0U :	// 240p
		case 0x00U :	// 480i
		case 0xA0U :	// 480p
		case 0xC0U :	// 720p
		case 0xB0U :	// 1080i
			switch(GetComponentType() & 0x0FU){
				case 0x02U :
					return true;	// パンベクトルあり
				
				case 0x01U :
				case 0x03U :				
				case 0x04U :
					return false;	// パンベクトルなし
		
				default :
					// 未定義の解像度
					return false;
				}
			break;

		default :	// 未定義の解像度
			return false;
		}
}

CComponentDesc::CComponentDesc(IBonObject *pOwner)
	: CDescBase(pOwner)
{
	// 何もしない
}

CComponentDesc::~CComponentDesc(void)
{
	// 何もしない
}

const bool CComponentDesc::ParseData(void)
{
	// サイズのみチェックする
	return (m_byDescLen >= 6U)? true : false;
}


/////////////////////////////////////////////////////////////////////////////
// [0x52] Stream Identifier 記述子クラス
/////////////////////////////////////////////////////////////////////////////

const BYTE CStreamIdDesc::GetComponentTag(void) const
{
	// コンポーネントタグを返す
	return m_pDescData[0];
}

CStreamIdDesc::CStreamIdDesc(IBonObject *pOwner)
	: CDescBase(pOwner)
{
	// 何もしない
}

CStreamIdDesc::~CStreamIdDesc(void)
{
	// 何もしない
}

const bool CStreamIdDesc::ParseData(void)
{
	// サイズのみチェックする
	return (m_byDescLen == 1U)? true : false;
}


/////////////////////////////////////////////////////////////////////////////
// [0x54] Content 記述子クラス
/////////////////////////////////////////////////////////////////////////////

const DWORD CContentDesc::GetGenreNum(void) const
{
	// ジャンルの数を返す
	return m_byDescLen / 2U;
}

const BYTE CContentDesc::GetGenreLevel1(const DWORD dwIndex) const
{
	// ジャンル1を返す
	return (dwIndex < GetGenreNum())? (m_pDescData[dwIndex / 2UL + 0UL] >> 4) : 0x0FU;
}

const BYTE CContentDesc::GetGenreLevel2(const DWORD dwIndex) const
{
	// ジャンル2を返す
	return (dwIndex < GetGenreNum())? (m_pDescData[dwIndex / 2UL + 0UL] & 0x0F) : 0x0FU;
}

const BYTE CContentDesc::GetUserGenre1(const DWORD dwIndex) const
{
	// ユーザジャンル1を返す
	return (dwIndex < GetGenreNum())? (m_pDescData[dwIndex / 2UL + 1UL] >> 4) : 0x0FU;
}

const BYTE CContentDesc::GetUserGenre2(const DWORD dwIndex) const
{
	// ユーザジャンル2を返す
	return (dwIndex < GetGenreNum())? (m_pDescData[dwIndex / 2UL + 1UL] & 0x0F) : 0x0FU;
}

CContentDesc::CContentDesc(IBonObject *pOwner)
	: CDescBase(pOwner)
{
	// 何もしない
}

CContentDesc::~CContentDesc(void)
{
	// 何もしない
}

const bool CContentDesc::ParseData(void)
{
	// サイズのみチェックする
	return (m_byDescLen && !(m_byDescLen % 2U))? true : false;
}


/////////////////////////////////////////////////////////////////////////////
// [0xC4] 音声 Component 記述子クラス
/////////////////////////////////////////////////////////////////////////////

const BYTE CAudioComponentDesc::GetComponentType(void) const
{
	// コンポーネント種別を返す
	return m_pDescData[1];
}

const BYTE CAudioComponentDesc::GetComponentTag(void) const
{
	// コンポーネントタグを返す
	return m_pDescData[2];
}

const BYTE CAudioComponentDesc::GetStreamType(void) const
{
	// ストリーム形式を返す
	return m_pDescData[3];
}

const BYTE CAudioComponentDesc::GetSimulcastGroupTag(void) const
{
	// サイマルキャストグループタグを返す
	return m_pDescData[4];
}

const bool CAudioComponentDesc::IsEsMultiLanguage(void) const
{
	// ES多言語フラグを返す
	return (m_pDescData[5] & 0x80U)? true : false;
}

const bool CAudioComponentDesc::IsMainComponent(void) const
{
	// 主コンポーネントフラグを返す
	return (m_pDescData[5] & 0x40U)? true : false;
}

const BYTE CAudioComponentDesc::GetQualityIndicator(void) const
{
	// 音質表示を返す
	return (m_pDescData[5] >> 4) & 0x03U;
}

const BYTE CAudioComponentDesc::GetSamplingRate(void) const
{
	// サンプリング周波数を返す
	return (m_pDescData[5] >> 1) & 0x07U;
}

const DWORD CAudioComponentDesc::GetMainLanguageCode(void) const
{
	// 第1音声の言語コードを返す
	return ((WORD)m_pDescData[6] << 16) | ((WORD)m_pDescData[7] << 8) | (WORD)m_pDescData[8];
}

const DWORD CAudioComponentDesc::GetSubLanguageCode(void) const
{
	// 第2音声の言語コードを返す
	if(IsEsMultiLanguage() && (m_byDescLen >= 12U)){
		// 第2音声あり
		return ((WORD)m_pDescData[9] << 16) | ((WORD)m_pDescData[10] << 8) | (WORD)m_pDescData[11];
		}
	else{
		// 第2音声なし
		return 0x00000000UL;
		}
}

const DWORD CAudioComponentDesc::GetComponentText(LPTSTR lpszDst) const
{
	// コンポーネント記述を返す
	TCHAR szText[256] = TEXT("\0");

	if(IsEsMultiLanguage() && (m_byDescLen >= 13U)){
		// 第2音声あり
		CAribString::AribToString(szText, &m_pDescData[12], m_byDescLen - 12UL);
		}
	else if(m_byDescLen >= 10U){
		// 第2音声なし
		CAribString::AribToString(szText, &m_pDescData[9], m_byDescLen - 9UL);
		}
	else{
		// コンポーネント記述なし
		return 0UL;
		}

	// バッファにコピー
	if(lpszDst)::_tcscpy(lpszDst, szText);

	// バッファに必要な文字数を返す
	return ::_tcslen(szText);
}

const bool CAudioComponentDesc::IsDualMonoMode(void) const
{
	// 二重音声の有無を返す
	return (GetComponentType() == 0x02U)? true : false;
}

const bool CAudioComponentDesc::IsEnableCenter(void) const
{
	// センタースピーカーの有無
	switch(GetComponentType()){
		case 0x01U :	// 1/0
		case 0x02U :	// 1/0+1/0
		case 0x05U :	// 3/0
		case 0x07U :	// 3/1
		case 0x08U :	// 3/2
		case 0x09U :	// 3/2+LFE
			return true;
		
		default	:
			return false;		
		}
}

const bool CAudioComponentDesc::IsEnableFront(void) const
{
	// フロントメインスピーカーの有無
	switch(GetComponentType()){
		case 0x03U :	// 2/0		Stereo
		case 0x04U :	// 2/1
		case 0x05U :	// 3/0
		case 0x06U :	// 2/2
		case 0x07U :	// 3/1
		case 0x08U :	// 3/2
		case 0x09U :	// 3/2+LFE
			return true;
		
		default	:
			return false;		
		}
}

const bool CAudioComponentDesc::IsEnableSurround(void) const
{
	// リアサラウンドスピーカーの有無
	switch(GetComponentType()){
		case 0x04U :	// 2/1
		case 0x06U :	// 2/2
		case 0x07U :	// 3/1
		case 0x08U :	// 3/2
		case 0x09U :	// 3/2+LFE
			return true;
		
		default	:
			return false;		
		}
}

const bool CAudioComponentDesc::IsEnableLFE(void) const
{
	// LFEスピーカーの有無
	switch(GetComponentType()){
		case 0x09U :	// 3/2+LFE
			return true;
		
		default	:
			return false;		
		}
}

CAudioComponentDesc::CAudioComponentDesc(IBonObject *pOwner)
	: CDescBase(pOwner)
{
	// 何もしない
}

CAudioComponentDesc::~CAudioComponentDesc(void)
{
	// 何もしない
}

const bool CAudioComponentDesc::ParseData(void)
{
	// サイズのみチェックする
	if(m_byDescLen < 9U){
		// サイズが不正
		return false;
		}
	else if(IsEsMultiLanguage()){
		// 第2音声あり
		return (m_byDescLen >= 12U)? true : false;
		}
	else{
		// 第2音声なし
		return (m_byDescLen >= 9U)? true : false;
		}
}
