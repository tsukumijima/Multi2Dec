// FileReader.cpp: ファイルリーダデコーダ
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "FileReader.h"


/////////////////////////////////////////////////////////////////////////////
// ファイルローカル定数設定
/////////////////////////////////////////////////////////////////////////////

// デフォルトのバッファサイズ
#define DEF_BUFFSIZE	0x100000UL							// 1MB
#define DEF_BUFFNUM		8UL									// 250ms  相当


/////////////////////////////////////////////////////////////////////////////
// ファイルリーダデコーダ
/////////////////////////////////////////////////////////////////////////////

const bool CFileReader::OpenFile(LPCTSTR lpszFileName, const bool bAsyncRead, const DWORD dwBuffSize)
{
	CloseFile();

	try{
		// バッファサイズ計算
		m_dwBuffSize = (dwBuffSize)? dwBuffSize : DEF_BUFFSIZE;
		const DWORD dwReadNum = (bAsyncRead)? DEF_BUFFNUM : 1UL;

		// スマートファイルインスタンス生成
		if(!(m_pFile = ::BON_SAFE_CREATE<ISmartFile *>(TEXT("CSmartFile"))))throw __LINE__;

		// ファイルオープン
		if(!m_pFile->Open(lpszFileName, ISmartFile::MF_READ, m_dwBuffSize))throw __LINE__;

		// ファイルサイズチェック
		if(!m_pFile->GetLength())throw __LINE__;

		// メディアプールサイズリサイズ
		m_MediaPool.resize(dwReadNum);
		if(m_MediaPool.size() < dwReadNum)throw __LINE__;
		::ZeroMemory(&m_MediaPool[0], sizeof(IMediaData *) * dwReadNum);

		// メディアデータインスタンス確保
		for(DWORD dwIndex = 0UL ; dwIndex < dwReadNum ; dwIndex++){
			m_MediaPool[dwIndex] = ::BON_SAFE_CREATE<IMediaData *>(TEXT("CMediaData"));
			if(!m_MediaPool[dwIndex])throw __LINE__;

			// バッファ確保
			m_MediaPool[dwIndex]->SetSize(m_dwBuffSize);
			}

		// メディアプール先頭イテレータセット
		m_itFreeMedia = m_MediaPool.begin();
		}
	catch(...){
		// エラー発生
		CloseFile();
		return false;
		}

	return true;
}

bool CFileReader::CloseFile(void)
{
	// デコーダグラフを停止
	StopDecoder();

	// ファイルクローズ
	if(m_pFile)m_pFile->Close();

	// スマートファイルインスタンス開放
	BON_SAFE_RELEASE(m_pFile);

	// メディアキュークリア
	while(!m_MediaQue.empty())m_MediaQue.pop();

	// メディアデータインスタンス開放
	for(DWORD dwIndex = 0UL ; dwIndex < m_MediaPool.size() ; dwIndex++){
		BON_SAFE_RELEASE(m_MediaPool[dwIndex]);
		}

	// メディアプール開放
	m_MediaPool.clear();

	return true;
}

const DWORD CFileReader::ReadSync(const DWORD dwReadSize)
{
	if(!m_pFile || !dwReadSize)return 0UL;

	// 要求サイズ確保
	if((*m_itFreeMedia)->SetSize(dwReadSize) != dwReadSize)return 0UL;

	// データ読み込み
	const DWORD dwActualSize = m_pFile->ReadData((*m_itFreeMedia)->GetData(), dwReadSize);
	if(!dwActualSize)return 0UL;

	// 有効サイズ設定
	if((*m_itFreeMedia)->SetSize(dwActualSize) != dwActualSize)return 0UL;

	// メディアデータ出力
	return OutputMedia(*m_itFreeMedia)? dwActualSize : 0UL;
}

const bool CFileReader::SetReadPos(const ULONGLONG llPos)
{
	CBlockLock AutoLock(m_pLock);

	// ファイルシーク
	return (m_pFile)? m_pFile->SeekPos(llPos) : false;
}

const ULONGLONG CFileReader::GetReadPos(void)
{
	CBlockLock AutoLock(m_pLock);

	// ファイルポジションを返す
	return (m_pFile)? m_pFile->GetPos() : 0ULL;
}

const ULONGLONG CFileReader::GetFileLength(void)
{
	CBlockLock AutoLock(m_pLock);

	// ファイルサイズを返す
	return (m_pFile)? m_pFile->GetLength() : 0ULL;
}

CFileReader::CFileReader(IBonObject *pOwner)
	: CMediaDecoder(pOwner, 0UL, 1UL, new CSmartLock)
	, m_pFile(NULL)
	, m_dwBuffSize(DEF_BUFFSIZE)
{
	// 何もしない
}

CFileReader::~CFileReader(void)
{
	// ファイルを閉じる
	CloseFile();
}

const bool CFileReader::OnPlay(void)
{
	OnStop();

	// メディアキュークリア
	while(!m_MediaQue.empty())m_MediaQue.pop();

	// メディアプール先頭イテレータセット
	m_itFreeMedia = m_MediaPool.begin();

	// スレッド開始
	if(m_MediaPool.size()){
		// 出力スレッド開始
		if(!m_OutputThread.StartThread(this, &CFileReader::OutputThread, NULL, false))return false;

		// 読み込みスレッド開始
		if(!m_ReadThread.StartThread(this, &CFileReader::ReadThread)){
			m_OutputThread.EndThread(true);
			}
		}

	return true;
}

const bool CFileReader::OnStop(void)
{
	// 読み込みスレッド終了
	m_ReadThread.EndThread(true);

	// 出力スレッド終了
	m_OutputThread.EndThread(true);

	return true;
}

void CFileReader::ReadThread(CSmartThread<CFileReader> *pThread, bool &bKillSignal, PVOID pParam)
{
	DWORD dwIdleNum;

	// 開始イベント通知
	SendDecoderEvent(IFileReader::EID_READ_START);

	while(!bKillSignal){
		// キューに空きがある限りファイルを読み込む
		while(!bKillSignal){
			// アイドルキュー数取得
			m_pLock->Lock();
			dwIdleNum = m_MediaPool.size() - m_MediaQue.size();
			m_pLock->Unlock();
			if(!dwIdleNum)break;

			// 読み込み前イベント通知
			SendDecoderEvent(IFileReader::EID_PRE_READ);

			// ファイル読み込み
			if(!ReadAsync(m_dwBuffSize)){

				// ファイル終端、出力キューのデータが全て処理されるまで待つ
				while(m_OutputThread.IsRunning()){
					// 処理中キュー数取得
					m_pLock->Lock();
					dwIdleNum = m_MediaQue.size();
					m_pLock->Unlock();
					if(!dwIdleNum)break;

					// スレッドサスペンド、キューに空きが出来るまで待つ
					pThread->SuspendThread();
					}

				bKillSignal = true;
				break;
				}

			// 読み込み後イベント通知
			SendDecoderEvent(IFileReader::EID_POST_READ);

			// キューに追加
			m_pLock->Lock();
			m_MediaQue.push(*m_itFreeMedia);
			if(++m_itFreeMedia == m_MediaPool.end())m_itFreeMedia = m_MediaPool.begin();
			m_pLock->Unlock();

			// 出力スレッドレジューム
			m_OutputThread.ResumeThread();
			}

		// スレッドサスペンド、キューに空きが出来るまで待つ
		if(!bKillSignal)pThread->SuspendThread();
		}

	// 終了イベント通知
	SendDecoderEvent(IFileReader::EID_READ_END);
}

void CFileReader::OutputThread(CSmartThread<CFileReader> *pThread, bool &bKillSignal, PVOID pParam)
{
	IMediaData *pMediaData;

	while(!bKillSignal){
		while(!bKillSignal){
			// 出力待ちキュー数取得
			m_pLock->Lock();
			pMediaData = (!m_MediaQue.empty())? m_MediaQue.front() : NULL;
			m_pLock->Unlock();
			if(!pMediaData)break;

			// メディアデータ出力
			OutputMedia(pMediaData);

			// キュー取り出し
			m_pLock->Lock();
			m_MediaQue.pop();
			m_pLock->Unlock();

			// 読み込みスレッドレジューム
			m_ReadThread.ResumeThread();
			}

		// スレッドサスペンド、キューにデータが来るまで待つ
		if(!bKillSignal)pThread->SuspendThread();
		}
}

const DWORD CFileReader::ReadAsync(const DWORD dwReadSize)
{
	if(!m_pFile)return 0UL;

	// 要求サイズ確保
	if((*m_itFreeMedia)->SetSize(dwReadSize) != dwReadSize)return 0UL;

	// データ読み込み
	const DWORD dwActualSize = m_pFile->ReadData((*m_itFreeMedia)->GetData(), m_dwBuffSize);
	if(!dwActualSize)return 0UL;

	// 有効サイズ設定
	if((*m_itFreeMedia)->SetSize(dwActualSize) != dwActualSize)return 0UL;

	return dwActualSize;
}
