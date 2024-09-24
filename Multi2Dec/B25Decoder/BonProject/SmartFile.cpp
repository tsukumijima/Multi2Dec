// SmartFile.cpp: 汎用ファイルストレージクラス
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "SmartFile.h"
#include <StdIo.h>
#include <Share.h >


/////////////////////////////////////////////////////////////////////////////
// 汎用ファイルストレージクラス
/////////////////////////////////////////////////////////////////////////////

// デフォルトのバッファサイズ
#define DEF_BUFFSIZE	0x100000UL	// 1MB


const bool CSmartFile::Open(LPCTSTR lpszFileName, const DWORD dwModeFlags, const DWORD dwBuffSize)
{
	Close();
	
	TCHAR szOpenMode[16] = {TEXT('\0')};
	int iShareFlags = 0;

	// モードのチェック(読み書き両方には対応していない)
	switch(dwModeFlags){
		case MF_READ :
			::_tcscpy(szOpenMode, TEXT("rb"));
			iShareFlags = _SH_DENYNO;
			break;
			
		case MF_WRITE :
			::_tcscpy(szOpenMode, TEXT("wb"));
			iShareFlags = _SH_DENYWR;		
			break;
		
		default :
			return false;
		}

	// ファイルストリームオープン
	if(!(m_pFileStream = ::_tfsopen(lpszFileName, szOpenMode, iShareFlags)))return false;
	
	// バッファサイズ設定
	if(::setvbuf(m_pFileStream, NULL, _IOFBF, (dwBuffSize)? dwBuffSize : DEF_BUFFSIZE)){
		Close();
		return false;
		}

	return true;
}

void CSmartFile::Close(void)
{
	// データフラッシュ
	FlushData();
	
	// ファイルクローズ
	if(m_pFileStream){
		::fclose(m_pFileStream);
		m_pFileStream = NULL;
		}
}

const DWORD CSmartFile::ReadData(BYTE *pBuff, const DWORD dwLen)
{
	if(!m_pFileStream || !pBuff || !dwLen)return 0UL;
	
	// ファイル読み取り
	const DWORD dwReturn = ::_fread_nolock(pBuff, 1UL, dwLen, m_pFileStream);
	
	// エラーorファイル終端チェック
	if(dwReturn != dwLen){
		if(::ferror(m_pFileStream)){
			::clearerr(m_pFileStream);
			return 0UL;
			}
		}

	// 読み取ったバイト数を返す
	return dwReturn;
}

const DWORD CSmartFile::ReadData(BYTE *pBuff, const DWORD dwLen, const ULONGLONG llPos)
{
	// ファイルポジションシーク
	if(!SeekPos(llPos))return 0UL;

	// ファイル読み取り
	return ReadData(pBuff, dwLen);
}

const DWORD CSmartFile::WriteData(const BYTE *pBuff, const DWORD dwLen)
{
	if(!m_pFileStream || !pBuff || !dwLen)return 0UL;

	// ファイル書き込み
	const DWORD dwReturn = ::_fwrite_nolock(pBuff, 1UL, dwLen, m_pFileStream);

	// エラーチェック
	if(dwReturn != dwLen){
		::clearerr(m_pFileStream);
		return 0UL;
		}

	// 書き込んだバイト数を返す
	return dwReturn;
}

const DWORD CSmartFile::WriteData(const BYTE *pBuff, const DWORD dwLen, const ULONGLONG llPos)
{
	// ファイルポジションシーク
	if(!SeekPos(llPos))return 0UL;

	// ファイル書き込み
	return WriteData(pBuff, dwLen);
}

const bool CSmartFile::FlushData(void)
{
	if(!m_pFileStream)return false;

	// ファイルフラッシュ
	return (!::fflush(m_pFileStream))? true : false;
}

const bool CSmartFile::SeekPos(const ULONGLONG llPos)
{
	if(!m_pFileStream)return false;

	// ファイルポジションシーク
	return (!::_fseeki64(m_pFileStream, llPos, SEEK_SET))? true : false;
}

const ULONGLONG CSmartFile::GetPos(void)
{
	if(!m_pFileStream)return 0ULL;

	// ファイルポジション取得
	const __int64 iReturn = ::_ftelli64(m_pFileStream);

	return (iReturn != -1LL)? (ULONGLONG)iReturn : 0ULL;
}

const ULONGLONG CSmartFile::GetLength(void)
{
	if(!m_pFileStream)return 0ULL;
	
	// 現在のファイルポジション取得
	const __int64 iCurPos = ::_ftelli64(m_pFileStream);
	if(iCurPos == -1LL)return 0ULL;

	// ファイルポジションを終端に移動
	if(::_fseeki64(m_pFileStream, 0LL, SEEK_END))return 0ULL;
	
	// 終端のファイルポジション取得
	const __int64 iEndPos = ::_ftelli64(m_pFileStream);

	// ファイルポジションを元に戻す
	if(::_fseeki64(m_pFileStream, iCurPos, SEEK_SET))return 0ULL;

	// ファイルサイズを返す
	return (iEndPos != -1LL)? (ULONGLONG)iEndPos : 0ULL;
}

CSmartFile::CSmartFile(IBonObject *pOwner)
	: CBonObject(pOwner)
	, m_pFileStream(NULL)
{
	// 何もしない
}

CSmartFile::~CSmartFile(void)
{
	// ファイルを閉じる
	Close();
}
