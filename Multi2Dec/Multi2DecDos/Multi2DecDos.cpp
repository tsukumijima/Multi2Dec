// Multi2DecDos.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "Multi2DecDosApp.h"




// ファイルローカル関数プロトタイプ
static void PrintUsage(void);
static const bool ParseParam(CMulti2DecDosApp::CONFIGSET * const pConfigSet, const int iParamNum, LPTSTR aszParam[]);
static LPCTSTR FindParam(const int iParamNum, LPTSTR aszParam[], LPCTSTR lpszFind);
static const bool ParsePath(CMulti2DecDosApp::CONFIGSET * const pConfigSet);




// エントリーポイント
int _tmain(int argc, _TCHAR* argv[])
{
	CMulti2DecDosApp::CONFIGSET ConfigSet;
	CMulti2DecDosApp App(NULL);

	// バーション情報
	::printf("\n- Multi2DecDos Ver.2.10 - http://2sen.dip.jp/dtv/\n\n");

	// パラメータを解析
	if(!ParseParam(&ConfigSet, argc, argv)){
		// パラメータが不正
		PrintUsage();
		return -1;	
		}

	// ファイル名を生成
	if(!ParsePath(&ConfigSet)){
		// ファイル名が不正
		return -1;
		}

	// コンバート実行
	if(!App.Run(&ConfigSet)){
		return -1;
		}

	return 0;
}

static void PrintUsage(void)
{
	// ヘルプを表示
	::printf("\nMulti2DecDos [[/D | /C | /U] | [/OUT] [/B25] [/EMM] [/PURE] [/NULL]] [/N]\n");
	::printf("             入力ファイル [出力ファイル]\n\n");
	::printf("  /D            スクランブル解除を行います。(デフォルト動作)\n");
	::printf("  /C            TSファイルのエラーチェックを行います。\n");
	::printf("  /U            B-CASカードの契約情報の更新を行います。(EMM処理)\n");
	::printf("  /OUT          ファイルを出力します。\n");
	::printf("  /B25          Multi2復号を行います。\n");
	::printf("  /EMM          EMM処理を行います。\n");
	::printf("  /PURE         復号漏れパケットを出力しません。\n");
	::printf("  /NULL         NULLパケットを出力しません。\n");
	::printf("  /N            ログファイルを出力しません。\n");
	::printf("  入力ファイル  入力TSファイルのパス名\n");
	::printf("  出力ファイル  入力TSファイルのパス名\n\n");
}

static const bool ParseParam(CMulti2DecDosApp::CONFIGSET * const pConfigSet, const int iParamNum, LPTSTR aszParam[])
{
	// デフォルトパラメータ(/D 相当)
	pConfigSet->bOutput = true;
	pConfigSet->bB25	= true;
	pConfigSet->bEMM	= false;
	pConfigSet->bPURE	= false;
	pConfigSet->bNULL	= true;
	pConfigSet->bNoLog	= false;
	pConfigSet->szInputPath[0] = TEXT('\0');
	pConfigSet->szOutputPath[0] = TEXT('\0');

	// ファイル名を解析
	if(iParamNum >= 3){
		if((aszParam[iParamNum - 2][0] != TEXT('/')) && (aszParam[iParamNum - 1][0] != TEXT('/'))){
			// 入力/出力ファイル名を保存
			::lstrcpy(pConfigSet->szInputPath,  aszParam[iParamNum - 2]);
			::lstrcpy(pConfigSet->szOutputPath, aszParam[iParamNum - 1]);
			}
		else if(aszParam[iParamNum - 1][0] != TEXT('/')){
			// 入力ファイル名を保存
			::lstrcpy(pConfigSet->szInputPath, aszParam[iParamNum - 1]);
			}
		else{
			// ファイル名がない
			::printf("エラー: 入力ファイルが指定されていません。\n");
			return false;
			}		
		}
	else if(iParamNum >= 2){
		if(aszParam[1][0] != TEXT('/')){
			// 入力ファイル名を保存
			::lstrcpy(pConfigSet->szInputPath, aszParam[1]);
			return true;
			}
		else{
			// ファイル名がない
			::printf("エラー: 入力ファイル名が指定されていません。\n");
			return false;
			}		
		}
	else{
		// パラメータが1つもない
		::printf("エラー: パラメータが指定されていません。\n");
		return false;
		}

	// ログ出力制御パラメータを解析
	pConfigSet->bNoLog = (FindParam(iParamNum, aszParam, TEXT("/N")))? true : false;

	// プリセットパラメータを解析
	int iPresetNum = 0;
		
	if(FindParam(iParamNum, aszParam, TEXT("/D"))){
		// /D
		iPresetNum++;
		pConfigSet->bOutput = true;
		pConfigSet->bB25	= true;
		pConfigSet->bEMM	= false;
		pConfigSet->bPURE	= false;
		pConfigSet->bNULL	= true;
		}

	if(FindParam(iParamNum, aszParam, TEXT("/C"))){
		// /C
		iPresetNum++;
		pConfigSet->bOutput = false;
		pConfigSet->bB25	= false;
		pConfigSet->bEMM	= false;
		pConfigSet->bPURE	= false;
		pConfigSet->bNULL	= true;
		}

	if(FindParam(iParamNum, aszParam, TEXT("/U"))){
		// /U
		iPresetNum++;
		pConfigSet->bOutput = false;
		pConfigSet->bB25	= true;
		pConfigSet->bEMM	= true;
		pConfigSet->bPURE	= false;
		pConfigSet->bNULL	= true;
		}

	if(iPresetNum == 1){
		// プリセットモード
		return true;
		}
	else if(iPresetNum > 1){
		::printf("エラー: /D /C /U パラメータは同時に1つしか指定できません。\n");
		return false;
		}

	// カスタムパラメータ解析
	pConfigSet->bOutput = (FindParam(iParamNum, aszParam, TEXT("/OUT")))?  true : false;
	pConfigSet->bB25	= (FindParam(iParamNum, aszParam, TEXT("/B25")))?  true : false;
	pConfigSet->bEMM	= (FindParam(iParamNum, aszParam, TEXT("/EMM")))?  true : false;
	pConfigSet->bPURE	= (FindParam(iParamNum, aszParam, TEXT("/PURE")))? true : false;
	pConfigSet->bNULL	= (FindParam(iParamNum, aszParam, TEXT("/NULL")))? true : false;

	return true;
}

static LPCTSTR FindParam(const int iParamNum, LPTSTR aszParam[], LPCTSTR lpszFind)
{
	// パラメータリストから指定した文字列を検索する
	for(int i = 1 ; i < iParamNum ; i++){
		if(!::lstrcmpi(aszParam[i], lpszFind)){
			return aszParam[i];
			}		
		}

	return NULL;
}

static const bool ParsePath(CMulti2DecDosApp::CONFIGSET * const pConfigSet)
{
	const int iInputLen = ::lstrlen(pConfigSet->szInputPath);

	// ファイル長をチェックする
	if(iInputLen < 4){
		::printf("エラー: 入力ファイル名が短すぎます。\n");
		return false;
		}
		
	// ファイルパスをチェックする
	if(::lstrcmpi(&pConfigSet->szInputPath[iInputLen - 3], TEXT(".ts"))){
		::printf("エラー: 入力ファイルの拡張子がTSファイルではありません。\n");
		return false;
		}

	// 出力ファイルパスを生成
	if(pConfigSet->szOutputPath[0] == TEXT('\0')){
		::lstrcpy(pConfigSet->szOutputPath, pConfigSet->szInputPath);
		pConfigSet->szOutputPath[iInputLen - 3] = TEXT('\0');
		::lstrcat(pConfigSet->szOutputPath, TEXT("_dec.ts"));
		}

	return true;
}
