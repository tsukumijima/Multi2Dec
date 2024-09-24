// Multi2DecDosApp.cpp: Multi2DecDosアプリケーションクラス
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "Multi2DecDosApp.h"
#include <Share.h >


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// Multi2DecDosアプリケーションクラス
/////////////////////////////////////////////////////////////////////////////

CMulti2DecDosApp::CMulti2DecDosApp(IBonObject * const pOwner)
	: CBonObject(pOwner)
	, m_pMulti2Converter(NULL)
{
	// インスタンス生成
	m_pMulti2Converter = ::BON_SAFE_CREATE<IMulti2Converter *>(TEXT("CMulti2Converter"), dynamic_cast<IMulti2Converter::IHandler *>(this));
}

CMulti2DecDosApp::~CMulti2DecDosApp(void)
{
	// インスタンス開放
	BON_SAFE_RELEASE(m_pMulti2Converter);
}

const bool CMulti2DecDosApp::Run(const CONFIGSET *pConfigSet)
{
	// 設定を保存
	m_ConfigSet = *pConfigSet;

	if(!m_pMulti2Converter){
		// インスタンス生成失敗
		::printf("エラー: bon_TsConverter.dllをロードできません。\n");
		return false;
		}

	// コンバート開始
	const DWORD dwErrorCode = m_pMulti2Converter->StartConvert(pConfigSet->szInputPath, (pConfigSet->bOutput)? pConfigSet->szOutputPath : NULL, pConfigSet->bB25, pConfigSet->bEMM, pConfigSet->bNULL, pConfigSet->bPURE);

	switch(dwErrorCode){
		case IMulti2Converter::EC_FILE_ERROR :
			::printf("エラー: ファイルをオープンできません。\n");
			return false;

		case IMulti2Converter::EC_BCAS_ERROR :
			::printf("エラー: B-CASカードをオープンできません。\n");
			return false;

		case IMulti2Converter::EC_CLASS_ERROR :
			::printf("エラー: 動作に必要なモジュールが見つかりません。\n");
			return false;

		case IMulti2Converter::EC_INTERNAL_ERROR :
			::printf("エラー: 内部エラー。\n");
			return false;
		}

	// 完了イベントを待つ
	m_ConvertEvent.WaitEvent();

	// コンバート終了
	m_pMulti2Converter->EndConvert();

	return true;
}

void CMulti2DecDosApp::OnMulti2ConverterEvent(IMulti2Converter *pMulti2Converter, const DWORD dwEventID, PVOID pParam)
{
	switch(dwEventID){
		case IMulti2Converter::EID_CONV_START:
			// コンバート開始
			m_dwLastProgress = 0UL;

			::printf("Now Processing...\n\n");
			::printf("0%%                                50%%                                100%%\n");
			::printf("|-----+------+------+------+------|------+------+------+------+------|\n");
			break;
		
		case IMulti2Converter::EID_CONV_END:
			// ログ出力
			::printf("\n\n");
			PrintLogFile(stdout);

			// ログファイル出力
			if(!m_ConfigSet.bNoLog){					
				OutputLogFile();
				}
			
			// 完了イベントセット
			m_ConvertEvent.SetEvent();
			break;

		case IMulti2Converter::EID_CONV_PROGRESS:
			// 進捗状況更新
			UpdateProgress((DWORD)pParam);
			break;

		case IMulti2Converter::EID_CONV_ERROR:
			// エラー発生
			::printf("\n\nエラー: 内部エラー。\n");

			// 完了イベントセット
			m_ConvertEvent.SetEvent();
			break;
		}
}

void CMulti2DecDosApp::UpdateProgress(const DWORD dwProgress)
{
	// 進捗状況を更新
	const DWORD dwCurProgress = (70UL * dwProgress) / 1000UL;

	for( ; m_dwLastProgress < dwCurProgress ; m_dwLastProgress++){
		::printf("|");
		}
}

void CMulti2DecDosApp::OutputLogFile(void)
{
	// ログファイル名を構築
	TCHAR szPath[_MAX_PATH + 1] = {TEXT('\0')};
	TCHAR szDrive[_MAX_DRIVE + 1] = {TEXT('\0')};
	TCHAR szDir[_MAX_DIR + 1] = {TEXT('\0')};
	TCHAR szFile[_MAX_FNAME + 1] = {TEXT('\0')};
	::_tsplitpath(m_ConfigSet.szOutputPath, szDrive, szDir, szFile, NULL);
	::_tmakepath(szPath, szDrive, szDir, szFile, TEXT("txt"));

	// ログファイルオープン
	FILE *pLogFile = ::_tfsopen(szPath, TEXT("wt"), _SH_DENYWR);
	if(!pLogFile)return;

	// ログ出力
	PrintLogFile(pLogFile);

	// ログファイルクローズ
	::fclose(pLogFile);
}

void CMulti2DecDosApp::PrintLogFile(FILE * const pStream)
{
	// ログ出力
	TCHAR szText[1024];
	char szMbText[_MAX_PATH];
		
	// 入力ファイル情報
	::ZeroMemory(szMbText, sizeof(szMbText));
	::WideCharToMultiByte(CP_OEMCP, WC_COMPOSITECHECK, m_ConfigSet.szInputPath, ::lstrlen(m_ConfigSet.szInputPath), szMbText, sizeof(szMbText), NULL, NULL);
	::_ftprintf(pStream, TEXT("Input File: %S\n"), szMbText);
	::_ftprintf(pStream, TEXT("Length: %12s (%s Byte)\n\n"),
		ToUnitedText(m_pMulti2Converter->GetSrcFileLength()),
		ToCommaText(m_pMulti2Converter->GetSrcFileLength())
		);		

	// 出力ファイル情報
	if(m_ConfigSet.bOutput){
		::ZeroMemory(szMbText, sizeof(szMbText));
		::WideCharToMultiByte(CP_OEMCP, WC_COMPOSITECHECK, m_ConfigSet.szOutputPath, ::lstrlen(m_ConfigSet.szOutputPath), szMbText, sizeof(szMbText), NULL, NULL);
		::_ftprintf(pStream, TEXT("Output File: %s\n"), szMbText);
		::_ftprintf(pStream, TEXT("Length: %12s (%s Byte)\n\n"),
			ToUnitedText(m_pMulti2Converter->GetDstFileLength()),
			ToCommaText(m_pMulti2Converter->GetDstFileLength())
			);
		}

	// PID毎のエラー情報の最大幅を求める
	int iInWidth = 0, iOutWidth = 0, iDropWidth = 0, iScramblingWidth = 0;

	for(WORD wPID = 0x0000U ; wPID < 0x2000U ; wPID++){
		if(m_pMulti2Converter->GetInputPacketNum(wPID)){
			::_stprintf(szText, TEXT("%lu"), m_pMulti2Converter->GetInputPacketNum(wPID));
			if(::lstrlen(szText) > iInWidth)iInWidth = ::lstrlen(szText);

			::_stprintf(szText, TEXT("%lu"), m_pMulti2Converter->GetOutputPacketNum(wPID));
			if(::lstrlen(szText) > iOutWidth)iOutWidth = ::lstrlen(szText);

			::_stprintf(szText, TEXT("%lu"), m_pMulti2Converter->GetContinuityErrNum(wPID));
			if(::lstrlen(szText) > iDropWidth)iDropWidth = ::lstrlen(szText);

			::_stprintf(szText, TEXT("%lu"), m_pMulti2Converter->GetScramblePacketNum(wPID));
			if(::lstrlen(szText) > iScramblingWidth)iScramblingWidth = ::lstrlen(szText);
			}
		}

	// PID毎のエラー情報
	for(WORD wPID = 0x0000U ; wPID < 0x2000U ; wPID++){
		if(m_pMulti2Converter->GetInputPacketNum(wPID)){
			if(m_ConfigSet.bOutput){
				::_ftprintf(pStream, TEXT("[PID: 0x%04X]  In: %*lu,  Out: %*lu,  Drop: %*lu,  Scrambling: %*lu\n"),
					wPID,
					iInWidth,			m_pMulti2Converter->GetInputPacketNum(wPID),
					iOutWidth,			m_pMulti2Converter->GetOutputPacketNum(wPID),
					iDropWidth,			m_pMulti2Converter->GetContinuityErrNum(wPID),
					iScramblingWidth,	m_pMulti2Converter->GetScramblePacketNum(wPID)
					);
				}
			else{
				::_ftprintf(pStream, TEXT("[PID: 0x%04X]  In: %*lu,  Drop: %*lu,  Scrambling: %*lu\n"),
					wPID,
					iInWidth,			m_pMulti2Converter->GetInputPacketNum(wPID),
					iDropWidth,			m_pMulti2Converter->GetContinuityErrNum(wPID),
					iScramblingWidth,	m_pMulti2Converter->GetScramblePacketNum(wPID)
					);
				}
			}
		}

	::_ftprintf(pStream, TEXT("\nSync Error       : %12s\n"), ToCommaText(m_pMulti2Converter->GetSyncErrNum()));
	::_ftprintf(pStream, TEXT("Format Error     : %12s\n"), ToCommaText(m_pMulti2Converter->GetFormatErrNum()));
	::_ftprintf(pStream, TEXT("Transport Error  : %12s Packet\n"), ToCommaText(m_pMulti2Converter->GetTransportErrNum()));
	::_ftprintf(pStream, TEXT("Total Drop Error : %12s Packet\n"), ToCommaText(m_pMulti2Converter->GetContinuityErrNum()));
	::_ftprintf(pStream, TEXT("Total Scrambling : %12s Packet\n"), ToCommaText(m_pMulti2Converter->GetScramblePacketNum()));
	::_ftprintf(pStream, TEXT("\nTotal Input      : %12s Packet\n"), ToCommaText(m_pMulti2Converter->GetInputPacketNum()));

	if(m_ConfigSet.bOutput){
		::_ftprintf(pStream, TEXT("Total Output     : %12s Packet\n"), ToCommaText(m_pMulti2Converter->GetOutputPacketNum()));
		}

	::_ftprintf(pStream, TEXT("Packet Stride    : %12lu Byte\n"), m_pMulti2Converter->GetPacketStride());

	if(m_ConfigSet.bB25){
		::_ftprintf(pStream, TEXT("\nECM Process      : %12s\n"), ToCommaText(m_pMulti2Converter->GetEcmProcessNum()));

		if(m_ConfigSet.bEMM){
			::_ftprintf(pStream, TEXT("EMM Process      : %12s\n"), ToCommaText(m_pMulti2Converter->GetEmmProcessNum()));
			}
		}
}

LPCTSTR CMulti2DecDosApp::ToUnitedText(const ULONGLONG llValue)
{
	static TCHAR szBuffer[256];

	DWORD dwValue;
	double lfValue;
	TCHAR cUnit;

	// 単位判定、スケーリング
	if(llValue < 0x400ULL){
		// 単位なし
		::_stprintf(szBuffer, TEXT("%lu B"), (DWORD)llValue);
		return szBuffer;
		}
	else if(llValue < 0x100000ULL){
		// K
		cUnit = TEXT('K');
		dwValue = (DWORD)(llValue >> 10);
		lfValue = (double)llValue / (double)(1ULL << 10);
		}
	else if(llValue < 0x40000000ULL){
		// M
		cUnit = TEXT('M');
		dwValue = (DWORD)(llValue >> 20);
		lfValue = (double)llValue / (double)(1ULL << 20);
		}
	else if(llValue < 0x10000000000ULL){
		// G
		cUnit = TEXT('G');
		dwValue = (DWORD)(llValue >> 30);
		lfValue = (double)llValue / (double)(1ULL << 30);
		}
	else{
		// T
		cUnit = TEXT('T');
		dwValue = (DWORD)(llValue >> 40);
		lfValue = (double)llValue / (double)(1ULL << 40);
		}

	// 桁あわせ
	if(dwValue < 10UL){
		// X.XX
		::_stprintf(szBuffer, TEXT("%.2f %CB"), lfValue, cUnit);
		}
	else if(dwValue < 100UL){
		// XX.X
		::_stprintf(szBuffer, TEXT("%.1f %CB"), lfValue, cUnit);
		}
	else{
		// XXX
		::_stprintf(szBuffer, TEXT("%.0f %CB"), lfValue, cUnit);
		}

	return szBuffer;
}

LPCTSTR CMulti2DecDosApp::ToCommaText(const ULONGLONG llValue)
{
	static TCHAR szBuffer[256];
	TCHAR szValue[256];
	TCHAR szTemp[256];

	// 文字列に変換
	::_stprintf(szValue, TEXT("%llu"), llValue);
	const int iValueLen = ::lstrlen(szValue);

	szBuffer[0] = TEXT('\0');

	// コンマで区切る
	for(int iPos = 0 ; iPos < iValueLen ; iPos++){
		::lstrcpy(szTemp, szBuffer);

		if(!(iPos % 3) && iPos){
			::_stprintf(szBuffer, TEXT("%c,%s"), szValue[iValueLen - iPos - 1], szTemp);
			}
		else{
			::_stprintf(szBuffer, TEXT("%c%s"), szValue[iValueLen - iPos - 1], szTemp);
			}
		}

	return szBuffer;
}
