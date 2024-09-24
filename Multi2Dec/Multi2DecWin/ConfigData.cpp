// ConfigData.cpp: ConfigData クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ConfigData.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#pragma warning(disable: 4996) // warning C4996: "This function or variable may be unsafe. Consider using _wsplitpath_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details"


//////////////////////////////////////////////////////////////////////
// 定数定義
//////////////////////////////////////////////////////////////////////

// セクションの定義
static const TCHAR SectionArray[][256] =
{
	TEXT("General")		// CONFSECT_GENERAL
};


//////////////////////////////////////////////////////////////////////
// CConfigData のインプリメンテーション
//////////////////////////////////////////////////////////////////////

CConfigData::CConfigData(const UINT nSection, LPCTSTR lpszKeyName)
	: m_nSection(nSection)
	, m_csKeyName(lpszKeyName)
{

}

CConfigData::~CConfigData(void)
{

}


//////////////////////////////////////////////////////////////////////
// CConfigFile のインプリメンテーション
//////////////////////////////////////////////////////////////////////

CConfigData::CConfigFile::CConfigFile(void)
{
	// 設定ファイルのパス名を構築する
	TCHAR szPath[_MAX_PATH + 1] = {TEXT('\0')};
	TCHAR szDrive[_MAX_DRIVE + 1] = {TEXT('\0')};
	TCHAR szDir[_MAX_DIR + 1] = {TEXT('\0')};
	TCHAR szFile[_MAX_FNAME + 1] = {TEXT('\0')};

	::GetModuleFileName(NULL, szPath, sizeof(szPath) - 1);
	::_tsplitpath(szPath, szDrive, szDir, szFile, NULL);
	::_tmakepath(m_szConfigPath, szDrive, szDir, szFile, TEXT("ini"));
}

CConfigData::CConfigFile::~CConfigFile(void)
{

}

LPCTSTR CConfigData::CConfigFile::GetPath(void)
{
	// 設定ファイルのパスを返す
	return m_szConfigPath;	
}

LPCTSTR CConfigData::CConfigFile::GetSection(const UINT nSection)
{
	// セクション名を返す
	return SectionArray[nSection];
}


//////////////////////////////////////////////////////////////////////
// CConfigData のインプリメンテーション
//////////////////////////////////////////////////////////////////////

CConfigData::CConfigFile CConfigData::m_ConfigFile;


//////////////////////////////////////////////////////////////////////
// CConfigBool のインプリメンテーション
//////////////////////////////////////////////////////////////////////

CConfigBool::CConfigBool(const UINT nSection, LPCTSTR lpszKeyName, const BOOL bDefault)
	: CConfigData(nSection, lpszKeyName)
	, m_bValue(bDefault)
	, m_bDefault(bDefault)
{
	// データ読み出し
	Load();
}

CConfigBool::~CConfigBool(void)
{
	// データ保存
	Save();
}

void CConfigBool::Load(void)
{
	// データロード
	m_bValue = (::GetPrivateProfileInt(m_ConfigFile.GetSection(m_nSection), m_csKeyName, (m_bDefault)? 1 : 0, m_ConfigFile.GetPath()) == 0)? FALSE : TRUE;
}

void CConfigBool::Save(void)
{
	// データセーブ
	::WritePrivateProfileString(m_ConfigFile.GetSection(m_nSection), m_csKeyName, (m_bValue)? TEXT("1") : TEXT("0"), m_ConfigFile.GetPath());
}

CConfigBool::operator const BOOL () const
{
	// キャスト演算子
	return m_bValue;
}

CConfigBool::operator BOOL & ()
{
	// キャスト演算子
	return m_bValue;
}

CConfigBool::operator BOOL * ()
{
	// キャスト演算子
	return &m_bValue;
}

const BOOL & CConfigBool::operator = (const BOOL bValue)
{
	// 代入演算子
	return (m_bValue = bValue);
}


//////////////////////////////////////////////////////////////////////
// CConfigDword のインプリメンテーション
//////////////////////////////////////////////////////////////////////

CConfigDword::CConfigDword(const UINT nSection, LPCTSTR lpszKeyName, const DWORD dwDefault, const DWORD dwRangeMax, const DWORD dwRangeMin)
	: CConfigData(nSection, lpszKeyName)
	, m_dwValue(dwDefault)
	, m_dwDefault(dwDefault)
	, m_dwRangeMax(dwRangeMax)
	, m_dwRangeMin(dwRangeMin)
{
	// データ読み出し
	Load();
}

CConfigDword::~CConfigDword(void)
{
	// データ保存
	Save();
}

void CConfigDword::Load(void)
{
	// データロード
	const DWORD dwValue = ::GetPrivateProfileInt(m_ConfigFile.GetSection(m_nSection), m_csKeyName, m_dwDefault, m_ConfigFile.GetPath());

	// クリップ
	if(dwValue > m_dwRangeMax)m_dwValue = m_dwRangeMax;
	else if(dwValue < m_dwRangeMin)m_dwValue = m_dwRangeMin;
	else m_dwValue = dwValue;
}

void CConfigDword::Save(void)
{
	// データセーブ
	DWORD dwValue;

	// クリップ
	if(m_dwValue > m_dwRangeMax)dwValue = m_dwRangeMax;
	else if(m_dwValue < m_dwRangeMin)dwValue = m_dwRangeMin;
	else dwValue = m_dwValue;

	CString csValue;
	csValue.Format(TEXT("%lu"), dwValue);

	::WritePrivateProfileString(m_ConfigFile.GetSection(m_nSection), m_csKeyName, csValue, m_ConfigFile.GetPath());
}

CConfigDword::operator DWORD () const
{
	// キャスト演算子
	return m_dwValue;
}

CConfigDword::operator DWORD & ()
{
	// キャスト演算子
	return m_dwValue;
}

CConfigDword::operator DWORD * ()
{
	// キャスト演算子
	return &m_dwValue;
}

const DWORD & CConfigDword::operator = (const DWORD dwValue)
{
	// 代入演算子
	return (m_dwValue = dwValue);
}


//////////////////////////////////////////////////////////////////////
// CConfigString のインプリメンテーション
//////////////////////////////////////////////////////////////////////

CConfigString::CConfigString(const UINT nSection, LPCTSTR lpszKeyName, LPCTSTR lpszDefault, const DWORD dwLengthMax)
	: CConfigData(nSection, lpszKeyName)
	, CString(lpszDefault)
	, m_csDefault(lpszDefault)
	, m_dwLengthMax(1024UL)
{
	// データ読み出し
	Load();
}

CConfigString::~CConfigString(void)
{
	// データ保存
	Save();
}

void CConfigString::Load(void)
{
	// データロード
	const DWORD dwLength = ::GetPrivateProfileString(m_ConfigFile.GetSection(m_nSection), m_csKeyName, m_csDefault, GetBuffer(m_dwLengthMax + 1), m_dwLengthMax, m_ConfigFile.GetPath());
	ReleaseBuffer();

	if(!dwLength || !GetLength()){
		*this = m_csDefault;
		}
}

void CConfigString::Save(void)
{
	// データセーブ
	*this = Left(m_dwLengthMax);
	
	::WritePrivateProfileString(m_ConfigFile.GetSection(m_nSection), m_csKeyName, (CString &)*this, m_ConfigFile.GetPath());
}

const CString& CConfigString::operator = (const CString& stringSrc)
{
	// 代入演算子
	(CString &)*this = stringSrc;
	return (CString &)*this;
}

const CString& CConfigString::operator = (TCHAR ch)
{
	// 代入演算子
	(CString &)*this = ch;
	return (CString &)*this;
}

const CString& CConfigString::operator = (const unsigned char* psz)
{
	// 代入演算子
	(CString &)*this = psz;
	return (CString &)*this;
}

const CString& CConfigString::operator = (LPCWSTR lpsz)
{
	// 代入演算子
	(CString &)*this = lpsz;
	return (CString &)*this;
}

const CString& CConfigString::operator = (LPCSTR lpsz)
{
	// 代入演算子
	(CString &)*this = lpsz;
	return (CString &)*this;
}
