// DownloadDemonDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DownloadDemon.h"
#include "DownloadDemonDlg.h"
#include "Registry.h"
#include "DlgStoppingNotice.h"
#include "DlgProperties.h" //Also gets DlgSettingsModem.h
#include "DlgWaiver.h"
#include "InetAuxFunctions.h"
#include "Process.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DOWNLOAD_BUFFER_SIZE	2048
#define ARCADEATHOME_BUFFER_SIZE	32768
#define SPACING					7
#define SLEEP_INTERVAL			250
#define PARSE_REFERRER			_T("REFERRER=")
#define PARSE_COOKIE			_T("COOKIE=")
#define PARSE_DOWNLOAD			_T("DOWNLOAD=")
#define PARSE_DOWNLOADASCII		_T("DOWNLOADASCII=")
#define PARSE_MAXTHREADS		_T("MAXTHREADS=")
#define PARSE_ARCADEATHOME		_T("ARCADEATHOME")

#define ARCADEATHOME_REFERRER	_T("http://206.161.194.225")

//////////////////////////////////////////////////
// Internal Thread structure
// Used to pass pointer to THIS and the 
// thread number
//////////////////////////////////////////////////
struct SThreadInfo
{
	CDownloadDemonDlg* pDDDialog;
	int iThreadNum;
};

//////////////////////////////////////////////////
// WORKER FUNCTIONS
//////////////////////////////////////////////////

//Worker thread declarations (do callbacks to CDownloadDemonDlg* pDownloadDemonDialog);

//This function goes out and downloads files.  
//It will end when it asks for a file and there isn't any available.
//This one should have the thread count incremented BEFORE entering.
//It will decrement the count itself
UINT WorkerDownloadFiles(LPVOID pInThreadInfo)
{
	//Ensure pointer was passed in
	ASSERT(pInThreadInfo);

	SThreadInfo* pThreadInfo = (SThreadInfo*) pInThreadInfo;
	
	CDownloadDemonDlg* pDownloadDemonDialog = pThreadInfo->pDDDialog;

	//do thread processing
	UINT iReturn = pDownloadDemonDialog->DownloadFiles(pThreadInfo->iThreadNum);

	// delete the temporary data
	delete pThreadInfo;

	//decrement threads used
	pDownloadDemonDialog->DecrementThreadsUsed();	

	return iReturn;
}

//Monitors the threads running so the program will know when
//the downloads are finished
UINT WorkerMonitorDownloads(LPVOID pDownloadDemonDialogIn)
{	
	//Ensure pointer was passed in
	ASSERT(pDownloadDemonDialogIn);

	CDownloadDemonDlg* pDownloadDemonDialog = (CDownloadDemonDlg*)pDownloadDemonDialogIn;

	return pDownloadDemonDialog->MonitorDownloads();
}

//Downloads HTTP in a thread, since it seems to have timeout/retry issues
UINT WorkerDownloadHTTP(LPVOID pHTTPData)
{
	//Ensure pointer was passed in
	ASSERT(pHTTPData);
	CWorkerHttpTransfer* pWorkerData = (CWorkerHttpTransfer*)pHTTPData;

	ASSERT(pWorkerData->m_pParentDlg);

	CDownloadDemonDlg* pDownloadDemonDialog = (CDownloadDemonDlg*)pWorkerData->m_pParentDlg;

	UINT iReturn = pDownloadDemonDialog->DownloadHTTP(pWorkerData);	
	// If the status is what we're watching for, and we know we're done
	// obviously, something went wrong.  Force to unknown error
	if (pWorkerData->m_httpStatus == XFER_PROCESSING)
		pWorkerData->m_httpStatus = XFER_OTHER_ERROR;

	// This flag indicates we're done and allows memory to be cleared
	pWorkerData->m_bDone = TRUE;

	return iReturn;
}

//Counts down the timer
UINT WorkerCountDown(LPVOID pHTTPData)
{
	//Ensure pointer was passed in
	ASSERT(pHTTPData);
	CDownloadDemonDlg* pDownloadDemonDialog = (CDownloadDemonDlg*)pHTTPData;
	pDownloadDemonDialog->CountDown();

	return 0;
}

//////////////////////////////////////////////////
// END WORKER FUNCTIONS
//////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDownloadDemonDlg dialog

CDownloadDemonDlg::CDownloadDemonDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDownloadDemonDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDownloadDemonDlg)
	m_iWhenDownload = DOWNLOADNOW;
	m_strTimeStart = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_cfObjectDescriptor = NULL;	
	
	m_lThreadsStarted = 0;
	m_pDropTarget = NULL;
	m_tsDownloading = WAITING;
	m_AllowShutdown = FALSE;	
	m_bLiabilityShown = FALSE;
	m_bCancelledDownload = FALSE;
	m_pModemConnection = NULL;
	m_bRegistrationShown = FALSE;

	m_strReferrer = _T("");
	m_strReferrerAndCookie = _T("");
	m_bAutoURLTerminate = FALSE;
	m_bArcadeAtHomeMode = FALSE;

	// Set all threads to not be processing
	for (int i=0; i<MAX_THREADS; i++)
		bProcessing[i] = FALSE;

	m_bRunning = TRUE;
}

CDownloadDemonDlg::~CDownloadDemonDlg()
{
	// Ensure any dialing threads terminate
	m_bRunning = FALSE;

	if (m_pDropTarget)
	{
		delete m_pDropTarget;
		m_pDropTarget = NULL;
	}
	if (m_pModemConnection)
	{
		delete m_pModemConnection;
		m_pModemConnection = NULL;
	}
}

void CDownloadDemonDlg::ParseCommandLine(char* lpszCommandLine)
{	
	CString strCookies(_T(""));
	char* pszLookAt = lpszCommandLine;	
	char* pszToken = NULL;
	CString strToken(_T(""));
	CString strStripNo(_T(""));
	strStripNo.LoadString(IDS_NO);


	m_strReferrer = _T("");
	m_strReferrerAndCookie = _T("");
	while (pszToken = strtok(pszLookAt, _T(" ")))
	{
		pszLookAt = NULL;
		strToken = pszToken;
		
		if (strToken.Left(strlen(PARSE_MAXTHREADS)) == PARSE_MAXTHREADS)
		{
			int iSetThreadCount = atoi(strToken.Mid(strlen(PARSE_MAXTHREADS)))-1;
			if ((iSetThreadCount >= 0) && (iSetThreadCount < MAX_THREADS))
			{
				m_dlgSettings.m_iNumSimultDownloads = iSetThreadCount;
				m_dlgSettings.m_iLockSimultDownloads = iSetThreadCount;
			}
		}
		else if (strToken.Left(strlen(PARSE_REFERRER)) == PARSE_REFERRER)
		{
			m_strReferrer = strToken.Mid(strlen(PARSE_REFERRER));
			m_strReferrerAndCookie = m_strReferrer;
		}
		else if (strToken.Left(strlen(PARSE_COOKIE)) == PARSE_COOKIE)
		{
			strCookies += _T('\n');
			strCookies += _T("Cookie:");
			strCookies += strToken.Mid(strlen(PARSE_COOKIE));
		}
		else if (strToken.Left(strlen(PARSE_DOWNLOAD)) == PARSE_DOWNLOAD)
		{
			m_bAutoURLTerminate = TRUE;
			m_lstFTPItems.AddItem(strToken.Mid(strlen(PARSE_DOWNLOAD)), CDlgProperties::BINARY, strStripNo,
								  FALSE, _T(""), _T(""), _T(""));
		}
		else if (strToken.Left(strlen(PARSE_DOWNLOADASCII)) == PARSE_DOWNLOADASCII)
		{
			m_bAutoURLTerminate = TRUE;
			m_lstFTPItems.AddItem(strToken.Mid(strlen(PARSE_DOWNLOADASCII)), CDlgProperties::ASCII, strStripNo,
								  FALSE, _T(""), _T(""), _T(""));

		}
		else if (strToken.Left(strlen(PARSE_ARCADEATHOME)) == PARSE_ARCADEATHOME)
		{
			m_bArcadeAtHomeMode = TRUE;
			BOOL bSuccess = TRUE;

			if (!m_dlgSettings.m_strDownloadAcceleratorLocation.GetLength())
			{
				bSuccess = FALSE;

				AfxMessageBox("You must point to Download Accelerator for Arcade@Home mode");

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
				browseInfo.pszDisplayName = m_dlgSettings.m_strDownloadAcceleratorLocation.GetBuffer(MAX_PATH);	
				browseInfo.lpszTitle = _T("Select the directory where DAP.exe exists");
				browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_RETURNFSANCESTORS;
				browseInfo.lpfn = NULL;

				// If the user selected a folder
				if (pItemSelected = ::SHBrowseForFolder(&browseInfo))
				{
					// release buffer - done below also - since it's reused
					m_dlgSettings.m_strDownloadAcceleratorLocation.ReleaseBuffer();

					// Ensure data in memory matches display
					UpdateData();

					// Modify buffer in memory
					::SHGetPathFromIDList(pItemSelected, m_dlgSettings.m_strDownloadAcceleratorLocation.GetBuffer(MAX_PATH));
					m_dlgSettings.m_strDownloadAcceleratorLocation.ReleaseBuffer();
					
					// Set display data to match memory
					UpdateData(FALSE);

					pMalloc->Free(pItemSelected);

					if (m_dlgSettings.m_strDownloadAcceleratorLocation.Right(1) != _T("\\"))
						m_dlgSettings.m_strDownloadAcceleratorLocation += _T("\\");
					m_dlgSettings.m_strDownloadAcceleratorLocation += "DAP.exe";
					FILE* fpCheckDAP = fopen(m_dlgSettings.m_strDownloadAcceleratorLocation, "r");
					if (fpCheckDAP)
					{
						fclose(fpCheckDAP);
						bSuccess = TRUE;
						m_dlgSettings.LoadSaveSettings(FALSE);
					}
				}
				else //Cancel
				{
					//release buffer
					m_dlgSettings.m_strDownloadAcceleratorLocation.ReleaseBuffer();
				}

			}

			if (!bSuccess)
			{
				AfxMessageBox("Download Demon will now terminate since it could not find DAP.exe");
				EndDialog(0);
			}
		}
	}
	if (m_bArcadeAtHomeMode)
	{
		m_strReferrer = ARCADEATHOME_REFERRER;
		m_strReferrerAndCookie = m_strReferrer;

		// Build our own set of headers...  The regular ones don't work with AAH
		m_strReferrerAndCookie += _T('\r');
		m_strReferrerAndCookie += _T('\n');
//		m_strReferrerAndCookie += _T("Connection: Keep-Alive");
//		m_strReferrerAndCookie += _T('\r');
//		m_strReferrerAndCookie += _T('\n');
		m_strReferrerAndCookie += _T("User-Agent: Mozilla/4.76 [en] (Windows NT.5.0; U)");
		m_strReferrerAndCookie += _T('\r');
		m_strReferrerAndCookie += _T('\n');
		m_strReferrerAndCookie += _T("Host:.63.218.226.192");
		m_strReferrerAndCookie += _T('\r');
		m_strReferrerAndCookie += _T('\n');
		m_strReferrerAndCookie += _T("Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, image/png, */*");
		m_strReferrerAndCookie += _T('\r');
		m_strReferrerAndCookie += _T('\n');
		m_strReferrerAndCookie += _T("Accept-Encoding: gzip");
		m_strReferrerAndCookie += _T('\r');
		m_strReferrerAndCookie += _T('\n');
		m_strReferrerAndCookie += _T("Accept-Language: en");
		m_strReferrerAndCookie += _T('\r');
		m_strReferrerAndCookie += _T('\n');
		m_strReferrerAndCookie += _T("Accept-Charset: iso-8859-1,*,utf-8");
		m_strReferrerAndCookie += _T('\r');
		m_strReferrerAndCookie += _T('\n');
		m_strReferrerAndCookie += _T('\r');
		m_strReferrerAndCookie += _T('\n');
	}


	m_strReferrerAndCookie += strCookies;
}

void CDownloadDemonDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDownloadDemonDlg)
	DDX_Control(pDX, IDC_COMBO_STARTTIME, m_comboTimeStart);
	DDX_Control(pDX, IDC_CHECK_DISCONNECTDONE, m_chkDisconnectDone);
	DDX_Control(pDX, IDC_COMBO_WHENDOWNLOAD, m_comboWhenStart);
	DDX_Control(pDX, IDC_BUTTON_DOWNLOAD, m_btnDownload);
	DDX_Control(pDX, IDC_LIST_FTP_ITEMS, m_lstFTPItems);
	DDX_CBIndex(pDX, IDC_COMBO_WHENDOWNLOAD, m_iWhenDownload);
	DDX_CBString(pDX, IDC_COMBO_STARTTIME, m_strTimeStart);
	DDV_MaxChars(pDX, m_strTimeStart, 8);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDownloadDemonDlg, CDialog)
	//{{AFX_MSG_MAP(CDownloadDemonDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_COMMAND(ID_MENU_SETTINGS, OnMenuSettings)
	ON_BN_CLICKED(IDC_BUTTON_DOWNLOAD, OnButtonDownload)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_COMMAND(ID_MENU_ABOUT, OnMenuAbout)
	ON_COMMAND(ID_MENU_EXIT, OnMenuExit)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_ADD_NEW_FILE, OnAddNewFile)
	ON_COMMAND(ID_DELETE_COMPLETED_ITEMS, OnDeleteCompletedItems)
	ON_COMMAND(ID_DELETE_SELECTED_FILE, OnDeleteSelectedFile)
	ON_COMMAND(ID_PROPERTIES_SELECTED_FILE, OnPropertiesSelectedFile)
	ON_COMMAND(ID_MENU_NEW_LIST, OnMenuNewList)
	ON_COMMAND(ID_MENU_OPEN_LIST, OnMenuOpenList)
	ON_COMMAND(ID_MENU_SAVE_LIST, OnMenuSaveList)
	ON_COMMAND(ID_MENU_SAVE_LIST_AS, OnMenuSaveListAs)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST_FTP_ITEMS, OnKeydownListFtpItems)
	ON_CBN_SELCHANGE(IDC_COMBO_WHENDOWNLOAD, OnSelchangeComboWhenDownload)
	ON_COMMAND(ID_REGISTER, OnRegister)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDownloadDemonDlg message handlers

BOOL CDownloadDemonDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu(_T(""));
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	RECT rectConfiguration = {0, 0, 0, 0};
	LoadWindowRegistrySettings(TRUE, rectConfiguration, GetResolutionString());

	BOOL bUseExistingConfig(TRUE);

	//If the values retrieved weren't valid, use the defaults
	if ((rectConfiguration.left >= rectConfiguration.right) ||
		(rectConfiguration.top >= rectConfiguration.bottom))
	{
		bUseExistingConfig = FALSE;

		rectConfiguration.left = 0;
		rectConfiguration.right = 553;
		rectConfiguration.top = 0;
		rectConfiguration.bottom = 300;
	}

	//Set the window size and location
	SetWindowPos(&wndTop,
				 rectConfiguration.left, rectConfiguration.top, 
				 rectConfiguration.right - rectConfiguration.left + 1, 
				 rectConfiguration.bottom - rectConfiguration.top + 1,
				 SWP_DRAWFRAME);
	
	//Add my column headings and set Parent window to this one
	m_lstFTPItems.AddColumnHeadings(bUseExistingConfig);
	m_lstFTPItems.SetParent(this);

	//Create our instance of DropTarget
	m_pDropTarget = new CDownloadDemonDropTarget(m_cfObjectDescriptor);
	if (!m_pDropTarget)
	{
		AfxMessageBox(IDS_MEMORY_ERROR);
		return FALSE;
	}

	//Register our drop target			
	if (!m_pDropTarget->Register(&m_lstFTPItems))
	{
		AfxMessageBox(IDS_DROPTARGET_REGISTER_FAILED);
	}

	//Set the button text back to "Start Downloads"
	CString strButtonText(_T(""));
	strButtonText.LoadString(IDS_BUTTON_NOTRUNNING);
	m_btnDownload.SetWindowText(strButtonText);

	//Create a modem connection
	m_pModemConnection = new CModemConnection(this);

	SetDownloadState(WAITING);
	m_lstConnections.RemoveAll();

	UpdateMenuOptions(GetMenu());

	OnSelchangeComboWhenDownload();

	m_btnDownload.SetFocus();

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CDownloadDemonDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDownloadDemonDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();

		// Only display registration stuff on first go-round
		if (!m_bRegistrationShown)
		{
			m_bRegistrationShown = TRUE;
			/*			
			if (m_dlgRegister.CheckRegistrationStatus() != CDlgRegister::LICENSED)
			{
				m_dlgRegister.DoModal();
				if (m_dlgRegister.CheckRegistrationStatus() == CDlgRegister::EXPIRED)
				{
					AfxMessageBox(IDS_LICENSE_PERIOD_OVER);
					EndDialog(0);
				}
			}
			*/

			if (!m_bLiabilityShown)
			{
				m_bLiabilityShown = TRUE;
				//Show the Liability agreement
				CDlgWaiver dlgWaiver;
				if (dlgWaiver.DoModal() == IDCANCEL)
				{
					AfxMessageBox(IDS_AGREE_TERMS);
					EndDialog(0);
				}
			}

			// Parse the command line and process it
			// if there were parameters on there
			ParseCommandLine(AfxGetApp()->m_lpCmdLine);
			if (m_lstFTPItems.GetItemCount() > 0)
			{
				OnButtonDownload();				
			}
		}		
	}
}

void CDownloadDemonDlg::OnMenuSettings() 
{	
	m_dlgSettings.DoModal();
}

void CDownloadDemonDlg::OnMenuExit() 
{
	//If the user elects to terminate
	if (CloseFTPDialog())
	{
		// Ensure any dialing threads terminate
		m_bRunning = FALSE;

		CDlgStoppingNotice dlgStopping(this);
		dlgStopping.SetMaximumWaitTime(m_dlgSettings.m_iConnectionTimeout);
		dlgStopping.AllowStopTrick();
		dlgStopping.DoModal();

		//Give a little bit of time for cleanup to occur
		Sleep(SLEEP_INTERVAL);

		EndDialog(0);
	}
}

void CDownloadDemonDlg::OnClose() 
{
	OnMenuExit();
}


void CDownloadDemonDlg::OnButtonDownload() 
{
	m_bCancelledDownload = FALSE;

	m_lstFTPItems.SetFocus();

	//If we're pressed "Cancel"
	if (GetThreadsUsed() > 0 || m_pModemConnection->GetRunningConnect())
	{
		if (m_tsDownloading == DOWNLOADING)
		{
			//If we're currently downloading and they want to exit
			//Ask the user if they want to cancel the downloads
			if (AfxMessageBox(IDS_RUNNING_CANCEL, MB_YESNO) == IDYES)
			{
				//Here, we're just going to wait for the threads to stop
				//So, set the Downloading flag to false and then the 
				//monitor thread will clear up the menu items to display
				//and such when it's done.
				m_tsDownloading = WAITING;
				m_bCancelledDownload = TRUE;
			}			
		} 
		else //we're still waiting for system to stop
		{
		}
		
		return;
	}
	else
	{
		BOOL bFilesToDownload(FALSE);
		BOOL bAskedReset(FALSE);
		//Check to see if any items are stopped.  If they are, 
		//prompt user to reset them
		int i = 0;
		for (i = 0; i<m_lstFTPItems.GetItemCount(); i++)
		{
			int iStatus = m_lstFTPItems.GetItemStatus(i);

			//Set a flag that we can actually download files
			if (iStatus == CDownloadListCtrl.WAIT)
				bFilesToDownload = TRUE;

			if ((iStatus == CDownloadListCtrl.STOPPED) ||
				(iStatus == CDownloadListCtrl.SKIPPED) ||
				(iStatus == CDownloadListCtrl.FAILED))
			{
				if (!bAskedReset && (AfxMessageBox(IDS_RESETSTOPPEDFAILEDSKIPPED, MB_YESNO) == IDYES))
				{
					//only allow asking once
					bAskedReset = TRUE;

					CString strStatus(_T(""));
					strStatus.LoadString(IDS_NOTSTARTED);
					for (int j = i; j<m_lstFTPItems.GetItemCount(); j++)
					{
						int iStatus2 = m_lstFTPItems.GetItemStatus(j);
						if ((iStatus2 == CDownloadListCtrl.STOPPED) ||
							(iStatus2 == CDownloadListCtrl.SKIPPED) ||
							(iStatus2 == CDownloadListCtrl.FAILED))
						{
							m_lstFTPItems.UpdateStatus(j, CDownloadListCtrl.WAIT, strStatus);
							bFilesToDownload = TRUE;
						}
					}
				} else
					bAskedReset = TRUE;
			}
		}


		//If there's nothing to download, bail out.
		if (!bFilesToDownload)
		{
			AfxMessageBox(IDS_NOTHING_TO_DOWNLOAD);
			return;
		}

		//Validate directory to place downloaded files
		if (!m_dlgSettings.m_strDownloadDir.GetLength())
		{
			AfxMessageBox(IDS_MUST_SPECIFY_DOWNLOAD_DIR);
			return;
		}

		// Display a message asking to register if not licensed
		/*
		if (m_dlgRegister.CheckRegistrationStatus() != CDlgRegister::LICENSED)
		{
			AfxMessageBox(IDS_ASK_REGISTER);
		}
		*/

		if (m_iWhenDownload != DOWNLOADNOW)
		{			
			if (!ValidateTimeStart())
			{
				if (m_iWhenDownload == DOWNLOADAT)
					AfxMessageBox(IDS_INVALID_TIME_AT);
				else
					AfxMessageBox(IDS_INVALID_TIME_IN);
				return;
			}
		}
		AfxBeginThread(WorkerCountDown, this);

		m_comboWhenStart.EnableWindow(FALSE);
		m_comboTimeStart.EnableWindow(FALSE);

		// Set the button text to "Cancel"
		CString strButtonText(_T(""));
		strButtonText.LoadString(IDS_BUTTON_RUNNING);
		m_btnDownload.SetWindowText(strButtonText);

		SetDownloadState(DOWNLOADING);

		// Set all threads to be processing according to if we will
		// create a thread for this number
		for (i=0; i<MAX_THREADS; i++)
			bProcessing[i] = (i<=m_dlgSettings.m_iNumSimultDownloads);

		// Launch the configured number of threads
		for (i=0; i<=m_dlgSettings.m_iNumSimultDownloads; i++)
		{
			// Note the Thread Info gets deleted by WorkerDownloadFiles
			SThreadInfo* pThreadInfo = new SThreadInfo;

			// Set up information to go to worker threads
			pThreadInfo->pDDDialog = this;
			pThreadInfo->iThreadNum = i;

			// increment threads started to GUARANTEE that the 
			// monitor thread is working with at least the correct number
			IncrementThreadsUsed();

			// Launch worker thread.  This one is responsible for decrementing
			// the thread count.
			AfxBeginThread(WorkerDownloadFiles, pThreadInfo);
		}
		AfxBeginThread(WorkerMonitorDownloads, this);		
	}
}

void CDownloadDemonDlg::IncrementThreadsUsed()
{
	InterlockedIncrement(&m_lThreadsStarted);
}

// Decrements threads currently running in a controlled fashion
void CDownloadDemonDlg::DecrementThreadsUsed()
{
	InterlockedDecrement(&m_lThreadsStarted);
}

// Gets number of threads currently running in a controlled fashion
int CDownloadDemonDlg::GetThreadsUsed()
{
	int iThreadCopy(0);

	CSingleLock cslThreadLock(&m_csThreadCount, FALSE);

	cslThreadLock.Lock();
	iThreadCopy = m_lThreadsStarted;
	cslThreadLock.Unlock();

	return iThreadCopy;
}

int CDownloadDemonDlg::GetFile(CString& strConnection, int iDownloadType)
{
	CSingleLock cslThreadLock(&m_csAdjustDownloadList, FALSE);	
	CString strServer(_T(""));
	CString strObject(_T(""));
	DWORD dwServiceType(0);
	INTERNET_PORT nPort(0);
	CString strFileToDownload(_T(""));
	int iItem(-1); //Initialize to failure
	int iXferMode(0);
	CString strConnect(_T(""));

	strConnect.LoadString(IDS_CONNECT);
	
	cslThreadLock.Lock();

	for (int i = 0; i < m_lstFTPItems.GetItemCount(); i++)
	{
		int iImage(0);
		CString strFullPath(_T(""));
		CString strStrip(_T(""));
		CString strDestDir(_T(""));
		CString strUser(_T(""));
		CString strPW(_T(""));
		CString strCustomReferrer(_T(""));
		//Get the next item that has a waiting image
		if ((m_lstFTPItems.RetrieveItem(i, iImage, strFullPath, iXferMode, 
										strDestDir, strStrip, strUser, strPW,
										strCustomReferrer)) && 
			(iImage == 0))
		{
			AfxParseURL(strFullPath, dwServiceType, strServer, strObject, nPort);

			int j(0);
			BOOL bUsed = FALSE;
			POSITION pos = 0;

			switch (iDownloadType)
			{
				case USECONNECTION:
					if (strConnection == strServer)
					{
						strFileToDownload = strFullPath;
					}
					break;
				case UNUSEDCONNECTION:
					pos = m_lstConnections.GetHeadPosition();
					for (j = 0; j < m_lstConnections.GetCount(); j++)
					{
						if (pos != NULL)
						{
							if (m_lstConnections.GetNext(pos) == strServer)
							{
								bUsed = TRUE;
								break;
							}
						} else {
							bUsed = TRUE;
						}
					}
					//If this server isn't used by any connections,
					//use it here
					if (!bUsed)
					{
						strConnection = strServer;
						strFileToDownload = strFullPath;
					}
					break;
				case ANYCONNECTION:
					strConnection = strServer;
					strFileToDownload = strFullPath;
					break;
				default:
					break;
			}

			//If we found a file, drop out of this loop
			if (strFileToDownload.GetLength())
			{
				//This is the item we'll be using
				iItem = i;

				//Set this guy's state to downloading
				m_lstFTPItems.UpdateStatus(iItem, CDownloadListCtrl.CONNECT, strConnect);

				//Make this one of the connections we're using
				m_lstConnections.AddTail(strServer);

				break;
			}
		}
	}

	cslThreadLock.Unlock();
	return iItem;
}

BOOL CDownloadDemonDlg::AskForFile(CString& strConnection, int& iItem, int iThreadNum)
{
	BOOL bSuccess(FALSE);
	iItem = -1;

	// If we're using SmartDownload, try to get a file for this connection
	if ((m_dlgSettings.m_bSmartDownload) && (strConnection.GetLength()))
		iItem = GetFile(strConnection, USECONNECTION);
	
	// If we haven't found a file, try to get a file for a connection that isn't used
	if (iItem == -1)
		iItem = GetFile(strConnection, UNUSEDCONNECTION);

	// If we haven't found a file and we are allowing multiple connections, 
	// try to get a file for a connection that is already used.
	if ((iItem == -1) && (m_dlgSettings.m_bMultipleIdenticalConnections))
		iItem = GetFile(strConnection, ANYCONNECTION);

	// If we found a file to download, we've got success
	// Indicate this thread is processing
	if (iItem >= 0)
	{
		bProcessing[iThreadNum] = TRUE;
		bSuccess = TRUE;
	}

	return bSuccess;
}

// strWriteDir winds up being changed to the proper output directory
void CDownloadDemonDlg::GetLocalFileToWrite(CString& strFileToWrite, CString& strWriteDir, CString strFullPath)
{
	CString strTemp(strFullPath);

	strFileToWrite = strFullPath;

	int iCutLoc1 = strTemp.ReverseFind(_T('\\'));
	int iCutLoc2 = strTemp.ReverseFind(_T('/'));

	if ((iCutLoc1 != -1) || (iCutLoc2 != -1))
	{
		iCutLoc1 = max(iCutLoc1, iCutLoc2);

		strFileToWrite = strFullPath.Mid(iCutLoc1 + 1);
	}

	CString strDefaultDir(_T(""));
	strDefaultDir.LoadString(IDS_DEFAULT_DESTDIR);
	if (strWriteDir == strDefaultDir)
		strWriteDir = m_dlgSettings.m_strDownloadDir;

	strWriteDir.TrimLeft();
	strWriteDir.TrimRight();
	if (strWriteDir.Right(1) != _T("\\"))
		strWriteDir += _T("\\");

	strFileToWrite = strWriteDir + strFileToWrite;
}

UINT CDownloadDemonDlg::DownloadFiles(int iThreadNum)
{
	//Wait until we can download
	while (m_iWhenDownload != DOWNLOADNOW)
	{
		if (!AllowDownload())
			return 0;
		Sleep(SLEEP_INTERVAL);
	}

	//NOTES: HTTP has individual sessions for each file downloaded
	//       FTP has one persistent session.

	CString strConnection(_T("")); //machine this worker is connected to 
	int iDownloadItem(0);
	//Internet information
	CFtpConnection* pFtpConnection = NULL;		
	//Variables for retrieval item info
	int iImage(0);
	CString strFullPath(_T(""));
	int iXferMode(0);
	CString strStrip(_T(""));
	CString strDestDir(_T(""));
	CString strUser(_T(""));
	CString strPassword(_T(""));
	CString strCustomReferrer(_T(""));
	//Variables for details of retrieval item
	DWORD dwServiceType(0);
	CString strServer(_T(""));
	CString strObject(_T(""));
	INTERNET_PORT nPort(0);
	BYTE Buffer[DOWNLOAD_BUFFER_SIZE];
	void* pBuffer = &Buffer;
	DWORD dwDownloadedBytes(0);
	CFile fileDownloaded;
	BOOL bSessionOpened(FALSE);
	int iAttempts(0);

	CString strCurrentServer(_T(""));
	CString strStopped(_T(""));
	CString strSkipped(_T(""));
	CString strFailure(_T(""));
	CString strLooking(_T(""));
	CString strNotFound(_T(""));
	CString strConnect(_T(""));	
	CString strOK(_T(""));

	//Load strings that indicate status
	strStopped.LoadString(IDS_STOPPED);
	strSkipped.LoadString(IDS_SKIPPED);
	strFailure.LoadString(IDS_FAILURE);
	strLooking.LoadString(IDS_LOOKING);
	strNotFound.LoadString(IDS_NOTFOUND);
	strConnect.LoadString(IDS_CONNECT);
	strOK.LoadString(IDS_OK);

	CInternetSession* pInetSession = GetInternetSession();

	while (1)
	{
		iDownloadItem = -1; //assume invalid in case AllowDownload fails first below		

		// we're not allowed to run or don't have a file to download
		BOOL bTerminate;
		// Keep all the threads running until processing is complete
		while (!AllowDownload() || !AskForFile(strConnection, iDownloadItem, iThreadNum))
		{
			// Indicate that this thread is not processing
			bProcessing[iThreadNum] = FALSE;

			for (int i=0; i<MAX_THREADS; i++)
			{
				bTerminate = TRUE;
				if (bProcessing[i] == TRUE)
				{
					bTerminate = FALSE;
					break;
				}
			}

			// Stop if we're to Terminate or if no downloading is allowed
			if (bTerminate || !AllowDownload())
			{
				//Close up any connections
				DeleteFtpConnection(&pFtpConnection);
				DeleteInetSession(&pInetSession);

				//Don't update display if this is invalid
				if (iDownloadItem != -1)
				{
					m_lstFTPItems.UpdateStatus(iDownloadItem, CDownloadListCtrl.STOPPED, 
											   strStopped);				
				}
				return 0;
			}
			else
			{
				Sleep(SLEEP_INTERVAL * 4);
			}
		} 
/*
else
	// Indicate that this thread is processing
	bProcessing[i] = TRUE;
*/

		CString strKDownloaded(_T(""));
		BOOL bDefaultDestDir(TRUE);
		BOOL bDefaultUserPassword(TRUE);
		//only use user and password if specified, else default
		CString strDefaultDestDir(_T(""));
		CString strDefaultUser(_T(""));
		CString strDefaultPW(_T(""));
		strDefaultDestDir.LoadString(IDS_DEFAULT_DESTDIR);
		strDefaultUser.LoadString(IDS_DEFAULT_USER);
		strDefaultPW.LoadString(IDS_DEFAULT_PW);

		m_lstFTPItems.RetrieveItem(iDownloadItem, iImage, strFullPath, 
								   iXferMode, strStrip, strDestDir,
								   strUser, strPassword, strCustomReferrer);
		// Determine if we shouldn't be using defaults
		if (strDestDir != strDefaultDestDir)
			bDefaultDestDir = FALSE;
		if ((strUser != strDefaultUser) || (strPassword != strDefaultPW))
			bDefaultUserPassword = FALSE;

		CString strWriteFile(_T(""));		
		GetLocalFileToWrite(strWriteFile, strDestDir, strFullPath);

		//User wants to skip files that already exist
		if (m_dlgSettings.m_iRadioDupeSkip == 0) 
		{
			WIN32_FIND_DATA fd;
			HANDLE h = ::FindFirstFile(strWriteFile.GetBuffer(0), &fd);
			strWriteFile.ReleaseBuffer();

			//If the file was found, we might have to skip it
			if (h != INVALID_HANDLE_VALUE)
			{
				m_lstFTPItems.UpdateStatus(iDownloadItem, CDownloadListCtrl.SKIPPED, strSkipped);
				//we're done with this one.
				continue;
			}
		}

		//If we are using default passwords, set them up
		if (bDefaultDestDir)
		{
			strUser = m_dlgSettings.m_strDefaultLogin;
			strPassword = m_dlgSettings.m_strDefaultPassword;
		}

		//Figure out exactly what we need to get
		AfxParseURL(strFullPath, dwServiceType, strServer, strObject, nPort);

		//we've not tried to get this item yet.
		iAttempts = 0;

		//Close existing connection if we changed servers
		//We must check only FTP connections since HTTP is always a new connection
		if (pFtpConnection && (strCurrentServer != strServer))
			DeleteFtpConnection(&pFtpConnection);

		//Do appropriate action
		if (dwServiceType == AFX_INET_SERVICE_FTP)
		{
			CInternetFile* pFtpFile = NULL;

			//Open FTP connection if we don't already have one.
			if (!pFtpConnection)
			{
				//Do until we get a connection
				//Handle timeout will handle Terminations
				BOOL bContinue = TRUE;
				while (bContinue)
				{
					try 
					{
						//Set this guy's state to downloading
						m_lstFTPItems.UpdateStatus(iDownloadItem, CDownloadListCtrl.CONNECT, strConnect);

						//Open a new FTP connection
						pFtpConnection = pInetSession->GetFtpConnection(strServer, strUser, strPassword, nPort);
						bContinue = FALSE;
					}
					catch (CInternetException* pInetEx)
					{
						DeleteFtpConnection(&pFtpConnection);

						if (TimeoutAssociatedError(pInetEx->m_dwError))
							bContinue = HandleConnectTimeout(iDownloadItem, ++iAttempts);
						else //some other error
							bContinue = FALSE;

						pInetEx->Delete();
					}
				}
			}	
			if (pFtpConnection && AllowDownload())
			{
				BOOL bError(FALSE);
				try
				{
					m_lstFTPItems.UpdateStatus(iDownloadItem, CDownloadListCtrl.LOOKING, strLooking);

					//Open the new file
					DWORD dwTransferFlags = FTP_TRANSFER_TYPE_BINARY;
					if (iXferMode == CDlgProperties::ASCII)
						dwTransferFlags = FTP_TRANSFER_TYPE_ASCII;
					pFtpFile = pFtpConnection->OpenFile(strObject, GENERIC_READ, 
														dwTransferFlags);
				}
				catch(...)
				{
					m_lstFTPItems.UpdateStatus(iDownloadItem, CDownloadListCtrl.NOTFOUND, strNotFound);

					DeleteFtpFile(&pFtpFile);
					bError = TRUE;
				}

				if (pFtpFile)
				{
					DWORD dwDownloaded(0);
					BOOL bFileOpened(FALSE);
					BOOL bSuccess(TRUE);

					//Update display
					m_lstFTPItems.UpdateStatus(iDownloadItem, CDownloadListCtrl.TRANSFER, strOK);

					TransferStatus ftpStatus = XFER_PROCESSING;

					//Read in the file
					try
					{
						while (bSuccess && 
							   ((CDownloadDemonApp*)AfxGetApp())->m_iHiddenRegistrationValid && 
							   (dwDownloadedBytes = pFtpFile->Read(pBuffer, DOWNLOAD_BUFFER_SIZE)))
						{
							if (dwDownloaded == 0)
							{
								//Open a file for what we're going to get
								BOOL bSuccess = fileDownloaded.Open(strWriteFile,
																	CFile::modeCreate | CFile::modeWrite |
																	CFile::shareExclusive);
								bFileOpened = bSuccess;
							}

							if (bFileOpened)
							{
								//write to disk
								try 
								{
									fileDownloaded.Write(pBuffer, dwDownloadedBytes);
								}
								catch(...)
								{
									//Some sort of file write error
									ftpStatus = XFER_ERROR_FILEWRITE;
									bSuccess = FALSE;
									bError = TRUE;
								}

								//Update display of bytes downloaded for status
								dwDownloaded += dwDownloadedBytes;
								strKDownloaded.Format("%dK", dwDownloaded / 1024);
								m_lstFTPItems.UpdateStatus(iDownloadItem, CDownloadListCtrl.TRANSFER, 
														   strKDownloaded);

								//Check to see if we're running
								if (!AllowDownload())
								{
									m_lstFTPItems.UpdateStatus(iDownloadItem, CDownloadListCtrl.STOPPED, 
															   strStopped);

									if (bFileOpened)
										fileDownloaded.Close();

									DeleteFtpFile(&pFtpFile);
									DeleteFtpConnection(&pFtpConnection);
									DeleteInetSession(&pInetSession);
									return 0;
								}
							} else { //File not opened, we have an error
								ftpStatus = XFER_ERROR_FILEOPEN;
								bSuccess = FALSE;
								bError = TRUE;
							}
						}
					}
					catch(...)
					{
						//File download error
						bError = TRUE;
					}

					//Close the file
					DeleteFtpFile(&pFtpFile);

					if (bFileOpened)
						fileDownloaded.Close();

					//Display finished bitmap
					if (!bError)
						m_lstFTPItems.UpdateStatus(iDownloadItem, CDownloadListCtrl.FINISHED, strKDownloaded);
					else //error
					{
						//Display appropriate error
						CString strNotFound(_T(""));
						CString strErrFileOpen(_T(""));
						CString strErrFileWrite(_T(""));
						strNotFound.LoadString(IDS_NOTFOUND);
						strErrFileOpen.LoadString(IDS_ERRFILEOPEN);
						strErrFileWrite.LoadString(IDS_ERRFILEWRITE);
						switch (ftpStatus)
						{
							case XFER_ERROR_FILEOPEN:
								m_lstFTPItems.UpdateStatus(iDownloadItem, CDownloadListCtrl.FILEOPENERR, strErrFileOpen);
								break;
							case XFER_ERROR_FILEWRITE:
								m_lstFTPItems.UpdateStatus(iDownloadItem, CDownloadListCtrl.FILEWRITEERR, strErrFileWrite);
								break;
							default:
								m_lstFTPItems.UpdateStatus(iDownloadItem, CDownloadListCtrl.FAILED, strFailure);
								if (bFileOpened)
									DeleteFile(strWriteFile);
								break;
						}
					}
				}
			}
			else //No file, thus, there's been an error of some sort
			{
				if (!AllowDownload())
				{
					m_lstFTPItems.UpdateStatus(iDownloadItem, CDownloadListCtrl.STOPPED, strFailure);
				}
				else
				{
					m_lstFTPItems.UpdateStatus(iDownloadItem, CDownloadListCtrl.FAILED, strFailure);
				}
			}

		}
		else if (dwServiceType == AFX_INET_SERVICE_HTTP)
		{			
			BOOL bMakeAttempt(TRUE);

			while (bMakeAttempt && AllowDownload())
			{
				CWorkerHttpTransfer* pWorkerHttpData = new CWorkerHttpTransfer(this, strFullPath, strWriteFile, strDestDir, strUser, strPassword, strStrip, strCustomReferrer, bDefaultDestDir, bDefaultUserPassword, iDownloadItem);

				AfxBeginThread(WorkerDownloadHTTP, pWorkerHttpData);
				
				// Wait for processing to be completed
				while (!pWorkerHttpData->m_bDone)
				{
					Sleep(SLEEP_INTERVAL / 2);
				}

				// Analyze return code to see if timeout
				if (pWorkerHttpData->m_httpStatus == XFER_ERROR_TIMEOUT)
					bMakeAttempt = HandleConnectTimeout(iDownloadItem, ++iAttempts);
				else
					bMakeAttempt = FALSE;

				delete pWorkerHttpData;
				pWorkerHttpData = NULL;
			}				
		}
	}

	//Success - we never get here anyway
	DeleteInetSession(&pInetSession);

	return 1;
}

UINT CDownloadDemonDlg::MonitorDownloads()
{
	BOOL bStoppingNotice(FALSE);
	CDlgStoppingNotice dlgStopping(this);

	dlgStopping.SetMaximumWaitTime(m_dlgSettings.m_iConnectionTimeout);

	while (1)
	{
		//Sleep 1/4 second so we don't thrash CPU
		Sleep(SLEEP_INTERVAL);
		if (GetThreadsUsed() == 0)
			break;

		//End the monitor if we're shutting program down
		if (m_tsDownloading == TERMINATING)
			return 0;

		//If the downloads have been stopped by the user
		//Display the stopping dialog (since they might need to wait)
		if ((m_tsDownloading != DOWNLOADING) && 
			(m_tsDownloading != TERMINATING) && //let the above catch this
			!bStoppingNotice)
		{
			bStoppingNotice = TRUE;
			dlgStopping.DoModal();
			//Issue a break, since if the dialog disappears,
			//we know we're done here.
			break;
		}
	}

	//Set the button text back to "Start Downloads"
	CString strButtonText(_T(""));
	strButtonText.LoadString(IDS_BUTTON_NOTRUNNING);
	m_btnDownload.SetWindowText(strButtonText);
	SetDownloadState(WAITING);

	//Cleanup
	m_lstConnections.RemoveAll();
	m_comboWhenStart.EnableWindow(TRUE);
	m_comboTimeStart.EnableWindow(TRUE);

	//Disconnect modem if applicable
	if (m_chkDisconnectDone.GetCheck() == 1)
	{
		if (!m_bCancelledDownload || (AfxMessageBox(IDS_SURE_DISCONNECT, MB_YESNO) == IDYES))
		{
			m_pModemConnection->Disconnect();
		}
	}
	m_bCancelledDownload = FALSE;

	// Exit if we had everything on the command line
	if (m_bAutoURLTerminate)
	{
		OnMenuExit();
	}

	return 0;
}

BOOL CDownloadDemonDlg::AllowDownload()
{
	return (m_tsDownloading == DOWNLOADING);
}

void CDownloadDemonDlg::WinHelp(DWORD dwData, UINT nCmd) 
{
	return; //do nothing for help
}

void CDownloadDemonDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	//Ensure this guy is a window
	//If you don't the first time this hits, we crash
	if (::IsWindow(m_btnDownload))
	{
		CRect rectDownload;
		CRect rectStartWhen;
		CRect rectStartTime;
		CRect rectDisconnectDone;
		
		//Find out how big the download controls are
		m_btnDownload.GetClientRect(&rectDownload);
		m_comboWhenStart.GetClientRect(&rectStartWhen);
		m_comboTimeStart.GetClientRect(&rectStartTime);
		m_chkDisconnectDone.GetClientRect(&rectDisconnectDone);

		int iTotalWidth = rectDownload.Width() + SPACING + rectStartWhen.Width() + SPACING + rectStartTime.Width();

		int iXLoc(0);
		int iYLoc(0);

		if (m_dlgSettings.m_strlConnections.GetCount())
		{
			iYLoc = cy - SPACING - rectDisconnectDone.Height();
			m_chkDisconnectDone.MoveWindow((cx - rectDisconnectDone.Width()) / 2, iYLoc,									   
										   rectDisconnectDone.Width(), rectDisconnectDone.Height());
		} 
		else
		{
			m_chkDisconnectDone.ShowWindow(SW_HIDE);
			
			// The next iYLoc i is dependant on where we are now
			iYLoc = cy;
		}
		iYLoc = iYLoc - SPACING - rectDownload.Height();

		iXLoc = (cx - iTotalWidth) / 2;		
		//Place the download button on the form
		m_btnDownload.MoveWindow(iXLoc, iYLoc,
								 rectDownload.Width(), rectDownload.Height());
		iXLoc += rectDownload.Width() + SPACING;
		m_comboWhenStart.MoveWindow(iXLoc, iYLoc,
									rectStartWhen.Width(), rectDownload.Height()*5);
		iXLoc += rectStartWhen.Width() + SPACING;
		m_comboTimeStart.MoveWindow(iXLoc, iYLoc, 
								   rectStartTime.Width(), rectStartWhen.Height());

		//Place the list on the form
		m_lstFTPItems.MoveWindow(SPACING, SPACING, cx-SPACING*2, iYLoc - rectDownload.Height() + SPACING);
	}
}

void CDownloadDemonDlg::LoadWindowRegistrySettings(BOOL bLoad, RECT& rectSettings, CString strResolution)
{
	COXRegistry regWindowSettings;

	if (regWindowSettings.Connect(COXRegistry::keyLocalMachine))
	{
		CString strRegKey(_T(""));
		strRegKey.LoadString(IDS_REGISTRY_KEY);
		if (regWindowSettings.CheckAndOpenKeyForWrite(strRegKey))
		{
			if (bLoad) //Load
			{
				RECT rectUninit = {0, 0, 0, 0};
				rectSettings = rectUninit;

				//Retrieve configuration from registry and only
				//update items from default if they were retrieved
				BOOL bSuccess(TRUE);
				DWORD dwRectSetting(0);

				CString strOldResolution(_T(""));
				bSuccess = regWindowSettings.GetValue("Resolution", strOldResolution);

				//Compare resolutions - if the resolution changed, we
				//want to fail so the dialog gets displayed with defaults
				if (bSuccess && (strResolution != strOldResolution))
					bSuccess = FALSE;

				if (bSuccess)
				{					
					bSuccess = regWindowSettings.GetValue("StartX", dwRectSetting);
				}
				if (bSuccess)
				{
					rectSettings.left = dwRectSetting;
					bSuccess = regWindowSettings.GetValue("EndX", dwRectSetting);
				}
				if (bSuccess)
				{
					rectSettings.right = dwRectSetting;
					bSuccess = regWindowSettings.GetValue("StartY", dwRectSetting);
				}
				if (bSuccess)
				{
					rectSettings.top = dwRectSetting;
					bSuccess = regWindowSettings.GetValue("EndY", dwRectSetting);
				}
				if (bSuccess)
				{
					rectSettings.bottom = dwRectSetting;
				}

				if (!bSuccess)
				{
				}
			} else { //Save
				//Save configuration to registry
				regWindowSettings.SetValue("StartX", rectSettings.left);
				regWindowSettings.SetValue("EndX", rectSettings.right-1);
				regWindowSettings.SetValue("StartY", rectSettings.top);
				regWindowSettings.SetValue("EndY", rectSettings.bottom-1);
				regWindowSettings.SetValue("Resolution", GetResolutionString());
			}
		}
		regWindowSettings.Disconnect();
	}
}

CString CDownloadDemonDlg::GetResolutionString()
{
	//Converts screen resolution into a string
	DWORD dwX = GetSystemMetrics(SM_CXSCREEN);
	DWORD dwY = GetSystemMetrics(SM_CYSCREEN);

	CString strResolution(_T(""));
	strResolution.Format("%dx%d", dwX, dwY);

	return strResolution;
}

void CDownloadDemonDlg::OnMenuAbout() 
{
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}

BOOL CDownloadDemonDlg::CloseFTPDialog()
{
	if (m_tsDownloading == DOWNLOADING)
	{
		//If we're currently downloading and they want to exit
		//Ask the user if they want to terminate
		if (AfxMessageBox(IDS_RUNNING_TERMINATE, MB_YESNO) == IDNO)
		{			
			return FALSE;
		}
		else
		{
			//Set the downloading flag to TERMINATING for shutdown procedures
			m_tsDownloading = TERMINATING;			
		}
	}

	//When the application closes, save off our window settings
	RECT rectPilotDlg = {0, 0, 0, 0};
	GetWindowRect(&rectPilotDlg);
	LoadWindowRegistrySettings(FALSE, rectPilotDlg);
	m_lstFTPItems.SaveColHeadingSizes();

	return TRUE;
}

void CDownloadDemonDlg::SetDownloadState(TransferState tsDownloading)
{
	m_tsDownloading = tsDownloading;

	//Update "Settings" enabledness
	UINT iMenuEnabled = MF_ENABLED;
	if (tsDownloading != WAITING)
		iMenuEnabled = MF_GRAYED;
	CMenu* pMenu = GetMenu();
	pMenu->EnableMenuItem(ID_MENU_SETTINGS, iMenuEnabled);

	//Change whether or not we are allowing additional entries
	m_pDropTarget->AllowAdditional(tsDownloading == WAITING);
}

BOOL CDownloadDemonDlg::HandleConnectTimeout(int iItem, int iAttempt)
{
	//Don't allow retry if we've already had too many attempts
	if (iAttempt > m_dlgSettings.m_iRetryCount)
		return FALSE;

	CString strTimedOut(_T(""));
	CString strStopped(_T(""));
	
	strTimedOut.LoadString(IDS_TIMEDOUT);
	strStopped.LoadString(IDS_STOPPED);

	//Wait through the retry time, but all the while check for
	//the stopping of downloading
	for (int i = 0; i <= m_dlgSettings.m_iRetryTime; i++)
	{
		CString strTimeOutMessage(_T(""));
		strTimeOutMessage.Format(strTimedOut, m_dlgSettings.m_iRetryTime - i);
		m_lstFTPItems.UpdateStatus(iItem, CDownloadListCtrl.TIMEDOUT, strTimeOutMessage);
	
		//Check to see if we're done downloading						
		if (!AllowDownload())
		{
			m_lstFTPItems.UpdateStatus(iItem, CDownloadListCtrl.STOPPED, 
									   strStopped);
			return FALSE;
		}

		//Sleep for 1 second increments
		if (i < m_dlgSettings.m_iRetryTime)
			Sleep(1000);
	}
	return TRUE;
}



void CDownloadDemonDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	//Get the rectangle for the list of files
	//If the user picked a point in the list, show the menu
	CRect rectFileList;
	m_lstFTPItems.GetClientRect(&rectFileList);
	m_lstFTPItems.ClientToScreen(&rectFileList);

	if (rectFileList.PtInRect(point))
	{
		//setup the menu
		CMenu menu;
		if (menu.LoadMenu(IDR_POPUP_FILE_LIST))
		{
			CMenu* pPopup = menu.GetSubMenu(0);
			ASSERT(pPopup != NULL);

			//Update which items are available in the popup
			UpdateMenuOptions(pPopup);

			//Show menu
			pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,
				point.x, point.y,
				AfxGetMainWnd()); // use main window for cmds
		}
	}
}

void CDownloadDemonDlg::UpdateMenuOptions(CMenu* pMenuUpdate)
{
	//Ensure we've got a menu to update
	if (!pMenuUpdate)
		return;

	if (AllowDownload())
	{
		pMenuUpdate->EnableMenuItem(ID_ADD_NEW_FILE, MF_GRAYED);
		pMenuUpdate->EnableMenuItem(ID_DELETE_SELECTED_FILE, MF_GRAYED);
		pMenuUpdate->EnableMenuItem(ID_PROPERTIES_SELECTED_FILE, MF_GRAYED);
		pMenuUpdate->EnableMenuItem(ID_DELETE_COMPLETED_ITEMS, MF_GRAYED);

		//Update file saving options if menu is the dialog's menu
		if (pMenuUpdate == GetMenu())
		{
			pMenuUpdate->EnableMenuItem(ID_MENU_NEW_LIST, MF_GRAYED);
			pMenuUpdate->EnableMenuItem(ID_MENU_OPEN_LIST, MF_GRAYED);
			pMenuUpdate->EnableMenuItem(ID_MENU_SAVE_LIST, MF_GRAYED);
			pMenuUpdate->EnableMenuItem(ID_MENU_SAVE_LIST_AS, MF_GRAYED);
		}	
	}
	else //Setup and Show the menu
	{
		UINT nEnableStatus = MF_GRAYED;
		//Update file saving options if menu is the same
		if (pMenuUpdate == GetMenu())
		{
			//we can always new and open a list
			pMenuUpdate->EnableMenuItem(ID_MENU_NEW_LIST, MF_ENABLED);
			pMenuUpdate->EnableMenuItem(ID_MENU_OPEN_LIST, MF_ENABLED);

			//we can save as if there are items in the list and 
			//we have a filename already in use
			nEnableStatus = MF_GRAYED;
			if (m_lstFTPItems.GetItemCount() && m_strFilename.GetLength())
				nEnableStatus = MF_ENABLED;
			pMenuUpdate->EnableMenuItem(ID_MENU_SAVE_LIST, nEnableStatus);

			//we can save as if there are items in the list
			nEnableStatus = MF_GRAYED;
			if (m_lstFTPItems.GetItemCount())
				nEnableStatus = MF_ENABLED;
			pMenuUpdate->EnableMenuItem(ID_MENU_SAVE_LIST_AS, nEnableStatus);
		}

		//Enable delete based on item selected
		nEnableStatus = MF_GRAYED;
		if (m_lstFTPItems.GetSelectedCount() > 0)
			nEnableStatus = MF_ENABLED;
		pMenuUpdate->EnableMenuItem(ID_DELETE_SELECTED_FILE, nEnableStatus);

		//Enable properties based on item selected and not completed
		nEnableStatus = MF_GRAYED;
		if (m_lstFTPItems.GetSelectedCount() == 1)
		{
			for (int i = 0; i<m_lstFTPItems.GetItemCount(); i++)
			{
				if ((m_lstFTPItems.GetItemState(i, LVIS_SELECTED)) &&
					(m_lstFTPItems.GetItemStatus(i) != CDownloadListCtrl.FINISHED))
				{
					nEnableStatus = MF_ENABLED;
				}
			}
		}
		pMenuUpdate->EnableMenuItem(ID_PROPERTIES_SELECTED_FILE, nEnableStatus);

		//Enable delete completed items based on items with completed status
		//existing
		nEnableStatus = MF_GRAYED;
		if (m_lstFTPItems.GetCountForStatus(CDownloadListCtrl.FINISHED) > 0)
			nEnableStatus = MF_ENABLED;
		pMenuUpdate->EnableMenuItem(ID_DELETE_COMPLETED_ITEMS, nEnableStatus);
	}
}

BOOL CDownloadDemonDlg::OnCmdMsg( UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo )
{
	//Update the action menu whenever a message is received
	//This will guarantee that its status is up to date.
	UpdateMenuOptions(GetMenu());
	
	BOOL bSuccess = CCmdTarget::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
	return bSuccess;
}


//Functions associated with menu
void CDownloadDemonDlg::OnAddNewFile() 
{	
	if (m_tsDownloading == WAITING)
	{
		CDlgProperties dlgAdd;
		dlgAdd.SetAddMode();
		if (dlgAdd.DoModal() == IDOK)
		{
			CString strStrip(_T(""));

			// Figure out which strip string
			if (dlgAdd.m_iStrip == CDlgProperties::STRIP_YES)
				strStrip.LoadString(IDS_YES);
			else
				strStrip.LoadString(IDS_NO);

			CString strUseUser(_T(""));
			CString strUsePW(_T(""));
			CString strUseDir(_T(""));
			//If user specified user and password
			if (dlgAdd.m_bUseSpecificLoginInfo)
			{
				strUseUser = dlgAdd.m_strLogin;
				strUsePW = dlgAdd.m_strPassword;
			}
			// If user specified path
			if (dlgAdd.m_bSpecifyDestDir)
			{
				strUseDir = dlgAdd.m_strEditDestDir;
			}

			m_lstFTPItems.AddItem(dlgAdd.m_strLink, dlgAdd.m_iTransferMode, strStrip,
								  TRUE, strUseDir, strUseUser, strUsePW);
		}
	}
}

void CDownloadDemonDlg::OnDeleteCompletedItems() 
{
	//Delete all the finished items
	m_lstFTPItems.DeleteAllItemsWithStatus(CDownloadListCtrl.FINISHED);
}

void CDownloadDemonDlg::OnDeleteSelectedFile() 
{
	if (m_tsDownloading == WAITING)
	{
		//Delete all items that are selected
		m_lstFTPItems.DeleteSelectedItems();
	}
}

void CDownloadDemonDlg::OnPropertiesSelectedFile() 
{
	int iItemSelected(0);
	BOOL bFound(FALSE);

	//Ensure there's only one selected
	if (m_lstFTPItems.GetSelectedCount() != 1)
		return;
	
	//Find the item that's selected
	for (iItemSelected = 0; iItemSelected < m_lstFTPItems.GetItemCount(); iItemSelected++)
	{
		if (m_lstFTPItems.GetItemState(iItemSelected, LVIS_SELECTED))
		{
			bFound = TRUE;
			break;
		}
	}
	//Ensure we actually found something
 	if (!bFound)
		return;

	int iImage(0);
	CString strFullPath(_T(""));
	int iMode(0);
	CString strYes(_T(""));
	strYes.LoadString(IDS_YES);
	CString strStrip(_T(""));
	CString strDestDir(_T(""));
	CString strUser(_T(""));
	CString strPW(_T(""));	
	CString strCustomReferrer(_T(""));
	m_lstFTPItems.RetrieveItem(iItemSelected, iImage, strFullPath, 
							   iMode, strStrip, strDestDir, strUser, strPW,
							   strCustomReferrer);

	CDlgProperties dlgProp;

	//Populate the properties dialog with this guy's data
	dlgProp.m_strLink = strFullPath;
	dlgProp.m_iTransferMode = iMode;
	dlgProp.m_iStrip = (strStrip != strYes) ? CDlgProperties::STRIP_NO : CDlgProperties::STRIP_YES;

	CString strDefaultDir(_T(""));
	strDefaultDir.LoadString(IDS_DEFAULT_DESTDIR);

	//only use user and password if specified, else default
	CString strDefaultDestDir(_T(""));
	CString strDefaultUser(_T(""));
	CString strDefaultPW(_T(""));
	strDefaultDestDir.LoadString(IDS_DEFAULT_DESTDIR);
	strDefaultUser.LoadString(IDS_DEFAULT_USER);
	strDefaultPW.LoadString(IDS_DEFAULT_PW);

	//If we're not using the default destination directory, fill in
	if (strDestDir != strDefaultDestDir)
	{
		dlgProp.m_strEditDestDir = strDestDir;
		dlgProp.m_bSpecifyDestDir = TRUE;
	}

	//If we're not using the default password stuff, fill in
	if ((strUser != strDefaultUser) || (strPW != strDefaultPW))
	{
		dlgProp.m_bUseSpecificLoginInfo = TRUE;
		dlgProp.m_strLogin = strUser;
		dlgProp.m_strPassword = strPW;
	}
	
	if (dlgProp.DoModal() == IDOK)
	{
		// Figure out which strip string
		if (dlgProp.m_iStrip == CDlgProperties::STRIP_YES)
			strStrip.LoadString(IDS_YES);
		else
			strStrip.LoadString(IDS_NO);


		CString strUseDestDir(_T(""));
		CString strUseUser(_T(""));
		CString strUsePassword(_T(""));		

		//If user specified user and password
		if (dlgProp.m_bUseSpecificLoginInfo)
		{
			strUseUser = dlgProp.m_strLogin;
			strUsePassword = dlgProp.m_strPassword;
		}

		// If user specified path
		if (dlgProp.m_bSpecifyDestDir)
		{
			strUseDestDir = dlgProp.m_strEditDestDir;
		}

		m_lstFTPItems.UpdateItem(iItemSelected, dlgProp.m_strLink, 
								 dlgProp.m_iTransferMode, strStrip,
								 strUseDestDir, strUseUser, strUsePassword,
								 strCustomReferrer);
	}
}

CInternetSession* CDownloadDemonDlg::GetInternetSession()
{
	CInternetSession* pInternetSession;

/*	pInternetSession = new CInternetSession(_T("DownloadDemon")); , 1, INTERNET_OPEN_TYPE_DIRECT,
								 NULL, NULL, INTERNET_FLAG_DONT_CACHE); */
	pInternetSession = new CInternetSession(_T("DownloadDemon"), 1, 
											INTERNET_OPEN_TYPE_DIRECT,
											NULL, NULL, 
						 					INTERNET_FLAG_EXISTING_CONNECT |
											INTERNET_FLAG_RESYNCHRONIZE | 
											INTERNET_FLAG_DONT_CACHE |
											INTERNET_FLAG_RELOAD /*Force download*/);
	pInternetSession->SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 
						  m_dlgSettings.m_iConnectionTimeout * 1000);
	pInternetSession->SetOption(INTERNET_OPTION_CONTROL_SEND_TIMEOUT,
						  m_dlgSettings.m_iConnectionTimeout * 1000);
	pInternetSession->SetOption(INTERNET_OPTION_CONTROL_RECEIVE_TIMEOUT,
						  m_dlgSettings.m_iConnectionTimeout * 1000);
	pInternetSession->SetOption(INTERNET_OPTION_SEND_TIMEOUT,
						  m_dlgSettings.m_iConnectionTimeout * 1000);
	pInternetSession->SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT,
						  m_dlgSettings.m_iConnectionTimeout * 1000);
	pInternetSession->SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT,
						  m_dlgSettings.m_iConnectionTimeout * 1000);
	pInternetSession->SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT,
						  m_dlgSettings.m_iConnectionTimeout * 1000);

	pInternetSession->SetOption(INTERNET_OPTION_CONNECT_RETRIES, 0);
	pInternetSession->SetOption(INTERNET_OPTION_CONNECT_BACKOFF, 0);

	return pInternetSession;
}

void CDownloadDemonDlg::OnMenuNewList() 
{
	//only clear the list if we're not downloading
	if (m_tsDownloading == WAITING)
	{
		if (AfxMessageBox(IDS_SURE_CLEARLIST, MB_YESNO) == IDYES)
		{
			m_strFilename = _T("");
			m_lstFTPItems.DeleteAllItems();
		}
	}
}

void CDownloadDemonDlg::OnMenuOpenList() 
{
	BOOL bSuccess(TRUE);
	CString strExtension(_T(""));
	CString strDescription(_T(""));
	strExtension.LoadString(IDS_FILEEXTENSION);
	strDescription.LoadString(IDS_FILEDESCRIPTION);

	CFileDialog dlgFile(TRUE, strExtension, NULL, 
						OFN_HIDEREADONLY|OFN_FILEMUSTEXIST,
						strDescription, this);
	if (dlgFile.DoModal() == IDOK)
	{
		m_strFilename = dlgFile.GetPathName();
		bSuccess = m_lstFTPItems.ProcessSaveLoad(m_strFilename, FALSE, FALSE);
	}
}

void CDownloadDemonDlg::OnMenuSaveList() 
{
	BOOL bSuccess(FALSE);
	if (m_strFilename.GetLength())
	{
		bSuccess = m_lstFTPItems.ProcessSaveLoad(m_strFilename, TRUE, FALSE);
	}
}

void CDownloadDemonDlg::OnMenuSaveListAs() 
{
	CString strExtension(_T(""));
	CString strDescription(_T(""));
	strExtension.LoadString(IDS_FILEEXTENSION);
	strDescription.LoadString(IDS_FILEDESCRIPTION);
	BOOL bSuccess(FALSE);

	CFileDialog dlgFile(FALSE, strExtension, NULL, 
						OFN_HIDEREADONLY,
						strDescription, this);
	if (dlgFile.DoModal() == IDOK)
	{
		m_strFilename = dlgFile.GetPathName();
		bSuccess = m_lstFTPItems.ProcessSaveLoad(m_strFilename, TRUE, TRUE);
	}
}

void CDownloadDemonDlg::OnKeydownListFtpItems(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;
	if (pLVKeyDow->wVKey == VK_INSERT)
	{
		OnAddNewFile();
	}
	else if (pLVKeyDow->wVKey == VK_DELETE)
	{
		OnDeleteSelectedFile();
	}	
	
	*pResult = 0;
}

UINT CDownloadDemonDlg::DownloadHTTP(CWorkerHttpTransfer* pHttpData)
{
	BOOL bSuccess(TRUE);
	BOOL bFileOpened(FALSE);

	CHttpConnection* pHttpConnection = NULL;
	CHttpFile* pHttpFile = NULL;
	CInternetSession* pInetSessionHttp = NULL;

	CString strServer(_T(""));
	CString strObject(_T(""));
	DWORD dwServiceType(0);
	INTERNET_PORT nPort(0);
	AfxParseURL(pHttpData->m_strFullPath, dwServiceType, strServer, strObject, nPort);
	ASSERT(dwServiceType == AFX_INET_SERVICE_HTTP);

	CString strConnect(_T(""));
	CString strFailure(_T(""));
	CString strOK(_T(""));
	CString strStopped(_T(""));

	strConnect.LoadString(IDS_CONNECT);
	strFailure.LoadString(IDS_FAILURE);
	strOK.LoadString(IDS_OK);
	strStopped.LoadString(IDS_STOPPED);

	pInetSessionHttp = GetInternetSession();

	// Arcade At Home specific data
	BOOL bIsZipExtension = !pHttpData->m_strWriteFile.Right(4).CompareNoCase(".zip");
	CString strAddRedirectedLink(_T(""));

	//Open HTTP connection since it's always new
	try 
	{
		//Set this guy's state to downloading
		pHttpData->m_pParentDlg->m_lstFTPItems.UpdateStatus(pHttpData->m_iDownloadItem, CDownloadListCtrl.CONNECT, strConnect);

		//Open a new HTTP connection
		pHttpConnection = pInetSessionHttp->GetHttpConnection(strServer, nPort, pHttpData->m_strUser, pHttpData->m_strPassword);
	}
	catch (CInternetException* pInetEx)
	{
		bSuccess = FALSE;
		pHttpData->m_dwErrorCode = pInetEx->m_dwError;		
		DeleteHttpConnection(&pHttpConnection);

		pInetEx->Delete();
	}

	//We get here when we're done getting a connection
	//If we've got one, then....
	if (pHttpConnection)
	{
		try
		{
			CString strLooking(_T(""));
			strLooking.LoadString(IDS_LOOKING);
			m_lstFTPItems.UpdateStatus(pHttpData->m_iDownloadItem, CDownloadListCtrl.LOOKING, strLooking);

			CString strHeaders = m_strReferrerAndCookie;

			// Change the referrer if available in Arcade At Home Mode
			if (m_bArcadeAtHomeMode && m_strReferrer.GetLength() && pHttpData->m_strReferrerUpdate.GetLength())
				strHeaders = pHttpData->m_strReferrerUpdate + m_strReferrerAndCookie.Mid(m_strReferrer.GetLength());

			pHttpFile = pHttpConnection->OpenRequest(CHttpConnection::HTTP_VERB_GET, 
													 strObject, 
													 strHeaders.GetBuffer(strHeaders.GetLength()+1), 
													 1,
													 NULL /*ACCEPT TYPES MUST CHANGE*/,
													 NULL, 
													 INTERNET_FLAG_EXISTING_CONNECT |
													 INTERNET_FLAG_RESYNCHRONIZE | 
													 INTERNET_FLAG_DONT_CACHE |
													 INTERNET_FLAG_RELOAD /*Force download*/);

			m_strReferrerAndCookie.ReleaseBuffer();
			pHttpFile->SendRequest();

			// Pull the moved location from the header...  Tricky
			if (m_bArcadeAtHomeMode && bIsZipExtension)
			{
				CString strQueryInfo(_T(""));			
				pHttpFile->QueryInfo(HTTP_QUERY_RAW_HEADERS_CRLF, strQueryInfo);
				
				// Find the right header
				CString strCurrentHeader(_T(""));
				int iFoundCRLF(0);

				while (strQueryInfo.GetLength() && ((iFoundCRLF = strQueryInfo.Find("\r\n")) >= 0))
				{
					strCurrentHeader = strQueryInfo.Left(iFoundCRLF);
					if ((strCurrentHeader.Find("Location:") == 0) && strCurrentHeader.Find("ftp://"))
					{
						strAddRedirectedLink = strCurrentHeader.Mid(strCurrentHeader.Find("ftp://"));
					}

					// Move Forward
					strQueryInfo = strQueryInfo.Mid(iFoundCRLF + 2);
				}

			}
		}
		catch (CInternetException* pInetEx)
		{
			bSuccess = FALSE;
			pHttpData->m_dwErrorCode = pInetEx->m_dwError;
			DeleteHttpFile(&pHttpFile);

			pInetEx->Delete();
		}
	}	

	if (pHttpFile)
	{
		CFile fileDownloaded;
		DWORD dwDownloaded(0);
		CString strStripHtml(_T(""));

		m_lstFTPItems.UpdateStatus(pHttpData->m_iDownloadItem, CDownloadListCtrl.TRANSFER, strOK);

		BYTE Buffer[DOWNLOAD_BUFFER_SIZE+1];
		void* pBuffer = &Buffer;
		DWORD dwDownloadedBytes(0);
		CString strKDownloaded(_T(""));

		// Set a flag indicating whether we're stripping this file
		CString strYes(_T(""));
		strYes.LoadString(IDS_YES);
		BOOL bStripHtml(FALSE);

		if ((pHttpData->m_strStrip == strYes) || (m_bArcadeAtHomeMode && !bIsZipExtension))
			bStripHtml = TRUE;		

		try
		{
			while (bSuccess && (dwDownloadedBytes = pHttpFile->Read(pBuffer, DOWNLOAD_BUFFER_SIZE)))
			{
				Buffer[dwDownloadedBytes] = _T('\0');

				// Only build up strip string if we're stripping
				//if (bStripHtml)
				if (bStripHtml)
				{
					strStripHtml = strStripHtml + Buffer;
					int i = strStripHtml.GetLength();
				}

				//Check to see if we're running
				if (!AllowDownload())
				{
					m_lstFTPItems.UpdateStatus(pHttpData->m_iDownloadItem, CDownloadListCtrl.STOPPED, 
											   strStopped);

					if (bFileOpened)
						fileDownloaded.Close();
					
					DeleteHttpFile(&pHttpFile);
					DeleteHttpConnection(&pHttpConnection);
					DeleteInetSession(&pInetSessionHttp);
					pHttpData->m_httpStatus = XFER_STOPPED;
					return 0;
				}				

				//open a file for this guy
				if (dwDownloaded == 0)
				{
					if (CheckFileExistance(strObject, pBuffer) || (m_bArcadeAtHomeMode && !bIsZipExtension))
					{
						// Arcade at home only writes files that end in .zip
						if (m_bArcadeAtHomeMode && !bIsZipExtension)
						{
							bSuccess = TRUE;
							bFileOpened = FALSE;
						}
						else // we need to really open a file
						{
							bSuccess = fileDownloaded.Open(pHttpData->m_strWriteFile,
														   CFile::modeCreate | CFile::modeWrite |
														   CFile::shareExclusive);
							bFileOpened = bSuccess;
						}
						
						if (!bSuccess)
							pHttpData->m_httpStatus = XFER_ERROR_FILEOPEN;
					}
					else
					{
						pHttpData->m_httpStatus = XFER_ERROR_NOTFOUND;
						bSuccess = FALSE;
					}
				}

				if (bSuccess)
				{
					try
					{
						if (bFileOpened)
							fileDownloaded.Write(pBuffer, dwDownloadedBytes);
						
						//Update display of bytes downloaded for status
						dwDownloaded += dwDownloadedBytes;
						strKDownloaded.Format("%dK", dwDownloaded / 1024);
						m_lstFTPItems.UpdateStatus(pHttpData->m_iDownloadItem, CDownloadListCtrl.TRANSFER, strKDownloaded);
					}
					catch(...)
					{
						pHttpData->m_httpStatus = XFER_ERROR_FILEWRITE;
						bSuccess = FALSE;
					}
				}
			}
		}
		catch(...)
		{
			//File download error
			bSuccess = FALSE;
		}

		//If there's an error, update the status
		if (!bSuccess)
		{
			CString strNotFound(_T(""));
			CString strErrFileOpen(_T(""));
			CString strErrFileWrite(_T(""));
			strNotFound.LoadString(IDS_NOTFOUND);
			strErrFileOpen.LoadString(IDS_ERRFILEOPEN);
			strErrFileWrite.LoadString(IDS_ERRFILEWRITE);

			switch(pHttpData->m_httpStatus)
			{
				case XFER_ERROR_NOTFOUND:
					m_lstFTPItems.UpdateStatus(pHttpData->m_iDownloadItem, CDownloadListCtrl.NOTFOUND, strNotFound);
					break;
				case XFER_ERROR_FILEOPEN:
					m_lstFTPItems.UpdateStatus(pHttpData->m_iDownloadItem, CDownloadListCtrl.FILEOPENERR, strErrFileOpen);
					break;
				case XFER_ERROR_FILEWRITE:
					m_lstFTPItems.UpdateStatus(pHttpData->m_iDownloadItem, CDownloadListCtrl.FILEWRITEERR, strErrFileWrite);
					break;
				default:
					m_lstFTPItems.UpdateStatus(pHttpData->m_iDownloadItem, CDownloadListCtrl.FAILED, strFailure);
					if (bFileOpened)
						DeleteFile(pHttpData->m_strWriteFile);
					break;
			}
		}

		DeleteHttpFile(&pHttpFile);

		if (bFileOpened)
			fileDownloaded.Close();

		//Display finished bitmap
		if (bSuccess)
		{
			m_lstFTPItems.UpdateStatus(pHttpData->m_iDownloadItem, CDownloadListCtrl.FINISHED, strKDownloaded);
			pHttpData->m_httpStatus = XFER_SUCCESS;

			// Arcade At Home Mode will do redirects to FTP via Download Accelerator
			if (m_bArcadeAtHomeMode && strAddRedirectedLink.GetLength())
			{
				ShellExecute(this->m_hWnd, "open", m_dlgSettings.m_strDownloadAcceleratorLocation, 
							 strAddRedirectedLink, "", SW_SHOWNOACTIVATE);
				// For some reason, Download Demon can't get ArcadeAtHome files - probably PASV mode
				//AddStrippedLink(strAddRedirectedLink, "", "C:\\temp", "anonymous", "email@someplace.com", "");
			}

			// Strip Links out of HTML if that option was selected
			if (bStripHtml)
			{
				CString strUseDestDir(_T(""));
				CString strUseUser(_T(""));
				CString strUsePassword(_T(""));
				if (!pHttpData->m_bDefaultDestDir)
					strUseDestDir = pHttpData->m_strDestDir;
				if (!pHttpData->m_bDefaultUserPassword)
				{
					strUseUser = pHttpData->m_strUser;
					strUsePassword = pHttpData->m_strPassword;
				}
				StripLinks(strStripHtml, pHttpData->m_strFullPath,
						   strUseDestDir, strUseUser, strUsePassword);
			}
		}
		else
		{
			pHttpData->m_httpStatus = XFER_OTHER_ERROR;
		}
	} 
	else //No file, thus, there's been an error of some sort
	{
		if (!AllowDownload())
		{
			m_lstFTPItems.UpdateStatus(pHttpData->m_iDownloadItem, CDownloadListCtrl.STOPPED, strFailure);
			pHttpData->m_httpStatus = XFER_STOPPED;
		}
		else
		{
			m_lstFTPItems.UpdateStatus(pHttpData->m_iDownloadItem, CDownloadListCtrl.FAILED, strFailure);
			pHttpData->m_httpStatus = XFER_OTHER_ERROR;
		}
	}

	//Ensure this thread gets closed
	if (pHttpData->m_httpStatus == XFER_PROCESSING)
		pHttpData->m_httpStatus = XFER_OTHER_ERROR;

	DeleteHttpFile(&pHttpFile);
	DeleteHttpConnection(&pHttpConnection);
	DeleteInetSession(&pInetSessionHttp);	

	return 0;
}

BOOL CDownloadDemonDlg::CheckFileExistance(CString strObject, void * pBuffer)
{
	BOOL bSuccess(TRUE);
	CString strExtension(_T(""));

	strObject.TrimRight();
	int iExtension = strObject.ReverseFind(_T('.'));
	if (iExtension > 0)
		strExtension = strObject.Right(strObject.GetLength() - iExtension);
	strExtension.MakeUpper();

	if ((strExtension != _T(".HTML")) &&
		(strExtension != _T(".HTM")))
	{
		CString strHTMLText = (LPSTR)pBuffer;
		strHTMLText.TrimLeft();
		strHTMLText.MakeUpper();
		if ((strHTMLText.Left(6) == _T("<TITLE")) || 
			(strHTMLText.Left(5) == _T("<HTML")) || 
			(strHTMLText.Left(5) == _T("<BODY")) || 
			(strHTMLText.Left(5) == _T("<HEAD")) || 
			(strHTMLText.Left(2) == _T("<!")))
		{
			bSuccess = FALSE;
		}
	}

	return bSuccess;

}

void CDownloadDemonDlg::OnSelchangeComboWhenDownload() 
{
	UpdateData();
	m_comboTimeStart.EnableWindow(m_iWhenDownload != DOWNLOADNOW);
}

BOOL CDownloadDemonDlg::ValidateTimeStart()
{
	BOOL bValidTime(TRUE);
	UpdateData();

	int iHour(0), iMinute(0), iSecond(0);

	bValidTime = GetTimePieces(m_strTimeStart, iHour, iMinute, iSecond);

	return bValidTime;
}

void CDownloadDemonDlg::CountDown()
{
	//NOTE: Since running within a thread, UpdateData doesn't work.
	if (m_iWhenDownload == DOWNLOADNOW)
	{
		m_iWhenDownload = DOWNLOADIN;
		CString strNewTime(_T("00:00:00"));
		m_comboTimeStart.SetWindowText(strNewTime);
	}
	
	CString strTime(m_strTimeStart);
	int iHour(0), iMinute(0), iSecond(0);
	if (GetTimePieces(strTime, iHour, iMinute, iSecond))
	{		
		if (m_iWhenDownload == DOWNLOADAT)
		{
			//Convert time gotten into a period of time from now.
			CTime timeNow = CTime::GetCurrentTime();
			int iCurrHour(0), iCurrMin(0), iCurrSec(0);			
			iCurrHour = timeNow.GetHour();
			iCurrMin = timeNow.GetMinute();			
			iCurrSec = timeNow.GetSecond();

			int iTimeNowSeconds = iCurrHour*60*60 + iCurrMin*60 + iCurrSec;
			int iTimeWhenSeconds = iHour*60*60 + iMinute*60 + iSecond;
			int iNewTimeSeconds = 0;

			if (iTimeNowSeconds < iTimeWhenSeconds) //Same Day
			{				
				iNewTimeSeconds = iTimeWhenSeconds - iTimeNowSeconds;
			}
			else //Next Day
			{
				iNewTimeSeconds = 60*60*24 - iTimeNowSeconds + iTimeWhenSeconds;
			}

			iSecond = iNewTimeSeconds % 60;
			iMinute = iNewTimeSeconds / 60;
			iHour = iMinute / 60;
			iMinute = iMinute % 60;

			//Change to download in time
			m_iWhenDownload = DOWNLOADIN;
			m_comboWhenStart.SetCurSel(m_iWhenDownload);
		}		

		while (iHour || iMinute || iSecond)
		{
			if (--iSecond == -1)
			{
				if (iHour || iMinute)
					iSecond = 59;
				if (--iMinute == -1)
				{
					if (iHour)
					{
						iMinute = 59;
						iHour--;
					}
				}
			}
			if (!AllowDownload())
				return;
			CString strNewTime(_T(""));
			strNewTime.Format("%02d:%02d:%02d", iHour, iMinute, iSecond);
			m_comboTimeStart.SetWindowText(strNewTime);
			Sleep(1000); //Sleep for 1 second
		}
	}

	//If the user entered an invalid time, and we got this far,
	//we'd want to download anyway, I guess
	if (AllowDownload())
	{
		//Establish a dialup connection if we're told to
		if (m_dlgSettings.m_bDialModem && m_dlgSettings.m_strDownloadConnection.GetLength())
			m_pModemConnection->EnsureConnection();

		m_iWhenDownload = DOWNLOADNOW;
		m_comboWhenStart.SetCurSel(m_iWhenDownload);
	}	

	return;
}

BOOL CDownloadDemonDlg::GetTimePieces(CString strTime, int& iHour, int& iMinute, int& iSecond)
{
	BOOL bSuccess(TRUE);
	int iMaxHour(24);
	int iAddHour(0);
	iHour = 0;
	iMinute = 0;
	iSecond = 0;

	strTime.MakeUpper();
	strTime.TrimLeft();
	strTime.TrimRight();

	//If downloading at a certain time, adjust for AM/PM
	if (m_iWhenDownload == DOWNLOADAT)
	{
		if ((strTime.Right(2) == _T("AM")) || 
			(strTime.Right(2) == _T("PM")))
		{
			iMaxHour = 12;
			if (strTime.Right(2) == _T("PM"))
				iAddHour = 12;
			strTime = strTime.Left(strTime.GetLength()-2);
			strTime.TrimRight();
		}
	}

	//Bail out if no time is specified
	if (!strTime.GetLength())
		return FALSE;

	//Ensure only valid characters are in the string
	for (int i=0; i<strTime.GetLength(); i++)
	{
		if ((strTime[i] != _T(':')) &&
			((strTime[i] < _T('0')) || (strTime[i] > _T('9'))))
			return FALSE;
	}
	
	int ColonLoc = strTime.Find(_T(':'));
	int ColonLoc2 = 0;
	switch (ColonLoc)
	{
		case -1:
			//Require download AT to have one colon, at least
			if (m_iWhenDownload == DOWNLOADAT)
				return FALSE;

			iHour = atoi(strTime) / 60;
			iMinute = atoi(strTime) % 60;
			iSecond = 0;
			break;
		case 1:
		case 2:
			iHour = atoi(strTime.Left(ColonLoc));
			strTime = strTime.Mid(ColonLoc + 1);
			ColonLoc2 = strTime.Find(_T(':'));
			switch (ColonLoc2)
			{
				case -1:
					if (strTime.GetLength() == 2)
					{
						iMinute = atoi(strTime);
						iSecond = 0;
					}
					else
						bSuccess = FALSE;
					break;
				case 2:
					iMinute = atoi(strTime.Left(ColonLoc2));
					strTime = strTime.Mid(ColonLoc2 + 1);
					if (strTime.GetLength() == 2)
						iSecond = atoi(strTime);
					else
						bSuccess = FALSE;
					break;
				default:
					bSuccess = FALSE;
					break;
			}
			break;
		default:
			bSuccess = FALSE;
			break;
	}

	if ((iHour == 0) && (iSecond == 0))
	{
		if ((iMinute < 0) || (iMinute > 60*24))
			bSuccess = FALSE;
	} 
	else 
	{
		//Ensure valid time
		if ((iHour < 0) || (iHour >= iMaxHour) ||
			(iMinute < 0) || (iMinute > 59) ||
			(iSecond < 0) || (iSecond > 59))
		{
			//Do not allow anything greater then 24:00:00
			if ((iHour == 24) && (iMinute || iSecond))
				bSuccess = FALSE;
		}
	}

	//Convert to military time
	if (iMaxHour == 12)
	{
		if ((iAddHour) && (iHour != 12))
		{
			iHour += iAddHour;
			if (iHour == 24)
				iHour = 0;
		}

		//12am is 0, 12pm is 12
		if (iHour == 12)
		{
			if (!iAddHour)
				iHour = 0;
		}
	}

	return bSuccess;
}

BOOL CDownloadDemonDlg::PreTranslateMessage(MSG* pMsg) 
{
	if ((pMsg->message == WM_KEYDOWN) &&
		((pMsg->wParam == VK_RETURN) ||
		 (pMsg->wParam == VK_ESCAPE)))
	{	
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CDownloadDemonDlg::OnRegister() 
{
	//m_dlgRegister.DoModal();	
}

void CDownloadDemonDlg::StripLinks(CString& strHTML, CString strParentFile,
								   CString strDestDir,
								   CString strUserName, CString strPassword)
{
	CString strUpper(strHTML);
	strUpper.MakeUpper();
	char* pLookingAt = strUpper.GetBuffer(0);
	char* pLookingBack = pLookingAt;

	// Convert all cr/lfs/newlines to spaces
	int iUpperLength = strUpper.GetLength();
	for (int i = 0; i<iUpperLength; i++)
	{
		if ((strUpper.GetAt(i) == _T('\r')) || 
			(strUpper.GetAt(i) == _T('\n')))
			strUpper.SetAt(i, _T(' '));
	}

	// Find the Directory of the parent file
	CString strDefaultDirectory(_T(""));
	int iSlashFound = strParentFile.ReverseFind('/');
	// Look for backslashes if a slash wasn't found
	if (iSlashFound == -1)
		iSlashFound = strParentFile.ReverseFind('\\');
	// If we found some sort of slash, we can get the Parent Directory
	if (iSlashFound != -1)
	{
		strDefaultDirectory = strParentFile.Left(iSlashFound + 1);
	}

	// Keep going until we can't find an HREF
	while(pLookingAt = strstr(pLookingAt, _T("HREF")))
	{
		// Move looking at forward so we advance links
		pLookingAt+=4;

		int iInLink(0); // 0=No, 1=Yes, 2=Found 'A', but not '<'
		//go back and try to find a "<A" before a '>'
		char* pLoop = NULL;
		for (pLoop=pLookingAt-5; pLoop>=pLookingBack; pLoop--)
		{
			// If we find a ending bracket, we're done
			if (*pLoop == _T('>'))
			{
				iInLink = 0;
				break;
			}

			// We already found the 'A', look for '<'
			if (iInLink == 2)
			{
				// Look for matching '<'
				if (*pLoop == _T('<'))
				{
					iInLink = 1;
					break;
				}

				// look for ctrl characters - they're allowed, plus space too
				else if (*pLoop <= 32)
				{
					continue;
				}

				// otherwise, we know we grabbed some invalid character
				// maybe this A was for some other reason, keep looking back
				iInLink = 0;
				continue;
			}

			// If we find the 'A', we need to look for '<'
			if (*pLoop == _T('A'))
				iInLink = 2;
		}

		BOOL bEqualFound(FALSE);
		BOOL iQuotedFile(0); // 0=No Quotes, 1=', 2="
		// Now it's time to skip the '=' sign
		if (iInLink == 1)
		{
			for (pLoop=pLookingAt; *pLoop!=_T('\0'); pLoop++)
			{
				// Skip all control chars and spaces
				if (*pLoop <= 32)
				{
					continue;
				}
				// if we found the equals
				else if (*pLoop == _T('='))
				{
					bEqualFound = TRUE;
				}
				// Found a single quote
				else if (*pLoop == _T('\''))
				{
					iQuotedFile = 1;
					pLoop++;
					break;
				}
				// Found a double quote
				else if (*pLoop == _T('"'))
				{
					iQuotedFile = 2;
					pLoop++;
					break;
				}
				// Some other char
				else
					break;
			}
		}
		
		char* pStartLink = NULL;
		char* pEndLink = NULL;
		// This is only used if no quotes are found
		// to determine if we've found the file yet
		BOOL bFileFound(FALSE);

		// Continue processing only if an '=' was found
		// We know we're in the filename at this point
		if (bEqualFound)
		{
			// We got here, so we know that the start is where we are
			pStartLink = pLoop;

			// Loop until end of string or we find 
			for (pLoop = pLoop; *pLoop != _T('\0'); pLoop++)
			{
				// Handle the close quotes if necessary
				if ((*pLoop == _T('"')) || (*pLoop == _T('\'')))
				{
					// If we found the ending quote or we find the '>' and
					// no quotes expected, we're done with this
					if (((*pLoop == _T('"')) && (iQuotedFile == 2)) ||
						((*pLoop == _T('\'')) && (iQuotedFile == 1)))						
					{
						pEndLink = pLoop-1;
						break;
					}
				}

				// If we find the close and this isn't in quotes, we're done
				if ((*pLoop == _T('>')) && !iQuotedFile)
				{
					pEndLink = pLoop-1;
					break;
				}

				// If this is a space character and we're not in quotes,
				// then we use these as delimiters - either push the start
				// of the string out, or we're done with this.
				if ((*pLoop <= 32) && !iQuotedFile)
				{
					if (!bFileFound)
					{
						*pStartLink++;
					}
					else
					{
						pEndLink = pLoop-1;
						break;
					}
				}
			}

			// If we've found a link, let's do some magic with it
			if ((pEndLink) && (pEndLink-pStartLink > 0))
			{
				CString strLink = strHTML.Mid(pStartLink-pLookingBack, pEndLink-pStartLink+1);

				if (!m_bArcadeAtHomeMode || (strLink.Find("/cgi/2.php?", 0) >= 0) || (strLink.Find("/cgi/3.php?", 0) >= 0))
				{
					AddStrippedLink(strLink, strDefaultDirectory, strDestDir,
									strUserName, strPassword, strParentFile);
				}
			}
			// Advance pLookingAt to the end of where we were
			pLookingAt = pLoop;
		}
	}

	strUpper.ReleaseBuffer();
}

void CDownloadDemonDlg::AddStrippedLink(CString strLink, CString strDefaultDirectory,
										CString strDestDir,
										CString strUserName, CString strPassword, 
										CString strParentFile)
{
	BOOL bUseDefaultDir(FALSE);
	CString strUseDirectory(strDefaultDirectory);

	// Trim all the strings
	strLink.TrimLeft();
	strLink.TrimRight();
	strUseDirectory.TrimLeft();
	strUseDirectory.TrimRight();

	// Swap slashes for backslashes
	int i(0);
	for (i=0; i<strLink.GetLength(); i++)
	{
		if (strLink.GetAt(i) == _T('\\'))
			strLink.SetAt(i, _T('/'));
	}
	for (i=0; i<strDefaultDirectory.GetLength(); i++)
	{
		if (strDefaultDirectory.GetAt(i) == _T('\\'))
			strDefaultDirectory.SetAt(i, _T('/'));
	}

	// Remove "./" if they're there
	if (strLink.Left(2) == _T("./"))
	{
		strLink = strLink.Mid(2);
		bUseDefaultDir = TRUE;
	}

	// Count how many directories the file is going back
	int iBackDirs(0);
	while (strLink.Left(3) == _T("../"))
	{
		strLink = strLink.Mid(3);
		iBackDirs++;
		bUseDefaultDir = TRUE;
	}

	// Move the Directory to use back to the file specified	
	for (i=0; i<iBackDirs; i++)
	{
		int iSlashFound = strUseDirectory.ReverseFind('/');
		if (iSlashFound != -1)
		{
			strUseDirectory = strUseDirectory.Left(iSlashFound + 1);
		}
		else
		{
			// we're trying to step back too far, so drop out
			return;
		}
	}

	// We found a colon
	CString strProtocol("http://");
	if (strLink.Find(':') != -1)
	{
		CString strLinkCheck(strLink);
		strLinkCheck.MakeUpper();

		// If we didn't find the protocol, and it was expected
		// then we know this is invalid
		if ((strLinkCheck.Find(_T("HTTP://")) != 0) && 
			(strLinkCheck.Find(_T("FTP://")) != 0))
			return;

		if (strLinkCheck.Find(_T("FTP://")) == 0)
			strProtocol = "ftp://";
	}
	else
		bUseDefaultDir = TRUE;

	// Append directory to files that aren't specified
	if (bUseDefaultDir)
	{
		if (strLink.Left(1) == "/")
		{
			DWORD dwServiceType;
			CString strServerOnly;
			CString strObject;
			INTERNET_PORT nPort(0);
			AfxParseURL(strUseDirectory, dwServiceType, strServerOnly, strObject, nPort);
			
			CString strLinkCopy(strLink);
			if (nPort == 80)
				strLink.Format("%s%s%s", strProtocol, strServerOnly, strLinkCopy);
			else
				strLink.Format("%s%s:%d%s", strProtocol, strServerOnly, nPort, strLinkCopy);
		}
		else
		{
			strLink = strUseDirectory + strLink;
		}
	}

	// Now ensure that this file has a valid strip extension
	CString strExtensionsCopy(m_dlgSettings.m_dlgStripExtensions.m_strStripExtensions);
	int iFoundExt(0);
	BOOL bFoundMatch(FALSE);
	CString strLinkCopy(strLink);
	strLinkCopy.MakeLower(); // to match extensions
	while ((iFoundExt = strExtensionsCopy.ReverseFind(_T('.'))) >= 0)
	{
		CString strCurrExt = strExtensionsCopy.Mid(iFoundExt);		
		if (strLinkCopy.Right(strCurrExt.GetLength()) == strCurrExt)
			bFoundMatch = TRUE;
		strExtensionsCopy = strExtensionsCopy.Left(iFoundExt);
	}

	// Only process extensions that were found - or ArcadeAtHome
	if (bFoundMatch || m_bArcadeAtHomeMode)
	{
		CString strStripNo(_T(""));
		strStripNo.LoadString(IDS_NO);

		// Lock the list and add the item
		CSingleLock cslThreadLock(&m_csAdjustDownloadList, FALSE);	
		cslThreadLock.Lock();
		m_lstFTPItems.AddItem(strLink, CDlgProperties::BINARY, strStripNo, 
							  FALSE, strDestDir, strUserName, strPassword, 
							  (m_bArcadeAtHomeMode) ? strParentFile : "");
		cslThreadLock.Unlock();
	}
}
