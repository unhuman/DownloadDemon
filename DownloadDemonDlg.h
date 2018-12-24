// DownloadDemonDlg.h : header file
//

#if !defined(AFX_DOWNLOADDEMONDLG_H__ACB6069B_AAE8_11D1_A5D8_444553540000__INCLUDED_)
#define AFX_DOWNLOADDEMONDLG_H__ACB6069B_AAE8_11D1_A5D8_444553540000__INCLUDED_

#include "DlgSettings.h"
//#include "DlgRegister.h"
#include "DownloadDemonDropTarget.h"
#include "DownloadListCtrl.h"
#include "ModemConnection.h"

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define MAX_THREADS		10

enum TransferStatus {XFER_PROCESSING, XFER_SUCCESS, XFER_STOPPED, XFER_ERROR_TIMEOUT, XFER_FAIL, XFER_ERROR_FILEOPEN, XFER_ERROR_FILEWRITE, XFER_ERROR_NOTFOUND, XFER_OTHER_ERROR};

//structure used for transferring data back and forth from WorkerDownloadHTTP
class CWorkerHttpTransfer; //defined below, since used in CDownloadDemonDlg

class CDownloadDemonDlg : public CDialog
{
// Construction
public:
	CDownloadDemonDlg(CWnd* pParent = NULL);	// standard constructor
	~CDownloadDemonDlg();

// Dialog Data
	//{{AFX_DATA(CDownloadDemonDlg)
	enum { IDD = IDD_DIALOG_DOWNLOADDEMON };
	CComboBox	m_comboTimeStart;
	CButton	m_chkDisconnectDone;
	CComboBox	m_comboWhenStart;
	CButton	m_btnDownload;
	CDownloadListCtrl m_lstFTPItems;
	int		m_iWhenDownload;
	CString	m_strTimeStart;
	//}}AFX_DATA
	CDlgSettings m_dlgSettings;

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDownloadDemonDlg)
	public:
	virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;	
	CLIPFORMAT m_cfObjectDescriptor;

	//Drag and Drop related members
	CDownloadDemonDropTarget* m_pDropTarget;	

	// Generated message map functions
	//{{AFX_MSG(CDownloadDemonDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnMenuSettings();
	afx_msg void OnButtonDownload();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg void OnMenuAbout();
	afx_msg void OnMenuExit();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnAddNewFile();
	afx_msg void OnDeleteCompletedItems();
	afx_msg void OnDeleteSelectedFile();
	afx_msg void OnPropertiesSelectedFile();
	afx_msg void OnMenuNewList();
	afx_msg void OnMenuOpenList();
	afx_msg void OnMenuSaveList();
	afx_msg void OnMenuSaveListAs();
	afx_msg void OnKeydownListFtpItems(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeComboWhenDownload();
	afx_msg void OnRegister();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:	
	void AddStrippedLink(CString strLink, CString strDefaultDirectory,
						 CString strDestDir,
						 CString strUserName, CString strPassword, 
						 CString strParentFile);
	void StripLinks(CString& strHTML, CString strParentFile, CString strDestDir,
					CString strUserName, CString strPassword);
	BOOL GetTimePieces(CString strTime, int& iHour, int& iMinute, int& iSecond);
	BOOL CheckFileExistance(CString strObject, void* pBuffer);
	CInternetSession* GetInternetSession();
	void UpdateMenuOptions(CMenu* pMenuUpdate);
	//return False if we are to stop trying to connect
	BOOL HandleConnectTimeout(int iItem, int iAttempt);
	enum DownloadTime {DOWNLOADNOW, DOWNLOADAT, DOWNLOADIN};
	enum TransferState {WAITING, DOWNLOADING, TERMINATING, COMPLETED};
	enum SearchTechnique {USECONNECTION, UNUSEDCONNECTION, ANYCONNECTION};
	void SetDownloadState(TransferState tsDownloading);
	BOOL CloseFTPDialog();
	//this function loads and saves Window size, location, and screen resolution
	void LoadWindowRegistrySettings(BOOL bLoad, RECT& rectSettings, CString strResolution = _T(""));
	void ParseCommandLine(char* lpszCommandLine);

	int GetFile(CString& strConnection, int iDownloadType);
	BOOL AskForFile(CString& strConnection, int& iItem, int iThreadNum);
	void GetLocalFileToWrite(CString& strFileToWrite, CString& strWriteDir, CString strFullPath);

	BOOL bProcessing[MAX_THREADS];
	BOOL m_bRegistrationShown;
//	CDlgRegister m_dlgRegister;
	CModemConnection* m_pModemConnection;
	BOOL m_bCancelledDownload;
	CString m_strFilename;
	CString m_strReferrer;
	CString m_strReferrerAndCookie;
	BOOL m_bAutoURLTerminate;	

	long m_lThreadsStarted;
	CStringList m_lstConnections;
	TransferState m_tsDownloading;
	BOOL m_bLiabilityShown;

	CCriticalSection m_csThreadCount;
	CCriticalSection m_csAdjustDownloadList;
public:
	BOOL m_bRunning;
	BOOL bRunning;
	BOOL m_bArcadeAtHomeMode;	

	BOOL ValidateTimeStart();
	UINT DownloadHTTP(CWorkerHttpTransfer* pHttpData);
	BOOL m_AllowShutdown;
	CString GetResolutionString();
	BOOL AllowDownload();
	//Increments threads currently running in a controlled fashion
	void IncrementThreadsUsed();
	//Decrements threads currently running in a controlled fashion
	void DecrementThreadsUsed();
	//Gets number of threads currently running in a controlled fashion
	int GetThreadsUsed();
	UINT DownloadFiles(int iThreadNum);
	UINT MonitorDownloads();
	void CountDown();
	void ShowItemProperties()
	{
		OnPropertiesSelectedFile();
	}

	virtual BOOL OnCmdMsg( UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo );
};


class CWorkerHttpTransfer {
public:
	BOOL m_bDone;
	BOOL m_bDefaultDestDir;
	BOOL m_bDefaultUserPassword;
	CDownloadDemonDlg* m_pParentDlg;
	CString m_strFullPath;
	CString m_strWriteFile;
	CString m_strDestDir;
	CString m_strUser;
	CString m_strPassword;
	CString m_strStrip;
	CString m_strReferrerUpdate;
	TransferStatus m_httpStatus;
	DWORD m_dwErrorCode;	
	int m_iDownloadItem;

	CWorkerHttpTransfer(CDownloadDemonDlg* pParentDlg = NULL,
					   CString strFullPath = _T(""),
					   CString strWriteFile = _T(""),
					   CString strDestDir = _T(""),
		               CString strUser = _T(""),
					   CString strPassword = _T(""),
					   CString strStrip = _T(""),
					   CString strReferrerUpdate = _T(""),
					   BOOL bDefaultDestDir = TRUE,
					   BOOL bDefaultUserPassword = TRUE,
					   int iDownloadItem = -1)
	{
		ASSERT(pParentDlg);
		ASSERT(strFullPath.GetLength() > 0);
		ASSERT(strWriteFile.GetLength() > 0);
		ASSERT(iDownloadItem > -1);

		m_pParentDlg = pParentDlg;

		m_strFullPath = strFullPath;
		m_strWriteFile = strWriteFile;
		m_strDestDir = strDestDir;
		m_strUser = strUser;
		m_strPassword = strPassword;
		m_strStrip = strStrip;
		m_strReferrerUpdate = strReferrerUpdate;
		m_bDefaultDestDir = bDefaultDestDir;
		m_bDefaultUserPassword = bDefaultUserPassword;

		m_httpStatus = XFER_PROCESSING;
		m_dwErrorCode = 0;
		m_iDownloadItem = iDownloadItem;
		m_bDone = FALSE;
	}
};



//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DOWNLOADDEMONDLG_H__ACB6069B_AAE8_11D1_A5D8_444553540000__INCLUDED_)
