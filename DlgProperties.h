#if !defined(AFX_DLGPROPERTIES_H__8E1CEFE2_DDE7_11D1_A25F_444553540000__INCLUDED_)
#define AFX_DLGPROPERTIES_H__8E1CEFE2_DDE7_11D1_A25F_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DlgProperties.h : header file
//

#include "Resource.h" //needed because this is included by others

/////////////////////////////////////////////////////////////////////////////
// CDlgProperties dialog

class CDlgProperties : public CDialog
{
// Construction
public:
	enum StripMode {STRIP_NO, STRIP_YES};
	enum XferMode {BINARY, ASCII};
	void SetAddMode();
	CDlgProperties(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgProperties)
	enum { IDD = IDD_LINK_PROPERTIES };
	CComboBox	m_comboStrip;
	CButton	m_btnLookupDestDir;
	CEdit	m_editDestDir;
	CEdit	m_editPassword;
	CEdit	m_editLogin;
	CButton	m_btnOk;
	CString	m_strLink;
	CString	m_strLogin;
	CString	m_strPassword;
	BOOL	m_bUseSpecificLoginInfo;
	int		m_iTransferMode;
	int		m_iStrip;
	BOOL	m_bSpecifyDestDir;
	CString	m_strEditDestDir;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgProperties)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgProperties)
	virtual BOOL OnInitDialog();
	afx_msg void OnUpdateEnabled();
	afx_msg void OnCheckSpecifyDestinationDir();
	afx_msg void OnButtonLookupDestDir();
	virtual void OnOK();
	afx_msg void OnChangeEditDestinationDirectory();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	BOOL m_bAddMode;
	void CDlgProperties::UpdateOkButtonStatus();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGPROPERTIES_H__8E1CEFE2_DDE7_11D1_A25F_444553540000__INCLUDED_)
