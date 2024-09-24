// Multi2DecWin.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです。
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル


// CMulti2DecWinApp:
// このクラスの実装については、Multi2DecWin.cpp を参照してください。
//

class CMulti2DecWinApp : public CWinApp
{
public:
	CMulti2DecWinApp();

// オーバーライド
	public:
	virtual BOOL InitInstance();

// 実装

	DECLARE_MESSAGE_MAP()
};

extern CMulti2DecWinApp theApp;