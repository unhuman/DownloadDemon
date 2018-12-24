// DlgStoppingNotice.cpp : implementation file
//

#include "stdafx.h"
#include "DownloadDemon.h"
#include "DlgStoppingNotice.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define REFRESH_TIMER_ID	060470

/////////////////////////////////////////////////////////////////////////////
// CDlgStoppingNotice dialog


CDlgStoppingNotice::CDlgStoppingNotice(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgStoppingNotice::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgStoppingNotice)
	m_strThreadsRunning = _T("");
	m_strTimeLeft = _T("");
	//}}AFX_DATA_INIT

	m_iWaitTime = 0;
	m_iWaitTimeBackup = m_iWaitTime;
	m_bAllowStopTrick = FALSE;
	m_iStopTrick = 0;
	m_iStopAttempts = 1;

	try
	{
		m_pParentDownloadDemonDlg = (CDownloadDemonDlg*)pParent;
	}
	catch(...)
	{
		m_pParentDownloadDemonDlg = NULL;
	}
}


void CDlgStoppingNotice::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgStoppingNotice)
	DDX_Control(pDX, IDC_STATIC_TIMELEFT, m_staticTime);
	DDX_Control(pDX, IDC_STATIC_THREADS, m_staticThreads);
	DDX_Control(pDX, IDC_STATIC_MESSAGE, m_staticMessage);
	DDX_Text(pDX, IDC_STATIC_THREADS, m_strThreadsRunning);
	DDX_Text(pDX, IDC_STATIC_TIMELEFT, m_strTimeLeft);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgStoppingNotice, CDialog)
	//{{AFX_MSG_MAP(CDlgStoppingNotice)
	ON_WM_TIMER()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgStoppingNotice message handlers


void CDlgStoppingNotice::OnTimer(UINT nIDEvent) 
{
	if ((m_pParentDownloadDemonDlg) && (nIDEvent == REFRESH_TIMER_ID))
		UpdateDisplay();

	CDialog::OnTimer(nIDEvent);
}

BOOL CDlgStoppingNotice::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_iStopAttempts = 1;

	//Start off
	UpdateDisplay();

	//Only set the timer if we've got a valid parent
	if (m_pParentDownloadDemonDlg)
		SetTimer(REFRESH_TIMER_ID, 1000, NULL);
	else
		EndDialog(0);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgStoppingNotice::SetMaximumWaitTime(int iWaitTime)
{
	ASSERT(iWaitTime > 0);
	m_iWaitTime = iWaitTime;
	m_iWaitTimeBackup = iWaitTime;
}

void CDlgStoppingNotice::UpdateDisplay()
{
	long lThreadsUsed(-1);
	if (m_pParentDownloadDemonDlg)
	{
		CString strFormat(_T(""));

		//Update threads running
		strFormat.LoadString(IDS_THREADS_RUNNING);
		lThreadsUsed = m_pParentDownloadDemonDlg->GetThreadsUsed();
		m_strThreadsRunning.Format(strFormat, lThreadsUsed);

		//Update time left
		strFormat.LoadString(IDS_COMPLETION_TIME);
		if (--m_iWaitTime == -1)
		{
			m_iWaitTime = m_iWaitTimeBackup;
			m_iStopAttempts++;
		}
		m_strTimeLeft.Format(strFormat, m_iWaitTime, m_iStopAttempts);

		UpdateData(FALSE);

		if (lThreadsUsed == 0)
		{
			KillTimer(REFRESH_TIMER_ID);
			CloseDialog();
		}
	}
}

void CDlgStoppingNotice::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CRect rectMessage;
	CRect rectThreads;
	CRect rectTime;

	m_staticMessage.GetClientRect(&rectMessage);
	m_staticMessage.ClientToScreen(&rectMessage);
	ScreenToClient(&rectMessage);

	m_staticThreads.GetClientRect(&rectThreads);
	m_staticThreads.ClientToScreen(&rectThreads);
	ScreenToClient(&rectThreads);

	m_staticTime.GetClientRect(&rectTime);
	m_staticTime.ClientToScreen(&rectTime);
	ScreenToClient(&rectTime);

	//steps to close dialog trick:
	//   press static message
	//   press static threads
	//   press static time
	//   press static threads
	//   press static message	

	switch(m_iStopTrick)
	{
		case 1:
			if (rectThreads.PtInRect(point))
				m_iStopTrick++;
			break;
		case 2:
			if (rectTime.PtInRect(point))
				m_iStopTrick++;
			break;
		case 3:
			if (rectThreads.PtInRect(point))
				m_iStopTrick++;
			break;
		case 4:
			if (rectMessage.PtInRect(point) && m_bAllowStopTrick)
			{
				CloseDialog();
			}
			break;
		default:
			if (rectMessage.PtInRect(point))
				m_iStopTrick = 1;
	}

	CDialog::OnLButtonUp(nFlags, point);
}

void CDlgStoppingNotice::AllowStopTrick()
{
	m_bAllowStopTrick = TRUE;
}

void CDlgStoppingNotice::CloseDialog()
{
	CSingleLock cslEndDialog(&m_csEndDialog, FALSE);
	cslEndDialog.Lock();
	EndDialog(0);
	cslEndDialog.Unlock();
}

BOOL CDlgStoppingNotice::PreTranslateMessage(MSG* pMsg) 
{
	if ((pMsg->message == WM_KEYDOWN) &&
		((pMsg->wParam == VK_RETURN) ||
		 (pMsg->wParam == VK_ESCAPE)))
	{	
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}
