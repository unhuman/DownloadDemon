// DlgSettings.cpp : implementation file
//
#include "stdafx.h"
#include "DownloadDemon.h"
#include "DlgSettings.h"
#include "Registry.h"
#include "DlgSettingsModem.h"
#include "DownloadDemonDlg.h"
#include "SHLObj.h"
#include "DialUp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//Used to load up a string in the dropdown list
BOOL EnumConnectionsProc(LPCTSTR lpszConnectionName, LPARAM lParam)
{
	CStringList* pStrList = (CStringList*) lParam;

	pStrList->AddTail(lpszConnectionName);

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CDlgSettings dialog

CDlgSettings::CDlgSettings(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSettings::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgSettings)
	m_strDefaultLogin = _T("");
	m_strDefaultPassword = _T("");	
	m_iNumSimultDownloads = 0;
	m_iRetryTime = 30;
	m_strDownloadDir = _T("");
	m_iConnectionTimeout = 30;	
	m_iRetryCount = 3;
	m_strDownloadConnection = _T("");
	m_bDialModem = FALSE;
	m_bMinimizeDialup = FALSE;
	m_bSmartDownload = TRUE;
	m_bMultipleIdenticalConnections = TRUE;
	m_iRadioDupeSkip = 0;
	//}}AFX_DATA_INIT

	m_strDefaultLogin.LoadString(IDS_DEFAULTLOGIN);
	m_strDefaultPassword.LoadString(IDS_DEFAULTPASSWORD);

	m_strDownloadAcceleratorLocation = _T("");

	//Load Items from registry
	LoadSaveSettings(TRUE);

	//Determine which internet connections might be available
	COXRegistry regInternet;

	m_iLockSimultDownloads = -1;
	EnumDialUpConnections(EnumConnectionsProc, (LPARAM)&m_strlConnections);
}

void CDlgSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSettings)
	DDX_Control(pDX, IDC_COMBO_SIMULTANEOUS_DOWNLOADS, m_comboSimultDownloads);
	DDX_Control(pDX, IDC_BUTTON_MODEMSTRINGS, m_btnModemWindowStrings);
	DDX_Control(pDX, IDC_CHECK_MINIMIZEDIALUP, m_chkMinimizeConnection);
	DDX_Control(pDX, IDC_CHECK_DIALMODEM, m_chkDialModem);
	DDX_Control(pDX, IDC_COMBO_DIALCONNECTION, m_comboDownloadConnection);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Text(pDX, IDC_EDIT_DEFAULTLOGIN, m_strDefaultLogin);
	DDX_Text(pDX, IDC_EDIT_DEFAULTPASSWORD, m_strDefaultPassword);
	DDX_Check(pDX, IDC_CHECK_SMARTDOWNLOAD, m_bSmartDownload);
	DDX_CBIndex(pDX, IDC_COMBO_SIMULTANEOUS_DOWNLOADS, m_iNumSimultDownloads);
	DDX_Text(pDX, IDC_EDIT_RETRY_TIME, m_iRetryTime);
	DDV_MinMaxInt(pDX, m_iRetryTime, 1, 360);
	DDX_Text(pDX, IDC_EDIT_DOWNLOAD_DIR, m_strDownloadDir);
	DDX_Text(pDX, IDC_EDIT_CONNECTION_TIMEOUT, m_iConnectionTimeout);
	DDV_MinMaxInt(pDX, m_iConnectionTimeout, 1, 360);
	DDX_Text(pDX, IDC_EDIT_RETRY_COUNT, m_iRetryCount);
	DDV_MinMaxInt(pDX, m_iRetryCount, 0, 100);
	DDX_CBString(pDX, IDC_COMBO_DIALCONNECTION, m_strDownloadConnection);
	DDX_Check(pDX, IDC_CHECK_DIALMODEM, m_bDialModem);
	DDX_Check(pDX, IDC_CHECK_MINIMIZEDIALUP, m_bMinimizeDialup);
	DDX_Check(pDX, IDC_CHECK_MULTIPLE_IDENTICAL, m_bMultipleIdenticalConnections);
	DDX_Radio(pDX, IDC_RADIO_DUPE_SKIP, m_iRadioDupeSkip);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgSettings, CDialog)
	//{{AFX_MSG_MAP(CDlgSettings)
	ON_BN_CLICKED(IDC_CHECK_LOG_FILE, UpdateEnabledItems)
	ON_EN_CHANGE(IDC_EDIT_DOWNLOAD_DIR, OnChangeEditDownloadDir)
	ON_BN_CLICKED(IDC_BUTTON_MODEMSTRINGS, OnButtonModemStrings)
	ON_BN_CLICKED(IDC_CHECK_DIALMODEM, OnCheckDialmodem)
	ON_BN_CLICKED(IDC_BUTTON_LOOKUP_DIR, OnButtonLookupDir)
	ON_CBN_SELCHANGE(IDC_COMBO_SIMULTANEOUS_DOWNLOADS, UpdateEnabledItems)
	ON_BN_CLICKED(IDC_BUTTON_MANAGE_STRIP_EXTENSIONS, OnButtonManageStripExtensions)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgSettings message handlers

void CDlgSettings::OnOK() 
{
	CString strDownloadCheck(_T(""));
	UpdateData();

	WIN32_FIND_DATA fd;
	UpdateData();

	if (m_iLockSimultDownloads != -1)
		m_iNumSimultDownloads = m_iLockSimultDownloads;

	if (m_strDownloadDir.GetLength() && (m_strDownloadDir.GetAt(m_strDownloadDir.GetLength() - 1) != '\\'))
	{
		m_strDownloadDir += '\\';
		UpdateData(FALSE);
	}

	strDownloadCheck = m_strDownloadDir;
	strDownloadCheck += "*.*";
	UpdateData(FALSE);
	HANDLE h = ::FindFirstFile(strDownloadCheck.GetBuffer(0), &fd);
	strDownloadCheck.ReleaseBuffer();

	//Ensure the data is validated before trying to write it out.
	if (h != INVALID_HANDLE_VALUE)
	{
		//Save items to registry
		LoadSaveSettings(FALSE);
		CDialog::OnOK();
	} else
		AfxMessageBox(IDS_INVALID_PATH);
}

void CDlgSettings::LoadSaveSettings(BOOL bLoadSettings)
{
	COXRegistry regSettings;
	CString strDefaultLogin(_T(""));
	CString strDefaultPassword(_T(""));
	CString strDownloadDir(_T(""));
	CString strDefaultDialup(_T(""));
	CString strDownloadAcceleratorLocation(_T(""));
	DWORD dwSmartDownload(1);
	DWORD dwSimultDownloads(0);
	DWORD dwRetryTime(30);
	DWORD dwRetryCount(10);
	DWORD dwAllowMultipleConnections(0);
	DWORD dwCreateLogFile(0);
	DWORD dwAlwaysClearLogFile(1);
	DWORD dwConnectionTimeout(30);
	DWORD dwDialModem(0);
	DWORD dwMinimizeConnection(1);
	DWORD dwSkipDupe(0);

	if (regSettings.Connect(COXRegistry::keyLocalMachine))
	{
		CString strRegKey(_T(""));
		strRegKey.LoadString(IDS_REGISTRY_KEY);
		if (regSettings.CheckAndOpenKeyForWrite(strRegKey))
		{
			if (bLoadSettings) //Load
			{
				//Retrieve configuration from registry and only
				//update items from default if they were retrieved
				if (regSettings.GetValue("DefaultLogin", strDefaultLogin))
					m_strDefaultLogin = strDefaultLogin;
				if (regSettings.GetValue("DefaultPassword", strDefaultPassword))
					m_strDefaultPassword = strDefaultPassword;
				if (regSettings.GetValue("DownloadDirectory", strDownloadDir))
					m_strDownloadDir = strDownloadDir;
				if (regSettings.GetValue("SmartDownload", dwSmartDownload))
					m_bSmartDownload = dwSmartDownload;
				if (regSettings.GetValue("SimultDownloads", dwSimultDownloads))
					m_iNumSimultDownloads = dwSimultDownloads;
				if (regSettings.GetValue("RetryTime", dwRetryTime))
					m_iRetryTime = dwRetryTime;
				if (regSettings.GetValue("RetryCount", dwRetryCount))
					m_iRetryCount = dwRetryCount;
				if (regSettings.GetValue("AllowMultipleConnections", dwAllowMultipleConnections))
					m_bMultipleIdenticalConnections = dwAllowMultipleConnections;
				if (regSettings.GetValue("ConnectionTimeout", dwConnectionTimeout))
					m_iConnectionTimeout = dwConnectionTimeout;
				if (regSettings.GetValue("DialModem", dwDialModem))
					m_bDialModem = dwDialModem;
				if (regSettings.GetValue("DialupConnection", strDefaultDialup))
					m_strDownloadConnection = strDefaultDialup;
				if (regSettings.GetValue("MinimizeDialup", dwMinimizeConnection))
					m_bMinimizeDialup = dwMinimizeConnection;
				if (regSettings.GetValue("FileAlreadyExistsHandler", dwSkipDupe))
					m_iRadioDupeSkip = dwSkipDupe;				
				if (regSettings.GetValue("DownloadAcceleratorLocation", strDownloadAcceleratorLocation))
					m_strDownloadAcceleratorLocation = strDownloadAcceleratorLocation;
			} else { //Save
				//Trim off starting and trailing white space
				m_strDownloadDir.TrimLeft();
				m_strDownloadDir.TrimRight();

				//Save configuration to registry
				regSettings.SetValue("DefaultLogin", m_strDefaultLogin);
				regSettings.SetValue("DefaultPassword", m_strDefaultPassword);
				regSettings.SetValue("DownloadDirectory", m_strDownloadDir);
				regSettings.SetValue("SmartDownload", m_bSmartDownload);
				// Only save SimultDownloads if command line doesn't set it
				if (m_iLockSimultDownloads == -1)
					regSettings.SetValue("SimultDownloads", m_iNumSimultDownloads);
				regSettings.SetValue("RetryTime", m_iRetryTime);
				regSettings.SetValue("RetryCount", m_iRetryCount);
				regSettings.SetValue("AllowMultipleConnections", m_bMultipleIdenticalConnections);
				regSettings.SetValue("ConnectionTimeout", m_iConnectionTimeout);
				regSettings.SetValue("DialModem", m_bDialModem);
				regSettings.SetValue("DialupConnection", m_strDownloadConnection);
				regSettings.SetValue("MinimizeDialup", m_bMinimizeDialup);
				regSettings.SetValue("FileAlreadyExistsHandler", m_iRadioDupeSkip);
				if (m_strDownloadAcceleratorLocation.GetLength())
					regSettings.SetValue("DownloadAcceleratorLocation", m_strDownloadAcceleratorLocation);
			}
		}
		regSettings.Disconnect();
	}
}

void CDlgSettings::UpdateEnabledItems()
{
	//Ensure data is validated
	if (UpdateData())
	{
		CWnd* pWnd = NULL; 

		//Update Options based on log file operations
		pWnd = GetDlgItem(IDC_CHECK_CLEAR_LOG_ON_START);
		pWnd = GetDlgItem(IDC_BUTTON_CLEAR_LOG_NOW);

		//Update availability of "Allow multiple connections to same server." checkbox
		pWnd = GetDlgItem(IDC_CHECK_MULTIPLE_IDENTICAL);
		if (pWnd)
		{
			//m_iNumSimultDownloads is zero based, so 1 thread = 0
			pWnd->EnableWindow(m_iNumSimultDownloads > 0);
		}
	}
}

BOOL CDlgSettings::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CString strConnectionBackup = m_strDownloadConnection;

	UpdateEnabledItems();

	BackupConfigurationValues(TRUE);

	m_chkDialModem.EnableWindow(FALSE);
	POSITION pos = m_strlConnections.GetHeadPosition();
	BOOL bFound(FALSE);
	while (pos)
	{
		int iAddedLoc(0);
		CString strCurrConnection = m_strlConnections.GetNext(pos);
		if (strCurrConnection.GetLength())
		{			
			iAddedLoc = m_comboDownloadConnection.AddString(strCurrConnection);		
			if (strCurrConnection == strConnectionBackup)
			{
				m_comboDownloadConnection.SetCurSel(iAddedLoc);
				bFound = TRUE;
			}
		}
		m_chkDialModem.EnableWindow(TRUE);
	}

	if (!bFound)
	{
		m_chkDialModem.SetCheck(FALSE);
		m_bDialModem = FALSE;
	}

	//Enable or disable the OK button
	OnChangeEditDownloadDir();
	OnCheckDialmodem();

	// Enable or disable the Simult downloads
	m_comboSimultDownloads.EnableWindow(m_iLockSimultDownloads == -1);
	
	//Ensure our data is up to date
	UpdateData();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgSettings::BackupConfigurationValues(BOOL bBackup)
{
	if (bBackup)
	{
		//Backup
		m_strDefaultLoginBack = m_strDefaultLogin;
		m_strDefaultPasswordBack = m_strDefaultPassword;
		m_bSmartDownloadBack = m_bSmartDownload;
		m_iNumSimultDownloadsBack = m_iNumSimultDownloads;
		m_bMultipleIdenticalConnectionsBack = m_bMultipleIdenticalConnections;
		m_iRetryTimeBack = m_iRetryTime;
		m_strDownloadDirBack = m_strDownloadDir;
		m_iConnectionTimeoutBack = m_iConnectionTimeout;
	} else {
		//Restore
		m_strDefaultLogin = m_strDefaultLoginBack;
		m_strDefaultPassword = m_strDefaultPasswordBack;
		m_bSmartDownload = m_bSmartDownloadBack;
		m_iNumSimultDownloads = m_iNumSimultDownloadsBack;
		m_bMultipleIdenticalConnections = m_bMultipleIdenticalConnectionsBack;
		m_iRetryTime = m_iRetryTimeBack;
		m_strDownloadDir = m_strDownloadDirBack;
		m_iConnectionTimeout = m_iConnectionTimeoutBack;
	}
}

void CDlgSettings::OnCancel() 
{
	//Go back to the original config, since we're cancelling
	BackupConfigurationValues(FALSE);
	
	CDialog::OnCancel();
}

void CDlgSettings::OnChangeEditDownloadDir()
{
	BOOL bEnableOK(FALSE);

	UpdateData();

	// validate standard path
	if ((m_strDownloadDir.GetLength() >= 3) &&
		(m_strDownloadDir.GetAt(1) == _T(':')) &&
		(m_strDownloadDir.GetAt(2) == _T('\\')))
	{
		bEnableOK = TRUE;
	}

	// Allow network drives
	if ((m_strDownloadDir.GetLength() >= 2) &&
		(m_strDownloadDir.GetAt(0) == _T('\\')) &&
		(m_strDownloadDir.GetAt(1) == _T('\\')))
	{
		bEnableOK = TRUE;
	}

	m_btnOK.EnableWindow(bEnableOK);
}

void CDlgSettings::OnButtonModemStrings() 
{
	m_dlgSettingsModem.DoModal();	
}

void CDlgSettings::OnCheckDialmodem() 
{	
 	m_comboDownloadConnection.EnableWindow(m_chkDialModem.GetCheck() == 1);
	m_chkMinimizeConnection.EnableWindow(m_chkDialModem.GetCheck() == 1);
	m_btnModemWindowStrings.EnableWindow(m_chkDialModem.GetCheck() == 1);
}

void CDlgSettings::OnButtonLookupDir() 
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
	browseInfo.pszDisplayName = m_strDownloadDir.GetBuffer(MAX_PATH);	
	browseInfo.lpszTitle = _T("Select a directory");
	browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_RETURNFSANCESTORS;
	browseInfo.lpfn = NULL;

	// If the user selected a folder
	if (pItemSelected = ::SHBrowseForFolder(&browseInfo))
	{
		// release buffer - done below also - since it's reused
		m_strDownloadDir.ReleaseBuffer();

		// Ensure data in memory matches display
		UpdateData();

		// Modify buffer in memory
		::SHGetPathFromIDList(pItemSelected, m_strDownloadDir.GetBuffer(MAX_PATH));
		m_strDownloadDir.ReleaseBuffer();
		
		// Set display data to match memory
		UpdateData(FALSE);

		pMalloc->Free(pItemSelected);
		OnChangeEditDownloadDir();
	}
	else //Cancel
	{
		//release buffer
		m_strDownloadDir.ReleaseBuffer();
	}
}

void CDlgSettings::OnButtonManageStripExtensions() 
{
	m_dlgStripExtensions.DoModal();
}

BOOL CDlgSettings::PreTranslateMessage(MSG* pMsg) 
{
	if ((pMsg->message == WM_KEYDOWN) &&
		((pMsg->wParam == VK_RETURN) ||
		 (pMsg->wParam == VK_ESCAPE)))
	{	
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}
