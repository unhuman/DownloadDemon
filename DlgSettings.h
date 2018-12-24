#if !defined(AFX_DLGSETTINGS_H__ACB606A3_AAE8_11D1_A5D8_444553540000__INCLUDED_)
#define AFX_DLGSETTINGS_H__ACB606A3_AAE8_11D1_A5D8_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "DlgSettingsModem.h"
#include "DlgStripExtensions.h"

// DlgSettings.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgSettings dialog

class CDlgSettings : public CDialog
{
// Construction
public:
	CDlgSettings(CWnd* pParent = NULL);   // standard constructor	
	int  m_iLockSimultDownloads;

// Dialog Data
	//{{AFX_DATA(CDlgSettings)
	enum { IDD = IDD_DIALOG_SETTINGS };
	CComboBox	m_comboSimultDownloads;
	CButton	m_btnModemWindowStrings;
	CButton	m_chkMinimizeConnection;
	CButton	m_chkDialModem;
	CComboBox	m_comboDownloadConnection;
	CButton	m_btnOK;
	CString	m_strDefaultLogin;
	CString	m_strDefaultPassword;
	BOOL	m_bSmartDownload;
	int		m_iNumSimultDownloads;
	int		m_iRetryTime;
	CString	m_strDownloadDir;
	int		m_iConnectionTimeout;
	int		m_iRetryCount;
	CDlgSettingsModem m_dlgSettingsModem;
	CDlgStripExtensions m_dlgStripExtensions;
	CString	m_strDownloadConnection;
	BOOL	m_bDialModem;
	BOOL	m_bMinimizeDialup;
	BOOL	m_bMultipleIdenticalConnections;
	int		m_iRadioDupeSkip;
	//}}AFX_DATA
	CStringList m_strlConnections;
	CString m_strDownloadAcceleratorLocation;

	void LoadSaveSettings(BOOL bLoadSettings);	

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgSettings)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL	

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgSettings)
	virtual void OnOK();
	afx_msg void UpdateEnabledItems();
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnChangeEditDownloadDir();
	afx_msg void OnButtonModemStrings();
	afx_msg void OnCheckDialmodem();
	afx_msg void OnButtonLookupDir();
	afx_msg void OnButtonManageStripExtensions();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void BackupConfigurationValues(BOOL bBackup);	

	//These are the configuration backups used when cancel is pressed
	CString	m_strDefaultLoginBack;
	CString	m_strDefaultPasswordBack;
	BOOL	m_bSmartDownloadBack;
	int		m_iNumSimultDownloadsBack;
	BOOL	m_bClearLogFileOnStartBack;
	BOOL	m_bLogFileBack;
	BOOL	m_bMultipleIdenticalConnectionsBack;
	int		m_iRetryTimeBack;
	CString	m_strDownloadDirBack;
	int		m_iConnectionTimeoutBack;	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSETTINGS_H__ACB606A3_AAE8_11D1_A5D8_444553540000__INCLUDED_)
