// DlgRegister.cpp : implementation file
//

#include "stdafx.h"
#include "downloaddemon.h"
#include "DlgRegister.h"
#include "Registry.h"
//#include "..\Registration\RegCode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgRegister dialog
CDlgRegister::CDlgRegister(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgRegister::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgRegister)
	m_strLicenseCode = _T("");
	m_strUserName = _T("");
	m_staticNotice = _T("");
	//}}AFX_DATA_INIT

	m_iRegistrationStatus = EXPIRED;
	m_iTrialDaysLeft = -1;

	GetSetRegistry(TRUE);
}


void CDlgRegister::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgRegister)
	DDX_Text(pDX, IDC_EDIT_LICENSE, m_strLicenseCode);
	DDX_Text(pDX, IDC_EDIT_USERNAME, m_strUserName);
	DDX_Text(pDX, IDC_STATIC_NOTICE, m_staticNotice);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgRegister, CDialog)
	//{{AFX_MSG_MAP(CDlgRegister)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgRegister message handlers

int CDlgRegister::DoModal() 
{
	if (m_iRegistrationStatus == LICENSED)	
	{
		AfxMessageBox(IDS_ALREADY_LICENSED);
		return 0;
	}

	if (m_iRegistrationStatus == EXPIRED)
		m_staticNotice.LoadString(IDS_EXPIRED_NOTICE);
	else // TRIAL
		m_staticNotice.Format(IDS_TRIAL_NOTICE, m_iTrialDaysLeft);

	return CDialog::DoModal();
}

int CDlgRegister::CheckRegistrationStatus()
{
	return m_iRegistrationStatus;
}

void CDlgRegister::GetSetRegistry(BOOL bGetFromRegistry /*TRUE*/)
{
	COXRegistry regRegistration;
	BOOL bSuccess(FALSE);
	CString strUser(_T(""));
	CString strCode(_T(""));	
	CString strWinDir(_T(""));

	GetWindowsDirectory(strWinDir.GetBuffer(MAX_PATH), MAX_PATH);
	strWinDir.ReleaseBuffer();
	if (!strWinDir.GetLength())
	{
		AfxMessageBox(IDS_FAILURE_GETWINDIR);
		exit(99);
	}

	if (strWinDir.GetAt(strWinDir.GetLength()-1) != _T('\\'))
		strWinDir += _T('\\');

	if (regRegistration.Connect(COXRegistry::keyLocalMachine))
	{
		CString strRegKey(_T(""));
		CString strWinTime(_T(""));
		CString strDDTime(_T(""));
		CString strRegTime(_T(""));
		strRegKey.LoadString(IDS_REGISTRY_KEY);
		if (regRegistration.CheckAndOpenKeyForWrite(strRegKey))
		{	if (bGetFromRegistry)
			{
				bSuccess = regRegistration.GetValue(_T("UserName"), m_strUserName);
				if (bSuccess)
					bSuccess = regRegistration.GetValue(_T("RegistrationCode"), m_strLicenseCode);

				//If there's valid info in the registry				
				if (bSuccess && RegCode(m_strUserName, m_strLicenseCode))
					m_iRegistrationStatus = LICENSED;
				else //this copy is *NOT* registered - date check time
				{
					//Invalid stuff in registry - blank it out
					m_strUserName = _T("");
					m_strLicenseCode = _T("");

					// Get the 3 times (registry, Win.ini, and DD.ini)
					time_t tmtNow(0);
					time(&tmtNow);
					DWORD dwTime = tmtNow;
					GetPrivateProfileString(_T("InternetTransaction"),
											_T("Configuration"), _T(""),
											strWinTime.GetBuffer(MAX_PATH), MAX_PATH,
											strWinDir + _T("WIN.INI"));
					strWinTime.ReleaseBuffer();
					GetPrivateProfileString(_T("DownloadDemon"),
											_T("InstallTime"), _T(""),
											strDDTime.GetBuffer(MAX_PATH), MAX_PATH,
											strWinDir + _T("DownloadDemon.INI"));
					strDDTime.ReleaseBuffer();

					// This is the first time this thing has been run, set
					// up registry, WIN.INI, and DownloadDemon.INI
					if (!regRegistration.GetValue(_T("FirstRunTime"), dwTime) &&
						(strWinTime == _T("")) && (strDDTime == _T("")))
					{					
						regRegistration.SetValue(_T("FirstRunTime"), tmtNow);
											
						strWinTime.Format("%x", tmtNow);
						strDDTime = strWinTime;
						WritePrivateProfileString(_T("InternetTransaction"),
												  _T("Configuration"),
												  strWinTime,
												  strWinDir + _T("WIN.INI"));
						WritePrivateProfileString(_T("DownloadDemon"),
												  _T("InstallTime"),
												  strDDTime,
												  strWinDir + _T("DownloadDemon.INI"));
					}

					// Okay, now all the numbers have to match up.
					// If they don't, require the user to register.
					strRegTime.Format("%x", dwTime);
					
					if ((strWinTime != strDDTime) || (strWinTime != strRegTime))
					{						
						m_iRegistrationStatus = EXPIRED;
					}
					else
					{
						// Check the time - if it's over 30 days,
						// expire, otherwise, we're still in a trial
						m_iRegistrationStatus = EXPIRED;

						// Ensure the user didn't set the clock back 8^)
						CTime tmInstall(dwTime);
						CTime tmNow(tmtNow);
						if (tmNow >= tmInstall)
						{
							CTimeSpan tmDiff(tmNow - tmInstall);
							m_iTrialDaysLeft = 30 - tmDiff.GetDays();
							if (m_iTrialDaysLeft >= 0)
							{
								((CDownloadDemonApp*)AfxGetApp())->m_iHiddenRegistrationValid = 1;
								m_iRegistrationStatus = TRIAL;
							}
						}
					}
					// If the license expired, write garbage back out
					// so the program won't be able to be used again
					if (m_iRegistrationStatus == EXPIRED)
					{
						// Populate INI files with random values
						// so registration never works, even if clock
						// is set back
						srand(tmtNow);
						strRegTime.Format("%x", rand() + 2353829);
						WritePrivateProfileString(_T("InternetTransaction"),
												  _T("Configuration"),
												  strRegTime,
												  strWinDir + _T("WIN.INI"));
						strRegTime.Format("%x", rand() + 53324853);
						WritePrivateProfileString(_T("DownloadDemon"),
												  _T("InstallTime"),
												  strRegTime,
												  strWinDir + _T("DownloadDemon.INI"));
					}
				}
			}
			else
			{
				UpdateData();
				regRegistration.SetValue("UserName", m_strUserName);
				regRegistration.SetValue("RegistrationCode", m_strLicenseCode);
			}
		}
		regRegistration.Disconnect();
	}
}

void CDlgRegister::OnOK() 
{
	UpdateData();

	// Make trimmed off copies of the username and licensecode
	CString strTestUserName(m_strUserName);
	strTestUserName.TrimLeft();
	strTestUserName.TrimRight();
	CString strTestLicenseCode(m_strLicenseCode);
	strTestLicenseCode.TrimLeft();
	strTestLicenseCode.TrimRight();

	if (RegCode(strTestUserName, strTestLicenseCode))
	{	
		//We're licensed
		m_iRegistrationStatus = LICENSED;
		GetSetRegistry(FALSE);

		AfxMessageBox(IDS_THANK_YOU);

		CDialog::OnOK();
	}
	else
	{
		AfxMessageBox(IDS_INVALID_USERNAME_LICENSE);		
	}
}

void CDlgRegister::OnCancel() 
{
	CDialog::OnCancel();
}

BOOL CDlgRegister::RegCode(CString strUser, CString& strPassword)
{
	/*
	// Generate the registration code
	unsigned long L0 = 0x32423432L;
	unsigned long L1 = 0xAB32CD98L;
	unsigned long L2 = 0x9832D474L;
	unsigned long L3 = 0xDCA7832AL;
	unsigned long L4 = 0x2318FB98L;
	CString strTempRegCode(_T(""));
	CRegCode codeRegistration(L0, L1, L2, L3, L4);
	if (!codeRegistration.GenerateRegCode(strUser, strTempRegCode))
		return FALSE;

	((CDownloadDemonApp*)AfxGetApp())->m_iHiddenRegistrationValid = (strTempRegCode == strPassword);

	// Validate the password
	if (strTempRegCode != strPassword)
		return FALSE;
	*/
	
	return TRUE;
}
