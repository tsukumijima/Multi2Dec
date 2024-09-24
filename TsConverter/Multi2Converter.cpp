// Multi2Converter.cpp: Multi2復号コンバータクラス
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "Multi2Converter.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// Multi2復号コンバータクラス
/////////////////////////////////////////////////////////////////////////////

const DWORD CMulti2Converter::StartConvert(LPCTSTR lpszSrcFile, LPCTSTR lpszDstFile, const bool bB25, const bool bEMM, const bool bNULL, const bool bDiscard)
{
	if(!lpszSrcFile)return EC_FILE_ERROR;
	if(!m_pFileReader || !m_pTsPacketSync || !m_pTsDescrambler || !m_pFileWriter)return EC_CLASS_ERROR;

	// 全デコーダを閉じる
	CloseDecoder();

	// デコーダグラフを構築する
	IMediaDecoder * const pFileReaderDecoder	= dynamic_cast<IMediaDecoder *>(m_pFileReader    );
	IMediaDecoder * const pTsPacketSyncDecoder	= dynamic_cast<IMediaDecoder *>(m_pTsPacketSync  );
	IMediaDecoder * const pTsDescramblerDecoder	= dynamic_cast<IMediaDecoder *>(m_pTsDescrambler );
	IMediaDecoder * const pFileWriterDecoder	= dynamic_cast<IMediaDecoder *>(m_pFileWriter    );

	pFileReaderDecoder->SetOutputDecoder(pTsPacketSyncDecoder);
	pTsPacketSyncDecoder->SetOutputDecoder(pTsDescramblerDecoder);
	pTsDescramblerDecoder->SetOutputDecoder(pFileWriterDecoder);

	// デコーダを開く
	try{
		// 入力ファイルオープン
		if(!m_pFileReader->OpenFile(lpszSrcFile, true))throw (DWORD)EC_FILE_ERROR;

		// パケット同期デコーダ設定
		m_pTsPacketSync->DiscardNullPacket(bNULL);

		// デスクランブルデコーダオープン
		if(bB25){
			if(!m_pTsDescrambler->OpenDescrambler(TEXT("CBcasCardReader")))throw (DWORD)EC_BCAS_ERROR;
			}

		// デスクランブルデコーダ設定
		m_pTsDescrambler->EnableBuffering(true);
		m_pTsDescrambler->EnableEmmProcess(bEMM);
		m_pTsDescrambler->DiscardScramblePacket(bDiscard);

		// 出力ファイルオープン
		if(lpszDstFile){
			if(!m_pFileWriter->OpenFile(lpszDstFile), false)throw (DWORD)EC_FILE_ERROR;
			}

		// コンバート開始
		if(!pFileReaderDecoder->PlayDecoder())throw (DWORD)EC_INTERNAL_ERROR;
		}
	catch(DWORD dwErrorCode){
		// エラー発生
		CloseDecoder();
		return dwErrorCode;
		}

	return EC_NO_ERROR;
}

void CMulti2Converter::EndConvert(void)
{
	// デコーダグラフを停止する
	if(m_pFileReader){
		dynamic_cast<IMediaDecoder *>(m_pFileReader)->StopDecoder();
		}

	// デコーダグラフを閉じる
	CloseDecoder();
}

const DWORD CMulti2Converter::GetPacketStride(void) const
{
	// TSパケットサイズを返す
	return (m_pTsPacketSync)? m_pTsPacketSync->GetPacketStride() : 0UL;
}

const DWORD CMulti2Converter::GetInputPacketNum(const WORD wPID) const
{
	// 入力TSパケット数を返す
	return (m_pTsPacketSync)? m_pTsPacketSync->GetInputPacketNum(wPID) : 0UL;
}

const DWORD CMulti2Converter::GetOutputPacketNum(const WORD wPID) const
{
	// 出力TSパケット数を返す
	return (m_pTsDescrambler)? m_pTsDescrambler->GetOutputPacketNum(wPID) : 0UL;
}

const DWORD CMulti2Converter::GetSyncErrNum(void) const
{
	// 同期エラー数を返す
	return (m_pTsPacketSync)? m_pTsPacketSync->GetSyncErrNum() : 0UL;
}

const DWORD CMulti2Converter::GetFormatErrNum(void) const
{
	// フォーマットエラー数を返す
	return (m_pTsPacketSync)? m_pTsPacketSync->GetFormatErrNum() : 0UL;
}

const DWORD CMulti2Converter::GetTransportErrNum(void) const
{
	// トランスポートエラー数を返す
	return (m_pTsPacketSync)? m_pTsPacketSync->GetTransportErrNum() : 0UL;
}

const DWORD CMulti2Converter::GetContinuityErrNum(const WORD wPID) const
{
	// 連続性エラー数を返す
	return (m_pTsPacketSync)? m_pTsPacketSync->GetContinuityErrNum(wPID) : 0UL;
}

const DWORD CMulti2Converter::GetScramblePacketNum(const WORD wPID) const
{
	// 復号漏れパケット数を返す
	return (m_pTsDescrambler)? m_pTsDescrambler->GetScramblePacketNum(wPID) : 0UL;
}

const DWORD CMulti2Converter::GetEcmProcessNum(void) const
{
	// ECM処理数を返す
	return (m_pTsDescrambler)? m_pTsDescrambler->GetEcmProcessNum() : 0UL;
}

const DWORD CMulti2Converter::GetEmmProcessNum(void) const
{
	// EMM処理数を返す
	return (m_pTsDescrambler)? m_pTsDescrambler->GetEmmProcessNum() : 0UL;
}

const ULONGLONG CMulti2Converter::GetSrcFileLength(void) const
{
	// 入力ファイル長を返す
	return (m_pFileReader)? m_pFileReader->GetFileLength() : 0ULL;
}

const ULONGLONG CMulti2Converter::GetDstFileLength(void) const
{
	// 出力ファイル長を返す
	return (m_pFileWriter)? m_pFileWriter->GetFileLength() : 0ULL;
}

CMulti2Converter::CMulti2Converter(IBonObject * const pOwner)
	: CBonObject(pOwner)
	, m_pFileReader(::BON_SAFE_CREATE<IFileReader *>(TEXT("CFileReader"), dynamic_cast<IMediaDecoderHandler *>(this)))
	, m_pTsPacketSync(::BON_SAFE_CREATE<ITsPacketSync *>(TEXT("CTsPacketSync"), dynamic_cast<IMediaDecoderHandler *>(this)))
	, m_pTsDescrambler(::BON_SAFE_CREATE<ITsDescrambler *>(TEXT("CTsDescrambler"), dynamic_cast<IMediaDecoderHandler *>(this)))
	, m_pFileWriter(::BON_SAFE_CREATE<IFileWriter *>(TEXT("CFileWriter"), dynamic_cast<IMediaDecoderHandler *>(this)))
	, m_pHandler(dynamic_cast<IMulti2Converter::IHandler *>(pOwner))
{
	// 何もしない
}

CMulti2Converter::~CMulti2Converter(void)
{
	// クラスインスタンスを解放
	BON_SAFE_RELEASE(m_pFileReader);
	BON_SAFE_RELEASE(m_pTsPacketSync);
	BON_SAFE_RELEASE(m_pTsDescrambler);
	BON_SAFE_RELEASE(m_pFileWriter);
}

const DWORD CMulti2Converter::OnDecoderEvent(IMediaDecoder *pDecoder, const DWORD dwEventID, PVOID pParam)
{
	if(pDecoder == dynamic_cast<IMediaDecoder *>(m_pFileReader)){
		switch(dwEventID){
			case IFileReader::EID_READ_START :
				// コンバート開始
				SendConverterEvent(EID_CONV_START, reinterpret_cast<PVOID>(0UL));
				return 0UL;

			case IFileReader::EID_READ_END :
				// コンバート終了
				SendConverterEvent(EID_CONV_END, reinterpret_cast<PVOID>((DWORD)(m_pFileReader->GetReadPos() * 1000ULL / m_pFileReader->GetFileLength())));
				return 0UL;

			case IFileReader::EID_POST_READ :
				// コンバート進捗
				SendConverterEvent(EID_CONV_PROGRESS, reinterpret_cast<PVOID>((DWORD)(m_pFileReader->GetReadPos() * 1000ULL / m_pFileReader->GetFileLength())));
				return 0UL;
			}
		}

	return 0UL;
}

void CMulti2Converter::CloseDecoder(void)
{
	// 全てのデコーダを閉じる
	if(m_pFileReader)m_pFileReader->CloseFile();
	if(m_pTsPacketSync)m_pTsPacketSync->ResetStatistics();
	if(m_pTsDescrambler)m_pTsDescrambler->CloseDescrambler();
	if(m_pFileWriter)m_pFileWriter->CloseFile();
}

void CMulti2Converter::SendConverterEvent(const DWORD dwEventID, PVOID pParam)
{
	// コンバートイベント送信
	if(m_pHandler)m_pHandler->OnMulti2ConverterEvent(this, dwEventID, pParam);
}
