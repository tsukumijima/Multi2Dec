// TsConverterIf.h: TS�R���o�[�^�C���^�t�F�[�X��`
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// Multi2�����R���o�[�^�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IMulti2Converter :		public IBonObject
{
public:
	// Multi2�����R���o�[�^�n���h���C���^�t�F�[�X
	class IHandler :	public IBonObject
	{
	public:
		virtual void OnMulti2ConverterEvent(IMulti2Converter *pMulti2Converter, const DWORD dwEventID, PVOID pParam) = 0;
	};

	enum	// �R���o�[�^�C�x���gID				// [�C�x���g���e]				[pParam]
	{
		EID_CONV_START		= 0x00000000UL,		// �R���o�[�g�J�n				N/A
		EID_CONV_END		= 0x00000001UL,		// �R���o�[�g�I��				N/A
		EID_CONV_PROGRESS	= 0x00000002UL,		// �R���o�[�g�i��				100%/1000
		EID_CONV_ERROR		= 0x00000003UL		// �G���[����					N/A
	};

	enum	// StartConvert() �G���[�R�[�h
	{
		EC_NO_ERROR			= 0x00000000UL,		// �G���[�Ȃ�����
		EC_FILE_ERROR		= 0x00000001UL,		// �t�@�C�����J���Ȃ�
		EC_BCAS_ERROR		= 0x00000002UL,		// B-CAS�J�[�h���J���Ȃ�
		EC_CLASS_ERROR		= 0x00000003UL,		// �K�v�ȃN���X��������Ȃ�
		EC_INTERNAL_ERROR	= 0x00000004UL		// ���̑������G���[
	};

	virtual const DWORD StartConvert(LPCTSTR lpszSrcFile, LPCTSTR lpszDstFile, const bool bB25 = true, const bool bEMM = false, const bool bNULL = false, const bool bDiscard = false) = 0;
	virtual void EndConvert(void) = 0;

	virtual const DWORD GetPacketStride(void) const = 0;
	virtual const DWORD GetInputPacketNum(const WORD wPID = TS_INVALID_PID) const = 0;
	virtual const DWORD GetOutputPacketNum(const WORD wPID = TS_INVALID_PID) const = 0;
	virtual const DWORD GetSyncErrNum(void) const = 0;
	virtual const DWORD GetFormatErrNum(void) const = 0;
	virtual const DWORD GetTransportErrNum(void) const = 0;
	virtual const DWORD GetContinuityErrNum(const WORD wPID = TS_INVALID_PID) const = 0;
	virtual const DWORD GetScramblePacketNum(const WORD wPID = TS_INVALID_PID) const = 0;
	virtual const DWORD GetEcmProcessNum(void) const = 0;
	virtual const DWORD GetEmmProcessNum(void) const = 0;

	virtual const ULONGLONG GetSrcFileLength(void) const = 0;
	virtual const ULONGLONG GetDstFileLength(void) const = 0;
};
