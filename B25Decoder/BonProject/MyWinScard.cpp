// MyWinScard.cpp: winscard.hのラッパー関数群
//
/////////////////////////////////////////////////////////////////////////////

// Linuxのlibpcsclite-devのwinscard.hはLONGをlong、DWORDをunsigned longとしていて
// LP64環境ではかなり使い勝手が悪いのでここで差異を吸収する

#include <winscard.h>
#include "MyWinScard.h"

static_assert(sizeof(MyWinScard::SCARDCONTEXT) == sizeof(::SCARDCONTEXT));
static_assert(sizeof(MyWinScard::SCARDHANDLE) == sizeof(::SCARDHANDLE));

namespace MyWinScard
{
int SCardEstablishContext(unsigned int dwScope,
                          /*@null@*/ const void *pvReserved1,
                          /*@null@*/ const void *pvReserved2,
                          /*@out@*/ SCARDCONTEXT *phContext)
{
	return (int)::SCardEstablishContext(dwScope, pvReserved1, pvReserved2, (::SCARDCONTEXT *)phContext);
}

int SCardReleaseContext(SCARDCONTEXT hContext)
{
	return (int)::SCardReleaseContext(hContext);
}

int SCardConnect(SCARDCONTEXT hContext,
                 LPCSTR szReader,
                 unsigned int dwShareMode,
                 unsigned int dwPreferredProtocols,
                 /*@out@*/ SCARDHANDLE *phCard,
                 /*@out@*/ unsigned int *pdwActiveProtocol)
{
	DWORD dwActiveProtocol;
	LONG nRet = ::SCardConnect(hContext, szReader, dwShareMode, dwPreferredProtocols,
	                           (::SCARDHANDLE *)phCard, pdwActiveProtocol ? &dwActiveProtocol : NULL);
	if (pdwActiveProtocol) {
		*pdwActiveProtocol = (unsigned int)dwActiveProtocol;
	}
	return (int)nRet;
}

int SCardDisconnect(SCARDHANDLE hCard,
                    unsigned int dwDisposition)
{
	return (int)::SCardDisconnect(hCard, dwDisposition);
}

int SCardTransmit(SCARDHANDLE hCard,
                  const void *pioSendPci,
                  const BYTE *pbSendBuffer,
                  unsigned int cbSendLength,
                  /*@out@*/ void *pioRecvPci,
                  /*@out@*/ BYTE *pbRecvBuffer,
                  unsigned int *pcbRecvLength)
{
	if (pioSendPci || pioRecvPci) {
		return (int)SCARD_E_INVALID_PARAMETER;
	}
	DWORD cbRecvLength;
	LONG nRet = ::SCardTransmit(hCard, SCARD_PCI_T1, pbSendBuffer, cbSendLength, NULL,
	                            pbRecvBuffer, pcbRecvLength ? &cbRecvLength : NULL);
	if (pcbRecvLength) {
		*pcbRecvLength = (unsigned int)cbRecvLength;
	}
	return (int)nRet;
}

int SCardListReaders(SCARDCONTEXT hContext,
                     /*@null@*/ /*@out@*/ LPCSTR mszGroups,
                     /*@null@*/ /*@out@*/ LPSTR mszReaders,
                     /*@out@*/ unsigned int *pcchReaders)
{
	DWORD cchReaders;
	LONG nRet = ::SCardListReaders(hContext, mszGroups, mszReaders, pcchReaders ? &cchReaders : NULL);
	if (pcchReaders) {
		*pcchReaders = (unsigned int)cchReaders;
	}
	return (int)nRet;
}
}
