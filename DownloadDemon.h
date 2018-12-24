// DownloadDemon.h : main header file for the DOWNLOAD DEMON application
//

#if !defined(AFX_DOWNLOADDEMON_H__ACB60699_AAE8_11D1_A5D8_444553540000__INCLUDED_)
#define AFX_DOWNLOADDEMON_H__ACB60699_AAE8_11D1_A5D8_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CDownloadDemonApp:
// See DownloadDemon.cpp for the implementation of this class
//

class CDownloadDemonApp : public CWinApp
{
public:
	int m_iHiddenRegistrationValid;
	CDownloadDemonApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDownloadDemonApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CDownloadDemonApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	COleTemplateServer m_oleServer;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DOWNLOADDEMON_H__ACB60699_AAE8_11D1_A5D8_444553540000__INCLUDED_)
