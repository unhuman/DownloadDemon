// DownloadDemon.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "DownloadDemon.h"
#include "DownloadDemonDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDownloadDemonApp

// this is the GUID for OCLIENT documents
static const GUID BASED_CODE clsid =
	{ 0x00021842, 0, 0, { 0xC0, 0, 0, 0, 0, 0, 0, 0x46 } };


BEGIN_MESSAGE_MAP(CDownloadDemonApp, CWinApp)
	//{{AFX_MSG_MAP(CDownloadDemonApp)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDownloadDemonApp construction

CDownloadDemonApp::CDownloadDemonApp()
{
	m_iHiddenRegistrationValid = 0;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDownloadDemonApp object

CDownloadDemonApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CDownloadDemonApp initialization

BOOL CDownloadDemonApp::InitInstance()
{
	// Initialize OLE 2.0 libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDS_AFXOLEINIT_FAILED);
		return FALSE;
	}

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CDownloadDemonDlg dlg;
	m_pMainWnd = &dlg;

	try
	{
		int nResponse = dlg.DoModal();
		if (nResponse == IDOK)
		{
			// TODO: Place code here to handle when the dialog is
			//  dismissed with OK
		}
		else if (nResponse == IDCANCEL)
		{
			// TODO: Place code here to handle when the dialog is
			//  dismissed with Cancel
		}
	}
	catch(...)
	{
		//Do nothing
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
