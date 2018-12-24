#include "stdafx.h"
#include "ModemConnection.h"
#include "DownloadDemonDlg.h"
#include "Dialup.h"

UINT WorkerPressAppropriateButtons(LPVOID pModemConnectionIn)
{	
	//Ensure pointer was passed in
	ASSERT(pModemConnectionIn);

	CModemConnection* pModemConnection = (CModemConnection*)pModemConnectionIn;
	pModemConnection->PressAppropriateButtons();

	return 0;
}

CModemConnection::CModemConnection(CDialog* pDlgParent /*=NULL*/)
{
	ASSERT(pDlgParent != NULL);

	m_pDlgParent = pDlgParent;
	CDownloadDemonDlg* pDDD = (CDownloadDemonDlg*)pDlgParent;

	//This is the button we're looking for
	//Prepare 2 copies - 1 for pushing buttons,
	//the other for minimize
	for (int i = 0; i<=1; i++)
	{
		CButtonPush* pButtonPush = NULL;
		pButtonPush = new CButtonPush
			(pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_DISCONNECT][DD_REGISTRY_SETTING].m_strButton,
			 pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_DISCONNECT][DD_REGISTRY_SETTING].m_bPartialButton,
			 pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_DISCONNECT][DD_REGISTRY_SETTING].m_strParent,
			 pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_DISCONNECT][DD_REGISTRY_SETTING].m_bPartialParent,
			 (i == 0) ? BM_CLICK : WM_SYSCOMMAND,
			 (i == 0) ? FALSE : TRUE,
			 (i == 0) ? 0 : SC_MINIMIZE,
			 0);
		pButtonPush->AddSibling(pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_DISCONNECT][DD_REGISTRY_SETTING].m_strSibling1, 
								pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_DISCONNECT][DD_REGISTRY_SETTING].m_bPartialSibling1);
		pButtonPush->AddSibling(pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_DISCONNECT][DD_REGISTRY_SETTING].m_strSibling2, 
								pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_DISCONNECT][DD_REGISTRY_SETTING].m_bPartialSibling2);
		pButtonPush->AddSibling(pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_DISCONNECT][DD_REGISTRY_SETTING].m_strSibling3, 
								pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_DISCONNECT][DD_REGISTRY_SETTING].m_bPartialSibling3);

		if (i == 0)
			m_pBtnConnected = pButtonPush;
		else
			m_pBtnConnectedMinimize = pButtonPush;
		pButtonPush = NULL;
	}
	//Copy button for both watching functions
	m_pBtnConnectedDialWatch = new CButtonPush(m_pBtnConnected);
	m_pBtnConnectedErrorWatch = new CButtonPush(m_pBtnConnected);

	//This is the button we press to dial modem
	m_pBtnDial = new CButtonPush
		(pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_CONNECT][DD_REGISTRY_SETTING].m_strButton,
		 pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_CONNECT][DD_REGISTRY_SETTING].m_bPartialButton,
		 pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_CONNECT][DD_REGISTRY_SETTING].m_strParent,
		 pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_CONNECT][DD_REGISTRY_SETTING].m_bPartialParent);
	m_pBtnDial->AddSibling(pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_CONNECT][DD_REGISTRY_SETTING].m_strSibling1, 
						   pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_CONNECT][DD_REGISTRY_SETTING].m_bPartialSibling1);
	m_pBtnDial->AddSibling(pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_CONNECT][DD_REGISTRY_SETTING].m_strSibling2, 
						   pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_CONNECT][DD_REGISTRY_SETTING].m_bPartialSibling2);
	m_pBtnDial->AddSibling(pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_CONNECT][DD_REGISTRY_SETTING].m_strSibling3, 
						   pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_CONNECT][DD_REGISTRY_SETTING].m_bPartialSibling3);

	//This is the button we press to redial for busy signal
	m_pBtnBusy = new CButtonPush
		(pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_BUSY][DD_REGISTRY_SETTING].m_strButton,
		 pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_BUSY][DD_REGISTRY_SETTING].m_bPartialButton,
		 pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_BUSY][DD_REGISTRY_SETTING].m_strParent,
		 pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_BUSY][DD_REGISTRY_SETTING].m_bPartialParent);
	m_pBtnBusy->AddSibling(pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_BUSY][DD_REGISTRY_SETTING].m_strSibling1, 
						   pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_BUSY][DD_REGISTRY_SETTING].m_bPartialSibling1);
	m_pBtnBusy->AddSibling(pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_BUSY][DD_REGISTRY_SETTING].m_strSibling2, 
						   pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_BUSY][DD_REGISTRY_SETTING].m_bPartialSibling2);
	m_pBtnBusy->AddSibling(pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_BUSY][DD_REGISTRY_SETTING].m_strSibling3, 
						   pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_BUSY][DD_REGISTRY_SETTING].m_bPartialSibling3);
	m_pBtnBusy->SetAuxWindow(TRUE);

	//This is the button we press to redial for no answer
	m_pBtnNoAnswer = new CButtonPush
		(pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_NOANSWER][DD_REGISTRY_SETTING].m_strButton,
		 pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_NOANSWER][DD_REGISTRY_SETTING].m_bPartialButton,
		 pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_NOANSWER][DD_REGISTRY_SETTING].m_strParent,
		 pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_NOANSWER][DD_REGISTRY_SETTING].m_bPartialParent);
	m_pBtnNoAnswer->AddSibling(pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_NOANSWER][DD_REGISTRY_SETTING].m_strSibling1, 
							   pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_NOANSWER][DD_REGISTRY_SETTING].m_bPartialSibling1);
	m_pBtnNoAnswer->AddSibling(pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_NOANSWER][DD_REGISTRY_SETTING].m_strSibling2, 
							   pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_NOANSWER][DD_REGISTRY_SETTING].m_bPartialSibling2);
	m_pBtnNoAnswer->AddSibling(pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_NOANSWER][DD_REGISTRY_SETTING].m_strSibling3, 
							   pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_NOANSWER][DD_REGISTRY_SETTING].m_bPartialSibling3);
	m_pBtnNoAnswer->SetAuxWindow(TRUE);

	//This is the button we press to redial for no dialtone
	m_pBtnNoDialTone = new CButtonPush
		(pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_NODIALTONE][DD_REGISTRY_SETTING].m_strButton,
		 pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_NODIALTONE][DD_REGISTRY_SETTING].m_bPartialButton,
		 pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_NODIALTONE][DD_REGISTRY_SETTING].m_strParent,
		 pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_NODIALTONE][DD_REGISTRY_SETTING].m_bPartialParent);
	m_pBtnNoDialTone->AddSibling(pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_NODIALTONE][DD_REGISTRY_SETTING].m_strSibling1, 
								 pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_NODIALTONE][DD_REGISTRY_SETTING].m_bPartialSibling1);
	m_pBtnNoDialTone->AddSibling(pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_NODIALTONE][DD_REGISTRY_SETTING].m_strSibling2, 
								 pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_NODIALTONE][DD_REGISTRY_SETTING].m_bPartialSibling2);
	m_pBtnNoDialTone->AddSibling(pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_NODIALTONE][DD_REGISTRY_SETTING].m_strSibling3, 
								 pDDD->m_dlgSettings.m_dlgSettingsModem.m_mdmButtonInfo[DD_NODIALTONE][DD_REGISTRY_SETTING].m_bPartialSibling3);
	m_pBtnNoDialTone->SetAuxWindow(TRUE);

	m_bRunningConnect = FALSE;
	m_iStatus = NOTPROCESSING;
}

CModemConnection::~CModemConnection()
{
	m_pDlgParent = NULL; //don't delete, we didn't new
	delete m_pBtnDial;
	m_pBtnDial = NULL;
	delete m_pBtnBusy;
	m_pBtnBusy = NULL;
	delete m_pBtnNoAnswer;
	m_pBtnNoAnswer = NULL;
	delete m_pBtnNoDialTone;
	m_pBtnNoDialTone = NULL;
	delete m_pBtnConnected;
	m_pBtnConnected = NULL;
	delete m_pBtnConnectedMinimize;
	m_pBtnConnectedMinimize = NULL;
	delete m_pBtnConnectedDialWatch;
	m_pBtnConnectedDialWatch = NULL;
	delete m_pBtnConnectedErrorWatch;
	m_pBtnConnectedErrorWatch = NULL;
}


BOOL CModemConnection::EnsureConnection()
{
	BOOL bSuccess(FALSE);
	m_iStatus = NOTPROCESSING;

	CDownloadDemonDlg* pDDD = (CDownloadDemonDlg*)m_pDlgParent;

	if ((pDDD->m_dlgSettings.m_bDialModem) && (pDDD->m_dlgSettings.m_strDownloadConnection.GetLength()))
	{		
		DWORD dwConnectionFlags(0);
		DWORD dwConnection(0);
		if (!m_pBtnConnected->AttemptFindButton())
		{
			DWORD dwDialResult(0);

			//Start up button watching thread
			m_bRunningConnect = TRUE;
			m_iStatus = CONNECTING;
			AfxBeginThread(WorkerPressAppropriateButtons, this);

			ExecuteDialUpConnection(pDDD->m_dlgSettings.m_strDownloadConnection.GetBuffer(0));
			pDDD->m_dlgSettings.m_strDownloadConnection.ReleaseBuffer();

			MSG message;
			int i(0);
			while (pDDD->m_bRunning && !m_pBtnConnected->AttemptFindButton())
			{				
				if (!pDDD->AllowDownload())
				{
					m_bRunningConnect = FALSE;
					m_iStatus = NOTPROCESSING;
					return FALSE;
				}

				int iCounter = 0;
				//Process messages so the app doesn't appear to lock
				while (::PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) 
				{ 
					::TranslateMessage(&message); 
					::DispatchMessage(&message); 
					if (++iCounter > 250)
						break;
				} 
				Sleep(250);
			}

			m_bRunningConnect = FALSE;

			//If we're connected and we're supposed to minimize, minimize
			if ((bSuccess = m_pBtnConnected->AttemptFindButton()) && pDDD->m_dlgSettings.m_bMinimizeDialup)
			{
				m_pBtnConnectedMinimize->AttemptPressButton();
			}			
		}
	}

	m_iStatus = NOTPROCESSING;
	return bSuccess;
}

void CModemConnection::PressAppropriateButtons()
{
	CDownloadDemonDlg* pDDD = (CDownloadDemonDlg*)m_pDlgParent;

	while (pDDD->m_bRunning)
	{
		if (!m_bRunningConnect || !pDDD->AllowDownload())
			break;

		try 
		{
 			if (!m_pBtnConnectedErrorWatch->AttemptFindButton() && m_bRunningConnect)
			{					
				m_pBtnDial->AttemptPressButton();
				m_pBtnBusy->AttemptPressButton();
				m_pBtnNoDialTone->AttemptPressButton();
				m_pBtnNoAnswer->AttemptPressButton();
			}
		}
		catch(...)
		{
		}		
		if (!m_bRunningConnect || !pDDD->AllowDownload())
			break;
		Sleep(250);
	}
}

void CModemConnection::Disconnect()
{
	m_pBtnConnected->AttemptPressButton();	
}