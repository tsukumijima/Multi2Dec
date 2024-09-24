// BcasCardReader.cpp: B-CASカードリーダクラス
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include <Memory>
#include <TChar.h>
#include "BcasCardReader.h"


#pragma comment(lib, "WinScard.lib")


/////////////////////////////////////////////////////////////////////////////
// ファイルローカル定数設定
/////////////////////////////////////////////////////////////////////////////

// デバイス名
#define DEVICENAME	TEXT("B-CAS 汎用スマートカードリーダ")


/////////////////////////////////////////////////////////////////////////////
// 汎用スマートカードリーダ用 B-CASカードインタフェース実装クラス
/////////////////////////////////////////////////////////////////////////////

const BONGUID CBcasCardReader::GetDeviceType(void)
{
	// デバイスのタイプを返す
	return ::BON_NAME_TO_GUID(TEXT("IHalBcasCard"));
}

const DWORD CBcasCardReader::GetDeviceName(LPTSTR lpszName)
{
	// デバイス名を返す
	if(lpszName)::_tcscpy(lpszName, DEVICENAME);

	return ::_tcslen(DEVICENAME);
}

const DWORD CBcasCardReader::GetTotalDeviceNum(void)
{
	// B-CASカードが挿入されているカードリーダ数を返す

	// スマートカードリーダ列挙
	if(!EnumBcasCardReader())return 0UL;
	
	SCARDHANDLE hBcasCard;
	DWORD dwBcasCardNum = 0UL;
	DWORD dwActiveProtocol;
	
	// 初期化試行
	for(DWORD dwIndex = 0UL ; dwIndex < m_CardReaderArray.size() ; dwIndex++){

		hBcasCard = NULL;
		dwActiveProtocol = SCARD_PROTOCOL_UNDEFINED;

		// カードリーダに接続
		if(::SCardConnect(m_ScardContext, m_CardReaderArray[dwIndex].c_str(), SCARD_SHARE_SHARED, SCARD_PROTOCOL_T1, &hBcasCard, &dwActiveProtocol) == SCARD_S_SUCCESS){
			// プロトコルチェック
			if(dwActiveProtocol == SCARD_PROTOCOL_T1){
				// 初期化コマンド送信
				if(InitialSetting(hBcasCard)){
					// B-CASカード数インクリメント
					dwBcasCardNum++;
					}
				}
			// カードリーダ切断
			::SCardDisconnect(hBcasCard, SCARD_LEAVE_CARD);	
			}
		}

	return dwBcasCardNum;
}

const DWORD CBcasCardReader::GetActiveDeviceNum(void)
{
	// スマートカードリーダは共用可能なため常に0を返す
	return 0UL;
}

const bool CBcasCardReader::OpenCard(void)
{
	// 一旦クローズする
	CloseCard();

	// スマートカードリーダ列挙
	if(!EnumBcasCardReader())return false;

	DWORD dwActiveProtocol;
	
	// 初期化試行
	for(DWORD dwIndex = 0UL ; dwIndex < m_CardReaderArray.size() ; dwIndex++){

		m_hBcasCard = NULL;
		dwActiveProtocol = SCARD_PROTOCOL_UNDEFINED;

		// カードリーダに接続
		if(::SCardConnect(m_ScardContext, m_CardReaderArray[dwIndex].c_str(), SCARD_SHARE_SHARED, SCARD_PROTOCOL_T1, &m_hBcasCard, &dwActiveProtocol) == SCARD_S_SUCCESS){
			// プロトコルチェック
			if(dwActiveProtocol == SCARD_PROTOCOL_T1){
				// 初期化コマンド送信
				if(InitialSetting(m_hBcasCard)){
					// B-CASカード数インクリメント
					break;
					}
				}

			// カードリーダ切断
			::SCardDisconnect(m_hBcasCard, SCARD_LEAVE_CARD);
			m_hBcasCard = NULL;
			}
		}

	return (m_hBcasCard)? true : false;
}

void CBcasCardReader::CloseCard(void)
{
	// カードリーダ切断
	if(m_hBcasCard){
		::SCardDisconnect(m_hBcasCard, SCARD_LEAVE_CARD);
		m_hBcasCard = NULL;
		}
}

const bool CBcasCardReader::IsInteractive(void)
{
	// 双方向通信の可否を返す
	return true;
}

const BYTE * CBcasCardReader::GetBcasCardID(void)
{
	// Card ID を返す
	return (m_hBcasCard)? m_BcasCardInfo.BcasCardID : NULL;
}

const BYTE * CBcasCardReader::GetInitialCbc(void)
{
	// Descrambler CBC Initial Value を返す
	return (m_hBcasCard)? m_BcasCardInfo.InitialCbc : NULL;
}

const BYTE * CBcasCardReader::GetSystemKey(void)
{
	// Descrambling System Key を返す
	return (m_hBcasCard)? m_BcasCardInfo.SystemKey : NULL;
}

const BYTE * CBcasCardReader::GetKsFromEcm(const BYTE *pEcmData, const DWORD dwEcmSize)
{
	static const BYTE abyEcmReceiveCmd[] = {0x90U, 0x34U, 0x00U, 0x00U};

	// 「ECM Receive Command」を処理する
	if(!m_hBcasCard){
		m_dwEcmError = EC_NO_ERROR;
		return NULL;
		}

	// ECMサイズをチェック
	if(!pEcmData || (dwEcmSize < 30UL) || (dwEcmSize > 256UL)){
		m_dwEcmError = EC_BADARGUMENT;
		return NULL;
		}

	// バッファ準備
	DWORD dwRecvSize = 0UL;
	BYTE SendData[1024];
	BYTE RecvData[1024];
	::ZeroMemory(RecvData, sizeof(RecvData));

	// コマンド構築
	::CopyMemory(SendData, abyEcmReceiveCmd, sizeof(abyEcmReceiveCmd));			// CLA, INS, P1, P2
	SendData[sizeof(abyEcmReceiveCmd)] = (BYTE)dwEcmSize;						// COMMAND DATA LENGTH
	::CopyMemory(&SendData[sizeof(abyEcmReceiveCmd) + 1], pEcmData, dwEcmSize);	// ECM
	SendData[sizeof(abyEcmReceiveCmd) + dwEcmSize + 1] = 0x00U;					// RESPONSE DATA LENGTH

	// コマンド送信
	if(!TransmitCommand(m_hBcasCard, SendData, sizeof(abyEcmReceiveCmd) + dwEcmSize + 2UL, RecvData, sizeof(RecvData), &dwRecvSize)){
		::ZeroMemory(&m_EcmStatus, sizeof(m_EcmStatus));
		m_dwEcmError = EC_TRANSMIT_ERROR;
		return NULL;
		}

	// サイズチェック
	if(dwRecvSize != 25UL){
		::ZeroMemory(&m_EcmStatus, sizeof(m_EcmStatus));
		m_dwEcmError = EC_TRANSMIT_ERROR;
		return NULL;
		}	
	
	// レスポンス解析
	::CopyMemory(m_EcmStatus.KsData, &RecvData[6], sizeof(m_EcmStatus.KsData));

	// リターンコード解析
	switch(((WORD)RecvData[4] << 8) | (WORD)RecvData[5]){
		// 購入済み: 視聴中
		case 0x0200U :	// 後払いPPV
		case 0x0400U :	// 前払いPPV
		case 0x0800U :	// ティア
		
		// 購入可: プレビュー中
		case 0x4480U :	// 後払いPPV
		case 0x4280U :	// 前払いPPV
			m_dwEcmError = EC_NO_ERROR;
			return m_EcmStatus.KsData;
		
		// 上記以外(視聴不可)
		default :
			m_dwEcmError = EC_NOT_CONTRACTED;
			return NULL;
		}
}

const DWORD CBcasCardReader::GetEcmError(void)
{
	// ECM処理エラーコードを返す
	return m_dwEcmError;
}

const bool CBcasCardReader::SendEmmSection(const BYTE *pEmmData, const DWORD dwEmmSize)
{
	static const BYTE abyEmmReceiveCmd[] = {0x90U, 0x36U, 0x00U, 0x00U};

	// 「EMM Receive Command」を処理する
	if(!m_hBcasCard)return false;

	// EMMサイズをチェック
	if(!pEmmData || (dwEmmSize < 17UL) || (dwEmmSize > 263UL))return false;

	// バッファ準備
	DWORD dwRecvSize = 0UL;
	BYTE SendData[1024];
	BYTE RecvData[1024];
	::ZeroMemory(RecvData, sizeof(RecvData));

	// コマンド構築
	::CopyMemory(SendData, abyEmmReceiveCmd, sizeof(abyEmmReceiveCmd));			// CLA, INS, P1, P2
	SendData[sizeof(abyEmmReceiveCmd)] = (BYTE)dwEmmSize;						// COMMAND DATA LENGTH
	::CopyMemory(&SendData[sizeof(abyEmmReceiveCmd) + 1], pEmmData, dwEmmSize);	// EMM
	SendData[sizeof(abyEmmReceiveCmd) + dwEmmSize + 1] = 0x00U;					// RESPONSE DATA LENGTH

	// コマンド送信
	if(!TransmitCommand(m_hBcasCard, SendData, sizeof(abyEmmReceiveCmd) + dwEmmSize + 2UL, RecvData, sizeof(RecvData), &dwRecvSize))return false;

	// サイズチェック
	if(dwRecvSize != 8UL)return false;
	
	// リターンコード解析
	switch(((WORD)RecvData[4] << 8) | (WORD)RecvData[5]){
		case 0x2100U :	// 正常終了
			return true;

		case 0xA102U :	// 非運用(運用外プロトコル番号)
		case 0xA107U :	// セキュリティエラー(EMM改ざんエラー)
			return false;
		
		// 上記以外(例外)
		default :
			return false;
		}
}

const WORD CBcasCardReader::GetPowerCtrlInfo(POWERCTRLINFO *pPowerCtrlInfo)
{
	BYTE abyReqPowerCtrlCmd[] = {0x90U, 0x80U, 0x00U, 0x00U, 0x01U, 0x00U, 0x00U};

	// 通電制御情報を取得する
	if(!m_hBcasCard)return 0U;

	// バッファ準備
	DWORD dwRecvSize = 0UL;
	BYTE RecvData[1024];
	::ZeroMemory(RecvData, sizeof(RecvData));

	// コマンド送信
	if(!TransmitCommand(m_hBcasCard, abyReqPowerCtrlCmd, sizeof(abyReqPowerCtrlCmd), RecvData, sizeof(RecvData), &dwRecvSize))return 0U;

	// 受信サイズチェック
	if(dwRecvSize != 20UL)return 0U;		

	// リターンコード解析
	if((((WORD)RecvData[4] << 8) | (WORD)RecvData[5]) != 0x2100U)return 0U;

	// レスポンス解析
	const WORD wInfoNum = (WORD)RecvData[7] + 1U;
	if(!pPowerCtrlInfo)return wInfoNum;		// 引数がNULLのときは必要なバッファ数を返す

	CTsTime TsTime;

	for(WORD wIndex = 0U ; wIndex < wInfoNum ; wIndex++){
		// コマンド設定
		abyReqPowerCtrlCmd[5] = (BYTE)wIndex;

		// コマンド送信
		if(!TransmitCommand(m_hBcasCard, abyReqPowerCtrlCmd, sizeof(abyReqPowerCtrlCmd), RecvData, sizeof(RecvData), &dwRecvSize))return 0U;

		// 受信サイズチェック
		if(dwRecvSize != 20UL)return 0U;		

		// リターンコード解析
		if((((WORD)RecvData[4] << 8) | (WORD)RecvData[5]) != 0x2100U)return 0U;
		
		// レスポンス解析
		TsTime.ClearTime();
		CAribTime::SplitAribMjd(((WORD)RecvData[9] << 8) | (WORD)RecvData[10], &TsTime.wYear, &TsTime.wMonth, &TsTime.wDay, &TsTime.wDayOfWeek);
		TsTime -= (DWORD)RecvData[11] * 24UL * 60UL * 60UL;
		pPowerCtrlInfo[wIndex].stStartTime = TsTime;
		TsTime += (DWORD)RecvData[12] * 24UL * 60UL * 60UL;
		pPowerCtrlInfo[wIndex].stEndTime = TsTime;
		pPowerCtrlInfo[wIndex].byDurTime = RecvData[13];
		pPowerCtrlInfo[wIndex].wNtID = (((WORD)RecvData[14] << 8) | (WORD)RecvData[15]);
		pPowerCtrlInfo[wIndex].wTsID = (((WORD)RecvData[16] << 8) | (WORD)RecvData[17]);
		}

	return wInfoNum;
}

CBcasCardReader::CBcasCardReader(IBonObject *pOwner)
	: CBonObject(pOwner)
	, m_ScardContext(NULL)
	, m_hBcasCard(NULL)
	, m_dwEcmError(EC_NOT_OPEN)
{
	// 内部状態初期化
	::ZeroMemory(&m_BcasCardInfo, sizeof(m_BcasCardInfo));
	::ZeroMemory(&m_EcmStatus, sizeof(m_EcmStatus));

	// リソースマネージャコンテキスト確立
	::SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &m_ScardContext);
}

CBcasCardReader::~CBcasCardReader(void)
{
	CloseCard();

	// リソースマネージャコンテキストの開放
	if(m_ScardContext)::SCardReleaseContext(m_ScardContext);
}

const bool CBcasCardReader::EnumBcasCardReader(void)
{
	// カードリーダを列挙する
	DWORD dwBuffSize = 0UL;
	
	// バッファサイズ取得
	if(::SCardListReaders(m_ScardContext, NULL, NULL, &dwBuffSize) != SCARD_S_SUCCESS)return false;

	// バッファ確保
	std::auto_ptr<TCHAR> szReaders(new TCHAR[dwBuffSize]);

	// カードリーダ列挙
	if(::SCardListReaders(m_ScardContext, NULL, szReaders.get(), &dwBuffSize) != SCARD_S_SUCCESS)return false;

	// カードリーダ名保存
	LPTSTR lpszCurReader = szReaders.get();
	m_CardReaderArray.clear();
			
	while(*lpszCurReader){
		m_CardReaderArray.push_back(lpszCurReader);
		lpszCurReader += m_CardReaderArray.back().length() + 1UL;
		}

	return (m_CardReaderArray.size())? true : false;
}

const bool CBcasCardReader::TransmitCommand(const SCARDHANDLE hBcasCard, const BYTE *pSendData, const DWORD dwSendSize, BYTE *pRecvData, const DWORD dwMaxRecv, DWORD *pdwRecvSize)
{
	DWORD dwRecvSize = dwMaxRecv;

	// データ送受信
	DWORD dwReturn = ::SCardTransmit(hBcasCard, SCARD_PCI_T1, pSendData, dwSendSize, NULL, pRecvData, &dwRecvSize);
	
	// 受信サイズ格納
	if(pdwRecvSize)*pdwRecvSize = dwRecvSize;

	return (dwReturn == SCARD_S_SUCCESS)? true : false;
}

const bool CBcasCardReader::InitialSetting(const SCARDHANDLE hBcasCard)
{
	static const BYTE abyInitSettingCmd[] = {0x90U, 0x30U, 0x00U, 0x00U, 0x00U};

	// 「Initial Setting Conditions Command」を処理する
	if(!hBcasCard)return false;

	// バッファ準備
	DWORD dwRecvSize = 0UL;
	BYTE RecvData[1024];
	::ZeroMemory(RecvData, sizeof(RecvData));
	
	// コマンド送信
	if(!TransmitCommand(hBcasCard, abyInitSettingCmd, sizeof(abyInitSettingCmd), RecvData, sizeof(RecvData), &dwRecvSize))return false;

	// 受信サイズチェック
	if(dwRecvSize < 57UL)return false;		

	// レスポンス解析
	::CopyMemory(m_BcasCardInfo.BcasCardID, &RecvData[8],   6UL);	// +8	Card ID
	::CopyMemory(m_BcasCardInfo.SystemKey,  &RecvData[16], 32UL);	// +16	Descrambling system key
	::CopyMemory(m_BcasCardInfo.InitialCbc, &RecvData[48],  8UL);	// +48	Descrambler CBC initial value

	// ECMステータス初期化
	::ZeroMemory(&m_EcmStatus, sizeof(m_EcmStatus));

	return true;
}
