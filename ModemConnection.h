#if !defined(CModemConnectionDownloadDemon)
#define CModemConnectionDownloadDemon

#include "Resource.h"
#include "ButtonPush.h"

class CModemConnection
{
	enum {NOTPROCESSING, CONNECTING};
	CDialog* m_pDlgParent;
	CButtonPush* m_pBtnDial;	
	CButtonPush* m_pBtnBusy;
	CButtonPush* m_pBtnNoAnswer;
	CButtonPush* m_pBtnNoDialTone;
	CButtonPush* m_pBtnConnected;
	CButtonPush* m_pBtnConnectedMinimize;
	CButtonPush* m_pBtnConnectedDialWatch;
	CButtonPush* m_pBtnConnectedErrorWatch;
	BOOL m_bRunningConnect;
	int m_iStatus;
public:
	CModemConnection(CDialog* pDlgParent = NULL);
	virtual ~CModemConnection();
	BOOL EnsureConnection();
	void PressAppropriateButtons();
	void Disconnect();
	int GetStatus()
	{
		return m_iStatus;
	}
	BOOL GetRunningConnect()
	{
		return m_bRunningConnect;
	}
};

#endif