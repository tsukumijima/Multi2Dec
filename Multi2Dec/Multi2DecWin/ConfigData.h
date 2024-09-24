// ConfigData.h: CConfigData クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////
// セクションの定義
//////////////////////////////////////////////////////////////////////

enum
{
	CONFSECT_GENERAL
};


//////////////////////////////////////////////////////////////////////
// 基底インタフェース(設定保存用に不揮発性のデータ型を実現する)
//////////////////////////////////////////////////////////////////////

class CConfigData  
{
public:
	CConfigData(const UINT nSection, LPCTSTR lpszKeyName);
	virtual ~CConfigData(void);

	virtual void Load(void) = 0;
	virtual void Save(void) = 0;

protected:
	// 設定ファイルのパスとセクションを管理する
	class CConfigFile
	{
	public:
		CConfigFile(void);
		virtual ~CConfigFile(void);

		LPCTSTR GetPath(void);
		LPCTSTR GetSection(const UINT nSection);

	protected:
		TCHAR m_szConfigPath[_MAX_PATH + 1];
	};

	static CConfigFile m_ConfigFile;

	// アイテム毎の情報(セクション、キー)
	CString m_csKeyName;
	UINT m_nSection;
};


//////////////////////////////////////////////////////////////////////
// 不揮発性 BOOL データ型
//////////////////////////////////////////////////////////////////////

class CConfigBool : public CConfigData
{
public:
	CConfigBool(const UINT nSection, LPCTSTR lpszKeyName, const BOOL bDefault = FALSE);
	virtual ~CConfigBool(void);

	virtual void Load(void);
	virtual void Save(void);

	operator const BOOL () const;
	operator BOOL & ();
	operator BOOL * ();
	const BOOL & operator = (const BOOL bValue);

private:
	BOOL m_bDefault;
	BOOL m_bValue;
};


//////////////////////////////////////////////////////////////////////
// 不揮発性 DWORD データ型
//////////////////////////////////////////////////////////////////////

class CConfigDword : public CConfigData
{
public:
	CConfigDword(const UINT nSection, LPCTSTR lpszKeyName, const DWORD dwDefault = 0, const DWORD dwRangeMax = 0xFFFFFFFF, const DWORD dwRangeMin = 0);
	virtual ~CConfigDword(void);

	virtual void Load(void);
	virtual void Save(void);

	operator DWORD () const;
	operator DWORD & ();
	operator DWORD * ();
	const DWORD & operator = (const DWORD dwValue);

private:
	DWORD m_dwDefault;
	DWORD m_dwRangeMax;
	DWORD m_dwRangeMin;
	DWORD m_dwValue;
};


//////////////////////////////////////////////////////////////////////
// 不揮発性 CString データ型
//////////////////////////////////////////////////////////////////////

class CConfigString : public CConfigData, public CString
{
public:
	CConfigString(const UINT nSection, LPCTSTR lpszKeyName, LPCTSTR lpszDefault = NULL, const DWORD dwLengthMax = 1024);
	virtual ~CConfigString(void);

	virtual void Load(void);
	virtual void Save(void);

	const CString& operator = (const CString& stringSrc);
	const CString& operator = (TCHAR ch);
	const CString& operator = (const unsigned char* psz);
	const CString& operator = (LPCWSTR lpsz);
	const CString& operator = (LPCSTR lpsz);

private:
	CString m_csDefault;
	DWORD m_dwLengthMax;
};
