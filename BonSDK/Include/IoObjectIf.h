// IoObjectIf.h: 入出力オブジェクトインタフェース
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// 標準ファイルストレージインタフェース
/////////////////////////////////////////////////////////////////////////////

class ISmartFile :				public IBonObject
{
public:
	enum
	{
		MF_READ			= 0x00000001UL,		// 読み出しモード
		MF_WRITE		= 0x00000002UL		// 書き込みモード
	};

	virtual const bool Open(LPCTSTR lpszFileName, const DWORD dwModeFlags = MF_READ, const DWORD dwBuffSize = 0UL) = 0;
	virtual void Close(void) = 0;

	virtual const DWORD ReadData(BYTE *pBuff, const DWORD dwLen) = 0;
	virtual const DWORD ReadData(BYTE *pBuff, const DWORD dwLen, const ULONGLONG llPos) = 0;

	virtual const DWORD WriteData(const BYTE *pBuff, const DWORD dwLen) = 0;
	virtual const DWORD WriteData(const BYTE *pBuff, const DWORD dwLen, const ULONGLONG llPos) = 0;

	virtual const bool FlushData(void) = 0;

	virtual const bool SeekPos(const ULONGLONG llPos) = 0;
	virtual const ULONGLONG GetPos(void) = 0;

	virtual const ULONGLONG GetLength(void) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// 標準ソケットインタフェース
/////////////////////////////////////////////////////////////////////////////

class ISmartSocket :			public IBonObject
{
public:
	enum	// HostNameToIPAddr()戻り値
	{
		INVALID_IPADDR = 0xFFFFFFFFUL
	};

	enum	// GetLastError()戻り値
	{
		EC_NO_ERROR			= 0UL,	// 正常終了
		EC_SOCK_ERROR		= 1UL,	// ソケットエラー
		EC_TIME_OUT			= 2UL,	// タイムアウト
		EC_INVALID_SOCK		= 3UL,	// ソケットが無効
		EC_INVALID_PARAM	= 4UL	// パラメータが無効
	};

// TCPソケット
	virtual const bool Connect(LPCTSTR lpszHostName, const WORD wPort, const DWORD dwTimeout = 0UL) = 0;
	virtual const bool Connect(const DWORD dwIP, const WORD wPort, const DWORD dwTimeout = 0UL) = 0;

	virtual const bool Listen(const WORD wPort) = 0;
	virtual ISmartSocket * Accept(const DWORD dwTimeout = 0UL) = 0;

	virtual const bool Send(const BYTE *pData, const DWORD dwLen, const DWORD dwTimeout = 0UL) = 0;
	virtual const bool Recv(BYTE *pData, const DWORD dwLen, const DWORD dwTimeout = 0UL) = 0;
	virtual const DWORD SendOnce(const BYTE *pData, const DWORD dwLen, const DWORD dwTimeout = 0UL) = 0;
	virtual const DWORD RecvOnce(BYTE *pData, const DWORD dwLen, const DWORD dwTimeout = 0UL) = 0;

	virtual const bool GetLocalAddr(DWORD *pdwIP, WORD *pwPort = NULL) = 0;
	virtual const bool GetPeerAddr(DWORD *pdwIP, WORD *pwPort = NULL) = 0;

// UDPソケット
	virtual const bool Bind(const WORD wPort = 0U) = 0;

	virtual const DWORD SendTo(const DWORD dwIP, const WORD wPort, const BYTE *pData, const DWORD dwLen, const DWORD dwTimeout = 0UL) = 0;
	virtual const DWORD SendTo(LPCTSTR lpszHostName, const WORD wPort, const BYTE *pData, const DWORD dwLen, const DWORD dwTimeout = 0UL) = 0;
	virtual const DWORD RecvFrom(BYTE *pData, const DWORD dwLen, DWORD *pdwIP = NULL, WORD *pwPort = NULL, const DWORD dwTimeout = 0UL) = 0;

// 共通
	virtual void Close(void) = 0;

	virtual const DWORD HostNameToIPAddr(LPCTSTR lpszHostName) = 0;
	virtual const DWORD IPAddrToHostName(LPTSTR lpszHostName, const DWORD dwIP) = 0;

	virtual const DWORD GetLastError(void) = 0;
};
