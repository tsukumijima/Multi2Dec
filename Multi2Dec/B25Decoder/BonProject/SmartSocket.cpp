// SmartSocket.cpp: TCP/UDP�\�P�b�g�N���X
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "SmartSocket.h"
#include <Locale.h>
#include "TChar.h"


#pragma comment(lib, "Ws2_32.lib")


//////////////////////////////////////////////////////////////////////
// �}�N����`
//////////////////////////////////////////////////////////////////////

#define VERIFY_VALIDSOCK(R)	if(m_Socket != INVALID_SOCKET){m_dwLastError = EC_INVALID_SOCK; return (R);}
#define VERIFY_TCPSOCK(R)	if((m_Socket == INVALID_SOCKET) || (m_SockType != SOCKTYPE_TCP)){m_dwLastError = EC_INVALID_SOCK; return (R);}
#define VERIFY_UDPSOCK(R)	if((m_Socket == INVALID_SOCKET) || (m_SockType != SOCKTYPE_UDP)){m_dwLastError = EC_INVALID_SOCK; return (R);}


/////////////////////////////////////////////////////////////////////////////
// TCP/UDP�\�P�b�g�N���X
/////////////////////////////////////////////////////////////////////////////

DWORD CSmartSocket::dwInstanceNum = 0UL;

const bool CSmartSocket::Connect(LPCTSTR lpszHostName, const WORD wPort, const DWORD dwTimeout)
{
	// �z�X�g���`�F�b�N
	if(!lpszHostName){
		m_dwLastError = EC_INVALID_PARAM;
		return false;
		}
		
	// �A�h���X������IP�A�h���X�擾
	const DWORD dwIP = HostNameToIPAddr(lpszHostName);

	if(dwIP == INVALID_IPADDR){
		m_dwLastError = EC_SOCK_ERROR;
		return false;		
		}
	
	// �R�l�N�g
	return Connect(dwIP, wPort, dwTimeout);
}

const bool CSmartSocket::Connect(const DWORD dwIP, const WORD wPort, const DWORD dwTimeout)
{
	// ��U�N���[�Y
	Close();

	// �\�P�b�g�쐬
	if((m_Socket = ::socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET){
		m_dwLastError = EC_SOCK_ERROR;
		return false;		
		}

	// �A�h���X�ݒ�
	SOCKADDR_IN SockAddr;
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = ::htons(wPort);
	SockAddr.sin_addr.S_un.S_addr = ::htonl(dwIP);

	// �����R�l�N�g
	if(!dwTimeout){
		if(::connect(m_Socket, (PSOCKADDR)&SockAddr, sizeof(sockaddr))){
			Close();
			m_dwLastError = EC_SOCK_ERROR;
			return false;		
			}
	
		m_dwLastError = EC_NO_ERROR;
		return true;
		}

	// �񓯊��R�l�N�g
	try{
		// �񓯊��ɐ؂�ւ�
		if(!SetAsyncMode(true))throw;

		// �R�l�N�g
		if(::connect(m_Socket, (PSOCKADDR)&SockAddr, sizeof(sockaddr)) != SOCKET_ERROR)throw;
		if(::WSAGetLastError() != WSAEWOULDBLOCK)throw;

		// �^�C���A�E�g����
		if(WaitAsyncTimeout(false, dwTimeout)){
			Close();
			m_dwLastError = EC_TIME_OUT;
			return false;
			}

		// �����ɐ؂�ւ�
		if(!SetAsyncMode(false))throw;
		}
	catch(...){
		// �G���[����
		Close();
		m_dwLastError = EC_SOCK_ERROR;
		return false;
		}

	m_SockType = SOCKTYPE_TCP;
	m_dwLastError = EC_NO_ERROR;

	return true;
}

const bool CSmartSocket::Listen(const WORD wPort)
{
	// ��U�N���[�Y
	Close();
	
	// �\�P�b�g�쐬
	if((m_Socket = ::socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET){
		m_dwLastError = EC_SOCK_ERROR;
		return false;		
		}

	// �A�h���X�ݒ�
	SOCKADDR_IN SockAddr;
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = ::htons(wPort);
	SockAddr.sin_addr.S_un.S_addr = INADDR_ANY;

	try{
		// �o�C���h
		if(::bind(m_Socket, (PSOCKADDR)&SockAddr, sizeof(SockAddr)) == SOCKET_ERROR)throw;

		// �ڑ��󂯓���
		if(::listen(m_Socket, 5) == SOCKET_ERROR)throw;
		}
	catch(...){
		// �G���[����
		Close();
		m_dwLastError = EC_SOCK_ERROR;
		return false;
		}

	m_SockType = SOCKTYPE_TCP;
	m_dwLastError = EC_NO_ERROR;

	return true;
}

ISmartSocket * CSmartSocket::Accept(const DWORD dwTimeout)
{
	VERIFY_TCPSOCK(NULL);
	
	SOCKADDR_IN AddrIn;
	::ZeroMemory(&AddrIn, sizeof(AddrIn));
	int iAddrLen = sizeof(AddrIn);

	// �R�l�N�g�҂�
	if(dwTimeout){
		if(WaitAsyncTimeout(true, dwTimeout)){
			// �^�C���A�E�g
			m_dwLastError = EC_TIME_OUT;
			return NULL;
			}
		}

	// �R�l�N�g�󂯓���
	SOCKET SockIn = ::accept(m_Socket, (sockaddr *)&AddrIn, &iAddrLen);

	if(SockIn == INVALID_SOCKET){
		m_dwLastError = EC_SOCK_ERROR;
		return NULL;
		}

	CSmartSocket *pNewSock = new CSmartSocket(NULL);
	::BON_ASSERT(pNewSock != NULL);
	
	pNewSock->m_Socket = SockIn;
	pNewSock->m_SockType = SOCKTYPE_TCP;

	m_dwLastError = EC_NO_ERROR;

	return pNewSock;
}

const bool CSmartSocket::Send(const BYTE *pData, const DWORD dwLen, const DWORD dwTimeout)
{
	VERIFY_TCPSOCK(false);

	if(!pData || !dwLen){
		m_dwLastError = EC_INVALID_PARAM;
		return false;
		}

	// �w��T�C�Y���M
	DWORD dwReturn = 0UL, dwSend = 0UL;

	do{
		if(!(dwReturn = SendOnce(&pData[dwSend], dwLen - dwSend, dwTimeout)))return false;
		}
	while((dwSend += dwReturn) < dwLen);

	return true;
}

const bool CSmartSocket::Recv(BYTE *pData, const DWORD dwLen, const DWORD dwTimeout)
{
	VERIFY_TCPSOCK(false);

	if(!pData || !dwLen){
		m_dwLastError = EC_INVALID_PARAM;
		return false;
		}

	// �w��T�C�Y��M
	DWORD dwReturn = 0UL, dwRecv = 0UL;

	do{
		if(!(dwReturn = RecvOnce(&pData[dwRecv], dwLen - dwRecv, dwTimeout)))return FALSE;
		}
	while((dwRecv += dwReturn) < dwLen);

	return true;
}

const DWORD CSmartSocket::SendOnce(const BYTE *pData, const DWORD dwLen, const DWORD dwTimeout)
{
	VERIFY_TCPSOCK(false);

	if(!pData || !dwLen){
		m_dwLastError = EC_INVALID_PARAM;
		return false;
		}
	
	// �^�C���A�E�g�ݒ�
	if(!SetSendTimeout(dwTimeout)){
		m_dwLastError = EC_SOCK_ERROR;
		return 0UL;
		}
	
	// ���M
	const int iReturn = ::send(m_Socket, (const char *)pData, dwLen, 0);

	if((iReturn == SOCKET_ERROR) || !iReturn){
		if(::WSAGetLastError() == WSAETIMEDOUT){
			m_dwLastError = EC_TIME_OUT;
			return 0UL;
			}
		else{
			m_dwLastError = EC_SOCK_ERROR;
			return 0UL;
			}
		}
		
	m_dwLastError = EC_NO_ERROR;
		
	return (DWORD)iReturn;
}

const DWORD CSmartSocket::RecvOnce(BYTE *pData, const DWORD dwLen, const DWORD dwTimeout)
{
	VERIFY_TCPSOCK(false);
	
	if(!pData || !dwLen){
		m_dwLastError = EC_INVALID_PARAM;
		return FALSE;
		}

	// �^�C���A�E�g�l�ݒ�
	if(!SetRecvTimeout(dwTimeout)){
		m_dwLastError = EC_SOCK_ERROR;
		return 0UL;
		}

	// ��M
	const int iReturn = ::recv(m_Socket, (char *)pData, dwLen, 0);

	if((iReturn == SOCKET_ERROR) || !iReturn){
		if(::WSAGetLastError() == WSAETIMEDOUT){
			m_dwLastError = EC_TIME_OUT;
			return 0UL;
			}
		else{
			m_dwLastError = EC_SOCK_ERROR;
			return 0UL;
			}
		}
		
	m_dwLastError = EC_NO_ERROR;
		
	return (DWORD)iReturn;
}

const bool CSmartSocket::GetLocalAddr(DWORD *pdwIP, WORD *pwPort)
{
	VERIFY_TCPSOCK(false);

	struct sockaddr_in LocalAddr;
	int iAddrLen = sizeof(LocalAddr);
	
	// ���[�J���A�h���X�擾
	if(::getsockname(m_Socket, (struct sockaddr *)&LocalAddr, &iAddrLen) == SOCKET_ERROR){
		m_dwLastError = EC_SOCK_ERROR;
		return false;
		}

	if(pdwIP)*pdwIP = ::htonl(LocalAddr.sin_addr.S_un.S_addr);
	if(pwPort)*pwPort = ::ntohs(LocalAddr.sin_port);

	m_dwLastError = EC_NO_ERROR;

	return true;
}

const bool CSmartSocket::GetPeerAddr(DWORD *pdwIP, WORD *pwPort)
{
	VERIFY_TCPSOCK(false);

	struct sockaddr_in PeerAddr;
	int iAddrLen = sizeof(PeerAddr);
	
	// �s�A�A�h���X�擾
	if(::getpeername(m_Socket, (struct sockaddr *)&PeerAddr, &iAddrLen) == SOCKET_ERROR){
		m_dwLastError = EC_SOCK_ERROR;
		return false;
		}

	if(pdwIP)*pdwIP = ::htonl(PeerAddr.sin_addr.S_un.S_addr);
	if(pwPort)*pwPort = ::ntohs(PeerAddr.sin_port);

	m_dwLastError = EC_NO_ERROR;

	return true;
}

const bool CSmartSocket::Bind(const WORD wPort)
{
	// ��U�N���[�Y
	Close();

	// UDP�\�P�b�g�쐬
	if((m_Socket = ::socket(PF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET){
		m_dwLastError = EC_SOCK_ERROR;
		return false;
		}

	// �A�h���X�ݒ�
	SOCKADDR_IN SockAddr;
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = ::htons(wPort);
	SockAddr.sin_addr.S_un.S_addr = INADDR_ANY;

	// �o�C���h
	if(::bind(m_Socket, (struct sockaddr *)&SockAddr, sizeof(SockAddr)) == SOCKET_ERROR){
		Close();
		m_dwLastError = EC_SOCK_ERROR;
		return false;
		}

	m_dwLastError = EC_NO_ERROR;
	m_SockType = SOCKTYPE_UDP;
	
	return true;
}

const DWORD CSmartSocket::SendTo(const DWORD dwIP, const WORD wPort, const BYTE *pData, const DWORD dwLen, const DWORD dwTimeout)
{
	VERIFY_UDPSOCK(0UL);

	if(!pData || !dwLen){
		m_dwLastError = EC_INVALID_PARAM;
		return 0UL;
		}

	// �A�h���X�ݒ�
	SOCKADDR_IN SockAddr;
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = ::htons(wPort);
	SockAddr.sin_addr.S_un.S_addr = ::htonl(dwIP);

	// ���M�҂�
	if(dwTimeout){
		if(WaitAsyncTimeout(false, dwTimeout)){
			// �^�C���A�E�g
			m_dwLastError = EC_TIME_OUT;
			return 0UL;
			}
		}
		
	// ���M
	int iReturn = sendto(m_Socket, (const char *)pData, dwLen, 0, (struct sockaddr *)&SockAddr, sizeof(SockAddr));
	
	if(iReturn == SOCKET_ERROR){
		m_dwLastError = EC_SOCK_ERROR;
		return 0UL;
		}

	m_dwLastError = EC_NO_ERROR;

	return (DWORD)iReturn;
}

const DWORD CSmartSocket::SendTo(LPCTSTR lpszHostName, const WORD wPort, const BYTE *pData, const DWORD dwLen, const DWORD dwTimeout)
{
	VERIFY_UDPSOCK(0UL);

	if(!lpszHostName || !pData || !dwLen){
		m_dwLastError = EC_INVALID_PARAM;
		return 0UL;
		}
		
	// �A�h���X������IP�A�h���X�擾
	const DWORD dwIP = HostNameToIPAddr(lpszHostName);

	if(dwIP == INVALID_IPADDR){
		m_dwLastError = EC_SOCK_ERROR;
		return 0UL;		
		}
	
	return SendTo(dwIP, wPort, pData, dwLen);
}

const DWORD CSmartSocket::RecvFrom(BYTE *pData, const DWORD dwLen, DWORD *pdwIP, WORD *pwPort, const DWORD dwTimeout)
{
	VERIFY_UDPSOCK(0UL);

	if(!pData || !dwLen){
		m_dwLastError = EC_INVALID_PARAM;
		return 0UL;
		}

	// �A�h���X�ݒ�
	int iSockSize = sizeof(SOCKADDR_IN);
	SOCKADDR_IN SockAddr;
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = 0U;
	SockAddr.sin_addr.S_un.S_addr = 0UL;

	// ��M�҂�
	if(dwTimeout){
		if(WaitAsyncTimeout(true, dwTimeout)){
			// �^�C���A�E�g
			m_dwLastError = EC_TIME_OUT;
			return 0UL;
			}
		}

	// ��M
	const int iReturn = ::recvfrom(m_Socket, (char *)pData, dwLen, 0, (struct sockaddr *)&SockAddr, &iSockSize);

	if(iReturn == SOCKET_ERROR){
		m_dwLastError = EC_SOCK_ERROR;
		return 0UL;
		}

	if(pdwIP)*pdwIP = SockAddr.sin_addr.S_un.S_addr;
	if(pwPort)*pwPort = ntohs(SockAddr.sin_port);

	m_dwLastError = EC_NO_ERROR;

	return (DWORD)iReturn;
}

void CSmartSocket::Close(void)
{
	// �\�P�b�g�N���[�Y
	if(m_Socket != INVALID_SOCKET){
		if(m_SockType == SOCKTYPE_TCP){
			char cData;
			::shutdown(m_Socket, SD_BOTH);
			while(::recv(m_Socket, &cData, 1, 0) == 1);
			}

		::closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
		}
	
	m_SockType = SOCKTYPE_NON;
	m_dwLastError = EC_NO_ERROR;
}

const DWORD CSmartSocket::HostNameToIPAddr(LPCTSTR lpszHostName)
{
	if(!lpszHostName)return 0UL;

#ifdef _UNICODE
	char szHostName[1024] = "";
	::WideCharToMultiByte(CP_OEMCP, WC_COMPOSITECHECK, lpszHostName, ::_tcslen(lpszHostName), szHostName, sizeof(szHostName), NULL, NULL);
#else
	LPCSTR szHostName = lpszHostName;
#endif

	// �z�X�g������IP�A�h���X�擾
	const DWORD dwIP = ::inet_addr(szHostName);

	if(dwIP == INADDR_NONE){
		struct hostent *pHost = ::gethostbyname(szHostName);
		if(!pHost){		
			return INVALID_IPADDR;
			}
		else return *((DWORD *)pHost->h_addr_list[0]);
		}
	else return ::htonl(dwIP);
}

const DWORD CSmartSocket::IPAddrToHostName(LPTSTR lpszHostName, const DWORD dwIP)
{
	// IP�A�h���X����z�X�g���擾
	const DWORD dwNetIP = ::htonl(dwIP);

	struct hostent *pHost = ::gethostbyaddr((const char *)&dwNetIP, sizeof(dwNetIP), AF_INET);
	if(!pHost)return 0UL;

#ifdef _UNICODE
	if(lpszHostName)::MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, pHost->h_name, -1, lpszHostName, ::strlen(pHost->h_name));
#else
	if(lpszHostName)::lstrcpy(lpszHostName, pHost->h_name);
#endif

	return ::strlen(pHost->h_name);
}

const DWORD CSmartSocket::GetLastError(void)
{
	// �Ō�ɔ��������G���[��Ԃ�
	return m_dwLastError;
}

CSmartSocket::CSmartSocket(IBonObject *pOwner)
	: CBonObject(pOwner)
	, m_Socket(INVALID_SOCKET)
	, m_SockType(SOCKTYPE_NON)
	, m_dwLastError(EC_NO_ERROR)
{
	// WinSock2������
	if(!(dwInstanceNum++)){
		::BON_ASSERT(InitializeWinSock());
		}
}

CSmartSocket::~CSmartSocket(void)
{
	// �\�P�b�g�N���[�Y
	Close();

	// WinSock2�J��
	if(!(--dwInstanceNum)){
		ReleaseWinSock();
		}
}

const bool CSmartSocket::InitializeWinSock(void)
{
	WSADATA WsaData;

#ifdef _UNICODE
	::_tsetlocale(LC_ALL, TEXT("japanese"));
#endif

	// WinSock2������
	if(::WSAStartup(MAKEWORD(2U, 2U), &WsaData))return false;
	if((LOBYTE(WsaData.wVersion) != 2U) || (HIBYTE(WsaData.wVersion) != 2U))return false;

	return true;
}

const bool CSmartSocket::ReleaseWinSock(void)
{
	// WinSock2�J��
	return (::WSACleanup())? true : false;
}

const bool CSmartSocket::SetAsyncMode(const bool bEnable)
{
	DWORD dwArg = (bEnable)? 1UL : 0UL;

	// �񓯊��ɐ؂�ւ�
	return (::ioctlsocket(m_Socket, FIONBIO, &dwArg) != SOCKET_ERROR)? true : false;
}

const bool CSmartSocket::WaitAsyncTimeout(const bool bIsRead, const DWORD dwTimeout)
{
	struct timeval TimeVal = {dwTimeout / 1000UL, (dwTimeout % 1000UL) * 1000UL};
	fd_set FdSet;

	FD_ZERO(&FdSet);
	FD_SET(m_Socket, &FdSet);

	int iReturn;

	// ���������҂�
	if(bIsRead){
		// Read
		::BON_ASSERT((iReturn = ::select(32, &FdSet, NULL, NULL, &TimeVal)) != SOCKET_ERROR);
		}
	else{
		// Write
		::BON_ASSERT((iReturn = ::select(32, NULL, &FdSet, NULL, &TimeVal)) != SOCKET_ERROR);
		}
	
	// �^�C���A�E�g����
	return (!iReturn)? true : false;
}

const bool CSmartSocket::SetSendTimeout(const DWORD dwTimeout)
{
	// ���M�^�C���A�E�g�ݒ�
	int iValue  = 0, iSize = sizeof(int);

	if(::getsockopt(m_Socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&iValue, &iSize)){
		Close();
		m_dwLastError = EC_SOCK_ERROR;
		return false;
		}
	
	if(iValue != (int)dwTimeout){
		if(::setsockopt(m_Socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&iValue, sizeof(int))){
			Close();
			m_dwLastError = EC_SOCK_ERROR;
			return false;
			}
		}

	return true;
}

const bool CSmartSocket::SetRecvTimeout(const DWORD dwTimeout)
{
	// ��M�^�C���A�E�g�ݒ�
	int iValue  = 0, iSize = sizeof(int);

	if(::getsockopt(m_Socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&iValue, &iSize)){
		Close();
		m_dwLastError = EC_SOCK_ERROR;
		return false;
		}

	if(iValue != (int)dwTimeout){
		if(::setsockopt(m_Socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&iValue, sizeof(int))){
			Close();
			m_dwLastError = EC_SOCK_ERROR;
			return false;
			}
		}

	return true;
}
