// MyWinScard.h: winscard.hのラッパー関数群
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stddef.h>
#include <stdint.h>

#ifndef SCARD_PROTOCOL_UNDEFINED
#define SCARD_PROTOCOL_UNDEFINED 0x00000000
#endif

#ifndef SCARD_PROTOCOL_T1
#define SCARD_PROTOCOL_T1 0x00000002
#endif

#ifndef SCARD_SHARE_SHARED
#define SCARD_SHARE_SHARED 2
#endif

#ifndef SCARD_S_SUCCESS
#define SCARD_S_SUCCESS 0
#endif

#ifndef SCARD_E_INVALID_PARAMETER
#define SCARD_E_INVALID_PARAMETER (int)(0x80100004U)
#endif

#ifndef SCARD_LEAVE_CARD
#define SCARD_LEAVE_CARD 0
#endif

#ifndef SCARD_SCOPE_USER
#define SCARD_SCOPE_USER 0
#endif

#ifndef SCARD_PCI_T1
#define SCARD_PCI_T1 NULL
#endif

namespace MyWinScard
{
typedef intptr_t SCARDCONTEXT;
typedef intptr_t SCARDHANDLE;

int SCardEstablishContext(unsigned int dwScope,
                          /*@null@*/ const void *pvReserved1,
                          /*@null@*/ const void *pvReserved2,
                          /*@out@*/ SCARDCONTEXT *phContext);

int SCardReleaseContext(SCARDCONTEXT hContext);

int SCardConnect(SCARDCONTEXT hContext,
                 LPCSTR szReader,
                 unsigned int dwShareMode,
                 unsigned int dwPreferredProtocols,
                 /*@out@*/ SCARDHANDLE *phCard,
                 /*@out@*/ unsigned int *pdwActiveProtocol);

int SCardDisconnect(SCARDHANDLE hCard,
                    unsigned int dwDisposition);

int SCardTransmit(SCARDHANDLE hCard,
                  const void *pioSendPci, // SCARD_PCI_T1であること
                  const BYTE *pbSendBuffer,
                  unsigned int cbSendLength,
                  /*@out@*/ void *pioRecvPci, // NULLであること
                  /*@out@*/ BYTE *pbRecvBuffer,
                  unsigned int *pcbRecvLength);

int SCardListReaders(SCARDCONTEXT hContext,
                     /*@null@*/ /*@out@*/ LPCSTR mszGroups,
                     /*@null@*/ /*@out@*/ LPSTR mszReaders,
                     /*@out@*/ unsigned int *pcchReaders);
}
