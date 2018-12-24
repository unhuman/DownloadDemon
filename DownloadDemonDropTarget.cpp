//DownloadDemonDropTarget.cpp : implementation file
//

#include "stdafx.h"
#include "DownloadDemonDropTarget.h"
#include "DlgProperties.h" //for Transfer Mode
/////////////////////////////////////////////////////////////////////////////
// CDownloadDemonDropTarget
CDownloadDemonDropTarget::CDownloadDemonDropTarget(CLIPFORMAT cfObjectDescriptor)
{	
	m_pDownloadListCtrl = NULL;

	m_prevDropEffect = DROPEFFECT_NONE;
	m_bAllowDrop = FALSE;
	m_bAllowAdditional = TRUE;
}

BOOL CDownloadDemonDropTarget::Register(CDownloadListCtrl* pDownloadListCtrlWnd)
{
	m_pDownloadListCtrl = pDownloadListCtrlWnd;
	return COleDropTarget::Register(pDownloadListCtrlWnd);
}

BOOL CDownloadDemonDropTarget::ObjectIsLink(COleDataObject* pDataObject, CRetrieveItem& cRetrieveItem)
{
	ASSERT(m_pDownloadListCtrl);

	BOOL bFound = FALSE;

	//Check if dropped item is a link
	if (pDataObject->IsDataAvailable(CF_TEXT))
	{
		bFound = TRUE;
		HGLOBAL hText = pDataObject->GetGlobalData(CF_TEXT);
		if( hText != NULL )
		{
			bFound = cRetrieveItem.ExtractURL(hText);
		}
	}

	return bFound;
}

DROPEFFECT CDownloadDemonDropTarget::OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject,
										   DWORD grfKeyState, CPoint point)
{
	ASSERT(m_pDownloadListCtrl);
	ASSERT(m_prevDropEffect == DROPEFFECT_NONE);

	CRetrieveItem cRetrieveItem;

	//If we're allowing drops, check for valid link, otherwise,
	//this isn't valid
	if (m_bAllowAdditional)	
		m_bAllowDrop = ObjectIsLink(pDataObject, cRetrieveItem);	
	else
		m_bAllowDrop = FALSE;

	return COleDropTarget::OnDragOver(pWnd, pDataObject, grfKeyState, point);
}

DROPEFFECT CDownloadDemonDropTarget::OnDragOver(CWnd* pWnd, COleDataObject*, 
										  DWORD grfKeyState, CPoint point)
{
	ASSERT(m_pDownloadListCtrl);

	DROPEFFECT de = DROPEFFECT_NONE;

	//Check to see if we are going to allow this guy to be dropped
	if (m_bAllowDrop)
	{
		de = DROPEFFECT_LINK;
	}

	return de;
}

void CDownloadDemonDropTarget::OnDragLeave(CWnd* pWnd)
{
	ASSERT(m_pDownloadListCtrl);

	CClientDC dc(pWnd);
	if (m_prevDropEffect != DROPEFFECT_NONE)
	{
		dc.DrawFocusRect(CRect(m_dragPoint,m_dragSize)); // erase previous focus rect
		m_prevDropEffect = DROPEFFECT_NONE;
	}
}

BOOL CDownloadDemonDropTarget::OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
								DROPEFFECT dropEffect, CPoint point)
{
	ASSERT(m_pDownloadListCtrl);
	ASSERT_VALID(pWnd);	

	CRetrieveItem cRetrieveItem;
	CString strNo(_T(""));
	strNo.LoadString(IDS_NO);

	// clean up focus rect
	OnDragLeave(pWnd);

	// offset point as appropriate for dragging
	if (m_bAllowDrop && 
		ObjectIsLink(pDataObject, cRetrieveItem))
	{
		//Do our drop/paste
		m_pDownloadListCtrl->AddItem(cRetrieveItem.m_strFullPath, 
									 CDlgProperties.BINARY, strNo, TRUE,
									 _T(""), _T(""), _T(""));
		
		return TRUE;
	}

	return FALSE;
}

void CDownloadDemonDropTarget::AllowAdditional(BOOL bAllowAdditional)
{
	m_bAllowAdditional = bAllowAdditional;
}