// TsConverterIf.h: TSコンバータインタフェース定義
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// Multi2復号コンバータインタフェース
/////////////////////////////////////////////////////////////////////////////

class IMulti2Converter :		public IBonObject
{
public:
	// Multi2復号コンバータハンドラインタフェース
	class IHandler :	public IBonObject
	{
	public:
		virtual void OnMulti2ConverterEvent(IMulti2Converter *pMulti2Converter, const DWORD dwEventID, PVOID pParam) = 0;
	};

	enum	// コンバータイベントID				// [イベント内容]				[pParam]
	{
		EID_CONV_START		= 0x00000000UL,		// コンバート開始				N/A
		EID_CONV_END		= 0x00000001UL,		// コンバート終了				N/A
		EID_CONV_PROGRESS	= 0x00000002UL,		// コンバート進捗				100%/1000
		EID_CONV_ERROR		= 0x00000003UL		// エラー発生					N/A
	};

	enum	// StartConvert() エラーコード
	{
		EC_NO_ERROR			= 0x00000000UL,		// エラーなし成功
		EC_FILE_ERROR		= 0x00000001UL,		// ファイルが開けない
		EC_BCAS_ERROR		= 0x00000002UL,		// B-CASカードを開けない
		EC_CLASS_ERROR		= 0x00000003UL,		// 必要なクラスが見つからない
		EC_INTERNAL_ERROR	= 0x00000004UL		// その他内部エラー
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
