#if !defined(AFX_DLGWAIVER_H__34C78BC2_FB16_11D1_A260_444553540000__INCLUDED_)
#define AFX_DLGWAIVER_H__34C78BC2_FB16_11D1_A260_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DlgWaiver.h : header file
//

#include "Label.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgWaiver dialog

class CDlgWaiver : public CDialog
{
// Construction
public:
	CDlgWaiver(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgWaiver)
	enum { IDD = IDD_WAIVER };
	CLabel m_labelWaiverBackground;
	CLabel m_labelWaiver3;
	CLabel m_labelWaiver2;
	CLabel m_labelWaiver;
	CButton	m_btnDeny;
	CButton	m_chkDisableWaiver;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgWaiver)
	public:
	virtual int DoModal();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgWaiver)
	virtual void OnCancel();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGWAIVER_H__34C78BC2_FB16_11D1_A260_444553540000__INCLUDED_)
