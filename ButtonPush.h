// ButtonPush.h: interface for the CButtonPush class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BUTTONPUSH_H__C147C362_FC3F_11D1_A260_444553540000__INCLUDED_)
#define AFX_BUTTONPUSH_H__C147C362_FC3F_11D1_A260_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CSiblingItem {
	CString m_strSibling;
	BOOL m_bPartial;
	BOOL m_bFoundAsSibling;	
public:
	CSiblingItem(CString strSibling = _T(""), BOOL bPartial = FALSE)
	{
		ASSERT(strSibling.GetLength() > 0);
		m_strSibling = strSibling;
		m_bPartial = bPartial;
		m_bFoundAsSibling = FALSE;
	}
	virtual ~CSiblingItem() {}
	BOOL GetSiblingPartial() 
	{
		return m_bPartial;
	}
	void SetSiblingFound(BOOL bFound)
	{
		m_bFoundAsSibling = bFound;
	}
	CString GetSiblingText()
	{
		return m_strSibling;
	}
	BOOL GetSiblingFound()
	{
		return m_bFoundAsSibling;
	}

};

class CButtonPush  
{
	CString m_strButton;
	BOOL m_bButtonPartial;
	CString m_strParent;
	BOOL m_bParentPartial;
	UINT m_uiMessageSend;
	BOOL m_bSendParentMessage;
	WPARAM m_wParam;
	LPARAM m_lParam;
	CTypedPtrList <CPtrList, CSiblingItem*> m_lstSiblings;	
	BOOL m_bClickButton;	

	void ClearSiblingList();
public:
	BOOL m_bPressed; //Used to transfer knowledge since we
	                 //can only pass one pointer into the 
	                 //EnumerateWindows functions
	BOOL m_bFound; //Used to transfer knowledge since we
	               //can only pass one pointer into the 
	               //EnumerateWindows functions


	CButtonPush(CString strButton, BOOL bButtonPartial,
				CString strParent, BOOL bParentPartial,
				UINT uiMessage = BM_CLICK, BOOL bSendParent = FALSE,
				WPARAM wParam = 0, LPARAM lParam = 0);
	virtual ~CButtonPush();
	CButtonPush(CButtonPush* pCopyButtonPush);
	void AddSibling(CString strSibling, BOOL bPartial = FALSE);
	BOOL AttemptPressButton();
	BOOL AttemptFindButton();
	void ValidateSiblingsAndClick(HWND hwnd); //returns if button was found
	void SetAuxWindow(BOOL bAuxWindow);
	CString GetButton()
	{
		return m_strButton;
	}
	BOOL GetButtonPartial()
	{
		return m_bButtonPartial;
	}
	CString GetParent()
	{
		return m_strParent;
	}
	BOOL GetParentPartial()
	{
		return m_bParentPartial;
	}
private:	
	BOOL m_bAuxWindow; // If this is a subwindow, then we need to know
};

#endif // !defined(AFX_BUTTONPUSH_H__C147C362_FC3F_11D1_A260_444553540000__INCLUDED_)
