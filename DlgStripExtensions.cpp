// DlgStripExtensions.cpp : implementation file
//

#include "stdafx.h"
#include "downloaddemon.h"
#include "DlgStripExtensions.h"
#include "Registry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgStripExtensions dialog


CDlgStripExtensions::CDlgStripExtensions(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgStripExtensions::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgStripExtensions)
	m_strExtension = _T("");
	//}}AFX_DATA_INIT

	m_strStripExtensions.LoadString(IDS_STRIP_DEFAULTS);
	LoadSaveStripList(TRUE);
}


void CDlgStripExtensions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgStripExtensions)
	DDX_Control(pDX, IDC_BUTTON_REMOVE_EXTENSION, m_btnRemoveExt);
	DDX_Control(pDX, IDC_EDIT_ADD_EXTENSION, m_editExtension);
	DDX_Control(pDX, IDC_LIST_EXTENSIONS, m_lstExtensions);
	DDX_Text(pDX, IDC_EDIT_ADD_EXTENSION, m_strExtension);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgStripExtensions, CDialog)
	//{{AFX_MSG_MAP(CDlgStripExtensions)
	ON_BN_CLICKED(IDC_BUTTON_ADD_EXTENSION, OnButtonAddExtension)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_EXTENSION, OnButtonRemoveExtension)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgStripExtensions message handlers

void CDlgStripExtensions::OnButtonAddExtension() 
{
	int i(0);

	UpdateData();
	m_strExtension.TrimLeft();
	m_strExtension.TrimRight();
	m_strExtension.MakeLower();

	BOOL bValidExtension(TRUE);

	if (m_strExtension.GetLength() < 2)
		bValidExtension = FALSE;
	else
	{
		for (i=0; i<m_strExtension.GetLength(); i++)
		{
			char chTest = m_strExtension.GetAt(i);
			if (i==0) 
			{
				// First character must be a period
				if (chTest != _T('.'))
				{
					bValidExtension = FALSE;
					break;
				}
			}
			else // i > 0
			{			
				// Only alpha numerics are allowed for extensions
				if (((chTest < _T('0')) || (chTest > _T('9'))) &&
					((chTest < _T('a')) || (chTest > _T('z'))))
				{
					bValidExtension = FALSE;
				}
			}
		}
	}
	if (!bValidExtension)
	{
		AfxMessageBox(IDS_INVALID_EXTENSION);
		return;
	}

	BOOL bExtFound(FALSE);	
	CString strCurrExt(_T(""));
	for (i=m_lstExtensions.GetCount()-1; i>=0; i--)
	{
		m_lstExtensions.GetText(i, strCurrExt);
		if (strCurrExt == m_strExtension)
			bExtFound = TRUE;
	}
	if (!bExtFound)
	{
		m_lstExtensions.AddString(m_strExtension);
		m_strExtension = _T("");
		UpdateData(FALSE);
	}
	else
		AfxMessageBox(IDS_EXTENSION_EXISTS);
}
		
void CDlgStripExtensions::OnButtonRemoveExtension() 
{
	for (int i=m_lstExtensions.GetCount()-1; i>=0; i--)
	{
		if (m_lstExtensions.GetSel(i))
			m_lstExtensions.DeleteString(i);
	}
}

void CDlgStripExtensions::OnOK() 
{
	m_strStripExtensions = _T("");
	CString strCurrExt(_T(""));
	for (int i=0; i<m_lstExtensions.GetCount(); i++)
	{
		m_lstExtensions.GetText(i, strCurrExt);
		m_strStripExtensions += strCurrExt;
	}
	LoadSaveStripList(FALSE);
	CDialog::OnOK();
}

BOOL CDlgStripExtensions::PreTranslateMessage(MSG* pMsg) 
{
	// Check all keydowns
	if (pMsg->message == WM_KEYDOWN)		 
	{	
		// If this is return and we're entering an extension,
		// use it
		if (pMsg->wParam == VK_RETURN) 
		{
			CWnd* pwndCurrFocus = m_editExtension.GetFocus();
			if (pwndCurrFocus->m_hWnd == m_editExtension.m_hWnd)
				OnButtonAddExtension();
			return TRUE;
		}
		// Block escape keypresses
		if (pMsg->wParam == VK_ESCAPE)
		{
			return TRUE;
		}
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CDlgStripExtensions::LoadSaveStripList(BOOL bLoadSettings)
{
	COXRegistry regSettings;
	if (regSettings.Connect(COXRegistry::keyLocalMachine))
	{
		CString strRegKey(_T(""));
		strRegKey.LoadString(IDS_REGISTRY_KEY);
		if (regSettings.CheckAndOpenKeyForWrite(strRegKey))
		{
			if (bLoadSettings) //Load
			{
				CString strStripSettings(_T(""));
				//Retrieve configuration from registry and only
				//update items from default if they were retrieved
				if (regSettings.GetValue(_T("StripExtensions"), strStripSettings))
					m_strStripExtensions = strStripSettings;
			} 
			else 
			{ //Save
				//Save configuration to registry
				regSettings.SetValue(_T("StripExtensions"), m_strStripExtensions);
			}
		}
		regSettings.Disconnect();
	}
}


BOOL CDlgStripExtensions::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Populate dialog with extensions
	CString strExtensionsCopy(m_strStripExtensions);
	int iFoundExt(0);
	while ((iFoundExt = strExtensionsCopy.ReverseFind(_T('.'))) >= 0)
	{
		CString strCurrExt = strExtensionsCopy.Mid(iFoundExt);
		m_lstExtensions.AddString(strCurrExt);
		strExtensionsCopy = strExtensionsCopy.Left(iFoundExt);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
