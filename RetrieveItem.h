#ifndef RETRIEVEITEM_H
#define RETRIEVEITEM_H

class CRetrieveItem
{
public:
	CRetrieveItem()
	{	
		m_strFullPath = _T("");
		m_dwServiceType = 0;
		m_strServer = _T("");
		m_strObject = _T("");
		m_nPort = 0;
	}

	BOOL ExtractURL(TCHAR* szText)
	{
		BOOL bURLFound = FALSE;
	
		//Ensure we have at least a string to work with
		if (!szText)
		{
			return FALSE;
		}
		//Extract the URL and ensure it's either FTP or HTTP
		bURLFound = (AfxParseURL(szText, m_dwServiceType, m_strServer, m_strObject, m_nPort) && 
					 ((m_dwServiceType == AFX_INET_SERVICE_FTP) || 
					  (m_dwServiceType == AFX_INET_SERVICE_HTTP)));
		if (bURLFound)
		{
			//We need an object for this to be a valid URL
			if ((m_strObject.GetLength()) &&
				(m_strObject.Right(1) != _T('/')) &&
				(m_strObject.Right(1) != _T('\\')))
			{
				m_strFullPath = szText;
				m_strFullPath.TrimLeft();
				m_strFullPath.TrimRight();
			} else 
				bURLFound = FALSE;
		}
		return bURLFound;
	}

	BOOL ExtractURL(HGLOBAL hText)
	{
		BOOL bURLFound = FALSE;
		TCHAR* szText = (TCHAR*)GlobalLock(hText);
	
		bURLFound = ExtractURL(szText);

		GlobalUnlock(hText);		

		return bURLFound;
	}

	CString m_strFullPath;
	DWORD m_dwServiceType;
	CString m_strServer;
	CString m_strObject;
	INTERNET_PORT m_nPort;
};
#endif //ndef RETRIEVEITEM_H