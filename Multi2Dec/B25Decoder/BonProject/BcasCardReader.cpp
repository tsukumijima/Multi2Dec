// BcasCardReader.cpp: B-CAS�J�[�h���[�_�N���X
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include <Memory>
#include <TChar.h>
#include "BcasCardReader.h"


#pragma comment(lib, "WinScard.lib")


/////////////////////////////////////////////////////////////////////////////
// �t�@�C�����[�J���萔�ݒ�
/////////////////////////////////////////////////////////////////////////////

// �f�o�C�X��
#define DEVICENAME	TEXT("B-CAS �ėp�X�}�[�g�J�[�h���[�_")


/////////////////////////////////////////////////////////////////////////////
// �ėp�X�}�[�g�J�[�h���[�_�p B-CAS�J�[�h�C���^�t�F�[�X�����N���X
/////////////////////////////////////////////////////////////////////////////

const BONGUID CBcasCardReader::GetDeviceType(void)
{
	// �f�o�C�X�̃^�C�v��Ԃ�
	return ::BON_NAME_TO_GUID(TEXT("IHalBcasCard"));
}

const DWORD CBcasCardReader::GetDeviceName(LPTSTR lpszName)
{
	// �f�o�C�X����Ԃ�
	if(lpszName)::_tcscpy(lpszName, DEVICENAME);

	return ::_tcslen(DEVICENAME);
}

const DWORD CBcasCardReader::GetTotalDeviceNum(void)
{
	// B-CAS�J�[�h���}������Ă���J�[�h���[�_����Ԃ�

	// �X�}�[�g�J�[�h���[�_��
	if(!EnumBcasCardReader())return 0UL;
	
	SCARDHANDLE hBcasCard;
	DWORD dwBcasCardNum = 0UL;
	DWORD dwActiveProtocol;
	
	// ���������s
	for(DWORD dwIndex = 0UL ; dwIndex < m_CardReaderArray.size() ; dwIndex++){

		hBcasCard = NULL;
		dwActiveProtocol = SCARD_PROTOCOL_UNDEFINED;

		// �J�[�h���[�_�ɐڑ�
		if(::SCardConnect(m_ScardContext, m_CardReaderArray[dwIndex].c_str(), SCARD_SHARE_SHARED, SCARD_PROTOCOL_T1, &hBcasCard, &dwActiveProtocol) == SCARD_S_SUCCESS){
			// �v���g�R���`�F�b�N
			if(dwActiveProtocol == SCARD_PROTOCOL_T1){
				// �������R�}���h���M
				if(InitialSetting(hBcasCard)){
					// B-CAS�J�[�h���C���N�������g
					dwBcasCardNum++;
					}
				}
			// �J�[�h���[�_�ؒf
			::SCardDisconnect(hBcasCard, SCARD_LEAVE_CARD);	
			}
		}

	return dwBcasCardNum;
}

const DWORD CBcasCardReader::GetActiveDeviceNum(void)
{
	// �X�}�[�g�J�[�h���[�_�͋��p�\�Ȃ��ߏ��0��Ԃ�
	return 0UL;
}

const bool CBcasCardReader::OpenCard(void)
{
	// ��U�N���[�Y����
	CloseCard();

	// �X�}�[�g�J�[�h���[�_��
	if(!EnumBcasCardReader())return false;

	DWORD dwActiveProtocol;
	
	// ���������s
	for(DWORD dwIndex = 0UL ; dwIndex < m_CardReaderArray.size() ; dwIndex++){

		m_hBcasCard = NULL;
		dwActiveProtocol = SCARD_PROTOCOL_UNDEFINED;

		// �J�[�h���[�_�ɐڑ�
		if(::SCardConnect(m_ScardContext, m_CardReaderArray[dwIndex].c_str(), SCARD_SHARE_SHARED, SCARD_PROTOCOL_T1, &m_hBcasCard, &dwActiveProtocol) == SCARD_S_SUCCESS){
			// �v���g�R���`�F�b�N
			if(dwActiveProtocol == SCARD_PROTOCOL_T1){
				// �������R�}���h���M
				if(InitialSetting(m_hBcasCard)){
					// B-CAS�J�[�h���C���N�������g
					break;
					}
				}

			// �J�[�h���[�_�ؒf
			::SCardDisconnect(m_hBcasCard, SCARD_LEAVE_CARD);
			m_hBcasCard = NULL;
			}
		}

	return (m_hBcasCard)? true : false;
}

void CBcasCardReader::CloseCard(void)
{
	// �J�[�h���[�_�ؒf
	if(m_hBcasCard){
		::SCardDisconnect(m_hBcasCard, SCARD_LEAVE_CARD);
		m_hBcasCard = NULL;
		}
}

const bool CBcasCardReader::IsInteractive(void)
{
	// �o�����ʐM�̉ۂ�Ԃ�
	return true;
}

const BYTE * CBcasCardReader::GetBcasCardID(void)
{
	// Card ID ��Ԃ�
	return (m_hBcasCard)? m_BcasCardInfo.BcasCardID : NULL;
}

const BYTE * CBcasCardReader::GetInitialCbc(void)
{
	// Descrambler CBC Initial Value ��Ԃ�
	return (m_hBcasCard)? m_BcasCardInfo.InitialCbc : NULL;
}

const BYTE * CBcasCardReader::GetSystemKey(void)
{
	// Descrambling System Key ��Ԃ�
	return (m_hBcasCard)? m_BcasCardInfo.SystemKey : NULL;
}

const BYTE * CBcasCardReader::GetKsFromEcm(const BYTE *pEcmData, const DWORD dwEcmSize)
{
	static const BYTE abyEcmReceiveCmd[] = {0x90U, 0x34U, 0x00U, 0x00U};

	// �uECM Receive Command�v����������
	if(!m_hBcasCard){
		m_dwEcmError = EC_NO_ERROR;
		return NULL;
		}

	// ECM�T�C�Y���`�F�b�N
	if(!pEcmData || (dwEcmSize < 30UL) || (dwEcmSize > 256UL)){
		m_dwEcmError = EC_BADARGUMENT;
		return NULL;
		}

	// �o�b�t�@����
	DWORD dwRecvSize = 0UL;
	BYTE SendData[1024];
	BYTE RecvData[1024];
	::ZeroMemory(RecvData, sizeof(RecvData));

	// �R�}���h�\�z
	::CopyMemory(SendData, abyEcmReceiveCmd, sizeof(abyEcmReceiveCmd));			// CLA, INS, P1, P2
	SendData[sizeof(abyEcmReceiveCmd)] = (BYTE)dwEcmSize;						// COMMAND DATA LENGTH
	::CopyMemory(&SendData[sizeof(abyEcmReceiveCmd) + 1], pEcmData, dwEcmSize);	// ECM
	SendData[sizeof(abyEcmReceiveCmd) + dwEcmSize + 1] = 0x00U;					// RESPONSE DATA LENGTH

	// �R�}���h���M
	if(!TransmitCommand(m_hBcasCard, SendData, sizeof(abyEcmReceiveCmd) + dwEcmSize + 2UL, RecvData, sizeof(RecvData), &dwRecvSize)){
		::ZeroMemory(&m_EcmStatus, sizeof(m_EcmStatus));
		m_dwEcmError = EC_TRANSMIT_ERROR;
		return NULL;
		}

	// �T�C�Y�`�F�b�N
	if(dwRecvSize != 25UL){
		::ZeroMemory(&m_EcmStatus, sizeof(m_EcmStatus));
		m_dwEcmError = EC_TRANSMIT_ERROR;
		return NULL;
		}	
	
	// ���X�|���X���
	::CopyMemory(m_EcmStatus.KsData, &RecvData[6], sizeof(m_EcmStatus.KsData));

	// ���^�[���R�[�h���
	switch(((WORD)RecvData[4] << 8) | (WORD)RecvData[5]){
		// �w���ς�: ������
		case 0x0200U :	// �㕥��PPV
		case 0x0400U :	// �O����PPV
		case 0x0800U :	// �e�B�A
		
		// �w����: �v���r���[��
		case 0x4480U :	// �㕥��PPV
		case 0x4280U :	// �O����PPV
			m_dwEcmError = EC_NO_ERROR;
			return m_EcmStatus.KsData;
		
		// ��L�ȊO(�����s��)
		default :
			m_dwEcmError = EC_NOT_CONTRACTED;
			return NULL;
		}
}

const DWORD CBcasCardReader::GetEcmError(void)
{
	// ECM�����G���[�R�[�h��Ԃ�
	return m_dwEcmError;
}

const bool CBcasCardReader::SendEmmSection(const BYTE *pEmmData, const DWORD dwEmmSize)
{
	static const BYTE abyEmmReceiveCmd[] = {0x90U, 0x36U, 0x00U, 0x00U};

	// �uEMM Receive Command�v����������
	if(!m_hBcasCard)return false;

	// EMM�T�C�Y���`�F�b�N
	if(!pEmmData || (dwEmmSize < 17UL) || (dwEmmSize > 263UL))return false;

	// �o�b�t�@����
	DWORD dwRecvSize = 0UL;
	BYTE SendData[1024];
	BYTE RecvData[1024];
	::ZeroMemory(RecvData, sizeof(RecvData));

	// �R�}���h�\�z
	::CopyMemory(SendData, abyEmmReceiveCmd, sizeof(abyEmmReceiveCmd));			// CLA, INS, P1, P2
	SendData[sizeof(abyEmmReceiveCmd)] = (BYTE)dwEmmSize;						// COMMAND DATA LENGTH
	::CopyMemory(&SendData[sizeof(abyEmmReceiveCmd) + 1], pEmmData, dwEmmSize);	// EMM
	SendData[sizeof(abyEmmReceiveCmd) + dwEmmSize + 1] = 0x00U;					// RESPONSE DATA LENGTH

	// �R�}���h���M
	if(!TransmitCommand(m_hBcasCard, SendData, sizeof(abyEmmReceiveCmd) + dwEmmSize + 2UL, RecvData, sizeof(RecvData), &dwRecvSize))return false;

	// �T�C�Y�`�F�b�N
	if(dwRecvSize != 8UL)return false;
	
	// ���^�[���R�[�h���
	switch(((WORD)RecvData[4] << 8) | (WORD)RecvData[5]){
		case 0x2100U :	// ����I��
			return true;

		case 0xA102U :	// ��^�p(�^�p�O�v���g�R���ԍ�)
		case 0xA107U :	// �Z�L�����e�B�G���[(EMM������G���[)
			return false;
		
		// ��L�ȊO(��O)
		default :
			return false;
		}
}

const WORD CBcasCardReader::GetPowerCtrlInfo(POWERCTRLINFO *pPowerCtrlInfo)
{
	BYTE abyReqPowerCtrlCmd[] = {0x90U, 0x80U, 0x00U, 0x00U, 0x01U, 0x00U, 0x00U};

	// �ʓd��������擾����
	if(!m_hBcasCard)return 0U;

	// �o�b�t�@����
	DWORD dwRecvSize = 0UL;
	BYTE RecvData[1024];
	::ZeroMemory(RecvData, sizeof(RecvData));

	// �R�}���h���M
	if(!TransmitCommand(m_hBcasCard, abyReqPowerCtrlCmd, sizeof(abyReqPowerCtrlCmd), RecvData, sizeof(RecvData), &dwRecvSize))return 0U;

	// ��M�T�C�Y�`�F�b�N
	if(dwRecvSize != 20UL)return 0U;		

	// ���^�[���R�[�h���
	if((((WORD)RecvData[4] << 8) | (WORD)RecvData[5]) != 0x2100U)return 0U;

	// ���X�|���X���
	const WORD wInfoNum = (WORD)RecvData[7] + 1U;
	if(!pPowerCtrlInfo)return wInfoNum;		// ������NULL�̂Ƃ��͕K�v�ȃo�b�t�@����Ԃ�

	CTsTime TsTime;

	for(WORD wIndex = 0U ; wIndex < wInfoNum ; wIndex++){
		// �R�}���h�ݒ�
		abyReqPowerCtrlCmd[5] = (BYTE)wIndex;

		// �R�}���h���M
		if(!TransmitCommand(m_hBcasCard, abyReqPowerCtrlCmd, sizeof(abyReqPowerCtrlCmd), RecvData, sizeof(RecvData), &dwRecvSize))return 0U;

		// ��M�T�C�Y�`�F�b�N
		if(dwRecvSize != 20UL)return 0U;		

		// ���^�[���R�[�h���
		if((((WORD)RecvData[4] << 8) | (WORD)RecvData[5]) != 0x2100U)return 0U;
		
		// ���X�|���X���
		TsTime.ClearTime();
		CAribTime::SplitAribMjd(((WORD)RecvData[9] << 8) | (WORD)RecvData[10], &TsTime.wYear, &TsTime.wMonth, &TsTime.wDay, &TsTime.wDayOfWeek);
		TsTime -= (DWORD)RecvData[11] * 24UL * 60UL * 60UL;
		pPowerCtrlInfo[wIndex].stStartTime = TsTime;
		TsTime += (DWORD)RecvData[12] * 24UL * 60UL * 60UL;
		pPowerCtrlInfo[wIndex].stEndTime = TsTime;
		pPowerCtrlInfo[wIndex].byDurTime = RecvData[13];
		pPowerCtrlInfo[wIndex].wNtID = (((WORD)RecvData[14] << 8) | (WORD)RecvData[15]);
		pPowerCtrlInfo[wIndex].wTsID = (((WORD)RecvData[16] << 8) | (WORD)RecvData[17]);
		}

	return wInfoNum;
}

CBcasCardReader::CBcasCardReader(IBonObject *pOwner)
	: CBonObject(pOwner)
	, m_ScardContext(NULL)
	, m_hBcasCard(NULL)
	, m_dwEcmError(EC_NOT_OPEN)
{
	// ������ԏ�����
	::ZeroMemory(&m_BcasCardInfo, sizeof(m_BcasCardInfo));
	::ZeroMemory(&m_EcmStatus, sizeof(m_EcmStatus));

	// ���\�[�X�}�l�[�W���R���e�L�X�g�m��
	::SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &m_ScardContext);
}

CBcasCardReader::~CBcasCardReader(void)
{
	CloseCard();

	// ���\�[�X�}�l�[�W���R���e�L�X�g�̊J��
	if(m_ScardContext)::SCardReleaseContext(m_ScardContext);
}

const bool CBcasCardReader::EnumBcasCardReader(void)
{
	// �J�[�h���[�_��񋓂���
	DWORD dwBuffSize = 0UL;
	
	// �o�b�t�@�T�C�Y�擾
	if(::SCardListReaders(m_ScardContext, NULL, NULL, &dwBuffSize) != SCARD_S_SUCCESS)return false;

	// �o�b�t�@�m��
	std::auto_ptr<TCHAR> szReaders(new TCHAR[dwBuffSize]);

	// �J�[�h���[�_��
	if(::SCardListReaders(m_ScardContext, NULL, szReaders.get(), &dwBuffSize) != SCARD_S_SUCCESS)return false;

	// �J�[�h���[�_���ۑ�
	LPTSTR lpszCurReader = szReaders.get();
	m_CardReaderArray.clear();
			
	while(*lpszCurReader){
		m_CardReaderArray.push_back(lpszCurReader);
		lpszCurReader += m_CardReaderArray.back().length() + 1UL;
		}

	return (m_CardReaderArray.size())? true : false;
}

const bool CBcasCardReader::TransmitCommand(const SCARDHANDLE hBcasCard, const BYTE *pSendData, const DWORD dwSendSize, BYTE *pRecvData, const DWORD dwMaxRecv, DWORD *pdwRecvSize)
{
	DWORD dwRecvSize = dwMaxRecv;

	// �f�[�^����M
	DWORD dwReturn = ::SCardTransmit(hBcasCard, SCARD_PCI_T1, pSendData, dwSendSize, NULL, pRecvData, &dwRecvSize);
	
	// ��M�T�C�Y�i�[
	if(pdwRecvSize)*pdwRecvSize = dwRecvSize;

	return (dwReturn == SCARD_S_SUCCESS)? true : false;
}

const bool CBcasCardReader::InitialSetting(const SCARDHANDLE hBcasCard)
{
	static const BYTE abyInitSettingCmd[] = {0x90U, 0x30U, 0x00U, 0x00U, 0x00U};

	// �uInitial Setting Conditions Command�v����������
	if(!hBcasCard)return false;

	// �o�b�t�@����
	DWORD dwRecvSize = 0UL;
	BYTE RecvData[1024];
	::ZeroMemory(RecvData, sizeof(RecvData));
	
	// �R�}���h���M
	if(!TransmitCommand(hBcasCard, abyInitSettingCmd, sizeof(abyInitSettingCmd), RecvData, sizeof(RecvData), &dwRecvSize))return false;

	// ��M�T�C�Y�`�F�b�N
	if(dwRecvSize < 57UL)return false;		

	// ���X�|���X���
	::CopyMemory(m_BcasCardInfo.BcasCardID, &RecvData[8],   6UL);	// +8	Card ID
	::CopyMemory(m_BcasCardInfo.SystemKey,  &RecvData[16], 32UL);	// +16	Descrambling system key
	::CopyMemory(m_BcasCardInfo.InitialCbc, &RecvData[48],  8UL);	// +48	Descrambler CBC initial value

	// ECM�X�e�[�^�X������
	::ZeroMemory(&m_EcmStatus, sizeof(m_EcmStatus));

	return true;
}
