#if !defined(AFX_DLGSTRIPEXTENSIONS_H__44BADD63_6921_11D2_A70F_9F4C2EF7995B__INCLUDED_)
#define AFX_DLGSTRIPEXTENSIONS_H__44BADD63_6921_11D2_A70F_9F4C2EF7995B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DlgStripExtensions.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgStripExtensions dialog

class CDlgStripExtensions : public CDialog
{
// Construction
public:
	CString m_strStripExtensions;
	CDlgStripExtensions(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgStripExtensions)
	enum { IDD = IDD_DIALOG_STRIP_EXTENSION };
	CButton	m_btnRemoveExt;
	CEdit	m_editExtension;
	CListBox	m_lstExtensions;
	CString	m_strExtension;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgStripExtensions)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgStripExtensions)
	afx_msg void OnButtonAddExtension();
	afx_msg void OnButtonRemoveExtension();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void LoadSaveStripList(BOOL bLoadSettings);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSTRIPEXTENSIONS_H__44BADD63_6921_11D2_A70F_9F4C2EF7995B__INCLUDED_)
