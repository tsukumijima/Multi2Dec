// Multi2DecWinDlg.cpp : �����t�@�C��
//

#include "stdafx.h"
#include "Multi2DecWin.h"
#include "Multi2DecWinDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ���̃t�@�C���J�n���b�Z�[�W
#define WM_CONVERTER_EVENT		(WM_USER + 0x100UL)


// �A�v���P�[�V�����̃o�[�W�������Ɏg���� CAboutDlg �_�C�A���O

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �_�C�A���O �f�[�^
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g

// ����
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CMulti2DecWinDlg �_�C�A���O
CMulti2DecWinDlg::CMulti2DecWinDlg(IBonObject * const pOwner)
	: CDialog(CMulti2DecWinDlg::IDD, NULL)
	, CBonObject(pOwner)
	, m_pMulti2Converter(NULL)
	, m_bCfgOutput(CONFSECT_GENERAL, TEXT("OutputEnable"), TRUE)
	, m_bCfgB25(CONFSECT_GENERAL, TEXT("B25Enable"), TRUE)
	, m_bCfgEmm(CONFSECT_GENERAL, TEXT("EmmEnable"), FALSE)
	, m_bCfgDiscard(CONFSECT_GENERAL, TEXT("DiscardEnable"), FALSE)
	, m_bCfgNull(CONFSECT_GENERAL, TEXT("NullEnable"), TRUE)
	, m_bCfgLogEnable(CONFSECT_GENERAL, TEXT("LogEnable"), TRUE)
	, m_bCfgPathEnable(CONFSECT_GENERAL, TEXT("PathEnable"), FALSE)
	, m_csOutputPath(CONFSECT_GENERAL, TEXT("OutputPath"), TEXT(""))
	, m_bIsConvert(FALSE)
	, m_dwSrcIndex(0UL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMulti2DecWinDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FILE_LIST, m_FileList);
	DDX_Control(pDX, IDC_MODECOMBO, m_ModeCombo);
	DDX_Control(pDX, IDC_PROGRESS, m_Progress);
}

BEGIN_MESSAGE_MAP(CMulti2DecWinDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
	ON_WM_DROPFILES()
	ON_WM_SIZE()
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNotify)
	ON_COMMAND(ID_CMD_START, &CMulti2DecWinDlg::OnCmdStart)
	ON_COMMAND(ID_CMD_STOP, &CMulti2DecWinDlg::OnCmdStop)
	ON_COMMAND(ID_CMD_LOG, &CMulti2DecWinDlg::OnCmdLog)
	ON_COMMAND(ID_CMD_EXIT, &CMulti2DecWinDlg::OnCmdExit)
	ON_COMMAND(ID_CMD_LIST_ADD, &CMulti2DecWinDlg::OnCmdListAdd)
	ON_COMMAND(ID_CMD_LIST_REMOVE, &CMulti2DecWinDlg::OnCmdListRemove)
	ON_COMMAND(ID_CMD_LIST_CLEAR, &CMulti2DecWinDlg::OnCmdListClear)
	ON_COMMAND(ID_CMD_LIST_UP, &CMulti2DecWinDlg::OnCmdListUp)
	ON_COMMAND(ID_CMD_LIST_DOWN, &CMulti2DecWinDlg::OnCmdListDown)
	ON_COMMAND(ID_CMD_PRE_DESCRAMBLE, &CMulti2DecWinDlg::OnCmdPreDescramble)
	ON_COMMAND(ID_CMD_PRE_ERRCHECK, &CMulti2DecWinDlg::OnCmdPreErrCheck)
	ON_COMMAND(ID_CMD_PRE_EMMUPDATE, &CMulti2DecWinDlg::OnCmdPreEmmUpdate)
	ON_COMMAND(ID_CMD_CFG_OUTPUT, &CMulti2DecWinDlg::OnCmdCfgOutput)
	ON_COMMAND(ID_CMD_CFG_B25, &CMulti2DecWinDlg::OnCmdCfgB25)
	ON_COMMAND(ID_CMD_CFG_EMM, &CMulti2DecWinDlg::OnCmdCfgEmm)
	ON_COMMAND(ID_CMD_CFG_DISCARD, &CMulti2DecWinDlg::OnCmdCfgDiscard)
	ON_COMMAND(ID_CMD_CFG_NULL, &CMulti2DecWinDlg::OnCmdCfgNull)
	ON_COMMAND(ID_CMD_CFG_LOGENABLE, &CMulti2DecWinDlg::OnCmdCfgLogEnable)
	ON_COMMAND(ID_CMD_CFG_PATHSPEC, &CMulti2DecWinDlg::OnCmdCfgPathSpec)
	ON_COMMAND(ID_CMD_CFG_BROWSE, &CMulti2DecWinDlg::OnCmdCfgBrowse)
	ON_COMMAND(ID_CMD_ABOUT, &CMulti2DecWinDlg::OnCmdAbout)
	ON_COMMAND(ID_CMD_LIST_ALLSEL, &CMulti2DecWinDlg::OnCmdListAllSel)
	ON_COMMAND(ID_CMD_LIST_ALLCHECK, &CMulti2DecWinDlg::OnCmdListAllcheck)
	ON_CBN_SELCHANGE(IDC_MODECOMBO, &CMulti2DecWinDlg::OnCbnSelchangeModeCombo)
	ON_MESSAGE(WM_CONVERTER_EVENT, &CMulti2DecWinDlg::OnConverterEvent)
	ON_NOTIFY(NM_RCLICK, IDC_FILE_LIST, &CMulti2DecWinDlg::OnNMRclickFileList)
	ON_NOTIFY(NM_DBLCLK, IDC_FILE_LIST, &CMulti2DecWinDlg::OnNMDblclkFileList)
END_MESSAGE_MAP()


// CMulti2DecWinDlg ���b�Z�[�W �n���h��

BOOL CMulti2DecWinDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ���̃_�C�A���O�̃A�C�R����ݒ肵�܂��B�A�v���P�[�V�����̃��C�� �E�B���h�E���_�C�A���O�łȂ��ꍇ�A
	//  Framework �́A���̐ݒ�������I�ɍs���܂��B
	SetIcon(m_hIcon, TRUE);			// �傫���A�C�R���̐ݒ�
	SetIcon(m_hIcon, FALSE);		// �������A�C�R���̐ݒ�

	// TODO: �������������ɒǉ����܂��B

	// �R���o�[�^�C���X�^���X����
	m_pMulti2Converter = ::BON_SAFE_CREATE<IMulti2Converter *>(TEXT("CMulti2Converter"), dynamic_cast<IMulti2Converter::IHandler *>(this));

	if(!m_pMulti2Converter){
		::AfxMessageBox(TEXT("CMulti2Converter�̃C���X�^���X�𐶐��ł��܂���ł����B\r\n�K�v�ȃ��W���[�����C���X�g�[������Ă��Ȃ��\��������܂��B"), MB_OK | MB_ICONEXCLAMATION );
		EndDialog(IDCANCEL);
		}

	// �X�e�[�^�X�o�[�쐬
	m_StatusBar.Create(this);
	m_StatusBar.GetStatusBarCtrl().SetText(TEXT("��~��"), 0, 0);

	// �v���O���X�o�[�ݒ�
	m_Progress.SetParent(&m_StatusBar);
	m_Progress.SetRange32(0, 1000);

	// �c�[���o�[�쐬
	m_ToolBar.Create(this, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_NOALIGN | CBRS_TOOLTIPS | CBRS_FLYBY, AFX_IDW_TOOLBAR);
	m_ToolBar.LoadToolBar(IDR_TOOLBAR1);
	m_ToolBar.SetBarStyle(m_ToolBar.GetBarStyle() & ~CBRS_BORDER_TOP);
	m_ToolBar.GetToolBarCtrl().SetStyle(m_ToolBar.GetToolBarCtrl().GetStyle() | TBSTYLE_FLAT | TBSTYLE_LIST | TBSTYLE_AUTOSIZE);
	m_ToolBar.SetBorders(0, 0, 500, 0);

	// ���o�[�쐬
	m_ReBar.Create(this, RBS_AUTOSIZE | RBS_FIXEDORDER | RBS_VARHEIGHT, WS_BORDER | WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_TOP);
	m_ReBar.SetBarStyle(CBRS_ALIGN_TOP | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_BORDER_TOP);
	m_ReBar.AddBar(&m_ToolBar, NULL, NULL, RBBS_FIXEDSIZE | RBBS_NOGRIPPER);
	m_ReBar.AddBar(&m_ModeCombo, TEXT("�v���Z�b�g"), NULL, RBBS_NOGRIPPER);

	// �R���g���[�������C�A�E�g
	RelayoutControl();
	RelayoutControl();

	// �t�@�C�����X�g�ݒ�
	m_FileList.SetExtendedStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);
	m_FileList.InsertColumn(0, TEXT("�t�@�C��"), LVCFMT_LEFT, 260);
	m_FileList.InsertColumn(1, TEXT("�T�C�Y"), LVCFMT_LEFT, 56);
	m_FileList.InsertColumn(2, TEXT("�������"), LVCFMT_LEFT, 80);
	m_FileList.InsertColumn(3, TEXT("�i��"), LVCFMT_LEFT, 50);
	m_FileList.InsertColumn(4, TEXT("�����װ"), LVCFMT_LEFT, 74);
	m_FileList.InsertColumn(5, TEXT("̫�ϯĴװ"), LVCFMT_LEFT, 74);
	m_FileList.InsertColumn(6, TEXT("�ޯĴװ"), LVCFMT_LEFT, 74);
	m_FileList.InsertColumn(7, TEXT("��ۯ��"), LVCFMT_LEFT, 74);
	m_FileList.InsertColumn(8, TEXT("�����R��"), LVCFMT_LEFT, 74);
	m_FileList.InsertColumn(9, TEXT("EMM����"), LVCFMT_LEFT, 74);

	// �R���g���[����ԍX�V
	UpdateCmdState();

	return TRUE;  // �t�H�[�J�X���R���g���[���ɐݒ肵���ꍇ�������ATRUE ��Ԃ��܂��B
}

void CMulti2DecWinDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// �_�C�A���O�ɍŏ����{�^����ǉ�����ꍇ�A�A�C�R����`�悷�邽�߂�
//  ���̃R�[�h���K�v�ł��B�h�L�������g/�r���[ ���f�����g�� MFC �A�v���P�[�V�����̏ꍇ�A
//  ����́AFramework �ɂ���Ď����I�ɐݒ肳��܂��B

void CMulti2DecWinDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �`��̃f�o�C�X �R���e�L�X�g

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// �N���C�A���g�̎l�p�`�̈���̒���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �A�C�R���̕`��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC dc(this); // �`��̃f�o�C�X �R���e�L�X�g

		// ���j���[�o�[�̉��ɐ�������
		CRect rcClient;
		GetClientRect(&rcClient);
		dc.FillSolidRect(0, 1, rcClient.Width(), 1, GetSysColor(COLOR_3DHILIGHT));

		CDialog::OnPaint();
	}
}

// ���[�U�[���ŏ��������E�B���h�E���h���b�O���Ă���Ƃ��ɕ\������J�[�\�����擾���邽�߂ɁA
//  �V�X�e�������̊֐����Ăяo���܂��B
HCURSOR CMulti2DecWinDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CMulti2DecWinDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: �����Ƀ��b�Z�[�W �n���h�� �R�[�h��ǉ����܂��B
	KillTimer(1);

	// �R���o�[�^�C���X�^���X�J��
	BON_SAFE_RELEASE(m_pMulti2Converter);
}

void CMulti2DecWinDlg::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: �����Ƀ��b�Z�[�W �n���h�� �R�[�h��ǉ����܂��B
	if(::IsWindow(m_ReBar.GetSafeHwnd())){
		RelayoutControl();
		RelayoutControl();
		}
}

void CMulti2DecWinDlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: �����Ƀ��b�Z�[�W �n���h�� �R�[�h��ǉ����邩�A����̏������Ăяo���܂��B

	// �t�@�C�������擾
	const DWORD dwNum = ::DragQueryFile(hDropInfo, (UINT)-1L, NULL, 0);

	// �t�@�C�����X�g�ɒǉ�
	CString csFilePath;

	for(DWORD dwPos = 0UL ; dwPos < dwNum ; dwPos++){
		// �t�@�C���p�X���擾
		if(!::DragQueryFile(hDropInfo, dwPos, csFilePath.GetBuffer(_MAX_PATH), _MAX_PATH))continue;
		csFilePath.ReleaseBuffer();

		// �g���q���`�F�b�N
		if(csFilePath.Right(3).MakeLower() != TEXT(".ts"))continue;

		// �t�@�C�����X�g�ɒǉ�
		AddSourceFile(csFilePath);
		}

	CDialog::OnDropFiles(hDropInfo);
}

void CMulti2DecWinDlg::RelayoutControl(void)
{
	// �R���g���[���o�[���Ĕz�u
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);

	CRect rcClient, rcReBar, rcStatusBar;
	GetClientRect(&rcClient);

	// �t�@�C�����X�g���T�C�Y
	m_ReBar.GetWindowRect(&rcReBar);
	m_StatusBar.GetWindowRect(&rcStatusBar);
	m_FileList.SetWindowPos(NULL, 0, rcReBar.Height(), rcClient.Width(), rcClient.Height() - rcReBar.Height() - rcStatusBar.Height(), SWP_NOZORDER);

	// �v���O���X�o�[���T�C�Y
	m_StatusBar.GetClientRect(&rcStatusBar);
	int aiPatrs[] = {150, rcStatusBar.Width() - 80, rcStatusBar.Width() - 1};
	m_StatusBar.GetStatusBarCtrl().SetParts(sizeof(aiPatrs) / sizeof(aiPatrs[0]), aiPatrs);
	m_StatusBar.GetStatusBarCtrl().GetRect(1, &rcStatusBar);
	m_Progress.MoveWindow(rcStatusBar);

	RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
}

void CMulti2DecWinDlg::UpdateCmdState(void)
{
	// �v���Z�b�g���X�g�X�V
	int iIndex;
																	// OUT B25 EMM DIS NUL
	if(m_bCfgOutput && m_bCfgB25 && !m_bCfgEmm)iIndex = 0;			//  T   T   F   -   -	�X�N�����u������
	else if(!m_bCfgOutput && !m_bCfgB25)iIndex = 1;					//  F   F   -   -   -  	TS�G���[�`�F�b�N
	else if(!m_bCfgOutput && m_bCfgB25 && m_bCfgEmm)iIndex = 2;		//  F   T   T   -   -	�_����X�V
	else iIndex = 3;												//  -   -   -   -   - 	���[�U��`

	m_ModeCombo.SetCurSel(iIndex);

	// ���j���[�A�c�[���o�[�̏�Ԃ��X�V����
	CMenu *pMenu = GetMenu();
	pMenu->CheckMenuItem(ID_CMD_CFG_OUTPUT,	   MF_BYCOMMAND | ((m_bCfgOutput)?	   MF_CHECKED : MF_UNCHECKED));
	pMenu->CheckMenuItem(ID_CMD_CFG_B25,	   MF_BYCOMMAND | ((m_bCfgB25)?		   MF_CHECKED : MF_UNCHECKED));
	pMenu->CheckMenuItem(ID_CMD_CFG_EMM,	   MF_BYCOMMAND | ((m_bCfgEmm)?		   MF_CHECKED : MF_UNCHECKED));
	pMenu->CheckMenuItem(ID_CMD_CFG_DISCARD,   MF_BYCOMMAND | ((m_bCfgDiscard)?	   MF_CHECKED : MF_UNCHECKED));
	pMenu->CheckMenuItem(ID_CMD_CFG_NULL,	   MF_BYCOMMAND | ((m_bCfgNull)?	   MF_CHECKED : MF_UNCHECKED));
	pMenu->CheckMenuItem(ID_CMD_CFG_PATHSPEC,  MF_BYCOMMAND | ((m_bCfgPathEnable)? MF_CHECKED : MF_UNCHECKED));
	pMenu->CheckMenuItem(ID_CMD_CFG_LOGENABLE, MF_BYCOMMAND | ((m_bCfgLogEnable)?  MF_CHECKED : MF_UNCHECKED));
	pMenu->EnableMenuItem(ID_CMD_CFG_EMM,	   MF_BYCOMMAND | ((m_bCfgB25)?		   MF_ENABLED : MF_GRAYED));
	pMenu->EnableMenuItem(ID_CMD_CFG_BROWSE,   MF_BYCOMMAND | ((m_bCfgPathEnable)? MF_ENABLED : MF_GRAYED));

	pMenu->CheckMenuRadioItem(ID_CMD_PRE_DESCRAMBLE, ID_CMD_PRE_EMMUPDATE, ID_CMD_PRE_DESCRAMBLE + iIndex, MF_BYCOMMAND);

	// ���b�N
	pMenu->EnableMenuItem(1UL,			 MF_BYPOSITION | ((!m_bIsConvert)?  MF_ENABLED : MF_GRAYED));
	pMenu->EnableMenuItem(2UL,			 MF_BYPOSITION | ((!m_bIsConvert)?  MF_ENABLED : MF_GRAYED));
	pMenu->EnableMenuItem(ID_CMD_START,	 MF_BYCOMMAND  | ((!m_bIsConvert)?  MF_ENABLED : MF_GRAYED));
	pMenu->EnableMenuItem(ID_CMD_LOG,	 MF_BYCOMMAND  | ((!m_bIsConvert)?  MF_ENABLED : MF_GRAYED));

	m_ToolBar.GetToolBarCtrl().EnableButton(ID_CMD_START,			(!m_bIsConvert)?  TRUE : FALSE);
	m_ToolBar.GetToolBarCtrl().EnableButton(ID_CMD_LOG,				(!m_bIsConvert)?  TRUE : FALSE);
	m_ToolBar.GetToolBarCtrl().EnableButton(ID_CMD_LIST_ADD,		(!m_bIsConvert)?  TRUE : FALSE);
	m_ToolBar.GetToolBarCtrl().EnableButton(ID_CMD_LIST_REMOVE,		(!m_bIsConvert)?  TRUE : FALSE);
	m_ToolBar.GetToolBarCtrl().EnableButton(ID_CMD_LIST_CLEAR,		(!m_bIsConvert)?  TRUE : FALSE);
	m_ToolBar.GetToolBarCtrl().EnableButton(ID_CMD_LIST_UP,			(!m_bIsConvert)?  TRUE : FALSE);
	m_ToolBar.GetToolBarCtrl().EnableButton(ID_CMD_LIST_DOWN,		(!m_bIsConvert)?  TRUE : FALSE);
	m_ToolBar.GetToolBarCtrl().EnableButton(ID_CMD_LIST_ALLCHECK,	(!m_bIsConvert)?  TRUE : FALSE);

	m_ModeCombo.EnableWindow((!m_bIsConvert)? TRUE : FALSE);
	m_FileList.EnableWindow((!m_bIsConvert)?  TRUE : FALSE);

	// �X�e�[�^�X�o�[�X�V
	m_StatusBar.GetStatusBarCtrl().SetText((m_bIsConvert)? TEXT("�������D�D�D") : TEXT("��~��"), 0, 0);
	if(!m_bIsConvert)m_StatusBar.GetStatusBarCtrl().SetText(TEXT(""), 2, 0);

	// �v���O���X�o�[�X�V
	if(!m_bIsConvert)m_Progress.SetPos(0);

	RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
}

const BOOL CMulti2DecWinDlg::AddSourceFile(LPCTSTR lpszFilePath)
{
	if(!lpszFilePath)return FALSE;

	LVFINDINFO FindInfo;
	FindInfo.flags = LVFI_STRING;
	FindInfo.psz = lpszFilePath;

	// �����̃t�@�C��������
	if(m_FileList.FindItem(&FindInfo) != -1)return FALSE;

	// �t�@�C�����X�g�ɒǉ�
	const int iItem = m_FileList.InsertItem(m_FileList.GetItemCount(), lpszFilePath);
	m_FileList.SetCheck(iItem, TRUE);

	// �t�@�C���T�C�Y�����߂�
	ISmartFile *pFile = ::BON_SAFE_CREATE<ISmartFile *>(TEXT("CSmartFile"));
	if(!pFile)return FALSE;

	if(pFile->Open(lpszFilePath)){
		m_FileList.SetItemText(iItem, 1, ToUnitedText(pFile->GetLength()));
		pFile->Close();
		}
	else{
		m_FileList.SetItemText(iItem, 1, TEXT("�s��"));
		}

	BON_SAFE_RELEASE(pFile);

	// �T�u�A�C�e��������
	m_FileList.SetItemText(iItem, 2, TEXT("������"));

	for(int iSubItem = 3 ; iSubItem <= 9 ; iSubItem++){
		m_FileList.SetItemText(iItem, iSubItem, TEXT("-"));
		}

	return TRUE;
}

const CString CMulti2DecWinDlg::ToUnitedText(const ULONGLONG llValue)
{
	CString csReturn;
	DWORD dwValue;
	double lfValue;
	TCHAR cUnit;

	// �P�ʔ���A�X�P�[�����O
	if(llValue < 0x400ULL){
		// �P�ʂȂ�
		csReturn.Format(TEXT("%lu B"), (DWORD)llValue);
		return csReturn;
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

	// �����킹
	if(dwValue < 10UL){
		// X.XX
		csReturn.Format(TEXT("%.2f %CB"), lfValue, cUnit);
		}
	else if(dwValue < 100UL){
		// XX.X
		csReturn.Format(TEXT("%.1f %CB"), lfValue, cUnit);
		}
	else{
		// XXX
		csReturn.Format(TEXT("%.0f %CB"), lfValue, cUnit);
		}

	return csReturn;
}

const CString CMulti2DecWinDlg::ToCommaText(const ULONGLONG llValue)
{
	CString csValue;
	CString csReturn;

	// ������ɕϊ�
	csValue.Format(TEXT("%llu"), llValue);

	// �R���}�ŋ�؂�
	for(int iPos = 0 ; iPos < csValue.GetLength() ; iPos++){
		if(!(iPos % 3) && iPos){
			csReturn = TEXT(',') + csReturn;
			}

		csReturn = csValue[csValue.GetLength() - iPos - 1] + csReturn;
		}

	return csReturn;
}

const BOOL CMulti2DecWinDlg::StartNextFile(void)
{
	// �`�F�b�N����Ă���t�@�C���̃R���o�[�g���J�n����
	int iSrcPos = -1;

	// �`�F�b�N����Ă���t�@�C������������
	for(iSrcPos = 0 ; iSrcPos < m_FileList.GetItemCount() ; iSrcPos++){
		if(m_FileList.GetCheck(iSrcPos))break;
		}

	if(iSrcPos >= m_FileList.GetItemCount())return FALSE;

	// �o�̓t�@�C�����ݒ�
	m_csDstPath = m_FileList.GetItemText(iSrcPos, 0);
	m_csDstPath = m_csDstPath.Left(m_csDstPath.GetLength() - 3) + TEXT("_dec.ts");

	// �\�[�X�t�@�C���̃C���f�b�N�X�ۑ�
	m_dwSrcIndex = (DWORD)iSrcPos;

	if(m_bCfgPathEnable){
		m_csDstPath = m_csOutputPath + m_csDstPath.Mid(m_csDstPath.ReverseFind(TEXT('\\')) + 1);
		}

	// �R���o�[�g�J�n
	const DWORD dwErrorCode = m_pMulti2Converter->StartConvert(m_FileList.GetItemText(iSrcPos, 0), (m_bCfgOutput)? m_csDstPath : NULL, (m_bCfgB25)? true : false, (m_bCfgEmm)? true : false, (m_bCfgNull)? true : false, (m_bCfgDiscard)? true : false);

	switch(dwErrorCode){
		case IMulti2Converter::EC_FILE_ERROR :
			m_FileList.SetItemText(m_dwSrcIndex, 2, TEXT("�G���[����"));
			::AfxMessageBox(TEXT("�t�@�C�����I�[�v���ł��܂���ł���"));
			return FALSE;

		case IMulti2Converter::EC_BCAS_ERROR :
			m_FileList.SetItemText(m_dwSrcIndex, 2, TEXT("�G���[����"));
			::AfxMessageBox(TEXT("B-CAS�J�[�h���I�[�v���ł��܂���ł���"));
			return FALSE;

		case IMulti2Converter::EC_CLASS_ERROR :
			m_FileList.SetItemText(m_dwSrcIndex, 2, TEXT("�G���[����"));
			::AfxMessageBox(TEXT("�K�v�ȃ��W���[����������܂���"));
			return FALSE;

		case IMulti2Converter::EC_INTERNAL_ERROR :
			m_FileList.SetItemText(m_dwSrcIndex, 2, TEXT("�G���[����"));
			::AfxMessageBox(TEXT("�����G���[���������܂���"));
			return FALSE;
		}

	return TRUE;
}

void CMulti2DecWinDlg::UpdateProgress(const DWORD dwProgress)
{
	CString csText;

	// �t�@�C�����X�g�X�V
	csText.Format(TEXT("%lu%%"), dwProgress / 10UL);
	m_FileList.SetItemText(m_dwSrcIndex, 3, csText);

	csText.Format(TEXT("%lu"), m_pMulti2Converter->GetSyncErrNum());
	m_FileList.SetItemText(m_dwSrcIndex, 4, csText);

	csText.Format(TEXT("%lu"), m_pMulti2Converter->GetFormatErrNum());
	m_FileList.SetItemText(m_dwSrcIndex, 5, csText);

	csText.Format(TEXT("%lu"), m_pMulti2Converter->GetTransportErrNum());
	m_FileList.SetItemText(m_dwSrcIndex, 6, csText);

	csText.Format(TEXT("%lu"), m_pMulti2Converter->GetContinuityErrNum());
	m_FileList.SetItemText(m_dwSrcIndex, 7, csText);

	csText.Format(TEXT("%lu"), m_pMulti2Converter->GetScramblePacketNum());
	m_FileList.SetItemText(m_dwSrcIndex, 8, csText);

	csText.Format(TEXT("%lu"), m_pMulti2Converter->GetEmmProcessNum());
	m_FileList.SetItemText(m_dwSrcIndex, 9, (m_bCfgB25 && m_bCfgEmm)? csText : TEXT("����"));

	// �X�e�[�^�X�o�[�X�V
	csText.Format(TEXT("%.1f%%"), (double)dwProgress / 10.0);
	m_StatusBar.GetStatusBarCtrl().SetText(csText, 2, 0);

	// �v���O���X�o�[�X�V
	m_Progress.SetPos(dwProgress);
}

const BOOL CMulti2DecWinDlg::OpenBrowseDialog(HWND hWnd, LPTSTR lpszPath, LPCTSTR lpszTitle)
{
	// �R�[���o�b�N�֐�
	class SHBrowseForFolderHelper
	{
		public:
			static int CALLBACK SHBrowseForFolderCallBack(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
			{
				if(uMsg == BFFM_INITIALIZED){
					::SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM)TRUE, lpData);
					}

				return 0;
			}
	};

	if(!lpszPath)return FALSE;

	TCHAR szPath[_MAX_PATH + 1] = {TEXT('\0')};

	BROWSEINFO BrowseInfo =	{
		hWnd,												// HWND hwndOwner
		NULL,												// LPCITEMIDLIST pidlRoot
		szPath,												// LPSTR pszDisplayName
		lpszTitle,											// LPCSTR lpszTitle
		BIF_RETURNONLYFSDIRS,								// UINT ulFlags
		SHBrowseForFolderHelper::SHBrowseForFolderCallBack,	// BFFCALLBACK lpfn
		(LPARAM)lpszPath,									// LPARAM lParam
		0													// int iImage
		};

	LPITEMIDLIST pList = ::SHBrowseForFolder(&BrowseInfo);

	if(pList){
		::SHGetPathFromIDList(pList, szPath);
		if(szPath[::lstrlen(szPath) - 1] != TEXT('\\') && ::lstrlen(szPath))::lstrcat(szPath, TEXT("\\"));
		::lstrcpy(lpszPath, szPath);
		::CoTaskMemFree(pList);
		return TRUE;
		}

	return FALSE;
}

void CMulti2DecWinDlg::OutputLogFile(void)
{
	CString csLog;

	// ���̓t�@�C�����
	csLog.AppendFormat(TEXT("Input File: %s\r\n"), m_FileList.GetItemText(m_dwSrcIndex, 0));

	csLog.AppendFormat(TEXT("Length: %12s (%s Byte)\r\n\r\n"),
		ToUnitedText(m_pMulti2Converter->GetSrcFileLength()),
		ToCommaText(m_pMulti2Converter->GetSrcFileLength())
		);

	// �o�̓t�@�C�����
	if(m_bCfgOutput){
		csLog.AppendFormat(TEXT("Output File: %s\r\n"), m_csDstPath);

		csLog.AppendFormat(TEXT("Length: %12s (%s Byte)\r\n\r\n"),
			ToUnitedText(m_pMulti2Converter->GetDstFileLength()),
			ToCommaText(m_pMulti2Converter->GetDstFileLength())
			);
		}

	// PID���̃G���[���̍ő啝�����߂�
	int iInWidth = 0, iOutWidth = 0, iDropWidth = 0, iScramblingWidth = 0;

	for(WORD wPID = 0x0000U ; wPID < 0x2000U ; wPID++){
		CString csText;

		if(m_pMulti2Converter->GetInputPacketNum(wPID)){
			csText.Format(TEXT("%lu"), m_pMulti2Converter->GetInputPacketNum(wPID));
			if(csText.GetLength() > iInWidth)iInWidth = csText.GetLength();

			csText.Format(TEXT("%lu"), m_pMulti2Converter->GetOutputPacketNum(wPID));
			if(csText.GetLength() > iOutWidth)iOutWidth = csText.GetLength();

			csText.Format(TEXT("%lu"), m_pMulti2Converter->GetContinuityErrNum(wPID));
			if(csText.GetLength() > iDropWidth)iDropWidth = csText.GetLength();

			csText.Format(TEXT("%lu"), m_pMulti2Converter->GetScramblePacketNum(wPID));
			if(csText.GetLength() > iScramblingWidth)iScramblingWidth = csText.GetLength();
			}
		}

	// PID���̃G���[���
	for(WORD wPID = 0x0000U ; wPID < 0x2000U ; wPID++){
		if(m_pMulti2Converter->GetInputPacketNum(wPID)){
			if(m_bCfgOutput){
				csLog.AppendFormat(TEXT("[PID: 0x%04X]    Input: %*lu,    Output: %*lu,    Drop: %*lu,    Scrambling: %*lu\r\n"),
					wPID,
					iInWidth,			m_pMulti2Converter->GetInputPacketNum(wPID),
					iOutWidth,			m_pMulti2Converter->GetOutputPacketNum(wPID),
					iDropWidth,			m_pMulti2Converter->GetContinuityErrNum(wPID),
					iScramblingWidth,	m_pMulti2Converter->GetScramblePacketNum(wPID)
					);
				}
			else{
				csLog.AppendFormat(TEXT("[PID: 0x%04X]    Input: %*lu,    Drop: %*lu,    Scrambling: %*lu\r\n"),
					wPID,
					iInWidth,			m_pMulti2Converter->GetInputPacketNum(wPID),
					iDropWidth,			m_pMulti2Converter->GetContinuityErrNum(wPID),
					iScramblingWidth,	m_pMulti2Converter->GetScramblePacketNum(wPID)
					);
				}
			}
		}

	csLog.AppendFormat(TEXT("\r\nSync Error       : %12s\r\n"), ToCommaText(m_pMulti2Converter->GetSyncErrNum()));
	csLog.AppendFormat(TEXT("Format Error     : %12s\r\n"), ToCommaText(m_pMulti2Converter->GetFormatErrNum()));
	csLog.AppendFormat(TEXT("Transport Error  : %12s Packet\r\n"), ToCommaText(m_pMulti2Converter->GetTransportErrNum()));
	csLog.AppendFormat(TEXT("Total Drop Error : %12s Packet\r\n"), ToCommaText(m_pMulti2Converter->GetContinuityErrNum()));
	csLog.AppendFormat(TEXT("Total Scrambling : %12s Packet\r\n"), ToCommaText(m_pMulti2Converter->GetScramblePacketNum()));
	csLog.AppendFormat(TEXT("\r\nTotal Input      : %12s Packet\r\n"), ToCommaText(m_pMulti2Converter->GetInputPacketNum()));

	if(m_bCfgOutput){
		csLog.AppendFormat(TEXT("Total Output     : %12s Packet\r\n"), ToCommaText(m_pMulti2Converter->GetOutputPacketNum()));
		}

	csLog.AppendFormat(TEXT("Packet Stride    : %12lu Byte\r\n"), m_pMulti2Converter->GetPacketStride());

	if(m_bCfgB25){
		csLog.AppendFormat(TEXT("\r\nECM Process      : %12s\r\n"), ToCommaText(m_pMulti2Converter->GetEcmProcessNum()));

		if(m_bCfgEmm){
			csLog.AppendFormat(TEXT("EMM Process      : %12s\r\n"), ToCommaText(m_pMulti2Converter->GetEmmProcessNum()));
			}
		}

	// ���O�t�@�C���I�[�v��
	CFile LogFile;
	if(!LogFile.Open(m_csDstPath.Left(m_csDstPath.GetLength() - 3) + TEXT(".txt"), CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite))return;

#ifdef _UNICODE
	// �e�L�X�g���}���`�o�C�g������ɕϊ�
	DWORD dwWriteSize = ::WideCharToMultiByte(CP_OEMCP, WC_COMPOSITECHECK, csLog, csLog.GetLength(), NULL, 0UL, NULL, NULL);
	char *szMbText = (char *)::alloca(dwWriteSize + 1UL);
	dwWriteSize = ::WideCharToMultiByte(CP_OEMCP, WC_COMPOSITECHECK, csLog, csLog.GetLength(), szMbText, csLog.GetLength() * 2UL, NULL, NULL);

	// ���O�t�@�C���o��
	LogFile.Write(szMbText, dwWriteSize);
#else
	// ���O�t�@�C���o��
	LogFile.Write((LPCTSTR)csLog, csLog.GetLength());
#endif

	LogFile.Close();
}

void CMulti2DecWinDlg::OnMulti2ConverterEvent(IMulti2Converter *pMulti2Converter, const DWORD dwEventID, PVOID pParam)
{
	// �O���t�O�̃X���b�h�ŏ������邽�߃��_�C���N�g����
	PostMessage(WM_CONVERTER_EVENT, (WPARAM)dwEventID, (LPARAM)pParam);
}

LRESULT CMulti2DecWinDlg::OnConverterEvent(WPARAM wParam, LPARAM lParam)
{
	const DWORD dwEventID = (const DWORD)wParam;
	const PVOID pParam = (const PVOID)lParam;

	switch(dwEventID){
		case IMulti2Converter::EID_CONV_START:
			// �R���o�[�g�J�n
			m_FileList.SetItemText(m_dwSrcIndex, 2, TEXT("�������D�D�D"));
			break;

		case IMulti2Converter::EID_CONV_END:
			// ���O�t�@�C���o��
			if(m_bCfgLogEnable)OutputLogFile();

			// �R���o�[�g�I��
			m_pMulti2Converter->EndConvert();

			if(m_bIsConvert){
				// ���̃t�@�C���R���o�[�g�J�n
				m_FileList.SetItemText(m_dwSrcIndex, 2, TEXT("����"));
				m_FileList.SetCheck(m_dwSrcIndex, FALSE);
				m_bIsConvert = StartNextFile();
				}
			else{
				// �R���o�[�g���~
				m_FileList.SetItemText(m_dwSrcIndex, 2, TEXT("�������~"));
				}

			UpdateCmdState();
			break;

		case IMulti2Converter::EID_CONV_PROGRESS:
			// �i���󋵍X�V
			UpdateProgress((DWORD)pParam);
			break;

		case IMulti2Converter::EID_CONV_ERROR:
			// �G���[����
			m_bIsConvert = FALSE;
			m_pMulti2Converter->EndConvert();
			::AfxMessageBox(TEXT("�������ɃG���[�������������ߏ������~���܂���"));
			UpdateCmdState();
			break;
		}

	return (LRESULT)TRUE;
}

void CMulti2DecWinDlg::OnOK(void)
{
	// �������Ȃ�
}

void CMulti2DecWinDlg::OnCancel(void)
{
	// �������Ȃ�
	if(m_bIsConvert){
		if(::AfxMessageBox(TEXT("�����𒆎~���ďI�����܂����H"), MB_YESNO) != IDYES)return;

		m_bIsConvert = FALSE;
		m_pMulti2Converter->EndConvert();
		}

	CDialog::OnCancel();
}

BOOL CMulti2DecWinDlg::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
{
	// �c�[���`�b�v�ɕ\�����镶���񃊃\�[�X���Z�b�g
	TOOLTIPTEXT *pTTText = (TOOLTIPTEXT *)pNMHDR;
	pTTText->lpszText = MAKEINTRESOURCE(pNMHDR->idFrom);
	pTTText->hinst = ::AfxGetResourceHandle();

	return TRUE;
}

void CMulti2DecWinDlg::OnCmdStart()
{
	// TODO: �����ɃR�}���h �n���h�� �R�[�h��ǉ����܂��B

	if(!m_FileList.GetItemCount()){
		::AfxMessageBox(TEXT("�t�@�C�����o�^����Ă��܂���"));
		return;
		}

	// �\�[�X�t�@�C�����𐔂���
	int iSrcCount = 0;

	for(int iPos = 0 ; iPos < m_FileList.GetItemCount() ; iPos++){
		if(m_FileList.GetCheck(iPos)){
			m_FileList.SetItemText(iPos, 2, TEXT("�����҂�"));
			iSrcCount++;
			}
		}

	if(!iSrcCount){
		::AfxMessageBox(TEXT("�t�@�C����1���I������Ă��܂���"));
		return;
		}

	// �R���o�[�g�J�n
	m_bIsConvert = (StartNextFile())? TRUE : FALSE;
	UpdateCmdState();
}

void CMulti2DecWinDlg::OnCmdStop()
{
	// TODO: �����ɃR�}���h �n���h�� �R�[�h��ǉ����܂��B
	if(m_bIsConvert){
		m_bIsConvert = FALSE;
		m_pMulti2Converter->EndConvert();
		::AfxMessageBox(TEXT("�����𒆎~���܂���"));
		}
}

void CMulti2DecWinDlg::OnCmdLog()
{
	// TODO: �����ɃR�}���h �n���h�� �R�[�h��ǉ����܂��B

	// �I������Ă���A�C�e�����擾
	POSITION pPos = m_FileList.GetFirstSelectedItemPosition();
	if(!pPos)return;

	int iItem = m_FileList.GetNextSelectedItem(pPos);
	if(iItem < 0)return;

	// ���O�t�@�C���̃p�X����
	CString csLogPath = m_FileList.GetItemText(iItem, 0);
	csLogPath = csLogPath.Left(csLogPath.GetLength() - 3) + TEXT("_dec.txt");

	if(m_bCfgPathEnable){
		csLogPath = m_csOutputPath + csLogPath.Mid(csLogPath.ReverseFind(TEXT('\\')) + 1);
		}

	// ���O���J��
	::ShellExecute(NULL, TEXT("edit"), csLogPath, NULL, NULL, SW_SHOWNORMAL);
}

void CMulti2DecWinDlg::OnCmdExit()
{
	// TODO: �����ɃR�}���h �n���h�� �R�[�h��ǉ����܂��B
	OnCancel();
}

void CMulti2DecWinDlg::OnCmdListAdd()
{
	// TODO: �����ɃR�}���h �n���h�� �R�[�h��ǉ����܂��B
	CFileDialog Dlg(TRUE, TEXT("ts"), NULL, OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, TEXT("TS�t�@�C�� (*.ts)|*.ts||"));
	if(Dlg.DoModal() != IDOK)return;

	// �t�@�C�����X�g�ɒǉ�
	POSITION pPos = Dlg.GetStartPosition();

	while(pPos){
		// �t�@�C�����X�g�ɒǉ�
		AddSourceFile(Dlg.GetNextPathName(pPos));
		}
}

void CMulti2DecWinDlg::OnCmdListRemove()
{
	// TODO: �����ɃR�}���h �n���h�� �R�[�h��ǉ����܂��B

	// �I������Ă���擪�̃A�C�e�����擾
	POSITION pPos = m_FileList.GetFirstSelectedItemPosition();
	if(!pPos)return;

	int iItem = m_FileList.GetNextSelectedItem(pPos);
	if(iItem < 0)return;

	// �I������Ă���A�C�e�����폜
	while(pPos = m_FileList.GetFirstSelectedItemPosition()){
		m_FileList.DeleteItem(m_FileList.GetNextSelectedItem(pPos));
		}

	// ���̈ʒu�̃A�C�e����I������
	if((iItem + 1) >= m_FileList.GetItemCount())iItem = m_FileList.GetItemCount() - 1;
	m_FileList.SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}

void CMulti2DecWinDlg::OnCmdListClear()
{
	// TODO: �����ɃR�}���h �n���h�� �R�[�h��ǉ����܂��B
	m_FileList.DeleteAllItems();
}

void CMulti2DecWinDlg::OnCmdListUp()
{
	// TODO: �����ɃR�}���h �n���h�� �R�[�h��ǉ����܂��B

	// �I������Ă���擪�̃A�C�e�����擾
	POSITION pPos = m_FileList.GetFirstSelectedItemPosition();
	if(!pPos)return;

	const int iItem = m_FileList.GetNextSelectedItem(pPos);
	if(iItem <= 0)return;

	// �S�A�C�e���̑I���N���A����
	while(pPos = m_FileList.GetFirstSelectedItemPosition()){
		m_FileList.SetItemState(m_FileList.GetNextSelectedItem(pPos), 0UL, LVIS_SELECTED | LVIS_FOCUSED);
		}

	// �ړ��p�̃A�C�e����ǉ�����
	m_FileList.InsertItem(iItem + 1, m_FileList.GetItemText(iItem - 1, 0));

	// ���e���R�s�[����
	m_FileList.SetCheck(iItem + 1, m_FileList.GetCheck(iItem - 1));
	for(int iSubItem = 1 ; iSubItem <= 9 ; iSubItem++){
		m_FileList.SetItemText(iItem + 1, iSubItem, m_FileList.GetItemText(iItem - 1, iSubItem));
		}

	// �ړ���A�C�e�����폜����
	m_FileList.DeleteItem(iItem - 1);

	// �A�C�e���I������
	m_FileList.SetItemState(iItem - 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}

void CMulti2DecWinDlg::OnCmdListDown()
{
	// TODO: �����ɃR�}���h �n���h�� �R�[�h��ǉ����܂��B

	// �I������Ă���擪�̃A�C�e�����擾
	POSITION pPos = m_FileList.GetFirstSelectedItemPosition();
	if(!pPos)return;

	const int iItem = m_FileList.GetNextSelectedItem(pPos);
	if((iItem  + 1) >= m_FileList.GetItemCount())return;

	// �S�A�C�e���̑I���N���A����
	while(pPos = m_FileList.GetFirstSelectedItemPosition()){
		m_FileList.SetItemState(m_FileList.GetNextSelectedItem(pPos), 0UL, LVIS_SELECTED | LVIS_FOCUSED);
		}

	// �ړ��p�̃A�C�e����ǉ�����
	m_FileList.InsertItem(iItem, m_FileList.GetItemText(iItem + 1, 0));

	// ���e���R�s�[����
	m_FileList.SetCheck(iItem, m_FileList.GetCheck(iItem + 2));
	for(int iSubItem = 1 ; iSubItem <= 9 ; iSubItem++){
		m_FileList.SetItemText(iItem, iSubItem, m_FileList.GetItemText(iItem + 2, iSubItem));
		}

	// �ړ���A�C�e�����폜����
	m_FileList.DeleteItem(iItem + 2);

	// �A�C�e���I������
	m_FileList.SetItemState(iItem + 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}

void CMulti2DecWinDlg::OnCmdPreDescramble()
{
	// TODO: �����ɃR�}���h �n���h�� �R�[�h��ǉ����܂��B
	m_ModeCombo.SetCurSel(0);
	OnCbnSelchangeModeCombo();
}

void CMulti2DecWinDlg::OnCmdPreErrCheck()
{
	// TODO: �����ɃR�}���h �n���h�� �R�[�h��ǉ����܂��B
	m_ModeCombo.SetCurSel(1);
	OnCbnSelchangeModeCombo();
}

void CMulti2DecWinDlg::OnCmdPreEmmUpdate()
{
	// TODO: �����ɃR�}���h �n���h�� �R�[�h��ǉ����܂��B
	m_ModeCombo.SetCurSel(2);
	OnCbnSelchangeModeCombo();
}

void CMulti2DecWinDlg::OnCmdCfgOutput()
{
	// TODO: �����ɃR�}���h �n���h�� �R�[�h��ǉ����܂��B
	m_bCfgOutput = (m_bCfgOutput)? FALSE : TRUE;
	UpdateCmdState();
}

void CMulti2DecWinDlg::OnCmdCfgB25()
{
	// TODO: �����ɃR�}���h �n���h�� �R�[�h��ǉ����܂��B
	m_bCfgB25 = (m_bCfgB25)? FALSE : TRUE;
	UpdateCmdState();
}

void CMulti2DecWinDlg::OnCmdCfgEmm()
{
	// TODO: �����ɃR�}���h �n���h�� �R�[�h��ǉ����܂��B
	m_bCfgEmm = (m_bCfgEmm)? FALSE : TRUE;
	UpdateCmdState();
}

void CMulti2DecWinDlg::OnCmdCfgDiscard()
{
	// TODO: �����ɃR�}���h �n���h�� �R�[�h��ǉ����܂��B
	m_bCfgDiscard = (m_bCfgDiscard)? FALSE : TRUE;
	UpdateCmdState();
}

void CMulti2DecWinDlg::OnCmdCfgNull()
{
	// TODO: �����ɃR�}���h �n���h�� �R�[�h��ǉ����܂��B
	m_bCfgNull = (m_bCfgNull)? FALSE : TRUE;
	UpdateCmdState();
}

void CMulti2DecWinDlg::OnCmdCfgLogEnable()
{
	// TODO: �����ɃR�}���h �n���h�� �R�[�h��ǉ����܂��B
	m_bCfgLogEnable = (m_bCfgLogEnable)? FALSE : TRUE;
	UpdateCmdState();
}

void CMulti2DecWinDlg::OnCmdCfgPathSpec()
{
	// TODO: �����ɃR�}���h �n���h�� �R�[�h��ǉ����܂��B
	if(m_bCfgPathEnable){
		m_bCfgPathEnable = FALSE;
		}
	else{
		if(m_csOutputPath.IsEmpty())OnCmdCfgBrowse();
		if(!m_csOutputPath.IsEmpty())m_bCfgPathEnable = TRUE;
		}

	UpdateCmdState();
}

void CMulti2DecWinDlg::OnCmdCfgBrowse()
{
	// TODO: �����ɃR�}���h �n���h�� �R�[�h��ǉ����܂��B
	OpenBrowseDialog(GetSafeHwnd(), m_csOutputPath.GetBuffer(_MAX_PATH + 1), TEXT("�o�͐�t�H���_�I��"));
	m_csOutputPath.ReleaseBuffer();
}

void CMulti2DecWinDlg::OnCmdAbout()
{
	// TODO: �����ɃR�}���h �n���h�� �R�[�h��ǉ����܂��B
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}

void CMulti2DecWinDlg::OnCmdListAllSel()
{
	// TODO: �����ɃR�}���h �n���h�� �R�[�h��ǉ����܂��B

	// �S�A�C�e����I������
	for(int iItem = 0 ; iItem < m_FileList.GetItemCount() ; iItem++){
		m_FileList.SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		}
}

void CMulti2DecWinDlg::OnCmdListAllcheck()
{
	// TODO: �����ɃR�}���h �n���h�� �R�[�h��ǉ����܂��B
	for(int iItem = 0 ; iItem < m_FileList.GetItemCount() ; iItem++){
		m_FileList.SetCheck(iItem, TRUE);
		}
}

void CMulti2DecWinDlg::OnCbnSelchangeModeCombo()
{
	// TODO: �����ɃR���g���[���ʒm�n���h�� �R�[�h��ǉ����܂��B
	switch(m_ModeCombo.GetCurSel()){
		case 0 : m_bCfgOutput = TRUE;  m_bCfgB25 = TRUE; m_bCfgEmm = FALSE;		break;
		case 1 : m_bCfgOutput = FALSE; m_bCfgB25 = FALSE;						break;
		case 2 : m_bCfgOutput = FALSE; m_bCfgB25 = TRUE; m_bCfgEmm = TRUE;		break;
		}

	UpdateCmdState();
}

void CMulti2DecWinDlg::OnNMRclickFileList(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: �����ɃR���g���[���ʒm�n���h�� �R�[�h��ǉ����܂��B
	CMenu MainMenu, *pSubMenu;
	if(!MainMenu.LoadMenu(IDR_LISTMENU))return;
	if(!(pSubMenu = MainMenu.GetSubMenu(0)))return;

	// ���j���[��\��
	CPoint ptCursor;
	::GetCursorPos(&ptCursor);

	pSubMenu->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON, ptCursor.x, ptCursor.y, this);
	pSubMenu->DestroyMenu();

	*pResult = 0;
}

void CMulti2DecWinDlg::OnNMDblclkFileList(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: �����ɃR���g���[���ʒm�n���h�� �R�[�h��ǉ����܂��B
	*pResult = 0;

	OnCmdLog();
}
