// Multi2DecDosApp.h: Multi2DecDos�A�v���P�[�V�����N���X
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


#include "TsConverterIf.h"


/////////////////////////////////////////////////////////////////////////////
// Multi2DecDos�A�v���P�[�V�����N���X
/////////////////////////////////////////////////////////////////////////////

class CMulti2DecDosApp :	public CBonObject,
							protected IMulti2Converter::IHandler
{
public:
// �R���t�B�M�����[�V�����\����
	typedef struct TAG_CONFIGSET
	{
		bool bOutput;
		bool bB25;
		bool bEMM;
		bool bPURE;
		bool bNULL;
		bool bNoLog;
		TCHAR szInputPath[_MAX_PATH];
		TCHAR szOutputPath[_MAX_PATH];
	} CONFIGSET;

// CBonObject
	DECLARE_IBONOBJECT(CMulti2DecDosApp)

// CMulti2DecDosApp
	CMulti2DecDosApp(IBonObject * const pOwner);
	virtual ~CMulti2DecDosApp(void);

	const bool Run(const CONFIGSET *pConfigSet);

protected:
	virtual void OnMulti2ConverterEvent(IMulti2Converter *pMulti2Converter, const DWORD dwEventID, PVOID pParam);
	void UpdateProgress(const DWORD dwProgress);
	void OutputLogFile(void);
	void PrintLogFile(FILE * const pStream);
	static LPCTSTR ToUnitedText(const ULONGLONG llValue);
	static LPCTSTR ToCommaText(const ULONGLONG llValue);


	IMulti2Converter *m_pMulti2Converter;
	CONFIGSET m_ConfigSet;

	CSmartEvent m_ConvertEvent;
	DWORD m_dwLastProgress;
};
