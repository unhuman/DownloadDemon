// DlgSettingsModem.cpp : implementation file
//

#include "stdafx.h"
#include "downloaddemon.h"
#include "DlgSettingsModem.h"
#include "Registry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgSettingsModem dialog

CDlgSettingsModem::CDlgSettingsModem(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSettingsModem::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgSettingsModem)
	m_bCheckPartialParent = FALSE;
	m_bCheckPartialSibling1 = FALSE;
	m_bCheckPartialSibling2 = FALSE;
	m_bCheckPartialSibling3 = FALSE;
	m_bCheckPartialButton = FALSE;
	m_strParent = _T("");
	m_strSibling1 = _T("");
	m_strSibling2 = _T("");
	m_strButton = _T("");
	m_iComboModemWindow = 0;
	m_strSibling3 = _T("");
	//}}AFX_DATA_INIT

	m_iComboModemWindowLast = -1; //Ensure a change occurs at first

	ResetSettingsToDefault();
	LoadSaveSettings(TRUE);
}


void CDlgSettingsModem::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSettingsModem)
	DDX_Control(pDX, IDC_COMBO_MODEMSCREEN, m_comboModemWindow);
	DDX_Check(pDX, IDC_CHECK_PARTIAL_PARENT, m_bCheckPartialParent);
	DDX_Check(pDX, IDC_CHECK_PARTIAL_SIBLING1, m_bCheckPartialSibling1);
	DDX_Check(pDX, IDC_CHECK_PARTIAL_SIBLING2, m_bCheckPartialSibling2);
	DDX_Check(pDX, IDC_CHECK_PARTIAL_SIBLING3, m_bCheckPartialSibling3);
	DDX_Check(pDX, IDC_CHECK_PARTIAL_BUTTON, m_bCheckPartialButton);
	DDX_Text(pDX, IDC_EDIT_PARENT, m_strParent);
	DDV_MaxChars(pDX, m_strParent, 512);
	DDX_Text(pDX, IDC_EDIT_SIBLING1, m_strSibling1);
	DDV_MaxChars(pDX, m_strSibling1, 512);
	DDX_Text(pDX, IDC_EDIT_SIBLING2, m_strSibling2);
	DDV_MaxChars(pDX, m_strSibling2, 512);
	DDX_Text(pDX, IDC_EDIT_BUTTON, m_strButton);
	DDV_MaxChars(pDX, m_strButton, 512);
	DDX_CBIndex(pDX, IDC_COMBO_MODEMSCREEN, m_iComboModemWindow);
	DDX_Text(pDX, IDC_EDIT_SIBLING3, m_strSibling3);
	DDV_MaxChars(pDX, m_strSibling3, 512);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgSettingsModem, CDialog)
	//{{AFX_MSG_MAP(CDlgSettingsModem)
	ON_BN_CLICKED(IDC_BUTTON_DEFAULT, OnButtonDefault)
	ON_CBN_SELCHANGE(IDC_COMBO_MODEMSCREEN, OnChangeComboModemAction)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgSettingsModem message handlers

void CDlgSettingsModem::ResetSettingsToDefault(BOOL bResetAll /* = TRUE*/)
{
	int iTopEnd(DD_REGISTRY_SETTING);

	if (!bResetAll)
		iTopEnd = DD_CURRENT_SETTING;
	
	//Do both DD_CURRENT_SETTING and DD_REGISTRY_SETTING for consistancy
	for (int i = DD_CURRENT_SETTING; i <= iTopEnd; i++)
	{
		m_mdmButtonInfo[DD_CONNECT][i].m_strParent.LoadString(IDS_DEFAULT_DIALPARENT);
		m_mdmButtonInfo[DD_CONNECT][i].m_strButton.LoadString(IDS_DEFAULT_DIALBUTTON);
		m_mdmButtonInfo[DD_CONNECT][i].m_strSibling1.LoadString(IDS_DEFAULT_DIALSIBLING1);
		m_mdmButtonInfo[DD_CONNECT][i].m_strSibling2.LoadString(IDS_DEFAULT_DIALSIBLING2);
		m_mdmButtonInfo[DD_CONNECT][i].m_strSibling3.LoadString(IDS_DEFAULT_DIALSIBLING3);
		m_mdmButtonInfo[DD_CONNECT][i].m_bPartialParent = FALSE;
		m_mdmButtonInfo[DD_CONNECT][i].m_bPartialButton = FALSE;
		m_mdmButtonInfo[DD_CONNECT][i].m_bPartialSibling1 = FALSE;
		m_mdmButtonInfo[DD_CONNECT][i].m_bPartialSibling2 = FALSE;
		m_mdmButtonInfo[DD_CONNECT][i].m_bPartialSibling3 = FALSE;

		m_mdmButtonInfo[DD_BUSY][i].m_strParent.LoadString(IDS_DEFAULT_BUSYPARENT);
		m_mdmButtonInfo[DD_BUSY][i].m_strButton.LoadString(IDS_DEFAULT_BUSYBUTTON);
		m_mdmButtonInfo[DD_BUSY][i].m_strSibling1.LoadString(IDS_DEFAULT_BUSYSIBLING1);
		m_mdmButtonInfo[DD_BUSY][i].m_strSibling2 = _T("");
		m_mdmButtonInfo[DD_BUSY][i].m_strSibling3 = _T("");
		m_mdmButtonInfo[DD_BUSY][i].m_bPartialParent = FALSE;
		m_mdmButtonInfo[DD_BUSY][i].m_bPartialButton = FALSE;
		m_mdmButtonInfo[DD_BUSY][i].m_bPartialSibling1 = TRUE;
		m_mdmButtonInfo[DD_BUSY][i].m_bPartialSibling2 = FALSE;
		m_mdmButtonInfo[DD_BUSY][i].m_bPartialSibling3 = FALSE;

		m_mdmButtonInfo[DD_NOANSWER][i].m_strParent.LoadString(IDS_DEFAULT_NOANSWERPARENT);
		m_mdmButtonInfo[DD_NOANSWER][i].m_strButton.LoadString(IDS_DEFAULT_NOANSWERBUTTON);
		m_mdmButtonInfo[DD_NOANSWER][i].m_strSibling1.LoadString(IDS_DEFAULT_NOANSWERSIBLING1);
		m_mdmButtonInfo[DD_NOANSWER][i].m_strSibling2 = _T("");
		m_mdmButtonInfo[DD_NOANSWER][i].m_strSibling3 = _T("");
		m_mdmButtonInfo[DD_NOANSWER][i].m_bPartialParent = FALSE;
		m_mdmButtonInfo[DD_NOANSWER][i].m_bPartialButton = FALSE;
		m_mdmButtonInfo[DD_NOANSWER][i].m_bPartialSibling1 = TRUE;
		m_mdmButtonInfo[DD_NOANSWER][i].m_bPartialSibling2 = FALSE;
		m_mdmButtonInfo[DD_NOANSWER][i].m_bPartialSibling3 = FALSE;

		m_mdmButtonInfo[DD_NODIALTONE][i].m_strParent.LoadString(IDS_DEFAULT_NODIALTONEPARENT);
		m_mdmButtonInfo[DD_NODIALTONE][i].m_strButton.LoadString(IDS_DEFAULT_NODIALTONEBUTTON);
		m_mdmButtonInfo[DD_NODIALTONE][i].m_strSibling1.LoadString(IDS_DEFAULT_NODIALTONESIBLING1);
		m_mdmButtonInfo[DD_NODIALTONE][i].m_strSibling2 = _T("");
		m_mdmButtonInfo[DD_NODIALTONE][i].m_strSibling3 = _T("");
		m_mdmButtonInfo[DD_NODIALTONE][i].m_bPartialParent = FALSE;
		m_mdmButtonInfo[DD_NODIALTONE][i].m_bPartialButton = FALSE;
		m_mdmButtonInfo[DD_NODIALTONE][i].m_bPartialSibling1 = TRUE;
		m_mdmButtonInfo[DD_NODIALTONE][i].m_bPartialSibling2 = FALSE;
		m_mdmButtonInfo[DD_NODIALTONE][i].m_bPartialSibling3 = FALSE;

		m_mdmButtonInfo[DD_DISCONNECT][i].m_strParent = _T("");
		m_mdmButtonInfo[DD_DISCONNECT][i].m_strButton.LoadString(IDS_DEFAULT_DISCONNECTBUTTON);
		m_mdmButtonInfo[DD_DISCONNECT][i].m_strSibling1.LoadString(IDS_DEFAULT_DISCONNECTSIBLING1);
		m_mdmButtonInfo[DD_DISCONNECT][i].m_strSibling2.LoadString(IDS_DEFAULT_DISCONNECTSIBLING2);
		m_mdmButtonInfo[DD_DISCONNECT][i].m_strSibling3.LoadString(IDS_DEFAULT_DISCONNECTSIBLING3);
		m_mdmButtonInfo[DD_DISCONNECT][i].m_bPartialParent = TRUE;
		m_mdmButtonInfo[DD_DISCONNECT][i].m_bPartialButton = FALSE;
		m_mdmButtonInfo[DD_DISCONNECT][i].m_bPartialSibling1 = FALSE;
		m_mdmButtonInfo[DD_DISCONNECT][i].m_bPartialSibling2 = FALSE;
		m_mdmButtonInfo[DD_DISCONNECT][i].m_bPartialSibling3 = FALSE;

		// The only thing different between DD_CANCELDIAL and the DD_CONNECT 
		// defaults is the button pressed
		m_mdmButtonInfo[DD_CANCELDIAL][i].m_strParent.LoadString(IDS_DEFAULT_DIALPARENT);
		m_mdmButtonInfo[DD_CANCELDIAL][i].m_strButton.LoadString(IDS_DEFAULT_CANCELBUTTON);
		m_mdmButtonInfo[DD_CANCELDIAL][i].m_strSibling1.LoadString(IDS_DEFAULT_DIALSIBLING1);
		m_mdmButtonInfo[DD_CANCELDIAL][i].m_strSibling2.LoadString(IDS_DEFAULT_DIALSIBLING2);
		m_mdmButtonInfo[DD_CANCELDIAL][i].m_strSibling3.LoadString(IDS_DEFAULT_DIALSIBLING3);
		m_mdmButtonInfo[DD_CANCELDIAL][i].m_bPartialParent = FALSE;
		m_mdmButtonInfo[DD_CANCELDIAL][i].m_bPartialButton = FALSE;
		m_mdmButtonInfo[DD_CANCELDIAL][i].m_bPartialSibling1 = FALSE;
		m_mdmButtonInfo[DD_CANCELDIAL][i].m_bPartialSibling2 = FALSE;
		m_mdmButtonInfo[DD_CANCELDIAL][i].m_bPartialSibling3 = FALSE;	
	}
}


void CDlgSettingsModem::OnButtonDefault() 
{
	ResetSettingsToDefault(FALSE); //Only reset displayed options
	//Trick combo modem window into refilling current item
	m_iComboModemWindowLast = -1;
	OnChangeComboModemAction(); 
}

BOOL CDlgSettingsModem::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	UpdateData(FALSE);

	OnChangeComboModemAction();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgSettingsModem::LoadSaveSettings(BOOL bLoadSettings)
{
	COXRegistry regSettings;
	CString strValue(_T(""));
	DWORD dwPartialValue(0);

	if (regSettings.Connect(COXRegistry::keyLocalMachine))
	{
		CString strRegKey(_T(""));
		strRegKey.LoadString(IDS_REGISTRY_KEY);
		if (regSettings.CheckAndOpenKeyForWrite(strRegKey))
		{
			if (bLoadSettings) //Load
			{
				//Retrieve configuration from registry and only
				//update items from default if they were retrieved
				//Do both DD_CURRENT_SETTING and DD_REGISTRY_SETTING for consistancy
				for (int i = DD_CURRENT_SETTING; i <= DD_REGISTRY_SETTING; i++)
				{
					if (regSettings.GetValue("ConnectParent", strValue))
						m_mdmButtonInfo[DD_CONNECT][i].m_strParent = strValue;
					if (regSettings.GetValue("ConnectButton", strValue))
						m_mdmButtonInfo[DD_CONNECT][i].m_strButton = strValue;
					if (regSettings.GetValue("ConnectSibling1", strValue))
						m_mdmButtonInfo[DD_CONNECT][i].m_strSibling1 = strValue;
					if (regSettings.GetValue("ConnectSibling2", strValue))
						m_mdmButtonInfo[DD_CONNECT][i].m_strSibling2 = strValue;
					if (regSettings.GetValue("ConnectSibling3", strValue))
						m_mdmButtonInfo[DD_CONNECT][i].m_strSibling3 = strValue;
					if (regSettings.GetValue("PartialConnectParent", dwPartialValue))
						m_mdmButtonInfo[DD_CONNECT][i].m_bPartialParent = dwPartialValue;
					if (regSettings.GetValue("PartialConnectButton", dwPartialValue))
						m_mdmButtonInfo[DD_CONNECT][i].m_bPartialButton = dwPartialValue;
					if (regSettings.GetValue("PartialConnectSibling1", dwPartialValue))
						m_mdmButtonInfo[DD_CONNECT][i].m_bPartialSibling1 = dwPartialValue;
					if (regSettings.GetValue("PartialConnectSibling2", dwPartialValue))
						m_mdmButtonInfo[DD_CONNECT][i].m_bPartialSibling2 = dwPartialValue;
					if (regSettings.GetValue("PartialConnectSibling3", dwPartialValue))
						m_mdmButtonInfo[DD_CONNECT][i].m_bPartialSibling3 = dwPartialValue;

					if (regSettings.GetValue("BusyParent", strValue))
						m_mdmButtonInfo[DD_BUSY][i].m_strParent = strValue;
					if (regSettings.GetValue("BusyButton", strValue))
						m_mdmButtonInfo[DD_BUSY][i].m_strButton = strValue;
					if (regSettings.GetValue("BusySibling1", strValue))
						m_mdmButtonInfo[DD_BUSY][i].m_strSibling1 = strValue;
					if (regSettings.GetValue("BusySibling2", strValue))
						m_mdmButtonInfo[DD_BUSY][i].m_strSibling2 = strValue;
					if (regSettings.GetValue("BusySibling3", strValue))
						m_mdmButtonInfo[DD_BUSY][i].m_strSibling3 = strValue;
					if (regSettings.GetValue("PartialBusyParent", dwPartialValue))
						m_mdmButtonInfo[DD_BUSY][i].m_bPartialParent = dwPartialValue;
					if (regSettings.GetValue("PartialBusyButton", dwPartialValue))
						m_mdmButtonInfo[DD_BUSY][i].m_bPartialButton = dwPartialValue;
					if (regSettings.GetValue("PartialBusySibling1", dwPartialValue))
						m_mdmButtonInfo[DD_BUSY][i].m_bPartialSibling1 = dwPartialValue;
					if (regSettings.GetValue("PartialBusySibling2", dwPartialValue))
						m_mdmButtonInfo[DD_BUSY][i].m_bPartialSibling2 = dwPartialValue;
					if (regSettings.GetValue("PartialBusySibling3", dwPartialValue))
						m_mdmButtonInfo[DD_BUSY][i].m_bPartialSibling3 = dwPartialValue;
				
					if (regSettings.GetValue("NoAnswerParent", strValue))
						m_mdmButtonInfo[DD_NOANSWER][i].m_strParent = strValue;
					if (regSettings.GetValue("NoAnswerButton", strValue))
						m_mdmButtonInfo[DD_NOANSWER][i].m_strButton = strValue;
					if (regSettings.GetValue("NoAnswerSibling1", strValue))
						m_mdmButtonInfo[DD_NOANSWER][i].m_strSibling1 = strValue;
					if (regSettings.GetValue("NoAnswerSibling2", strValue))
						m_mdmButtonInfo[DD_NOANSWER][i].m_strSibling2 = strValue;
					if (regSettings.GetValue("NoAnswerSibling3", strValue))
						m_mdmButtonInfo[DD_NOANSWER][i].m_strSibling3 = strValue;
					if (regSettings.GetValue("PartialNoAnswerParent", dwPartialValue))
						m_mdmButtonInfo[DD_NOANSWER][i].m_bPartialParent = dwPartialValue;
					if (regSettings.GetValue("PartialNoAnswerButton", dwPartialValue))
						m_mdmButtonInfo[DD_NOANSWER][i].m_bPartialButton = dwPartialValue;
					if (regSettings.GetValue("PartialNoAnswerSibling1", dwPartialValue))
						m_mdmButtonInfo[DD_NOANSWER][i].m_bPartialSibling1 = dwPartialValue;
					if (regSettings.GetValue("PartialNoAnswerSibling2", dwPartialValue))
						m_mdmButtonInfo[DD_NOANSWER][i].m_bPartialSibling2 = dwPartialValue;
					if (regSettings.GetValue("PartialNoAnswerSibling3", dwPartialValue))
						m_mdmButtonInfo[DD_NOANSWER][i].m_bPartialSibling3 = dwPartialValue;

					if (regSettings.GetValue("NoDialToneParent", strValue))
						m_mdmButtonInfo[DD_NODIALTONE][i].m_strParent = strValue;
					if (regSettings.GetValue("NoDialToneButton", strValue))
						m_mdmButtonInfo[DD_NODIALTONE][i].m_strButton = strValue;
					if (regSettings.GetValue("NoDialToneSibling1", strValue))
						m_mdmButtonInfo[DD_NODIALTONE][i].m_strSibling1 = strValue;
					if (regSettings.GetValue("NoDialToneSibling2", strValue))
						m_mdmButtonInfo[DD_NODIALTONE][i].m_strSibling2 = strValue;
					if (regSettings.GetValue("NoDialToneSibling3", strValue))
						m_mdmButtonInfo[DD_NODIALTONE][i].m_strSibling3 = strValue;
					if (regSettings.GetValue("PartialNoDialToneParent", dwPartialValue))
						m_mdmButtonInfo[DD_NODIALTONE][i].m_bPartialParent = dwPartialValue;
					if (regSettings.GetValue("PartialNoDialToneButton", dwPartialValue))
						m_mdmButtonInfo[DD_NODIALTONE][i].m_bPartialButton = dwPartialValue;
					if (regSettings.GetValue("PartialNoDialToneSibling1", dwPartialValue))
						m_mdmButtonInfo[DD_NODIALTONE][i].m_bPartialSibling1 = dwPartialValue;
					if (regSettings.GetValue("PartialNoDialToneSibling2", dwPartialValue))
						m_mdmButtonInfo[DD_NODIALTONE][i].m_bPartialSibling2 = dwPartialValue;
					if (regSettings.GetValue("PartialNoDialToneSibling3", dwPartialValue))
						m_mdmButtonInfo[DD_NODIALTONE][i].m_bPartialSibling3 = dwPartialValue;

					if (regSettings.GetValue("DisconnectParent", strValue))
						m_mdmButtonInfo[DD_DISCONNECT][i].m_strParent = strValue;
					if (regSettings.GetValue("DisconnectButton", strValue))
						m_mdmButtonInfo[DD_DISCONNECT][i].m_strButton = strValue;
					if (regSettings.GetValue("DisconnectSibling1", strValue))
						m_mdmButtonInfo[DD_DISCONNECT][i].m_strSibling1 = strValue;
					if (regSettings.GetValue("DisconnectSibling2", strValue))
						m_mdmButtonInfo[DD_DISCONNECT][i].m_strSibling2 = strValue;
					if (regSettings.GetValue("DisconnectSibling3", strValue))
						m_mdmButtonInfo[DD_DISCONNECT][i].m_strSibling3 = strValue;
					if (regSettings.GetValue("PartialDisconnectParent", dwPartialValue))
						m_mdmButtonInfo[DD_DISCONNECT][i].m_bPartialParent = dwPartialValue;
					if (regSettings.GetValue("PartialDisconnectButton", dwPartialValue))
						m_mdmButtonInfo[DD_DISCONNECT][i].m_bPartialButton = dwPartialValue;
					if (regSettings.GetValue("PartialDisconnectSibling1", dwPartialValue))
						m_mdmButtonInfo[DD_DISCONNECT][i].m_bPartialSibling1 = dwPartialValue;
					if (regSettings.GetValue("PartialDisconnectSibling2", dwPartialValue))
						m_mdmButtonInfo[DD_DISCONNECT][i].m_bPartialSibling2 = dwPartialValue;
					if (regSettings.GetValue("PartialDisconnectSibling3", dwPartialValue))
						m_mdmButtonInfo[DD_DISCONNECT][i].m_bPartialSibling3 = dwPartialValue;				

					if (regSettings.GetValue("CancelDialParent", strValue))
						m_mdmButtonInfo[DD_CANCELDIAL][i].m_strParent = strValue;
					if (regSettings.GetValue("CancelDialButton", strValue))
						m_mdmButtonInfo[DD_CANCELDIAL][i].m_strButton = strValue;
					if (regSettings.GetValue("CancelDialSibling1", strValue))
						m_mdmButtonInfo[DD_CANCELDIAL][i].m_strSibling1 = strValue;
					if (regSettings.GetValue("CancelDialSibling2", strValue))
						m_mdmButtonInfo[DD_CANCELDIAL][i].m_strSibling2 = strValue;
					if (regSettings.GetValue("CancelDialSibling3", strValue))
						m_mdmButtonInfo[DD_CANCELDIAL][i].m_strSibling3 = strValue;
					if (regSettings.GetValue("PartialCancelDialParent", dwPartialValue))
						m_mdmButtonInfo[DD_CANCELDIAL][i].m_bPartialParent = dwPartialValue;
					if (regSettings.GetValue("PartialCancelDialButton", dwPartialValue))
						m_mdmButtonInfo[DD_CANCELDIAL][i].m_bPartialButton = dwPartialValue;
					if (regSettings.GetValue("PartialCancelDialSibling1", dwPartialValue))
						m_mdmButtonInfo[DD_CANCELDIAL][i].m_bPartialSibling1 = dwPartialValue;
					if (regSettings.GetValue("PartialCancelDialSibling2", dwPartialValue))
						m_mdmButtonInfo[DD_CANCELDIAL][i].m_bPartialSibling2 = dwPartialValue;
					if (regSettings.GetValue("PartialCancelDialSibling3", dwPartialValue))
						m_mdmButtonInfo[DD_CANCELDIAL][i].m_bPartialSibling3 = dwPartialValue;				
				}
			} else { //Save
				//Save configuration to registry
				regSettings.SetValue("ConnectParent", m_mdmButtonInfo[DD_CONNECT][DD_REGISTRY_SETTING].m_strParent);
				regSettings.SetValue("ConnectButton", m_mdmButtonInfo[DD_CONNECT][DD_REGISTRY_SETTING].m_strButton);
				regSettings.SetValue("ConnectSibling1", m_mdmButtonInfo[DD_CONNECT][DD_REGISTRY_SETTING].m_strSibling1);
				regSettings.SetValue("ConnectSibling2", m_mdmButtonInfo[DD_CONNECT][DD_REGISTRY_SETTING].m_strSibling2);
				regSettings.SetValue("ConnectSibling3", m_mdmButtonInfo[DD_CONNECT][DD_REGISTRY_SETTING].m_strSibling3);
				regSettings.SetValue("PartialConnectParent", (DWORD)m_mdmButtonInfo[DD_CONNECT][DD_REGISTRY_SETTING].m_bPartialParent);
				regSettings.SetValue("PartialConnectButton", (DWORD)m_mdmButtonInfo[DD_CONNECT][DD_REGISTRY_SETTING].m_bPartialButton);
				regSettings.SetValue("PartialConnectSibling1", (DWORD)m_mdmButtonInfo[DD_CONNECT][DD_REGISTRY_SETTING].m_bPartialSibling1);
				regSettings.SetValue("PartialConnectSibling2", (DWORD)m_mdmButtonInfo[DD_CONNECT][DD_REGISTRY_SETTING].m_bPartialSibling2);
				regSettings.SetValue("PartialConnectSibling3", (DWORD)m_mdmButtonInfo[DD_CONNECT][DD_REGISTRY_SETTING].m_bPartialSibling3);

				regSettings.SetValue("BusyParent", m_mdmButtonInfo[DD_BUSY][DD_REGISTRY_SETTING].m_strParent);
				regSettings.SetValue("BusyButton", m_mdmButtonInfo[DD_BUSY][DD_REGISTRY_SETTING].m_strButton);
				regSettings.SetValue("BusySibling1", m_mdmButtonInfo[DD_BUSY][DD_REGISTRY_SETTING].m_strSibling1);
				regSettings.SetValue("BusySibling2", m_mdmButtonInfo[DD_BUSY][DD_REGISTRY_SETTING].m_strSibling2);
				regSettings.SetValue("BusySibling3", m_mdmButtonInfo[DD_BUSY][DD_REGISTRY_SETTING].m_strSibling3);
				regSettings.SetValue("PartialBusyParent", (DWORD)m_mdmButtonInfo[DD_BUSY][DD_REGISTRY_SETTING].m_bPartialParent);
				regSettings.SetValue("PartialBusyButton", (DWORD)m_mdmButtonInfo[DD_BUSY][DD_REGISTRY_SETTING].m_bPartialButton);
				regSettings.SetValue("PartialBusySibling1", (DWORD)m_mdmButtonInfo[DD_BUSY][DD_REGISTRY_SETTING].m_bPartialSibling1);
				regSettings.SetValue("PartialBusySibling2", (DWORD)m_mdmButtonInfo[DD_BUSY][DD_REGISTRY_SETTING].m_bPartialSibling2);
				regSettings.SetValue("PartialBusySibling3", (DWORD)m_mdmButtonInfo[DD_BUSY][DD_REGISTRY_SETTING].m_bPartialSibling3);
	
				regSettings.SetValue("NoAnswerParent", m_mdmButtonInfo[DD_NOANSWER][DD_REGISTRY_SETTING].m_strParent);
				regSettings.SetValue("NoAnswerButton", m_mdmButtonInfo[DD_NOANSWER][DD_REGISTRY_SETTING].m_strButton);
				regSettings.SetValue("NoAnswerSibling1", m_mdmButtonInfo[DD_NOANSWER][DD_REGISTRY_SETTING].m_strSibling1);
				regSettings.SetValue("NoAnswerSibling2", m_mdmButtonInfo[DD_NOANSWER][DD_REGISTRY_SETTING].m_strSibling2);
				regSettings.SetValue("NoAnswerSibling3", m_mdmButtonInfo[DD_NOANSWER][DD_REGISTRY_SETTING].m_strSibling3);
				regSettings.SetValue("PartialNoAnswerParent", (DWORD)m_mdmButtonInfo[DD_NOANSWER][DD_REGISTRY_SETTING].m_bPartialParent);
				regSettings.SetValue("PartialNoAnswerButton", (DWORD)m_mdmButtonInfo[DD_NOANSWER][DD_REGISTRY_SETTING].m_bPartialButton);
				regSettings.SetValue("PartialNoAnswerSibling1", (DWORD)m_mdmButtonInfo[DD_NOANSWER][DD_REGISTRY_SETTING].m_bPartialSibling1);
				regSettings.SetValue("PartialNoAnswerSibling2", (DWORD)m_mdmButtonInfo[DD_NOANSWER][DD_REGISTRY_SETTING].m_bPartialSibling2);
				regSettings.SetValue("PartialNoAnswerSibling3", (DWORD)m_mdmButtonInfo[DD_NOANSWER][DD_REGISTRY_SETTING].m_bPartialSibling3);
					
				regSettings.SetValue("NoDialToneParent", m_mdmButtonInfo[DD_NODIALTONE][DD_REGISTRY_SETTING].m_strParent);
				regSettings.SetValue("NoDialToneButton", m_mdmButtonInfo[DD_NODIALTONE][DD_REGISTRY_SETTING].m_strButton);
				regSettings.SetValue("NoDialToneSibling1", m_mdmButtonInfo[DD_NODIALTONE][DD_REGISTRY_SETTING].m_strSibling1);
				regSettings.SetValue("NoDialToneSibling2", m_mdmButtonInfo[DD_NODIALTONE][DD_REGISTRY_SETTING].m_strSibling2);
				regSettings.SetValue("NoDialToneSibling3", m_mdmButtonInfo[DD_NODIALTONE][DD_REGISTRY_SETTING].m_strSibling3);
				regSettings.SetValue("PartialNoDialToneParent", (DWORD)m_mdmButtonInfo[DD_NODIALTONE][DD_REGISTRY_SETTING].m_bPartialParent);
				regSettings.SetValue("PartialNoDialToneButton", (DWORD)m_mdmButtonInfo[DD_NODIALTONE][DD_REGISTRY_SETTING].m_bPartialButton);
				regSettings.SetValue("PartialNoDialToneSibling1", (DWORD)m_mdmButtonInfo[DD_NODIALTONE][DD_REGISTRY_SETTING].m_bPartialSibling1);
				regSettings.SetValue("PartialNoDialToneSibling2", (DWORD)m_mdmButtonInfo[DD_NODIALTONE][DD_REGISTRY_SETTING].m_bPartialSibling2);
				regSettings.SetValue("PartialNoDialToneSibling3", (DWORD)m_mdmButtonInfo[DD_NODIALTONE][DD_REGISTRY_SETTING].m_bPartialSibling3);

				regSettings.SetValue("DisconnectParent", m_mdmButtonInfo[DD_DISCONNECT][DD_REGISTRY_SETTING].m_strParent);
				regSettings.SetValue("DisconnectButton", m_mdmButtonInfo[DD_DISCONNECT][DD_REGISTRY_SETTING].m_strButton);
				regSettings.SetValue("DisconnectSibling1", m_mdmButtonInfo[DD_DISCONNECT][DD_REGISTRY_SETTING].m_strSibling1);
				regSettings.SetValue("DisconnectSibling2", m_mdmButtonInfo[DD_DISCONNECT][DD_REGISTRY_SETTING].m_strSibling2);
				regSettings.SetValue("DisconnectSibling3", m_mdmButtonInfo[DD_DISCONNECT][DD_REGISTRY_SETTING].m_strSibling3);
				regSettings.SetValue("PartialDisconnectParent", (DWORD)m_mdmButtonInfo[DD_DISCONNECT][DD_REGISTRY_SETTING].m_bPartialParent);
				regSettings.SetValue("PartialDisconnectButton", (DWORD)m_mdmButtonInfo[DD_DISCONNECT][DD_REGISTRY_SETTING].m_bPartialButton);
				regSettings.SetValue("PartialDisconnectSibling1", (DWORD)m_mdmButtonInfo[DD_DISCONNECT][DD_REGISTRY_SETTING].m_bPartialSibling1);
				regSettings.SetValue("PartialDisconnectSibling2", (DWORD)m_mdmButtonInfo[DD_DISCONNECT][DD_REGISTRY_SETTING].m_bPartialSibling2);
				regSettings.SetValue("PartialDisconnectSibling3", (DWORD)m_mdmButtonInfo[DD_DISCONNECT][DD_REGISTRY_SETTING].m_bPartialSibling3);
			
				regSettings.SetValue("CancelDialParent", m_mdmButtonInfo[DD_CANCELDIAL][DD_REGISTRY_SETTING].m_strParent);
				regSettings.SetValue("CancelDialButton", m_mdmButtonInfo[DD_CANCELDIAL][DD_REGISTRY_SETTING].m_strButton);
				regSettings.SetValue("CancelDialSibling1", m_mdmButtonInfo[DD_CANCELDIAL][DD_REGISTRY_SETTING].m_strSibling1);
				regSettings.SetValue("CancelDialSibling2", m_mdmButtonInfo[DD_CANCELDIAL][DD_REGISTRY_SETTING].m_strSibling2);
				regSettings.SetValue("CancelDialSibling3", m_mdmButtonInfo[DD_CANCELDIAL][DD_REGISTRY_SETTING].m_strSibling3);
				regSettings.SetValue("PartialCancelDialParent", (DWORD)m_mdmButtonInfo[DD_CANCELDIAL][DD_REGISTRY_SETTING].m_bPartialParent);
				regSettings.SetValue("PartialCancelDialButton", (DWORD)m_mdmButtonInfo[DD_CANCELDIAL][DD_REGISTRY_SETTING].m_bPartialButton);
				regSettings.SetValue("PartialCancelDialSibling1", (DWORD)m_mdmButtonInfo[DD_CANCELDIAL][DD_REGISTRY_SETTING].m_bPartialSibling1);
				regSettings.SetValue("PartialCancelDialSibling2", (DWORD)m_mdmButtonInfo[DD_CANCELDIAL][DD_REGISTRY_SETTING].m_bPartialSibling2);
				regSettings.SetValue("PartialCancelDialSibling3", (DWORD)m_mdmButtonInfo[DD_CANCELDIAL][DD_REGISTRY_SETTING].m_bPartialSibling3);
			}
		}
		regSettings.Disconnect();
	}
}

void CDlgSettingsModem::OnOK() 
{
	//Save out settings to registry
	UpdateData();
	OnChangeComboModemAction();

	//Copy all the current values into registry values
	for (int i = 0; i <= DD_CANCELDIAL; i++)
	{
		m_mdmButtonInfo[i][DD_REGISTRY_SETTING].m_strParent = m_mdmButtonInfo[i][DD_CURRENT_SETTING].m_strParent;
		m_mdmButtonInfo[i][DD_REGISTRY_SETTING].m_strButton = m_mdmButtonInfo[i][DD_CURRENT_SETTING].m_strButton;
		m_mdmButtonInfo[i][DD_REGISTRY_SETTING].m_strSibling1 = m_mdmButtonInfo[i][DD_CURRENT_SETTING].m_strSibling1;
		m_mdmButtonInfo[i][DD_REGISTRY_SETTING].m_strSibling2 = m_mdmButtonInfo[i][DD_CURRENT_SETTING].m_strSibling2;
		m_mdmButtonInfo[i][DD_REGISTRY_SETTING].m_strSibling3 = m_mdmButtonInfo[i][DD_CURRENT_SETTING].m_strSibling3;
		m_mdmButtonInfo[i][DD_REGISTRY_SETTING].m_bPartialParent = m_mdmButtonInfo[i][DD_CURRENT_SETTING].m_bPartialParent;
		m_mdmButtonInfo[i][DD_REGISTRY_SETTING].m_bPartialButton = m_mdmButtonInfo[i][DD_CURRENT_SETTING].m_bPartialButton;
		m_mdmButtonInfo[i][DD_REGISTRY_SETTING].m_bPartialSibling1 = m_mdmButtonInfo[i][DD_CURRENT_SETTING].m_bPartialSibling1;
		m_mdmButtonInfo[i][DD_REGISTRY_SETTING].m_bPartialSibling2 = m_mdmButtonInfo[i][DD_CURRENT_SETTING].m_bPartialSibling2;
		m_mdmButtonInfo[i][DD_REGISTRY_SETTING].m_bPartialSibling3 = m_mdmButtonInfo[i][DD_CURRENT_SETTING].m_bPartialSibling3;
	}

	LoadSaveSettings(FALSE);
	
	CDialog::OnOK();
}

void CDlgSettingsModem::OnCancel() 
{
	//Set all current values back to registry values
	for (int i = 0; i <= DD_CANCELDIAL; i++)
	{
		m_mdmButtonInfo[i][DD_CURRENT_SETTING].m_strParent = m_mdmButtonInfo[i][DD_REGISTRY_SETTING].m_strParent;
		m_mdmButtonInfo[i][DD_CURRENT_SETTING].m_strButton = m_mdmButtonInfo[i][DD_REGISTRY_SETTING].m_strButton;
		m_mdmButtonInfo[i][DD_CURRENT_SETTING].m_strSibling1 = m_mdmButtonInfo[i][DD_REGISTRY_SETTING].m_strSibling1;
		m_mdmButtonInfo[i][DD_CURRENT_SETTING].m_strSibling2 = m_mdmButtonInfo[i][DD_REGISTRY_SETTING].m_strSibling2;
		m_mdmButtonInfo[i][DD_CURRENT_SETTING].m_strSibling3 = m_mdmButtonInfo[i][DD_REGISTRY_SETTING].m_strSibling3;
		m_mdmButtonInfo[i][DD_CURRENT_SETTING].m_bPartialParent = m_mdmButtonInfo[i][DD_REGISTRY_SETTING].m_bPartialParent;
		m_mdmButtonInfo[i][DD_CURRENT_SETTING].m_bPartialButton = m_mdmButtonInfo[i][DD_REGISTRY_SETTING].m_bPartialButton;
		m_mdmButtonInfo[i][DD_CURRENT_SETTING].m_bPartialSibling1 = m_mdmButtonInfo[i][DD_REGISTRY_SETTING].m_bPartialSibling1;
		m_mdmButtonInfo[i][DD_CURRENT_SETTING].m_bPartialSibling2 = m_mdmButtonInfo[i][DD_REGISTRY_SETTING].m_bPartialSibling2;
		m_mdmButtonInfo[i][DD_CURRENT_SETTING].m_bPartialSibling3 = m_mdmButtonInfo[i][DD_REGISTRY_SETTING].m_bPartialSibling3;
	}

	//Trick combo modem window into refilling current item
	m_iComboModemWindowLast = -1;
	OnChangeComboModemAction(); 
	
	CDialog::OnCancel();
}


void CDlgSettingsModem::OnChangeComboModemAction() 
{
	UpdateData();
	//Update old values if window has changed
	if (m_iComboModemWindowLast != -1)
	{
		m_mdmButtonInfo[m_iComboModemWindowLast][DD_CURRENT_SETTING].m_strParent = m_strParent;
		m_mdmButtonInfo[m_iComboModemWindowLast][DD_CURRENT_SETTING].m_strButton = m_strButton;
		m_mdmButtonInfo[m_iComboModemWindowLast][DD_CURRENT_SETTING].m_strSibling1 = m_strSibling1;
		m_mdmButtonInfo[m_iComboModemWindowLast][DD_CURRENT_SETTING].m_strSibling2 = m_strSibling2;
		m_mdmButtonInfo[m_iComboModemWindowLast][DD_CURRENT_SETTING].m_strSibling3 = m_strSibling3;
		m_mdmButtonInfo[m_iComboModemWindowLast][DD_CURRENT_SETTING].m_bPartialParent = m_bCheckPartialParent;
		m_mdmButtonInfo[m_iComboModemWindowLast][DD_CURRENT_SETTING].m_bPartialButton = m_bCheckPartialButton;
		m_mdmButtonInfo[m_iComboModemWindowLast][DD_CURRENT_SETTING].m_bPartialSibling1 = m_bCheckPartialSibling1;
		m_mdmButtonInfo[m_iComboModemWindowLast][DD_CURRENT_SETTING].m_bPartialSibling2 = m_bCheckPartialSibling2;
		m_mdmButtonInfo[m_iComboModemWindowLast][DD_CURRENT_SETTING].m_bPartialSibling3 = m_bCheckPartialSibling3;
	}

	//If the selection has changed, display the new data
	if (m_iComboModemWindow != m_iComboModemWindowLast)
	{		
		//Display new values
		m_strParent = m_mdmButtonInfo[m_iComboModemWindow][DD_CURRENT_SETTING].m_strParent;
		m_strButton = m_mdmButtonInfo[m_iComboModemWindow][DD_CURRENT_SETTING].m_strButton;
		m_strSibling1 = m_mdmButtonInfo[m_iComboModemWindow][DD_CURRENT_SETTING].m_strSibling1;
		m_strSibling2 = m_mdmButtonInfo[m_iComboModemWindow][DD_CURRENT_SETTING].m_strSibling2;
		m_strSibling3 = m_mdmButtonInfo[m_iComboModemWindow][DD_CURRENT_SETTING].m_strSibling3;
		m_bCheckPartialParent = m_mdmButtonInfo[m_iComboModemWindow][DD_CURRENT_SETTING].m_bPartialParent;
		m_bCheckPartialButton = m_mdmButtonInfo[m_iComboModemWindow][DD_CURRENT_SETTING].m_bPartialButton;
		m_bCheckPartialSibling1 = m_mdmButtonInfo[m_iComboModemWindow][DD_CURRENT_SETTING].m_bPartialSibling1;
		m_bCheckPartialSibling2 = m_mdmButtonInfo[m_iComboModemWindow][DD_CURRENT_SETTING].m_bPartialSibling2;
		m_bCheckPartialSibling3 = m_mdmButtonInfo[m_iComboModemWindow][DD_CURRENT_SETTING].m_bPartialSibling3;

		UpdateData(FALSE);			

		//Keep track of last combo selection
		m_iComboModemWindowLast = m_iComboModemWindow;
	}	
}

BOOL CDlgSettingsModem::PreTranslateMessage(MSG* pMsg) 
{
	if ((pMsg->message == WM_KEYDOWN) &&
		((pMsg->wParam == VK_RETURN) ||
		 (pMsg->wParam == VK_ESCAPE)))
	{	
		return TRUE;
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}
