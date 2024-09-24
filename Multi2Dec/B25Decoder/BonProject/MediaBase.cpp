// MediaBase.cpp: Bon���f�B�A���N���X
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "MediaBase.h"


/////////////////////////////////////////////////////////////////////////////
// IMediaData���N���X
/////////////////////////////////////////////////////////////////////////////

#define	MINBUFSIZE	256UL		// �ŏ��o�b�t�@�T�C�Y
#define MINADDSIZE	256UL		// �ŏ��ǉ��m�ۃT�C�Y


BYTE * CMediaData::GetData(void) const
{
	// �o�b�t�@�|�C���^���擾����
	return (m_dwDataSize)? m_pData : NULL;
}

const DWORD CMediaData::GetSize(void) const
{
	// �f�[�^�T�C�Y���擾����
	return m_dwDataSize;
}

void CMediaData::SetAt(const DWORD dwPos, const BYTE byData)
{
	// 1�o�C�g�Z�b�g����
	if(dwPos < m_dwDataSize)m_pData[dwPos] = byData;
}

const BYTE CMediaData::GetAt(const DWORD dwPos) const
{
	// 1�o�C�g�擾����
	return (dwPos < m_dwDataSize)? m_pData[dwPos] : 0x00U;
}

const DWORD CMediaData::SetData(const BYTE *pData, const DWORD dwDataSize)
{
	if(dwDataSize){
		// �o�b�t�@�m��
		GetBuffer(dwDataSize);

		// �f�[�^�Z�b�g
		::CopyMemory(m_pData, pData, dwDataSize);
		}

	// �T�C�Y�Z�b�g
	m_dwDataSize = dwDataSize;
	
	return m_dwDataSize;
}

const DWORD CMediaData::AddData(const BYTE *pData, const DWORD dwDataSize)
{
	if(!dwDataSize)return m_dwDataSize;

	// �o�b�t�@�m��
	GetBuffer(m_dwDataSize + dwDataSize);
	
	// �f�[�^�ǉ�
	::CopyMemory(&m_pData[m_dwDataSize], pData, dwDataSize);

	// �T�C�Y�Z�b�g
	m_dwDataSize += dwDataSize;
	
	return m_dwDataSize;
}

const DWORD CMediaData::AddData(const IMediaData *pData)
{
	return AddData(pData->GetData(), pData->GetSize());
}

const DWORD CMediaData::AddByte(const BYTE byData)
{
	// �o�b�t�@�m��
	GetBuffer(m_dwDataSize + 1UL);
	
	// �f�[�^�ǉ�
	m_pData[m_dwDataSize] = byData;

	// �T�C�Y�X�V
	m_dwDataSize++;

	return m_dwDataSize;
}

const DWORD CMediaData::TrimHead(const DWORD dwTrimSize)
{
	// �f�[�^�擪��؂�l�߂�
	if(!m_dwDataSize || !dwTrimSize){
		// �������Ȃ�
		}
	else if(dwTrimSize >= m_dwDataSize){
		// �S�̂�؂�l�߂�
		m_dwDataSize = 0UL;		
		}
	else{
		// �f�[�^���ړ�����
		::MoveMemory(m_pData, m_pData + dwTrimSize, m_dwDataSize - dwTrimSize);
		m_dwDataSize -= dwTrimSize;
		}

	return m_dwDataSize;
}

const DWORD CMediaData::TrimTail(const DWORD dwTrimSize)
{
	// �f�[�^������؂�l�߂�
	if(!m_dwDataSize || !dwTrimSize){
		// �������Ȃ�
		}
	else if(dwTrimSize >= m_dwDataSize){
		// �S�̂�؂�l�߂�
		m_dwDataSize = 0UL;		
		}
	else{
		// �f�[�^������؂�l�߂�
		m_dwDataSize -= dwTrimSize;
		}

	return m_dwDataSize;
}

const DWORD CMediaData::CopyData(const IMediaData *pSrc)
{
	if(!pSrc)return 0UL;

	// �f�[�^�̃R�s�[
	return SetData(pSrc->GetData(), pSrc->GetSize());
}

const DWORD CMediaData::GetBuffer(const DWORD dwGetSize)
{
	if(dwGetSize <= m_dwBuffSize)return m_dwBuffSize;

	// ���Ȃ��Ƃ��w��T�C�Y���i�[�ł���o�b�t�@���m�ۂ���
	if(!m_pData){
		// �o�b�t�@�m�ۂ܂�
		m_dwBuffSize = (dwGetSize > MINBUFSIZE)? dwGetSize : MINBUFSIZE;
		m_pData = new BYTE [m_dwBuffSize];
		}
	else if(dwGetSize > m_dwBuffSize){
		// �v���T�C�Y�̓o�b�t�@�T�C�Y�𒴂���
		m_dwBuffSize = (dwGetSize > MINBUFSIZE)? dwGetSize : MINBUFSIZE;
		if(m_dwBuffSize < (m_dwDataSize * 2UL))m_dwBuffSize = m_dwDataSize * 2UL;

		BYTE *pNewBuffer = new BYTE [m_dwBuffSize];

		// �f�[�^�R�s�[
		if(m_dwDataSize){
			::CopyMemory(pNewBuffer, m_pData, m_dwDataSize);
			}
		
		// ���o�b�t�@�J��
		delete [] m_pData;

		// �o�b�t�@�����ւ�
		m_pData = pNewBuffer;
		}

	return m_dwBuffSize;
}

const DWORD CMediaData::SetSize(const DWORD dwSetSize)
{
	if(dwSetSize){
		// �o�b�t�@�m��
		GetBuffer(dwSetSize);
		}

	// �T�C�Y�Z�b�g
	m_dwDataSize = dwSetSize;
	
	return m_dwDataSize;
}

const DWORD CMediaData::SetSize(const DWORD dwSetSize, const BYTE byFiller)
{
	// �T�C�Y�Z�b�g
	SetSize(dwSetSize);
	
	// �f�[�^�Z�b�g
	if(dwSetSize){
		::FillMemory(m_pData, dwSetSize, byFiller);
		}
		
	return m_dwDataSize;
}

void CMediaData::ClearSize(void)
{
	// �f�[�^�T�C�Y���N���A����
	m_dwDataSize = 0UL;
}

CMediaData::CMediaData(IBonObject *pOwner)
	: CBonObject()
	, m_dwDataSize(0UL)
	, m_dwBuffSize(0UL)
	, m_pData(NULL)
{
	// ��̃o�b�t�@�𐶐�����
}

CMediaData::CMediaData(const CMediaData &Operand)
	: CBonObject()
	, m_dwDataSize(0UL)
	, m_dwBuffSize(0UL)
	, m_pData(NULL)
{
	// �R�s�[�R���X�g���N�^
	*this = Operand;
}

CMediaData::CMediaData(const DWORD dwBuffSize)
	: CBonObject()
	, m_dwDataSize(0UL)
	, m_dwBuffSize(0UL)
	, m_pData(NULL)
{
	// �o�b�t�@�T�C�Y���w�肵�ăo�b�t�@�𐶐�����
	GetBuffer(dwBuffSize);
}

CMediaData::CMediaData(const BYTE *pData, const DWORD dwDataSize)
	: CBonObject()
	, m_dwDataSize(0UL)
	, m_dwBuffSize(0UL)
	, m_pData(NULL)
{
	// �f�[�^�����l���w�肵�ăo�b�t�@�𐶐�����
	SetData(pData, dwDataSize);
}

CMediaData::CMediaData(const BYTE byFiller, const DWORD dwDataSize)
	: CBonObject()
	, m_dwDataSize(0UL)
	, m_dwBuffSize(0UL)
	, m_pData(NULL)
{
	// �t�B���f�[�^���w�肵�ăo�b�t�@�𐶐�����
	SetSize(dwDataSize, byFiller);
}

CMediaData::~CMediaData(void)
{
	// �o�b�t�@���J������
	if(m_pData)delete [] m_pData;
}

CMediaData & CMediaData::operator = (const CMediaData &Operand)
{
	// �o�b�t�@�T�C�Y�̏��܂ł̓R�s�[���Ȃ�
	SetData(Operand.GetData(), Operand.GetSize());

	return *this;
}


/////////////////////////////////////////////////////////////////////////////
// IMediaDecoder���N���X
/////////////////////////////////////////////////////////////////////////////

const DWORD CMediaDecoder::GetInputNum(void) const
{
	// ���͐���Ԃ�
	return m_dwInputNum;
}

const DWORD CMediaDecoder::GetOutputNum(void) const
{
	// �o�͐���Ԃ�
	return m_dwOutputNum;
}

const bool CMediaDecoder::PlayDecoder(void)
{
	DWORD dwOutputIndex;

	// �ŏ��ɉ��ʃf�R�[�_���Đ�����(�����͏d�v)
	for(dwOutputIndex = 0UL ; dwOutputIndex < GetOutputNum() ; dwOutputIndex++){
		if(m_aOutputDecoder[dwOutputIndex].pDecoder){
			if(!m_aOutputDecoder[dwOutputIndex].pDecoder->PlayDecoder())break;
			}
		}

	// �G���[�������͍Đ��ς݃f�R�[�_���~����
	if(dwOutputIndex < GetOutputNum()){
		while(dwOutputIndex--){
			if(m_aOutputDecoder[dwOutputIndex].pDecoder){
				m_aOutputDecoder[dwOutputIndex].pDecoder->StopDecoder();
				}
			}
		return false;
		}
	
	// �Ō�Ɏ��f�R�[�_���Đ�����
	return OnPlay();
}

const bool CMediaDecoder::StopDecoder(void)
{
	bool bReturn = true;

	// �ŏ��Ɏ��f�R�[�_���~����
	if(!OnStop()){
		bReturn = false;
		}

	// ���ɉ��ʃf�R�[�_���~����(�����͏d�v)
	for(DWORD dwOutputIndex = 0UL ; dwOutputIndex < GetOutputNum() ; dwOutputIndex++){
		if(m_aOutputDecoder[dwOutputIndex].pDecoder){
			if(!m_aOutputDecoder[dwOutputIndex].pDecoder->StopDecoder()){
				bReturn = false;
				}
			}
		}

	return bReturn;
}

const bool CMediaDecoder::ResetDecoder(void)
{
	bool bReturn = true;

	// �f�R�[�_�O���t�����b�N
	if(m_pLock)m_pLock->Lock();

	// �ŏ��ɉ��ʃf�R�[�_�����Z�b�g����(�����͏d�v)
	for(DWORD dwOutputIndex = 0UL ; dwOutputIndex < GetOutputNum() ; dwOutputIndex++){
		if(m_aOutputDecoder[dwOutputIndex].pDecoder){
			if(!m_aOutputDecoder[dwOutputIndex].pDecoder->ResetDecoder()){
				bReturn = false;
				}
			}
		}

	// �Ō�Ɏ��f�R�[�_�����Z�b�g����
	if(!OnReset()){
		bReturn = false;
		}

	// �f�R�[�_�O���t���A�����b�N
	if(m_pLock)m_pLock->Unlock();

	return bReturn;
}

const bool CMediaDecoder::SetOutputDecoder(IMediaDecoder *pDecoder, const DWORD dwOutputIndex, const DWORD dwInputIndex)
{
	// �o�̓f�R�[�_���Z�b�g����
	if(dwOutputIndex < GetOutputNum()){
		m_aOutputDecoder[dwOutputIndex].pDecoder = pDecoder;
		m_aOutputDecoder[dwOutputIndex].dwInputIndex = dwInputIndex;
		return true;
		}

	return false;
}

const bool CMediaDecoder::InputMedia(IMediaData *pMediaData, const DWORD dwInputIndex)
{
	// ��ɃG���[��Ԃ�
	return false;
}

CMediaDecoder::CMediaDecoder(IBonObject *pOwner, const DWORD dwInputNum, const DWORD dwOutputNum, CSmartLock *pLock)
	: CBonObject(pOwner)
	, m_pEventHandler(dynamic_cast<IMediaDecoderHandler *>(pOwner))
	, m_dwInputNum(dwInputNum)
	, m_dwOutputNum(dwOutputNum)
	, m_pLock(pLock)
{
	// �o�̓f�R�[�_�z����N���A����
	::ZeroMemory(m_aOutputDecoder, sizeof(m_aOutputDecoder));
}

CMediaDecoder::~CMediaDecoder(void)
{
	// �N���e�B�J���Z�N�V�����J��
	if(m_pLock)delete m_pLock;
}

const bool CMediaDecoder::OnPlay(void)
{
	// �f�t�H���g�̎����ł͉������Ȃ�
	return true;
}

const bool CMediaDecoder::OnStop(void)
{
	// �f�t�H���g�̎����ł͉������Ȃ�
	return true;
}

const bool CMediaDecoder::OnReset(void)
{
	// �f�t�H���g�̎����ł͉������Ȃ�
	return true;
}

const bool CMediaDecoder::OutputMedia(IMediaData *pMediaData, const DWORD dwOutptIndex)
{
	// �o�̓f�R�[�_�Ƀf�[�^��n��
	if(dwOutptIndex < GetOutputNum()){
		if(m_aOutputDecoder[dwOutptIndex].pDecoder){
			return m_aOutputDecoder[dwOutptIndex].pDecoder->InputMedia(pMediaData, m_aOutputDecoder[dwOutptIndex].dwInputIndex);
			}
		}

	return false;
}

const DWORD CMediaDecoder::SendDecoderEvent(const DWORD dwEventID, PVOID pParam)
{
	// �C�x���g��ʒm����
	return (m_pEventHandler)? m_pEventHandler->OnDecoderEvent(this, dwEventID, pParam) : 0UL;
}
