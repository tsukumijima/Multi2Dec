// Multi2Converter.cpp: Multi2�����R���o�[�^�N���X
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BonSDK.h"
#include "Multi2Converter.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// Multi2�����R���o�[�^�N���X
/////////////////////////////////////////////////////////////////////////////

const DWORD CMulti2Converter::StartConvert(LPCTSTR lpszSrcFile, LPCTSTR lpszDstFile, const bool bB25, const bool bEMM, const bool bNULL, const bool bDiscard)
{
	if(!lpszSrcFile)return EC_FILE_ERROR;
	if(!m_pFileReader || !m_pTsPacketSync || !m_pTsDescrambler || !m_pFileWriter)return EC_CLASS_ERROR;

	// �S�f�R�[�_�����
	CloseDecoder();

	// �f�R�[�_�O���t���\�z����
	IMediaDecoder * const pFileReaderDecoder	= dynamic_cast<IMediaDecoder *>(m_pFileReader    );
	IMediaDecoder * const pTsPacketSyncDecoder	= dynamic_cast<IMediaDecoder *>(m_pTsPacketSync  );
	IMediaDecoder * const pTsDescramblerDecoder	= dynamic_cast<IMediaDecoder *>(m_pTsDescrambler );
	IMediaDecoder * const pFileWriterDecoder	= dynamic_cast<IMediaDecoder *>(m_pFileWriter    );

	pFileReaderDecoder->SetOutputDecoder(pTsPacketSyncDecoder);
	pTsPacketSyncDecoder->SetOutputDecoder(pTsDescramblerDecoder);
	pTsDescramblerDecoder->SetOutputDecoder(pFileWriterDecoder);

	// �f�R�[�_���J��
	try{
		// ���̓t�@�C���I�[�v��
		if(!m_pFileReader->OpenFile(lpszSrcFile, true))throw (DWORD)EC_FILE_ERROR;

		// �p�P�b�g�����f�R�[�_�ݒ�
		m_pTsPacketSync->DiscardNullPacket(bNULL);

		// �f�X�N�����u���f�R�[�_�I�[�v��
		if(bB25){
			if(!m_pTsDescrambler->OpenDescrambler(TEXT("CBcasCardReader")))throw (DWORD)EC_BCAS_ERROR;
			}

		// �f�X�N�����u���f�R�[�_�ݒ�
		m_pTsDescrambler->EnableBuffering(true);
		m_pTsDescrambler->EnableEmmProcess(bEMM);
		m_pTsDescrambler->DiscardScramblePacket(bDiscard);

		// �o�̓t�@�C���I�[�v��
		if(lpszDstFile){
			if(!m_pFileWriter->OpenFile(lpszDstFile), false)throw (DWORD)EC_FILE_ERROR;
			}

		// �R���o�[�g�J�n
		if(!pFileReaderDecoder->PlayDecoder())throw (DWORD)EC_INTERNAL_ERROR;
		}
	catch(DWORD dwErrorCode){
		// �G���[����
		CloseDecoder();
		return dwErrorCode;
		}

	return EC_NO_ERROR;
}

void CMulti2Converter::EndConvert(void)
{
	// �f�R�[�_�O���t���~����
	if(m_pFileReader){
		dynamic_cast<IMediaDecoder *>(m_pFileReader)->StopDecoder();
		}

	// �f�R�[�_�O���t�����
	CloseDecoder();
}

const DWORD CMulti2Converter::GetPacketStride(void) const
{
	// TS�p�P�b�g�T�C�Y��Ԃ�
	return (m_pTsPacketSync)? m_pTsPacketSync->GetPacketStride() : 0UL;
}

const DWORD CMulti2Converter::GetInputPacketNum(const WORD wPID) const
{
	// ����TS�p�P�b�g����Ԃ�
	return (m_pTsPacketSync)? m_pTsPacketSync->GetInputPacketNum(wPID) : 0UL;
}

const DWORD CMulti2Converter::GetOutputPacketNum(const WORD wPID) const
{
	// �o��TS�p�P�b�g����Ԃ�
	return (m_pTsDescrambler)? m_pTsDescrambler->GetOutputPacketNum(wPID) : 0UL;
}

const DWORD CMulti2Converter::GetSyncErrNum(void) const
{
	// �����G���[����Ԃ�
	return (m_pTsPacketSync)? m_pTsPacketSync->GetSyncErrNum() : 0UL;
}

const DWORD CMulti2Converter::GetFormatErrNum(void) const
{
	// �t�H�[�}�b�g�G���[����Ԃ�
	return (m_pTsPacketSync)? m_pTsPacketSync->GetFormatErrNum() : 0UL;
}

const DWORD CMulti2Converter::GetTransportErrNum(void) const
{
	// �g�����X�|�[�g�G���[����Ԃ�
	return (m_pTsPacketSync)? m_pTsPacketSync->GetTransportErrNum() : 0UL;
}

const DWORD CMulti2Converter::GetContinuityErrNum(const WORD wPID) const
{
	// �A�����G���[����Ԃ�
	return (m_pTsPacketSync)? m_pTsPacketSync->GetContinuityErrNum(wPID) : 0UL;
}

const DWORD CMulti2Converter::GetScramblePacketNum(const WORD wPID) const
{
	// �����R��p�P�b�g����Ԃ�
	return (m_pTsDescrambler)? m_pTsDescrambler->GetScramblePacketNum(wPID) : 0UL;
}

const DWORD CMulti2Converter::GetEcmProcessNum(void) const
{
	// ECM��������Ԃ�
	return (m_pTsDescrambler)? m_pTsDescrambler->GetEcmProcessNum() : 0UL;
}

const DWORD CMulti2Converter::GetEmmProcessNum(void) const
{
	// EMM��������Ԃ�
	return (m_pTsDescrambler)? m_pTsDescrambler->GetEmmProcessNum() : 0UL;
}

const ULONGLONG CMulti2Converter::GetSrcFileLength(void) const
{
	// ���̓t�@�C������Ԃ�
	return (m_pFileReader)? m_pFileReader->GetFileLength() : 0ULL;
}

const ULONGLONG CMulti2Converter::GetDstFileLength(void) const
{
	// �o�̓t�@�C������Ԃ�
	return (m_pFileWriter)? m_pFileWriter->GetFileLength() : 0ULL;
}

CMulti2Converter::CMulti2Converter(IBonObject * const pOwner)
	: CBonObject(pOwner)
	, m_pFileReader(::BON_SAFE_CREATE<IFileReader *>(TEXT("CFileReader"), dynamic_cast<IMediaDecoderHandler *>(this)))
	, m_pTsPacketSync(::BON_SAFE_CREATE<ITsPacketSync *>(TEXT("CTsPacketSync"), dynamic_cast<IMediaDecoderHandler *>(this)))
	, m_pTsDescrambler(::BON_SAFE_CREATE<ITsDescrambler *>(TEXT("CTsDescrambler"), dynamic_cast<IMediaDecoderHandler *>(this)))
	, m_pFileWriter(::BON_SAFE_CREATE<IFileWriter *>(TEXT("CFileWriter"), dynamic_cast<IMediaDecoderHandler *>(this)))
	, m_pHandler(dynamic_cast<IMulti2Converter::IHandler *>(pOwner))
{
	// �������Ȃ�
}

CMulti2Converter::~CMulti2Converter(void)
{
	// �N���X�C���X�^���X�����
	BON_SAFE_RELEASE(m_pFileReader);
	BON_SAFE_RELEASE(m_pTsPacketSync);
	BON_SAFE_RELEASE(m_pTsDescrambler);
	BON_SAFE_RELEASE(m_pFileWriter);
}

const DWORD CMulti2Converter::OnDecoderEvent(IMediaDecoder *pDecoder, const DWORD dwEventID, PVOID pParam)
{
	if(pDecoder == dynamic_cast<IMediaDecoder *>(m_pFileReader)){
		switch(dwEventID){
			case IFileReader::EID_READ_START :
				// �R���o�[�g�J�n
				SendConverterEvent(EID_CONV_START, reinterpret_cast<PVOID>(0UL));
				return 0UL;

			case IFileReader::EID_READ_END :
				// �R���o�[�g�I��
				SendConverterEvent(EID_CONV_END, reinterpret_cast<PVOID>((DWORD)(m_pFileReader->GetReadPos() * 1000ULL / m_pFileReader->GetFileLength())));
				return 0UL;

			case IFileReader::EID_POST_READ :
				// �R���o�[�g�i��
				SendConverterEvent(EID_CONV_PROGRESS, reinterpret_cast<PVOID>((DWORD)(m_pFileReader->GetReadPos() * 1000ULL / m_pFileReader->GetFileLength())));
				return 0UL;
			}
		}

	return 0UL;
}

void CMulti2Converter::CloseDecoder(void)
{
	// �S�Ẵf�R�[�_�����
	if(m_pFileReader)m_pFileReader->CloseFile();
	if(m_pTsPacketSync)m_pTsPacketSync->ResetStatistics();
	if(m_pTsDescrambler)m_pTsDescrambler->CloseDescrambler();
	if(m_pFileWriter)m_pFileWriter->CloseFile();
}

void CMulti2Converter::SendConverterEvent(const DWORD dwEventID, PVOID pParam)
{
	// �R���o�[�g�C�x���g���M
	if(m_pHandler)m_pHandler->OnMulti2ConverterEvent(this, dwEventID, pParam);
}
