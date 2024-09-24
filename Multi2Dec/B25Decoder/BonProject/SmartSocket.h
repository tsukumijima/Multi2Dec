// SmartSocket.h: TCP/UDPソケットクラス
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


#include <WinSock2.h>


/////////////////////////////////////////////////////////////////////////////
// TCP/UDPソケットクラス
/////////////////////////////////////////////////////////////////////////////

class CSmartSocket :	public CBonObject,
						public ISmartSocket
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CSmartSocket)

// ISmartSocket
	virtual const bool Connect(LPCTSTR lpszHostName, const WORD wPort, const DWORD dwTimeout = 0UL);
	virtual const bool Connect(const DWORD dwIP, const WORD wPort, const DWORD dwTimeout = 0UL);
	virtual const bool Listen(const WORD wPort);
	virtual ISmartSocket * Accept(const DWORD dwTimeout = 0UL);
	virtual const bool Send(const BYTE *pData, const DWORD dwLen, const DWORD dwTimeout = 0UL);
	virtual const bool Recv(BYTE *pData, const DWORD dwLen, const DWORD dwTimeout = 0UL);
	virtual const DWORD SendOnce(const BYTE *pData, const DWORD dwLen, const DWORD dwTimeout = 0UL);
	virtual const DWORD RecvOnce(BYTE *pData, const DWORD dwLen, const DWORD dwTimeout = 0UL);
	virtual const bool GetLocalAddr(DWORD *pdwIP, WORD *pwPort = NULL);
	virtual const bool GetPeerAddr(DWORD *pdwIP, WORD *pwPort = NULL);

	virtual const bool Bind(const WORD wPort = 0U);
	virtual const DWORD SendTo(const DWORD dwIP, const WORD wPort, const BYTE *pData, const DWORD dwLen, const DWORD dwTimeout = 0UL);
	virtual const DWORD SendTo(LPCTSTR lpszHostName, const WORD wPort, const BYTE *pData, const DWORD dwLen, const DWORD dwTimeout = 0UL);
	virtual const DWORD RecvFrom(BYTE *pData, const DWORD dwLen, DWORD *pdwIP = NULL, WORD *pwPort = NULL, const DWORD dwTimeout = 0UL);

	virtual void Close(void);
	virtual const DWORD HostNameToIPAddr(LPCTSTR lpszHostName);
	virtual const DWORD IPAddrToHostName(LPTSTR lpszHostName, const DWORD dwIP);
	virtual const DWORD GetLastError(void);

// CSmartSocket
	CSmartSocket(IBonObject *pOwner);
	virtual ~CSmartSocket(void);

protected:
	static const bool InitializeWinSock(void);
	static const bool ReleaseWinSock(void);

	virtual const bool SetAsyncMode(const bool bEnable);
	virtual const bool WaitAsyncTimeout(const bool bIsRead, const DWORD dwTimeout);

	virtual const bool SetSendTimeout(const DWORD dwTimeout);
	virtual const bool SetRecvTimeout(const DWORD dwTimeout);

	enum SocetType{SOCKTYPE_NON, SOCKTYPE_TCP, SOCKTYPE_UDP};

	SOCKET m_Socket;				// ソケットハンドル
	SocetType m_SockType;			// ソケットタイプ
	DWORD m_dwLastError;			// 最終エラーコード

	static DWORD dwInstanceNum;		// インスタンスの数
};
