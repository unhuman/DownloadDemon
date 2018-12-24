#if !defined(AFX_DLGREGISTER_H__7177C641_20DA_11D2_A260_99C5EEDD4979__INCLUDED_)
#define AFX_DLGREGISTER_H__7177C641_20DA_11D2_A260_99C5EEDD4979__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DlgRegister.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgRegister dialog

class CDlgRegister : public CDialog
{
// Construction
public:
	enum {EXPIRED, LICENSED, TRIAL};

	int CheckRegistrationStatus();
	CDlgRegister(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgRegister)
	enum { IDD = IDD_DIALOG_REGISTER };
	CString	m_strLicenseCode;
	CString	m_strUserName;
	CString	m_staticNotice;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgRegister)
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgRegister)
	virtual void OnOK();
	afx_msg void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:	
	void GetSetRegistry(BOOL bGetFromRegistry = TRUE);
	BOOL RegCode(CString strUser, CString& strPassword);
	int m_iRegistrationStatus;
	int m_iTrialDaysLeft;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGREGISTER_H__7177C641_20DA_11D2_A260_99C5EEDD4979__INCLUDED_)
