// DlgProperties.cpp : implementation file
//

#include "stdafx.h"
#include "DownloadDemon.h"
#include "DlgProperties.h"
#include "RetrieveItem.h"
#include "SHLObj.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgProperties dialog


CDlgProperties::CDlgProperties(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgProperties::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgProperties)
	m_strLink = _T("");
	m_strLogin = _T("");
	m_strPassword = _T("");
	m_bUseSpecificLoginInfo = FALSE;
	m_iTransferMode = BINARY;
	m_iStrip = STRIP_NO;
	m_bSpecifyDestDir = FALSE;
	m_strEditDestDir = _T("");
	//}}AFX_DATA_INIT

	m_bAddMode = FALSE;
}


void CDlgProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgProperties)
	DDX_Control(pDX, IDC_COMBO_STRIP, m_comboStrip);
	DDX_Control(pDX, IDC_BUTTON_LOOKUP_DESTDIR, m_btnLookupDestDir);
	DDX_Control(pDX, IDC_EDIT_DESTINATION_DIRECTORY, m_editDestDir);
	DDX_Control(pDX, IDC_EDIT_PASSWORD, m_editPassword);
	DDX_Control(pDX, IDC_EDIT_LOGIN, m_editLogin);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Text(pDX, IDC_EDIT_LINK, m_strLink);
	DDX_Text(pDX, IDC_EDIT_LOGIN, m_strLogin);
	DDX_Text(pDX, IDC_EDIT_PASSWORD, m_strPassword);
	DDX_Check(pDX, IDC_CHECK_SPECIFY_LOGIN_INFO, m_bUseSpecificLoginInfo);
	DDX_CBIndex(pDX, IDC_COMBO_XFER_MODE, m_iTransferMode);
	DDX_CBIndex(pDX, IDC_COMBO_STRIP, m_iStrip);
	DDX_Check(pDX, IDC_CHECK_SPECIFY_DESTINATIONDIR, m_bSpecifyDestDir);
	DDX_Text(pDX, IDC_EDIT_DESTINATION_DIRECTORY, m_strEditDestDir);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgProperties, CDialog)
	//{{AFX_MSG_MAP(CDlgProperties)
	ON_BN_CLICKED(IDC_CHECK_SPECIFY_LOGIN_INFO, OnUpdateEnabled)
	ON_BN_CLICKED(IDC_CHECK_SPECIFY_DESTINATIONDIR, OnCheckSpecifyDestinationDir)
	ON_BN_CLICKED(IDC_BUTTON_LOOKUP_DESTDIR, OnButtonLookupDestDir)
	ON_EN_CHANGE(IDC_EDIT_LINK, OnUpdateEnabled)
	ON_EN_CHANGE(IDC_EDIT_DESTINATION_DIRECTORY, OnChangeEditDestinationDirectory)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgProperties message handlers

void CDlgProperties::SetAddMode()
{
	m_bAddMode = TRUE;
}

BOOL CDlgProperties::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	//If adding, set the window caption
	if (m_bAddMode)
	{
		CString strAddFileDownload(_T(""));
		strAddFileDownload.LoadString(IDS_ADD_FILE_DOWNLOAD);
		SetWindowText(strAddFileDownload);
	}

	OnUpdateEnabled();
	OnCheckSpecifyDestinationDir();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgProperties::OnUpdateEnabled() 
{
	UpdateData();
	OnCheckSpecifyDestinationDir();
	m_editLogin.EnableWindow(m_bUseSpecificLoginInfo);
	m_editPassword.EnableWindow(m_bUseSpecificLoginInfo);	

	UpdateOkButtonStatus();
}

void CDlgProperties::OnCheckSpecifyDestinationDir()
{
	UpdateData();	
	m_editDestDir.EnableWindow(m_bSpecifyDestDir);
	m_btnLookupDestDir.EnableWindow(m_bSpecifyDestDir);

	UpdateOkButtonStatus();
}

void CDlgProperties::UpdateOkButtonStatus()
{
	CString strLinkCopy(m_strLink);
	strLinkCopy.TrimLeft();
	strLinkCopy.TrimRight();

	CRetrieveItem cRetrieveItem;

	BOOL bIsLink(FALSE);
	BOOL bIsValidDir(FALSE);
	BOOL bAllowStrip(FALSE);
	
	// Validate the Link
	if (strLinkCopy.GetLength())
	{
		bIsLink = cRetrieveItem.ExtractURL(strLinkCopy.GetBuffer(strLinkCopy.GetLength()));
		strLinkCopy.ReleaseBuffer();

		if (bIsLink)
		{
			CString strLinkUpper(strLinkCopy);
			strLinkUpper.MakeUpper();
			if ((strLinkUpper.Left(4) == _T("HTTP")) &&
				((strLinkUpper.Right(4) == _T(".HTM")) ||
				 (strLinkUpper.Right(5) == _T(".HTML"))))
			{
				bAllowStrip = TRUE;
			}
		}
	}

	//Validate the directory
	if (!m_bSpecifyDestDir ||
		((m_strEditDestDir.GetLength() >= 3) &&
		 (m_strEditDestDir.GetAt(1) == _T(':')) &&
		 (m_strEditDestDir.GetAt(2) == _T('\\'))))
	{
		bIsValidDir = TRUE;
	}

	m_comboStrip.EnableWindow(bAllowStrip);
	m_btnOk.EnableWindow(bIsLink && bIsValidDir);
}

void CDlgProperties::OnButtonLookupDestDir() 
{
	LPMALLOC pMalloc = NULL;
	/* Get's the Shell's default allocator */
	if (::SHGetMalloc(&pMalloc) != NOERROR)
	{
		AfxMessageBox(IDS_DIRLOOKUPBROKE);
		return;
	}

	BROWSEINFO browseInfo;
	LPITEMIDLIST pItemSelected = NULL;

	browseInfo.hwndOwner = this->m_hWnd;
	browseInfo.pidlRoot = NULL;
	browseInfo.pszDisplayName = m_strEditDestDir.GetBuffer(MAX_PATH);	
	browseInfo.lpszTitle = _T("Select a directory");
	browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_RETURNFSANCESTORS;
	browseInfo.lpfn = NULL;

	// If the user selected a folder
	if (pItemSelected = ::SHBrowseForFolder(&browseInfo))
	{
		// release buffer - done below also - since it's reused
		m_strEditDestDir.ReleaseBuffer();

		// Ensure data in memory matches display
		UpdateData();

		// Modify buffer in memory
		::SHGetPathFromIDList(pItemSelected, m_strEditDestDir.GetBuffer(MAX_PATH));
		m_strEditDestDir.ReleaseBuffer();
		
		// Set display data to match memory
		UpdateData(FALSE);

		pMalloc->Free(pItemSelected);
		OnCheckSpecifyDestinationDir();
	}
	else //Cancel
	{
		//release buffer
		m_strEditDestDir.ReleaseBuffer();
	}	
}

void CDlgProperties::OnOK() 
{
	HANDLE h = INVALID_HANDLE_VALUE;

	if (m_bSpecifyDestDir)
	{
		CString strDestCheck(_T(""));
		UpdateData();

		WIN32_FIND_DATA fd;
		UpdateData();
		if (m_strEditDestDir.GetLength() && (m_strEditDestDir.GetAt(m_strEditDestDir.GetLength() - 1) != '\\'))
		{
			m_strEditDestDir += '\\';
			UpdateData(FALSE);
		}

		strDestCheck = m_strEditDestDir;
		strDestCheck += "*.*";
		UpdateData(FALSE);
		h = ::FindFirstFile(strDestCheck.GetBuffer(0), &fd);
		strDestCheck.ReleaseBuffer();
	}

	//Ensure the data is validated before trying to use it.
	if (!m_bSpecifyDestDir || (h != INVALID_HANDLE_VALUE))
	{
		CDialog::OnOK();
	} else
		AfxMessageBox(IDS_INVALID_PATH);
}

void CDlgProperties::OnChangeEditDestinationDirectory() 
{
	UpdateData();
	UpdateOkButtonStatus();
}
