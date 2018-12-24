#if !defined(AFX_DLGSETTINGSMODEM_H__EC508503_FE25_11D1_A260_444553540000__INCLUDED_)
#define AFX_DLGSETTINGSMODEM_H__EC508503_FE25_11D1_A260_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DlgSettingsModem.h : header file
//

enum {DD_CURRENT_SETTING, DD_REGISTRY_SETTING};
enum {DD_CONNECT, DD_BUSY, DD_NOANSWER, DD_NODIALTONE, DD_DISCONNECT, DD_CANCELDIAL};


/////////////////////////////////////////////////////////////////////////////
// CDlgSettingsModem dialog

class CModemData
{
public:
	BOOL	m_bPartialParent;
	BOOL	m_bPartialSibling1;
	BOOL	m_bPartialSibling2;
	BOOL	m_bPartialSibling3;
	BOOL	m_bPartialButton;
	CString	m_strParent;
	CString	m_strSibling1;
	CString	m_strSibling2;
	CString	m_strSibling3;
	CString	m_strButton;
};

class CDlgSettingsModem : public CDialog
{
// Construction
public:
	CDlgSettingsModem(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgSettingsModem)
	enum { IDD = IDD_MODEM_ADVANCED };
	CComboBox	m_comboModemWindow;
	BOOL	m_bCheckPartialParent;
	BOOL	m_bCheckPartialSibling1;
	BOOL	m_bCheckPartialSibling2;
	BOOL	m_bCheckPartialSibling3;
	BOOL	m_bCheckPartialButton;
	CString	m_strParent;
	CString	m_strSibling1;
	CString	m_strSibling2;
	CString	m_strButton;
	int		m_iComboModemWindow;
	CString	m_strSibling3;
	//}}AFX_DATA

	//This contains all the data for modem strings
	CModemData m_mdmButtonInfo[DD_CANCELDIAL+1][DD_REGISTRY_SETTING+1];

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgSettingsModem)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgSettingsModem)
	afx_msg void OnButtonDefault();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnChangeComboModemAction();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int m_iComboModemWindowLast; //Previous setting of combo window
	                             //Used to detect if combo window changed

	void ResetSettingsToDefault(BOOL bResetAll = TRUE);
	void LoadSaveSettings(BOOL bSaveSettings);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSETTINGSMODEM_H__EC508503_FE25_11D1_A260_444553540000__INCLUDED_)
