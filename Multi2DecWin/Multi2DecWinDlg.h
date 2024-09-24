// Multi2DecWinDlg.h : ヘッダー ファイル
//

#pragma once


#include "TsConverterIf.h"
#include "ConfigData.h"
#include "afxcmn.h"


// CMulti2DecWinDlg ダイアログ
class CMulti2DecWinDlg :	public CDialog,
							public CBonObject,
							protected IMulti2Converter::IHandler
{
// コンストラクション
public:
// CBonObject
	DECLARE_IBONOBJECT(CMulti2DecWinDlg)

// CMulti2DecWinDlg
	CMulti2DecWinDlg(IBonObject * const pOwner);

// ダイアログ データ
	enum { IDD = IDD_MULTI2DECWIN_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV サポート

// 実装
protected:
	virtual void OnMulti2ConverterEvent(IMulti2Converter *pMulti2Converter, const DWORD dwEventID, PVOID pParam);
	void RelayoutControl(void);
	void UpdateCmdState(void);
	const BOOL AddSourceFile(LPCTSTR lpszFilePath);
	static const CString ToUnitedText(const ULONGLONG llValue);
	static const CString ToCommaText(const ULONGLONG llValue);
	const BOOL StartNextFile(void);
	void UpdateProgress(const DWORD dwProgress);
	static const BOOL OpenBrowseDialog(HWND hWnd, LPTSTR lpszPath, LPCTSTR lpszTitle);
	void OutputLogFile(void);

	// 生成された、メッセージ割り当て関数
	void OnOK(void);
	void OnCancel(void);
	virtual BOOL OnInitDialog();
	BOOL OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnConverterEvent(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	HICON m_hIcon;
	CReBar m_ReBar;
	CToolBar m_ToolBar;
	CStatusBar m_StatusBar;
	CListCtrl m_FileList;
	CComboBox m_ModeCombo;
	CProgressCtrl m_Progress;

	IMulti2Converter *m_pMulti2Converter;

	CConfigBool m_bCfgOutput;
	CConfigBool m_bCfgB25;
	CConfigBool m_bCfgEmm;
	CConfigBool m_bCfgDiscard;
	CConfigBool m_bCfgNull;
	CConfigBool m_bCfgLogEnable;
	CConfigBool m_bCfgPathEnable;
	CConfigString m_csOutputPath;

	BOOL m_bIsConvert;
	DWORD m_dwSrcIndex;
	CString m_csDstPath;

public:
	afx_msg void OnDestroy();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCmdStart();
	afx_msg void OnCmdStop();
	afx_msg void OnCmdLog();
	afx_msg void OnCmdExit();
	afx_msg void OnCmdListAdd();
	afx_msg void OnCmdListRemove();
	afx_msg void OnCmdListClear();
	afx_msg void OnCmdListUp();
	afx_msg void OnCmdListDown();
	afx_msg void OnCmdPreDescramble();
	afx_msg void OnCmdPreErrCheck();
	afx_msg void OnCmdPreEmmUpdate();
	afx_msg void OnCmdCfgOutput();
	afx_msg void OnCmdCfgB25();
	afx_msg void OnCmdCfgEmm();
	afx_msg void OnCmdCfgDiscard();
	afx_msg void OnCmdCfgNull();
	afx_msg void OnCmdCfgLogEnable();
	afx_msg void OnCmdCfgPathSpec();
	afx_msg void OnCmdCfgBrowse();
	afx_msg void OnCmdAbout();
	afx_msg void OnCmdListAllSel();
	afx_msg void OnCmdListAllcheck();
	afx_msg void OnCbnSelchangeModeCombo();
	afx_msg void OnNMRclickFileList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkFileList(NMHDR *pNMHDR, LRESULT *pResult);
};
