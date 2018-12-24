#if !defined(AFX_DLGSTOPPINGNOTICE_H__BBC78FE5_D9F2_11D1_BC90_0060973CEA57__INCLUDED_)
#define AFX_DLGSTOPPINGNOTICE_H__BBC78FE5_D9F2_11D1_BC90_0060973CEA57__INCLUDED_

#include "DownloadDemonDlg.h"

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DlgStoppingNotice.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgStoppingNotice dialog

class CDlgStoppingNotice : public CDialog
{
// Construction
public:	
	void AllowStopTrick();
	void SetMaximumWaitTime(int iWaitTime);
	CDlgStoppingNotice(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgStoppingNotice)
	enum { IDD = IDD_DIALOG_STOPPING_NOTICE };
	CStatic	m_staticTime;
	CStatic	m_staticThreads;
	CStatic	m_staticMessage;
	CString	m_strThreadsRunning;
	CString	m_strTimeLeft;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgStoppingNotice)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgStoppingNotice)
	afx_msg void OnTimer(UINT nIDEvent);
	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:	
	void CloseDialog();
	CCriticalSection m_csEndDialog;
	void UpdateDisplay();
	CDownloadDemonDlg* m_pParentDownloadDemonDlg;
	BOOL m_bAllowStopTrick;
	int m_iStopTrick;
	int m_iWaitTime;
	int m_iStopAttempts;
	int m_iWaitTimeBackup;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSTOPPINGNOTICE_H__BBC78FE5_D9F2_11D1_BC90_0060973CEA57__INCLUDED_)
