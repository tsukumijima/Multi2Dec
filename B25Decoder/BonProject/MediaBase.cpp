// MediaBase.cpp: Bonメディア基底クラス
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BonSDK.h"
#include "MediaBase.h"


/////////////////////////////////////////////////////////////////////////////
// IMediaData基底クラス
/////////////////////////////////////////////////////////////////////////////

#define	MINBUFSIZE	256UL		// 最小バッファサイズ
#define MINADDSIZE	256UL		// 最小追加確保サイズ


BYTE * CMediaData::GetData(void) const
{
	// バッファポインタを取得する
	return (m_dwDataSize)? m_pData : NULL;
}

const DWORD CMediaData::GetSize(void) const
{
	// データサイズを取得する
	return m_dwDataSize;
}

void CMediaData::SetAt(const DWORD dwPos, const BYTE byData)
{
	// 1バイトセットする
	if(dwPos < m_dwDataSize)m_pData[dwPos] = byData;
}

const BYTE CMediaData::GetAt(const DWORD dwPos) const
{
	// 1バイト取得する
	return (dwPos < m_dwDataSize)? m_pData[dwPos] : 0x00U;
}

const DWORD CMediaData::SetData(const BYTE *pData, const DWORD dwDataSize)
{
	if(dwDataSize){
		// バッファ確保
		GetBuffer(dwDataSize);

		// データセット
		::CopyMemory(m_pData, pData, dwDataSize);
		}

	// サイズセット
	m_dwDataSize = dwDataSize;

	return m_dwDataSize;
}

const DWORD CMediaData::AddData(const BYTE *pData, const DWORD dwDataSize)
{
	if(!dwDataSize)return m_dwDataSize;

	// バッファ確保
	GetBuffer(m_dwDataSize + dwDataSize);

	// データ追加
	::CopyMemory(&m_pData[m_dwDataSize], pData, dwDataSize);

	// サイズセット
	m_dwDataSize += dwDataSize;

	return m_dwDataSize;
}

const DWORD CMediaData::AddData(const IMediaData *pData)
{
	return AddData(pData->GetData(), pData->GetSize());
}

const DWORD CMediaData::AddByte(const BYTE byData)
{
	// バッファ確保
	GetBuffer(m_dwDataSize + 1UL);

	// データ追加
	m_pData[m_dwDataSize] = byData;

	// サイズ更新
	m_dwDataSize++;

	return m_dwDataSize;
}

const DWORD CMediaData::TrimHead(const DWORD dwTrimSize)
{
	// データ先頭を切り詰める
	if(!m_dwDataSize || !dwTrimSize){
		// 何もしない
		}
	else if(dwTrimSize >= m_dwDataSize){
		// 全体を切り詰める
		m_dwDataSize = 0UL;
		}
	else{
		// データを移動する
		::MoveMemory(m_pData, m_pData + dwTrimSize, m_dwDataSize - dwTrimSize);
		m_dwDataSize -= dwTrimSize;
		}

	return m_dwDataSize;
}

const DWORD CMediaData::TrimTail(const DWORD dwTrimSize)
{
	// データ末尾を切り詰める
	if(!m_dwDataSize || !dwTrimSize){
		// 何もしない
		}
	else if(dwTrimSize >= m_dwDataSize){
		// 全体を切り詰める
		m_dwDataSize = 0UL;
		}
	else{
		// データ末尾を切り詰める
		m_dwDataSize -= dwTrimSize;
		}

	return m_dwDataSize;
}

const DWORD CMediaData::CopyData(const IMediaData *pSrc)
{
	if(!pSrc)return 0UL;

	// データのコピー
	return SetData(pSrc->GetData(), pSrc->GetSize());
}

const DWORD CMediaData::GetBuffer(const DWORD dwGetSize)
{
	if(dwGetSize <= m_dwBuffSize)return m_dwBuffSize;

	// 少なくとも指定サイズを格納できるバッファを確保する
	if(!m_pData){
		// バッファ確保まだ
		m_dwBuffSize = (dwGetSize > MINBUFSIZE)? dwGetSize : MINBUFSIZE;
		m_pData = new BYTE [m_dwBuffSize];
		}
	else if(dwGetSize > m_dwBuffSize){
		// 要求サイズはバッファサイズを超える
		m_dwBuffSize = (dwGetSize > MINBUFSIZE)? dwGetSize : MINBUFSIZE;
		if(m_dwBuffSize < (m_dwDataSize * 2UL))m_dwBuffSize = m_dwDataSize * 2UL;

		BYTE *pNewBuffer = new BYTE [m_dwBuffSize];

		// データコピー
		if(m_dwDataSize){
			::CopyMemory(pNewBuffer, m_pData, m_dwDataSize);
			}

		// 旧バッファ開放
		delete [] m_pData;

		// バッファ差し替え
		m_pData = pNewBuffer;
		}

	return m_dwBuffSize;
}

const DWORD CMediaData::SetSize(const DWORD dwSetSize)
{
	if(dwSetSize){
		// バッファ確保
		GetBuffer(dwSetSize);
		}

	// サイズセット
	m_dwDataSize = dwSetSize;

	return m_dwDataSize;
}

const DWORD CMediaData::SetSize(const DWORD dwSetSize, const BYTE byFiller)
{
	// サイズセット
	SetSize(dwSetSize);

	// データセット
	if(dwSetSize){
		::FillMemory(m_pData, dwSetSize, byFiller);
		}

	return m_dwDataSize;
}

void CMediaData::ClearSize(void)
{
	// データサイズをクリアする
	m_dwDataSize = 0UL;
}

CMediaData::CMediaData(IBonObject *pOwner)
	: CBonObject()
	, m_dwDataSize(0UL)
	, m_dwBuffSize(0UL)
	, m_pData(NULL)
{
	// 空のバッファを生成する
}

CMediaData::CMediaData(const CMediaData &Operand)
	: CBonObject()
	, m_dwDataSize(0UL)
	, m_dwBuffSize(0UL)
	, m_pData(NULL)
{
	// コピーコンストラクタ
	*this = Operand;
}

CMediaData::CMediaData(const DWORD dwBuffSize)
	: CBonObject()
	, m_dwDataSize(0UL)
	, m_dwBuffSize(0UL)
	, m_pData(NULL)
{
	// バッファサイズを指定してバッファを生成する
	GetBuffer(dwBuffSize);
}

CMediaData::CMediaData(const BYTE *pData, const DWORD dwDataSize)
	: CBonObject()
	, m_dwDataSize(0UL)
	, m_dwBuffSize(0UL)
	, m_pData(NULL)
{
	// データ初期値を指定してバッファを生成する
	SetData(pData, dwDataSize);
}

CMediaData::CMediaData(const BYTE byFiller, const DWORD dwDataSize)
	: CBonObject()
	, m_dwDataSize(0UL)
	, m_dwBuffSize(0UL)
	, m_pData(NULL)
{
	// フィルデータを指定してバッファを生成する
	SetSize(dwDataSize, byFiller);
}

CMediaData::~CMediaData(void)
{
	// バッファを開放する
	if(m_pData)delete [] m_pData;
}

CMediaData & CMediaData::operator = (const CMediaData &Operand)
{
	// バッファサイズの情報まではコピーしない
	SetData(Operand.GetData(), Operand.GetSize());

	return *this;
}


/////////////////////////////////////////////////////////////////////////////
// IMediaDecoder基底クラス
/////////////////////////////////////////////////////////////////////////////

const DWORD CMediaDecoder::GetInputNum(void) const
{
	// 入力数を返す
	return m_dwInputNum;
}

const DWORD CMediaDecoder::GetOutputNum(void) const
{
	// 出力数を返す
	return m_dwOutputNum;
}

const bool CMediaDecoder::PlayDecoder(void)
{
	DWORD dwOutputIndex;

	// 最初に下位デコーダを再生する(順序は重要)
	for(dwOutputIndex = 0UL ; dwOutputIndex < GetOutputNum() ; dwOutputIndex++){
		if(m_aOutputDecoder[dwOutputIndex].pDecoder){
			if(!m_aOutputDecoder[dwOutputIndex].pDecoder->PlayDecoder())break;
			}
		}

	// エラー発生時は再生済みデコーダを停止する
	if(dwOutputIndex < GetOutputNum()){
		while(dwOutputIndex--){
			if(m_aOutputDecoder[dwOutputIndex].pDecoder){
				m_aOutputDecoder[dwOutputIndex].pDecoder->StopDecoder();
				}
			}
		return false;
		}

	// 最後に自デコーダを再生する
	return OnPlay();
}

const bool CMediaDecoder::StopDecoder(void)
{
	bool bReturn = true;

	// 最初に自デコーダを停止する
	if(!OnStop()){
		bReturn = false;
		}

	// 次に下位デコーダを停止する(順序は重要)
	for(DWORD dwOutputIndex = 0UL ; dwOutputIndex < GetOutputNum() ; dwOutputIndex++){
		if(m_aOutputDecoder[dwOutputIndex].pDecoder){
			if(!m_aOutputDecoder[dwOutputIndex].pDecoder->StopDecoder()){
				bReturn = false;
				}
			}
		}

	return bReturn;
}

const bool CMediaDecoder::ResetDecoder(void)
{
	bool bReturn = true;

	// デコーダグラフをロック
	if(m_pLock)m_pLock->Lock();

	// 最初に下位デコーダをリセットする(順序は重要)
	for(DWORD dwOutputIndex = 0UL ; dwOutputIndex < GetOutputNum() ; dwOutputIndex++){
		if(m_aOutputDecoder[dwOutputIndex].pDecoder){
			if(!m_aOutputDecoder[dwOutputIndex].pDecoder->ResetDecoder()){
				bReturn = false;
				}
			}
		}

	// 最後に自デコーダをリセットする
	if(!OnReset()){
		bReturn = false;
		}

	// デコーダグラフをアンロック
	if(m_pLock)m_pLock->Unlock();

	return bReturn;
}

const bool CMediaDecoder::SetOutputDecoder(IMediaDecoder *pDecoder, const DWORD dwOutputIndex, const DWORD dwInputIndex)
{
	// 出力デコーダをセットする
	if(dwOutputIndex < GetOutputNum()){
		m_aOutputDecoder[dwOutputIndex].pDecoder = pDecoder;
		m_aOutputDecoder[dwOutputIndex].dwInputIndex = dwInputIndex;
		return true;
		}

	return false;
}

const bool CMediaDecoder::InputMedia(IMediaData *pMediaData, const DWORD dwInputIndex)
{
	// 常にエラーを返す
	return false;
}

CMediaDecoder::CMediaDecoder(IBonObject *pOwner, const DWORD dwInputNum, const DWORD dwOutputNum, CSmartLock *pLock)
	: CBonObject(pOwner)
	, m_pEventHandler(dynamic_cast<IMediaDecoderHandler *>(pOwner))
	, m_dwInputNum(dwInputNum)
	, m_dwOutputNum(dwOutputNum)
	, m_pLock(pLock)
{
	// 出力デコーダ配列をクリアする
	::ZeroMemory(m_aOutputDecoder, sizeof(m_aOutputDecoder));
}

CMediaDecoder::~CMediaDecoder(void)
{
	// クリティカルセクション開放
	if(m_pLock)delete m_pLock;
}

const bool CMediaDecoder::OnPlay(void)
{
	// デフォルトの実装では何もしない
	return true;
}

const bool CMediaDecoder::OnStop(void)
{
	// デフォルトの実装では何もしない
	return true;
}

const bool CMediaDecoder::OnReset(void)
{
	// デフォルトの実装では何もしない
	return true;
}

const bool CMediaDecoder::OutputMedia(IMediaData *pMediaData, const DWORD dwOutptIndex)
{
	// 出力デコーダにデータを渡す
	if(dwOutptIndex < GetOutputNum()){
		if(m_aOutputDecoder[dwOutptIndex].pDecoder){
			return m_aOutputDecoder[dwOutptIndex].pDecoder->InputMedia(pMediaData, m_aOutputDecoder[dwOutptIndex].dwInputIndex);
			}
		}

	return false;
}

const DWORD CMediaDecoder::SendDecoderEvent(const DWORD dwEventID, PVOID pParam)
{
	// イベントを通知する
	return (m_pEventHandler)? m_pEventHandler->OnDecoderEvent(this, dwEventID, pParam) : 0UL;
}
