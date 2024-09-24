// TsSourceTuner.cpp: TSソースチューナデコーダ
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "TsSourceTuner.h"


/////////////////////////////////////////////////////////////////////////////
// ファイルローカル定数設定
/////////////////////////////////////////////////////////////////////////////

#define TSBUFFER_SIZE	0x10000UL	// 64KB


/////////////////////////////////////////////////////////////////////////////
// TSソースチューナデコーダ
/////////////////////////////////////////////////////////////////////////////

const DWORD CTsSourceTuner::OpenTuner(LPCTSTR lpszBCId)
{
	// 一旦クローズ
	CloseTuner();

	IHalDevice *pHalDevice = NULL;

	try{
		// チューナのインスタンスを生成する
		if(!(pHalDevice = ::BON_SAFE_CREATE<IHalDevice *>(lpszBCId)))throw (DWORD)EC_INTERNAL_ERR;

		// デバイスタイプをチェック
		if(pHalDevice->GetDeviceType() != ::BON_NAME_TO_GUID(TEXT("IHalTsTuner")))throw (DWORD)EC_INTERNAL_ERR;

		// デバイスの存在をチェック
		if(!pHalDevice->GetTotalDeviceNum())throw (DWORD)EC_DEVICE_NONE;

		// デバイスの空きをチェック
		if(pHalDevice->GetActiveDeviceNum() >= pHalDevice->GetTotalDeviceNum())throw (DWORD)EC_DEVICE_FULL;
	
		// IHalTsTunerインタフェース取得
		if(!(m_pHalTsTuner = dynamic_cast<IHalTsTuner *>(pHalDevice)))throw (DWORD)EC_INTERNAL_ERR;

		// TSバッファインスタンス生成
		if(!(m_pTsBuffer = ::BON_SAFE_CREATE<IMediaData *>(TEXT("CMediaData"))))throw (DWORD)EC_INTERNAL_ERR;
	
		// TSバッファサイズ確保
		if(m_pTsBuffer->GetBuffer(TSBUFFER_SIZE) != TSBUFFER_SIZE)throw (DWORD)EC_INTERNAL_ERR;

		// チューナをオープン
		if(!m_pHalTsTuner->OpenTuner())throw (DWORD)EC_OPEN_FAILED;

		// TS受信スレッド起動
		if(!m_TsRecvThread.StartThread(this, &CTsSourceTuner::TsRecvThread))throw (DWORD)EC_INTERNAL_ERR;
		}
	catch(DWORD dwErrCode){
		// エラー発生
		BON_SAFE_RELEASE(pHalDevice);
		m_pHalTsTuner = NULL;
		
		CloseTuner();
		return dwErrCode;
		}

	return EC_OPEN_SUCCESS;
}

const bool CTsSourceTuner::CloseTuner(void)
{
	// TS受信スレッド停止
	if(m_bIsPlaying)StopDecoder();

	m_bIsPlaying = false;
	m_TsRecvThread.EndThread(true);

	// チューナをクローズ
	if(m_pHalTsTuner)m_pHalTsTuner->CloseTuner();

	// TSバッファを開放
	BON_SAFE_RELEASE(m_pTsBuffer);

	// チューナインスタンス開放
	BON_SAFE_RELEASE(m_pHalTsTuner);
	
	return true;
}

const DWORD CTsSourceTuner::GetDeviceName(LPTSTR lpszName)
{
	// IHalDeviceインタフェース取得
	IHalDevice * const pHalDevice = dynamic_cast<IHalDevice *>(m_pHalTsTuner);

	// デバイス名を返す
	return (pHalDevice)? pHalDevice->GetDeviceName(lpszName) : 0UL;
}

const DWORD CTsSourceTuner::GetTuningSpaceName(const DWORD dwSpace, LPTSTR lpszName)
{
	// チューナがオープンされていない
	if(!m_pHalTsTuner)return 0UL;

	// チューニング空間名を返す
	return m_pHalTsTuner->EnumTuningSpace(dwSpace, lpszName);
}

const DWORD CTsSourceTuner::GetChannelName(const DWORD dwSpace, const DWORD dwChannel, LPTSTR lpszName)
{
	// チューナがオープンされていない
	if(!m_pHalTsTuner)return 0UL;

	// チャンネル名を返す
	return m_pHalTsTuner->EnumChannelName(dwSpace, dwChannel, lpszName);
}

const DWORD CTsSourceTuner::GetCurSpace(void)
{
	// 現在のチューニング空間を返す
	return (m_pHalTsTuner)? m_pHalTsTuner->GetCurSpace() : 0UL;
}

const DWORD CTsSourceTuner::GetCurChannel(void)
{
	// 現在のチャンネルを返す
	return (m_pHalTsTuner)? m_pHalTsTuner->GetCurChannel() : 0UL;
}

const bool CTsSourceTuner::SetChannel(const DWORD dwSpace, const DWORD dwChannel)
{
	// チューナがオープンされていない
	if(!m_pHalTsTuner)return 0UL;

	// チャンネルを変更する
	if(!m_pHalTsTuner->SetChannel(dwSpace, dwChannel))return false;
	
	// 下位デコーダをリセットする
	ResetDecoder();

	return true;
}

const float CTsSourceTuner::GetSignalLevel(void)
{
	// チューナがオープンされていない
	if(!m_pHalTsTuner)return 0UL;

	// 信号レベルを返す
	return m_pHalTsTuner->GetSignalLevel();
}

IHalTsTuner * CTsSourceTuner::GetHalTsTuner(void)
{
	// チューナデバイスを返す
	return m_pHalTsTuner;
}

CTsSourceTuner::CTsSourceTuner(IBonObject *pOwner)
	: CMediaDecoder(pOwner, 0UL, 1UL, new CSmartLock)
	, m_pHalTsTuner(NULL)
	, m_pTsBuffer(NULL)
	, m_bIsPlaying(false)
{

}

CTsSourceTuner::~CTsSourceTuner(void)
{
	CloseTuner();
}

const bool CTsSourceTuner::OnPlay(void)
{
	// チューナがオープンされていない
	if(!m_pHalTsTuner)return false;

	// すでに再生中
	if(m_bIsPlaying)return true;

	// 未処理のストリームを破棄する
	m_pHalTsTuner->PurgeStream();

	// ストリームを再生状態にする
	m_bIsPlaying = true;

	return true;
}

const bool CTsSourceTuner::OnStop(void)
{
	// チューナがオープンされていない
	if(!m_pHalTsTuner)return false;

	// ストリームを停止状態にする
	m_bIsPlaying = false;
	
	return true;
}

const bool CTsSourceTuner::OnReset(void)
{
	// 未処理のストリームを破棄する
	if(m_bIsPlaying && m_pHalTsTuner)m_pHalTsTuner->PurgeStream();

	return true;
}

void CTsSourceTuner::TsRecvThread(CSmartThread<CTsSourceTuner> *pThread, bool &bKillSignal, PVOID pParam)
{
	BYTE *pStreamData = NULL;
	DWORD dwStreamSize = 0UL;
	DWORD dwRemainNum = 0UL;

	// チューナからTSデータを取り出すスレッド
	while(!bKillSignal){
		
		// 処理簡略化のためポーリング方式を採用する
		do{
			if(m_pHalTsTuner->GetStream(&pStreamData, &dwStreamSize, &dwRemainNum)){
				if(pStreamData && dwStreamSize){
					OnTsStream(pStreamData, dwStreamSize);
					}
				}
			}
		while(dwRemainNum);

		// ウェイト
		::Sleep(1UL);
		}
}

const bool CTsSourceTuner::PurgeStream(void)
{
	// チューナがオープンされていない
	if(!m_pHalTsTuner)return false;

	// 未処理のストリームを破棄する
	m_pHalTsTuner->PurgeStream();

	// 下位デコーダをリセットする
	ResetDecoder();

	return true;
}

void CTsSourceTuner::OnTsStream(BYTE *pStreamData, DWORD dwStreamSize)
{
	CBlockLock AutoLock(m_pLock);

	// 停止中は何もしない
	if(!m_bIsPlaying)return;

	// データをバッファにセットする
	m_pTsBuffer->SetData(pStreamData, dwStreamSize);

	// 次のデコーダにデータを渡す
	OutputMedia(m_pTsBuffer);
}
