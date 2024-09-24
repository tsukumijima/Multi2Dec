// FileWriter.cpp: ファイルライタデコーダ
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "FileWriter.h"


/////////////////////////////////////////////////////////////////////////////
// ファイルローカル定数設定
/////////////////////////////////////////////////////////////////////////////

// デフォルトのバッファサイズ
#define DEF_BUFFSIZE	0x100000UL							// 1MB
#define DEF_BUFFNUM		(DEF_BUFFSIZE / TS_PACKET_SIZE)		// 1MB 相当


/////////////////////////////////////////////////////////////////////////////
// ファイルライタデコーダ
/////////////////////////////////////////////////////////////////////////////

const bool CFileWriter::InputMedia(IMediaData *pMediaData, const DWORD dwInputIndex)
{
	if(!pMediaData || (dwInputIndex >= GetInputNum()) || !m_pFile)return false;

	// ファイル書き込み
	return (m_WriteThread.IsRunning())? WriteAsync(pMediaData) : WriteSync(pMediaData);
}

const bool CFileWriter::OpenFile(LPCTSTR lpszFileName, const bool bAsyncWrite, const DWORD dwBuffSize)
{
	CloseFile();

	try{
		CBlockLock AutoLock(m_pLock);

		// バッファサイズ計算
		DWORD dwWriteSize;
		DWORD dwWriteNum;
		
		if(!bAsyncWrite){
			// 同期書き込み
			dwWriteSize = (dwBuffSize)? dwBuffSize : DEF_BUFFSIZE;
			dwWriteNum	= 0UL;
			}
		else{
			// 非同期書き込み
			dwWriteSize = DEF_BUFFSIZE;
			dwWriteNum = (dwBuffSize)? dwBuffSize : DEF_BUFFNUM;
			}
	
		// スマートファイルインスタンス生成
		if(!(m_pFile = ::BON_SAFE_CREATE<ISmartFile *>(TEXT("CSmartFile"))))throw __LINE__;

		// ファイルオープン
		if(!m_pFile->Open(lpszFileName, ISmartFile::MF_WRITE, dwWriteSize))throw __LINE__;

		// メディアプール確保
		if(dwWriteNum){
			// メディアプールサイズリサイズ
			m_MediaPool.resize(dwWriteNum);
			if(m_MediaPool.size() < dwWriteNum)throw __LINE__;
			::ZeroMemory(&m_MediaPool[0], sizeof(IMediaData *) * dwWriteNum);
			
			// メディアデータインスタンス確保
			for(DWORD dwIndex = 0UL ; dwIndex < dwWriteNum ; dwIndex++){
				m_MediaPool[dwIndex] = ::BON_SAFE_CREATE<IMediaData *>(TEXT("CMediaData"));
				if(!m_MediaPool[dwIndex])throw __LINE__;
				}
			
			// メディアプール先頭イテレータセット
			m_itFreeMedia = m_MediaPool.begin();
			}
		}
	catch(...){
		// エラー発生
		CloseFile();
		return false;
		}
		
	return true;
}

bool CFileWriter::CloseFile(void)
{
	CBlockLock AutoLock(m_pLock);

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

const ULONGLONG CFileWriter::GetFileLength(void)
{
	// ファイルサイズを返す
	return (m_pFile)? m_pFile->GetLength() : 0ULL;
}

CFileWriter::CFileWriter(IBonObject *pOwner)
	: CMediaDecoder(pOwner, 1UL, 0UL, new CSmartLock)
	, m_pFile(NULL)
{
	// 何もしない
}

CFileWriter::~CFileWriter(void)
{
	// ファイルを閉じる
	CloseFile();
}

const bool CFileWriter::OnPlay(void)
{
	OnStop();

	// メディアキュークリア
	while(!m_MediaQue.empty())m_MediaQue.pop();
	
	// メディアプール先頭イテレータセット
	m_itFreeMedia = m_MediaPool.begin();

	// スレッド開始(サスペンド状態で開始)
	return (m_MediaPool.size())? m_WriteThread.StartThread(this, &CFileWriter::WriteThread, NULL, false) : true;
}

const bool CFileWriter::OnStop(void)
{
	// スレッド終了
	m_WriteThread.EndThread(true);

	return true;
}

const bool CFileWriter::WriteSync(const IMediaData *pMediaData)
{
	// データ取得
	const BYTE * const pData = pMediaData->GetData();
	const DWORD dwSize = pMediaData->GetSize();
	if(!pData || !dwSize)return true;

	// ファイル書き込み
	if(m_pFile->WriteData(pData, dwSize) != dwSize){
		CloseFile();
		return false;
		}
	
	return true;
}

const bool CFileWriter::WriteAsync(const IMediaData *pMediaData)
{
	DWORD dwIdleNum;

	// データチェック
	const BYTE * const pData = pMediaData->GetData();
	const DWORD dwSize = pMediaData->GetSize();
	if(!pData || !dwSize)return true;

	// アイドルキュー数取得
	m_pLock->Lock();
	dwIdleNum = m_MediaPool.size() - m_MediaQue.size();
	m_pLock->Unlock();	

	// キューがフルの場合の処理
	if(!dwIdleNum){
		// イベント通知
		if(SendDecoderEvent(EID_BUFF_OVERFLOW)){
			// キューが空くまでブロックする
			do{
				::Sleep(1UL);

				// ポーリング
				m_pLock->Lock();
				dwIdleNum = m_MediaPool.size() - m_MediaQue.size();
				m_pLock->Unlock();	
				}
			while(!dwIdleNum);			
			}
		else{
			// デフォルトの処理、古いデータを捨てる
			m_pLock->Lock();
			m_MediaQue.pop();		
			m_pLock->Unlock();
			}		
		}

	// データコピー
	if((*m_itFreeMedia)->CopyData(pMediaData) != dwSize)return false;

	// メディアプールに空きがあるとき、キューに投入
	m_pLock->Lock();
	m_MediaQue.push(*m_itFreeMedia);
	if(++m_itFreeMedia == m_MediaPool.end())m_itFreeMedia = m_MediaPool.begin();
	m_pLock->Unlock();	

	// 書き込みキューレジューム
	m_WriteThread.ResumeThread();

	return true;
}

void CFileWriter::WriteThread(CSmartThread<CFileWriter> *pThread, bool &bKillSignal, PVOID pParam)
{
	const IMediaData *pMediaData;

	while(!bKillSignal){
		// ファイル書き込み
		while(1){
			// キューチェック
			m_pLock->Lock();
			pMediaData = (!m_MediaQue.empty())? m_MediaQue.front() : NULL;
			m_pLock->Unlock();
			if(!pMediaData)break;

			// ファイル書き込み
			WriteSync(pMediaData);

			// キュー取り出し
			m_pLock->Lock();
			m_MediaQue.pop();
			m_pLock->Unlock();	
			}

		// キューにデータが到着するまでサスペンド
		if(!bKillSignal)pThread->SuspendThread();
		}
}
