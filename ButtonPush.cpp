// ButtonPush.cpp: implementation of the CButtonPush class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "downloaddemon.h"
#include "ButtonPush.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CMutex m_mutexOnlyOne;

// Callback functions

BOOL CALLBACK HandleSiblingWindowCheck(HWND hwnd, LPARAM lParam)
{
	try
	{
		if (!::IsWindow(hwnd))
			return TRUE; //skip to next

		CString strWindowText(_T(""));
		if ((!::GetWindowText(hwnd, strWindowText.GetBuffer(2048), 2048)) &&
			(::GetLastError() != ERROR_SUCCESS))
		{
			strWindowText.ReleaseBuffer();
			return TRUE; //skip to next
		} else
			strWindowText.ReleaseBuffer();

		if (strWindowText.GetLength())
		{
			CTypedPtrList <CPtrList, CSiblingItem*>* pLstSiblings = (CTypedPtrList <CPtrList, CSiblingItem*>*)lParam;

			POSITION pos = pLstSiblings->GetHeadPosition();
			while (pos)
			{
				CSiblingItem* pSibling = pLstSiblings->GetNext(pos);
				CString strSiblingText = pSibling->GetSiblingText();
				ASSERT(pSibling);
				if ((strSiblingText == strWindowText) ||
					((pSibling->GetSiblingPartial()) &&
					 (strWindowText.Find(strSiblingText) != -1)))
				{
					pSibling->SetSiblingFound(TRUE);				
				}
			}
		}
	}
	catch(...)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CALLBACK HandleChildWindow(HWND hwnd, /*handle to child window*/
								LPARAM lParam /* application-defined value*/)
{		
	try
	{
		CButtonPush* pButtonPush = (CButtonPush*) lParam;

		//Ensure this is a window
		if (!::IsWindow(hwnd))
			return TRUE; //skip to next

		HWND hwndParent = ::GetParent(hwnd);
		if (hwndParent == NULL)
			return FALSE;

		CString strWindowText(_T(""));
		if ((!::GetWindowText(hwnd, strWindowText.GetBuffer(2048), 2048)) &&
			(::GetLastError() != ERROR_SUCCESS))
		{
			strWindowText.ReleaseBuffer();
			return TRUE; //skip to next
		} else
			strWindowText.ReleaseBuffer();


		CString strButtonWatch = pButtonPush->GetButton();

		if ((strButtonWatch.GetLength() && strWindowText.GetLength()) &&
			((strButtonWatch == strWindowText) ||
			 ((pButtonPush->GetButtonPartial()) &&
			  (strWindowText.Find(strButtonWatch) != -1))))
		{		
			pButtonPush->ValidateSiblingsAndClick(hwnd);
			return FALSE;
		}
		if (pButtonPush->m_bFound)
			return FALSE;
		else
			return TRUE;
	}
	catch(...)
	{
		return FALSE;
	}	
}

BOOL CALLBACK HandleWindow(HWND hwnd, /*handle to parent window*/
						   LPARAM lParam /*application-defined value*/)
{
	try
	{
		CButtonPush* pButtonPush = (CButtonPush*) lParam;
		CString strParentWatch = pButtonPush->GetParent();
		CString strWindowText(_T(""));

		if (!::IsWindow(hwnd))
			return TRUE; //skip to next
		
		if ((!::GetWindowText(hwnd, strWindowText.GetBuffer(2048), 2048)) &&
			(::GetLastError() != ERROR_SUCCESS))
		{
			strWindowText.ReleaseBuffer();
			return TRUE; //skip to next
		} else
			strWindowText.ReleaseBuffer();

		//Ensure pWnd is a window first - to be safe
		if (::IsWindowVisible(hwnd))
		{
			if ((strParentWatch.GetLength() == 0) ||
				((strWindowText.GetLength()) && 
	 			 ((strParentWatch  == strWindowText) ||
				  ((pButtonPush->GetParentPartial()) &&
				   (strWindowText.Find(strParentWatch) != -1)))))
			{		
				EnumChildWindows(hwnd, HandleChildWindow, lParam);
			}
		}
		if (pButtonPush->m_bFound)
			return FALSE;
		else
			return TRUE;
	}
	catch(...)
	{
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CButtonPush::CButtonPush(CString strButton, BOOL bButtonPartial,
						 CString strParent, BOOL bParentPartial,
						 UINT uiMessage /*= BM_CLICK*/, BOOL bSendParent /*= FALSE*/,
						 WPARAM wParam /*= 0*/ , LPARAM lParam /*= 0*/)
{
	m_strButton = strButton;
	m_bButtonPartial = bButtonPartial;
	m_strParent = strParent;
	m_bParentPartial = bParentPartial;
	m_uiMessageSend = uiMessage;
	m_bSendParentMessage = bSendParent;
	m_lstSiblings.RemoveAll();
	m_bPressed = FALSE;
	m_bFound = FALSE;
	m_bClickButton = TRUE;
	m_wParam = wParam;
	m_lParam = lParam;
	m_bAuxWindow = FALSE;
}

CButtonPush::~CButtonPush()
{
	ClearSiblingList();	
}

CButtonPush::CButtonPush(CButtonPush* pCopyButtonPush)
{
	m_strButton = pCopyButtonPush->m_strButton;
	m_bButtonPartial = pCopyButtonPush->m_bButtonPartial;
	m_strParent = pCopyButtonPush->m_strParent;
	m_bParentPartial = pCopyButtonPush->m_bParentPartial;
	m_uiMessageSend = pCopyButtonPush->m_uiMessageSend;
	m_bSendParentMessage = pCopyButtonPush->m_bSendParentMessage;
	m_lstSiblings.RemoveAll();

	POSITION pos = pCopyButtonPush->m_lstSiblings.GetHeadPosition();
	while (pos)
	{
		CSiblingItem* pSibling = pCopyButtonPush->m_lstSiblings.GetNext(pos);
		ASSERT(pSibling);
		AddSibling(pSibling->GetSiblingText(), pSibling->GetSiblingPartial());
	}	

	m_bPressed = FALSE;
	m_bFound = FALSE;
	m_bClickButton = TRUE;
}

void CButtonPush::ClearSiblingList()
{
	POSITION pos = m_lstSiblings.GetHeadPosition();
	while (pos)
	{
		CSiblingItem* pSibling = m_lstSiblings.GetNext(pos);
		ASSERT(pSibling);
		if (pSibling)
		{
			delete pSibling;
			pSibling = NULL;
		}
	}
	
	m_lstSiblings.RemoveAll();
}

void CButtonPush::AddSibling(CString strSibling, BOOL bPartial /*= FALSE*/)
{
	//Ensure the sibling actually has a length before adding it
	if (strSibling.GetLength())
	{
		CSiblingItem* pSiblingAdd = new CSiblingItem(strSibling, bPartial);
		m_lstSiblings.AddTail(pSiblingAdd);
	}
}

BOOL CButtonPush::AttemptPressButton()
{
	try
	{
		m_bFound = FALSE;
		m_bPressed = FALSE;
		m_bClickButton = TRUE;

		//Allow only one lookup at a time
		//This is shared with AttemptFindButton	
		CSingleLock lockPressFind(&m_mutexOnlyOne);
		lockPressFind.Lock();
		EnumWindows(HandleWindow, (LPARAM)this);
		lockPressFind.Unlock();

		return m_bPressed;
	}
	catch(...)
	{
	}
	return FALSE;
}

BOOL CButtonPush::AttemptFindButton()
{
	try
	{
		m_bFound = FALSE;
		m_bPressed = FALSE;
		m_bClickButton = FALSE;

		//Allow only one lookup at a time
		//This is shared with AttemptPressButton
		CSingleLock lockPressFind(&m_mutexOnlyOne);
		lockPressFind.Lock();
		EnumWindows(HandleWindow, (LPARAM)this);
		lockPressFind.Unlock();

		return m_bFound;
	}
	catch(...)
	{
	}
	return FALSE;
}

void CButtonPush::ValidateSiblingsAndClick(HWND hwnd)
{
	try
	{
		POSITION pos;

		if (::IsWindow(hwnd))
		{
			HWND hwndParent = ::GetParent(hwnd);
			if (hwndParent != NULL)
			{
				//Set all siblings that they weren't found
				pos = m_lstSiblings.GetHeadPosition();
				while (pos)
				{
					CSiblingItem* pSibling = m_lstSiblings.GetNext(pos);
					ASSERT(pSibling);
					pSibling->SetSiblingFound(FALSE);
				}

				//Check for all siblings
				EnumChildWindows(hwndParent, HandleSiblingWindowCheck, (LPARAM)&m_lstSiblings);

				//Check that each sibling was found
				BOOL bFound = TRUE;
				pos = m_lstSiblings.GetHeadPosition();
				while (pos)
				{
					CSiblingItem* pSibling = m_lstSiblings.GetNext(pos);
					ASSERT(pSibling);
					if (!pSibling->GetSiblingFound())
					{
						bFound = FALSE;
						break;
					}
				}

				// If all the siblings were found, press the button
				if (bFound)
				{
					m_bFound = TRUE;
					
					// Ensure pWnd is a window first - to be safe
					if (::IsWindow(hwnd) && 
						::IsWindowVisible(hwndParent) && 
						::IsWindowVisible(hwnd))
					{
						// Only click if we're supposed to
						if (m_bClickButton)
						{
							// Move this window to the foreground
							// we use the parent window, since that will
							// work most effectively - using the button
							// window doesn't work as well.
							::SetForegroundWindow(hwndParent);

							// Sleeping for 1 second seems to cure some problems
							// for auxillary (sic) windows
							if (m_bAuxWindow)
								Sleep(1000);

							// Click the button
							if (::IsWindowVisible(hwnd))
							{
								if (m_bSendParentMessage)
								{
									PostMessage(hwndParent, m_uiMessageSend, m_wParam, m_lParam);
								}
								else
								{
									PostMessage(hwnd, m_uiMessageSend, m_wParam, m_lParam);
								}								
							}
							m_bPressed = TRUE;
						}
					}
				}
			}
		}
	}
	catch(...)	
	{
	}
}


void CButtonPush::SetAuxWindow(BOOL bAuxWindow)
{
	m_bAuxWindow = bAuxWindow;
}
