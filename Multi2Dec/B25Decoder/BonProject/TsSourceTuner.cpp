// TsSourceTuner.cpp: TS�\�[�X�`���[�i�f�R�[�_
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "TsSourceTuner.h"


/////////////////////////////////////////////////////////////////////////////
// �t�@�C�����[�J���萔�ݒ�
/////////////////////////////////////////////////////////////////////////////

#define TSBUFFER_SIZE	0x10000UL	// 64KB


/////////////////////////////////////////////////////////////////////////////
// TS�\�[�X�`���[�i�f�R�[�_
/////////////////////////////////////////////////////////////////////////////

const DWORD CTsSourceTuner::OpenTuner(LPCTSTR lpszBCId)
{
	// ��U�N���[�Y
	CloseTuner();

	IHalDevice *pHalDevice = NULL;

	try{
		// �`���[�i�̃C���X�^���X�𐶐�����
		if(!(pHalDevice = ::BON_SAFE_CREATE<IHalDevice *>(lpszBCId)))throw (DWORD)EC_INTERNAL_ERR;

		// �f�o�C�X�^�C�v���`�F�b�N
		if(pHalDevice->GetDeviceType() != ::BON_NAME_TO_GUID(TEXT("IHalTsTuner")))throw (DWORD)EC_INTERNAL_ERR;

		// �f�o�C�X�̑��݂��`�F�b�N
		if(!pHalDevice->GetTotalDeviceNum())throw (DWORD)EC_DEVICE_NONE;

		// �f�o�C�X�̋󂫂��`�F�b�N
		if(pHalDevice->GetActiveDeviceNum() >= pHalDevice->GetTotalDeviceNum())throw (DWORD)EC_DEVICE_FULL;
	
		// IHalTsTuner�C���^�t�F�[�X�擾
		if(!(m_pHalTsTuner = dynamic_cast<IHalTsTuner *>(pHalDevice)))throw (DWORD)EC_INTERNAL_ERR;

		// TS�o�b�t�@�C���X�^���X����
		if(!(m_pTsBuffer = ::BON_SAFE_CREATE<IMediaData *>(TEXT("CMediaData"))))throw (DWORD)EC_INTERNAL_ERR;
	
		// TS�o�b�t�@�T�C�Y�m��
		if(m_pTsBuffer->GetBuffer(TSBUFFER_SIZE) != TSBUFFER_SIZE)throw (DWORD)EC_INTERNAL_ERR;

		// �`���[�i���I�[�v��
		if(!m_pHalTsTuner->OpenTuner())throw (DWORD)EC_OPEN_FAILED;

		// TS��M�X���b�h�N��
		if(!m_TsRecvThread.StartThread(this, &CTsSourceTuner::TsRecvThread))throw (DWORD)EC_INTERNAL_ERR;
		}
	catch(DWORD dwErrCode){
		// �G���[����
		BON_SAFE_RELEASE(pHalDevice);
		m_pHalTsTuner = NULL;
		
		CloseTuner();
		return dwErrCode;
		}

	return EC_OPEN_SUCCESS;
}

const bool CTsSourceTuner::CloseTuner(void)
{
	// TS��M�X���b�h��~
	if(m_bIsPlaying)StopDecoder();

	m_bIsPlaying = false;
	m_TsRecvThread.EndThread(true);

	// �`���[�i���N���[�Y
	if(m_pHalTsTuner)m_pHalTsTuner->CloseTuner();

	// TS�o�b�t�@���J��
	BON_SAFE_RELEASE(m_pTsBuffer);

	// �`���[�i�C���X�^���X�J��
	BON_SAFE_RELEASE(m_pHalTsTuner);
	
	return true;
}

const DWORD CTsSourceTuner::GetDeviceName(LPTSTR lpszName)
{
	// IHalDevice�C���^�t�F�[�X�擾
	IHalDevice * const pHalDevice = dynamic_cast<IHalDevice *>(m_pHalTsTuner);

	// �f�o�C�X����Ԃ�
	return (pHalDevice)? pHalDevice->GetDeviceName(lpszName) : 0UL;
}

const DWORD CTsSourceTuner::GetTuningSpaceName(const DWORD dwSpace, LPTSTR lpszName)
{
	// �`���[�i���I�[�v������Ă��Ȃ�
	if(!m_pHalTsTuner)return 0UL;

	// �`���[�j���O��Ԗ���Ԃ�
	return m_pHalTsTuner->EnumTuningSpace(dwSpace, lpszName);
}

const DWORD CTsSourceTuner::GetChannelName(const DWORD dwSpace, const DWORD dwChannel, LPTSTR lpszName)
{
	// �`���[�i���I�[�v������Ă��Ȃ�
	if(!m_pHalTsTuner)return 0UL;

	// �`�����l������Ԃ�
	return m_pHalTsTuner->EnumChannelName(dwSpace, dwChannel, lpszName);
}

const DWORD CTsSourceTuner::GetCurSpace(void)
{
	// ���݂̃`���[�j���O��Ԃ�Ԃ�
	return (m_pHalTsTuner)? m_pHalTsTuner->GetCurSpace() : 0UL;
}

const DWORD CTsSourceTuner::GetCurChannel(void)
{
	// ���݂̃`�����l����Ԃ�
	return (m_pHalTsTuner)? m_pHalTsTuner->GetCurChannel() : 0UL;
}

const bool CTsSourceTuner::SetChannel(const DWORD dwSpace, const DWORD dwChannel)
{
	// �`���[�i���I�[�v������Ă��Ȃ�
	if(!m_pHalTsTuner)return 0UL;

	// �`�����l����ύX����
	if(!m_pHalTsTuner->SetChannel(dwSpace, dwChannel))return false;
	
	// ���ʃf�R�[�_�����Z�b�g����
	ResetDecoder();

	return true;
}

const float CTsSourceTuner::GetSignalLevel(void)
{
	// �`���[�i���I�[�v������Ă��Ȃ�
	if(!m_pHalTsTuner)return 0UL;

	// �M�����x����Ԃ�
	return m_pHalTsTuner->GetSignalLevel();
}

IHalTsTuner * CTsSourceTuner::GetHalTsTuner(void)
{
	// �`���[�i�f�o�C�X��Ԃ�
	return m_pHalTsTuner;
}

CTsSourceTuner::CTsSourceTuner(IBonObject *pOwner)
	: CMediaDecoder(pOwner, 0UL, 1UL, new CSmartLock)
	, m_pHalTsTuner(NULL)
	, m_pTsBuffer(NULL)
	, m_bIsPlaying(false)
{

}

CTsSourceTuner::~CTsSourceTuner(void)
{
	CloseTuner();
}

const bool CTsSourceTuner::OnPlay(void)
{
	// �`���[�i���I�[�v������Ă��Ȃ�
	if(!m_pHalTsTuner)return false;

	// ���łɍĐ���
	if(m_bIsPlaying)return true;

	// �������̃X�g���[����j������
	m_pHalTsTuner->PurgeStream();

	// �X�g���[�����Đ���Ԃɂ���
	m_bIsPlaying = true;

	return true;
}

const bool CTsSourceTuner::OnStop(void)
{
	// �`���[�i���I�[�v������Ă��Ȃ�
	if(!m_pHalTsTuner)return false;

	// �X�g���[�����~��Ԃɂ���
	m_bIsPlaying = false;
	
	return true;
}

const bool CTsSourceTuner::OnReset(void)
{
	// �������̃X�g���[����j������
	if(m_bIsPlaying && m_pHalTsTuner)m_pHalTsTuner->PurgeStream();

	return true;
}

void CTsSourceTuner::TsRecvThread(CSmartThread<CTsSourceTuner> *pThread, bool &bKillSignal, PVOID pParam)
{
	BYTE *pStreamData = NULL;
	DWORD dwStreamSize = 0UL;
	DWORD dwRemainNum = 0UL;

	// �`���[�i����TS�f�[�^�����o���X���b�h
	while(!bKillSignal){
		
		// �����ȗ����̂��߃|�[�����O�������̗p����
		do{
			if(m_pHalTsTuner->GetStream(&pStreamData, &dwStreamSize, &dwRemainNum)){
				if(pStreamData && dwStreamSize){
					OnTsStream(pStreamData, dwStreamSize);
					}
				}
			}
		while(dwRemainNum);

		// �E�F�C�g
		::Sleep(1UL);
		}
}

const bool CTsSourceTuner::PurgeStream(void)
{
	// �`���[�i���I�[�v������Ă��Ȃ�
	if(!m_pHalTsTuner)return false;

	// �������̃X�g���[����j������
	m_pHalTsTuner->PurgeStream();

	// ���ʃf�R�[�_�����Z�b�g����
	ResetDecoder();

	return true;
}

void CTsSourceTuner::OnTsStream(BYTE *pStreamData, DWORD dwStreamSize)
{
	CBlockLock AutoLock(m_pLock);

	// ��~���͉������Ȃ�
	if(!m_bIsPlaying)return;

	// �f�[�^���o�b�t�@�ɃZ�b�g����
	m_pTsBuffer->SetData(pStreamData, dwStreamSize);

	// ���̃f�R�[�_�Ƀf�[�^��n��
	OutputMedia(m_pTsBuffer);
}
