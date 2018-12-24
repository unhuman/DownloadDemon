// DlgWaiver.cpp : implementation file
//

#include "stdafx.h"
#include "downloaddemon.h"
#include "DlgWaiver.h"
#include "Registry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgWaiver dialog


CDlgWaiver::CDlgWaiver(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgWaiver::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgWaiver)
	//}}AFX_DATA_INIT
}


void CDlgWaiver::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgWaiver)
	DDX_Control(pDX, IDC_STATIC_BACKGROUND, m_labelWaiverBackground);
	DDX_Control(pDX, IDC_STATIC_WAIVER3, m_labelWaiver3);
	DDX_Control(pDX, IDC_STATIC_WAIVER2, m_labelWaiver2);
	DDX_Control(pDX, IDC_STATIC_WAIVER, m_labelWaiver);
	DDX_Control(pDX, IDCANCEL, m_btnDeny);
	DDX_Control(pDX, IDC_CHECK_DISABLE_LIABILITY, m_chkDisableWaiver);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgWaiver, CDialog)
	//{{AFX_MSG_MAP(CDlgWaiver)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgWaiver message handlers

void CDlgWaiver::OnCancel() 
{
	CDialog::OnCancel();
}

void CDlgWaiver::OnOK() 
{
	//If the user accepted the Waiver and checked off to not show 
	//again, set registry
	if (m_chkDisableWaiver.GetCheck() == 1)
	{
		COXRegistry regWaiver;
		if (regWaiver.Connect(COXRegistry::keyLocalMachine))
		{
			CString strRegKey(_T(""));
			strRegKey.LoadString(IDS_REGISTRY_KEY);
			if (regWaiver.CheckAndOpenKeyForWrite(strRegKey))
			{
				regWaiver.SetValue("WaiverAccepted", 1);
			}
			regWaiver.Disconnect();
		}
	}
	CDialog::OnOK();
}

int CDlgWaiver::DoModal() 
{
	COXRegistry regWaiver;
	DWORD dwWaiverAccepted(0);
	if (regWaiver.Connect(COXRegistry::keyLocalMachine))
	{
		CString strRegKey(_T(""));
		strRegKey.LoadString(IDS_REGISTRY_KEY);
		if (regWaiver.CheckAndOpenKeyForWrite(strRegKey))
		{
			regWaiver.GetValue("WaiverAccepted", dwWaiverAccepted);
		}
		regWaiver.Disconnect();
	}

	if (dwWaiverAccepted)
		return IDOK;
	
	return CDialog::DoModal();
}

BOOL CDlgWaiver::PreTranslateMessage(MSG* pMsg) 
{
	if ((pMsg->message == WM_KEYDOWN) &&
		(pMsg->wParam == VK_RETURN))
	{	
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CDlgWaiver::OnInitDialog() 
{
	CDialog::OnInitDialog();
	CRect rectWaiver;
	CRect rectWaiver3;
	m_labelWaiver.GetWindowRect(&rectWaiver);
	ScreenToClient(&rectWaiver);
	m_labelWaiver3.GetWindowRect(&rectWaiver3);
	ScreenToClient(&rectWaiver3);

	m_labelWaiverBackground.SetBkColor(RGB(255,255,255));
	m_labelWaiverBackground.SetWindowText(_T(""));	
	m_labelWaiverBackground.MoveWindow(rectWaiver.left - 3, rectWaiver.top - 2, 
									   rectWaiver.right - rectWaiver.left + 6, 
									   rectWaiver3.bottom - rectWaiver.top + 3);

	m_labelWaiver.SetBkColor(RGB(255,255,255));
	m_labelWaiver2.SetBkColor(RGB(255,255,255));
	m_labelWaiver3.SetBkColor(RGB(255,255,255));
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
