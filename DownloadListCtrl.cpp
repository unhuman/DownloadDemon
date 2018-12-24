//DownloadListControl.cpp : implementation file
//

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "stdafx.h"
#include "resource.h"
#include "DownloadListCtrl.h"
#include "Registry.h"
#include "DlgProperties.h" //to get XferMode
#include "DownloadDemonDlg.h"

#define NUMCOLS			6
#define IMAGE_MARGIN	2
#define HIDDEN_DATA_SEPARATOR	_T(" | ")

BEGIN_MESSAGE_MAP(CDownloadListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CDownloadListCtrl)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//Saves out the column widths
void CDownloadListCtrl::SaveColHeadingSizes()
{
	for (int i = 0; i < NUMCOLS; i++)
		GetRegistryColSize(FALSE, i);
}

//
void CDownloadListCtrl::AddColumnHeadings(BOOL bUseExistingConfig)
{
	int iColWidth(0);

	//Add my column headings
	CString strHeading(_T(""));
	iColWidth = (bUseExistingConfig) ? GetRegistryColSize(TRUE, 0, 94) : 94;
	strHeading.LoadString(IDS_COL_STATUS);
	InsertColumn(0, strHeading, LVCFMT_LEFT, iColWidth);

	iColWidth = (bUseExistingConfig) ? GetRegistryColSize(TRUE, 1, 180) : 180;
	strHeading.LoadString(IDS_COL_LINK);
	InsertColumn(1, strHeading, LVCFMT_LEFT, iColWidth);

	iColWidth = (bUseExistingConfig) ? GetRegistryColSize(TRUE, 2, 41) : 41;
	strHeading.LoadString(IDS_COL_XFER_MODE);
	InsertColumn(2, strHeading, LVCFMT_LEFT, iColWidth);

	iColWidth = (bUseExistingConfig) ? GetRegistryColSize(TRUE, 3, 35) : 35;
	strHeading.LoadString(IDS_COL_STRIP);
	InsertColumn(3, strHeading, LVCFMT_LEFT, iColWidth);

	iColWidth = (bUseExistingConfig) ? GetRegistryColSize(TRUE, 4, 116) : 116;
	strHeading.LoadString(IDS_COL_DESTINATION_DIR);
	InsertColumn(4, strHeading, LVCFMT_LEFT, iColWidth);

	iColWidth = (bUseExistingConfig) ? GetRegistryColSize(TRUE, 5, 56) : 56;
	strHeading.LoadString(IDS_COL_LOGIN);
	InsertColumn(5, strHeading, LVCFMT_LEFT, iColWidth);

	//Create our image list
	CBitmap bitmap;
	int iAdded = 0;
	
	m_cImageList.Create(16, 16, TRUE, 6, 7);
	bitmap.LoadBitmap(IDB_BITMAP_WAIT);
	iAdded = m_cImageList.Add(&bitmap, (COLORREF)0xFFFFFF);
	bitmap.DeleteObject();
	bitmap.LoadBitmap(IDB_BITMAP_CONNECT);
	iAdded = m_cImageList.Add(&bitmap, (COLORREF)0xFFFFFF);
	bitmap.DeleteObject();
	bitmap.LoadBitmap(IDB_BITMAP_TRANSFER);
	iAdded = m_cImageList.Add(&bitmap, (COLORREF)0xFFFFFF);
	bitmap.DeleteObject();
	bitmap.LoadBitmap(IDB_BITMAP_FINISHED);
	iAdded = m_cImageList.Add(&bitmap, (COLORREF)0xFFFFFF);
	bitmap.DeleteObject();
	bitmap.LoadBitmap(IDB_BITMAP_STOPPED);
	iAdded = m_cImageList.Add(&bitmap, (COLORREF)0xFFFFFF);
	bitmap.DeleteObject();
	bitmap.LoadBitmap(IDB_BITMAP_FAILED);
	iAdded = m_cImageList.Add(&bitmap, (COLORREF)0xFFFFFF);
	bitmap.DeleteObject();
	bitmap.LoadBitmap(IDB_BITMAP_TIMEDOUT);
	iAdded = m_cImageList.Add(&bitmap, (COLORREF)0xFFFFFF);
	bitmap.DeleteObject();
	bitmap.LoadBitmap(IDB_BITMAP_LOOKINGFORFILE);
	iAdded = m_cImageList.Add(&bitmap, (COLORREF)0xFFFFFF);
	bitmap.DeleteObject();
	bitmap.LoadBitmap(IDB_BITMAP_NOTFOUND);
	iAdded = m_cImageList.Add(&bitmap, (COLORREF)0xFFFFFF);
	bitmap.DeleteObject();
	bitmap.LoadBitmap(IDB_BITMAP_FILEOPENERR);
	iAdded = m_cImageList.Add(&bitmap, (COLORREF)0xFFFFFF);
	bitmap.DeleteObject();
	bitmap.LoadBitmap(IDB_BITMAP_FILEWRITEERR);
	iAdded = m_cImageList.Add(&bitmap, (COLORREF)0xFFFFFF);
	bitmap.DeleteObject();
	bitmap.LoadBitmap(IDB_BITMAP_SKIPPED);
	iAdded = m_cImageList.Add(&bitmap, (COLORREF)0xFFFFFF);
	bitmap.DeleteObject();

	SetImageList(&m_cImageList, LVSIL_SMALL);
}

BOOL CDownloadListCtrl::AddItem(CString strLink, int iMode, CString strStrip,
								BOOL bInformErr, CString strDestDir,
								CString strUserName, CString strPassword, 
								CString strCustomReferrer /*  = _T("") */)
{
	BOOL bSuccess = TRUE;
	int iLoc = GetItemCount();
		
	CString strItem(_T(""));
	for (int i = 0; i < GetItemCount(); i++)
	{
		GetItemText(i, 1, strItem.GetBuffer(1024), 1024);
		strItem.ReleaseBuffer();
		if (strItem == strLink)
		{
			if (bInformErr)
				AfxMessageBox(IDS_LINK_EXISTS);
			return FALSE;
		}
	}	

	//Now ensure everything is *NOT* selected
	if (GetSelectedCount() > 0)
	{
		for (int i = 0; i < GetItemCount(); i++)
		{
			SetItemState(i, 0, LVIS_SELECTED);
			SetItemState(i, 0, ODS_SELECTED);
		}
	}

	CString strTemp(_T(""));
	strTemp.LoadString(IDS_NOTSTARTED);
	LV_ITEM lvItem;
	lvItem.iItem = iLoc;
	lvItem.iSubItem  = 0; 
	lvItem.mask = LVIF_TEXT | LVIF_IMAGE ;
	lvItem.state = LVIF_IMAGE | LVIF_TEXT;	
	lvItem.iImage = 0;
	lvItem.pszText = strTemp.GetBuffer(strTemp.GetLength());
	iLoc = InsertItem(&lvItem);
	strTemp.ReleaseBuffer();	

	bSuccess = UpdateItem(iLoc, strLink, iMode, strStrip, 
						  strDestDir, strUserName, strPassword, strCustomReferrer);

	SetItemState(iLoc, LVIS_SELECTED, LVIS_SELECTED);
	SetItemState(iLoc, ODS_SELECTED, ODS_SELECTED);
	EnsureVisible(iLoc, FALSE);

	return bSuccess;
}

BOOL CDownloadListCtrl::RetrieveItem(int iItemRet, int& iImage, 
									 CString& strFullPath, int& iMode,
									 CString& strStrip, CString& strDestDir,
									 CString& strUser, CString& strPW, 
									 CString& strCustomReferrer)
{
	if (iItemRet >=  GetItemCount())
		return FALSE;

	CString strASCIIMode(_T(""));
	strASCIIMode.LoadString(IDS_MODE_ASCII);

	iImage = GetItemStatus(iItemRet);
	strFullPath	= GetItemText(iItemRet, 1);
	
	iMode = CDlgProperties.BINARY;
	if (GetItemText(iItemRet, 2) == strASCIIMode)
		iMode = CDlgProperties.ASCII;

	strStrip	= GetItemText(iItemRet, 3);
	strDestDir	= GetItemText(iItemRet, 4);
	strUser		= GetItemText(iItemRet, 5);
	CString* pStrPW = (CString*)GetItemData(iItemRet);
	strPW		= *pStrPW;
	strCustomReferrer = _T("");
	
	int iSeparatorFound = pStrPW->Find(HIDDEN_DATA_SEPARATOR);
	if (iSeparatorFound >= 0)
	{
		strCustomReferrer = pStrPW->Mid(iSeparatorFound + strlen(HIDDEN_DATA_SEPARATOR));
		strPW = strPW.Left(iSeparatorFound);		
	}
	return TRUE;
}

BOOL CDownloadListCtrl::UpdateStatus(int iItemUpdate, int iImage, CString strStatus)
{
	if (iItemUpdate < GetItemCount())
	{
		//Set this guy's state to downloading
		LVITEM lvItem;
		lvItem.iItem = iItemUpdate;
		lvItem.iSubItem  = 0; 
		lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
		lvItem.state = LVIF_IMAGE | LVIF_TEXT;
		lvItem.iImage = iImage;
		lvItem.pszText = strStatus.GetBuffer(strStatus.GetLength());
		SetItem(&lvItem);
		strStatus.ReleaseBuffer();
		
		return TRUE;
	} else
		return FALSE;
}

BOOL CDownloadListCtrl::UpdateItem(int iUpdateItem, CString strFullPath, int iMode,
								   CString strStrip, CString strDestDir,
								   CString strUser, CString strPW,
								   CString strCustomReferrer)
{
	if (iUpdateItem >=  GetItemCount())
		return FALSE;

	CString strASCIIMode(_T(""));
	CString strBinaryMode(_T(""));
	strASCIIMode.LoadString(IDS_MODE_ASCII);
	strBinaryMode.LoadString(IDS_MODE_BINARY);

	SetItemText(iUpdateItem, 1, strFullPath);
	
	CString strMode(_T(""));
	if (iMode == CDlgProperties.ASCII)
		strMode.LoadString(IDS_MODE_ASCII);
	else //BINARY
		strMode.LoadString(IDS_MODE_BINARY);
	SetItemText(iUpdateItem, 2, strMode);
	
	// Strip - use whatever we're told unless it's FTP or not an HTML file
	// In those cases, use N/A
	CString strNA(_T(""));
	strNA.LoadString(IDS_NA);
	CString strProtocol(_T(""));
	CString strExtension(_T(""));
	strProtocol = strFullPath.Left(4);
	strProtocol.MakeUpper();
	strExtension = strFullPath.Right(5);
	strExtension.MakeUpper();

	if ((strProtocol == _T("HTTP")) &&
		((strExtension == _T(".HTML")) ||
		 (strExtension.Right(4) == _T(".HTM"))))
		SetItemText(iUpdateItem, 3, strStrip);
	else
		SetItemText(iUpdateItem, 3, strNA);

	//Release any memory associated with old password
	CString* pOldPW = (CString*)GetItemData(iUpdateItem);
	if (pOldPW)
	{
		delete pOldPW;
		pOldPW = NULL;
		SetItemData(iUpdateItem, NULL);
	}

	if (!strDestDir.GetLength())
	{
		CString strDefaultDestDirUser(_T(""));
		strDefaultDestDirUser.LoadString(IDS_DEFAULT_DESTDIR);

		SetItemText(iUpdateItem, 4, strDefaultDestDirUser);
	} else {
		SetItemText(iUpdateItem, 4, strDestDir);
	}

	CString strItemData(_T(""));

	if (!strUser.GetLength())
	{
		CString strDefaultUser(_T(""));
		CString strDefaultPW(_T(""));
		strDefaultUser.LoadString(IDS_DEFAULT_USER);
		strDefaultPW.LoadString(IDS_DEFAULT_PW);

		SetItemText(iUpdateItem, 5, strDefaultUser);
		strItemData = strDefaultPW;
	} else {
		SetItemText(iUpdateItem, 5, strUser);
		strItemData = strPW;
	}

	if (strCustomReferrer.GetLength())
		strItemData += HIDDEN_DATA_SEPARATOR + strCustomReferrer;

	SetItemData(iUpdateItem, (unsigned long)new CString(strItemData));
	
	return TRUE;
}


int CDownloadListCtrl::GetRegistryColSize(BOOL bLoad, int iCol, int iDefault)
{
	int iColWidth = iDefault;
	CString strRegValue(_T(""));
	BOOL bSuccess(FALSE);

	strRegValue.Format("col%d", iCol);	
	COXRegistry regWindowSettings;

	if (regWindowSettings.Connect(COXRegistry::keyLocalMachine))
	{
		CString strRegKey(_T(""));
		strRegKey.LoadString(IDS_REGISTRY_KEY);
		if (regWindowSettings.CheckAndOpenKeyForWrite(strRegKey))
		{
			if (bLoad) //Load
			{				
				DWORD dwColWidth = iDefault;
				bSuccess = regWindowSettings.GetValue(strRegValue, dwColWidth);
				if (bSuccess)
					iColWidth = dwColWidth;
			} else { //Save
				//Save configuration to registry
				DWORD dwColWidth(0);
				dwColWidth = GetColumnWidth(iCol);
				regWindowSettings.SetValue(strRegValue, dwColWidth);
			}
		}
		regWindowSettings.Disconnect();
	}

	return iColWidth;
}

int CDownloadListCtrl::GetCountForStatus(int iStatus)
{
	int iCount(0);

	for (int i=0; i<GetItemCount(); i++)
	{
		//If the image status matches, increment count
		if (GetItemStatus(i) == iStatus)
			iCount++;
	}

	return iCount;
}

int CDownloadListCtrl::GetItemStatus(int iItem)
{
	LV_ITEM lvItem;
	lvItem.iItem = iItem;
	lvItem.iSubItem  = 0; 
	lvItem.mask = LVIF_IMAGE;
	GetItem(&lvItem);

	return lvItem.iImage;
}

void CDownloadListCtrl::DeleteAllItemsWithStatus(Status statDelete)
{
	//loop downward, so we don't need to worry about realignment
	for (int i=GetItemCount()-1; i>=0; i--)
	{
		//Delete all items with matching status
		if (GetItemStatus(i) == statDelete)
			DeleteItem(i);
	}
	//Force Redraw
	Invalidate();
}

void CDownloadListCtrl::DeleteSelectedItems()
{
	//loop downward, so we don't need to worry about realignment
	for (int i=GetItemCount()-1; i>=0; i--)
	{
		//Delete all selected items
		if (GetItemState(i, LVIS_SELECTED))
			DeleteItem(i);
	}
	//Force Redraw
	Invalidate();
}


BOOL CDownloadListCtrl::ProcessSaveLoad(CString strFilename, BOOL bSave, BOOL bPromptOverwrite)
{
	BOOL bSuccess(TRUE);
	BOOL bOpened(FALSE);
	
	CFile fileSaveLoad;

	if (bSave) //save
	{
		if (fileSaveLoad.Open(strFilename, CFile::modeCreate|CFile::modeWrite, NULL))
		{
			bOpened = TRUE;
		}
		else
			bSuccess = FALSE;
	}
	else //load
	{
		if (fileSaveLoad.Open(strFilename, CFile::modeRead, NULL))
		{
			bOpened = TRUE;
		}
		else
			bSuccess = FALSE;
	}

	if (bSuccess) //Load Save data
	{
	}

	if (bOpened)
		fileSaveLoad.Close();

	//Force Redraw
	Invalidate();

	return bSuccess;
}

void CDownloadListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
    //CActivity* pActivity = (CActivity*)lpDrawItemStruct->itemData;
    //ASSERT(pActivity);

    CDC* pdc;

	CRect rectListCtrl;
	GetClientRect(&rectListCtrl);
/*
	if (lpDrawItemStruct->rcItem.right < rectListCtrl.right)
	{		
		rectListCtrl.top = lpDrawItemStruct->rcItem.top;
		rectListCtrl.bottom = lpDrawItemStruct->rcItem.bottom;
		rectListCtrl.left = 0;
//		InvalidateRect(rectListCtrl);		
	}
*/
    CRect rc(&lpDrawItemStruct->rcItem);
    CRect rcFocus(rc);
    CString str;
	
	pdc = CDC::FromHandle(lpDrawItemStruct->hDC);

	//Draw in appropriate color if highlighted

	COLORREF clrBackground;
    if (lpDrawItemStruct->itemState & ODS_SELECTED)
    {
        pdc->SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
		clrBackground = GetSysColor(COLOR_HIGHLIGHT);        
    }
    else
    {
        pdc->SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
		clrBackground = GetSysColor(COLOR_WINDOW);        
    }
	pdc->SetBkColor(clrBackground);

	rcFocus.left = 0;
	rcFocus.right = rectListCtrl.right;

	// Account for scroll bar's location to draw properly
	rc.right = -1 - GetScrollPos(SB_HORZ);
	for (int i=0; i<NUMCOLS; i++)
	{
		rc.left = rc.right + 1;
		rc.right = rc.left + GetColumnWidth(i) - 1;
		rcFocus.right = rcFocus.left + GetColumnWidth(i) - 8;		

		//Clear out the background.  This prevents pixel damage
		pdc->FillSolidRect(rcFocus, clrBackground);

		// Figure out how much space we have to write text
		CSize size;
		int iMaxTextWidth = rcFocus.right - rcFocus.left + 1;
		if (i == 0)
			iMaxTextWidth -= rc.Height() + IMAGE_MARGIN * 2;


		// Only draw this if we're in the zone
		if (rcFocus.right > rcFocus.left)
		{			
			str = GetItemText(lpDrawItemStruct->itemID, i);

			// Make text look presentable in space provided
			BOOL bAppended(FALSE);
			do
			{
				size = pdc->GetTextExtent(str, str.GetLength());
				if (size.cx > iMaxTextWidth)
				{
					// Only append "..." the first time
					// Item #1 is right justified, otherwise left
					if (!bAppended)
					{
						if (i == 1)
							str = _T("...") + str;
						else
							str = str + _T("...");
					}
					bAppended = TRUE;

					// Remove characters if we have a character
					// to remove that's not appended
					if (str.GetLength() > 3)
					{
						// Item #1 is right justified, otherwise left
						if (i == 1)
							str = _T("...") + str.Mid(5);
						else
							str = str.Left(str.GetLength()-4) + _T("...");
					}
					else // Drop out if there's no characters to eliminate
						break;
				}
			} while (size.cx > iMaxTextWidth);


			rcFocus.right = rectListCtrl.right;
			pdc->ExtTextOut(rcFocus.left, rcFocus.top, ETO_OPAQUE, 
							rcFocus, "", 0, NULL);		

			//Only draw bitmap on left side
			if (i == 0)
			{
				if (lpDrawItemStruct->itemAction == ODA_DRAWENTIRE)
				{				
					CImageList* pImageList = GetImageList(LVSIL_SMALL);

					rc.left += IMAGE_MARGIN;
					pImageList->Draw(pdc, GetItemStatus(lpDrawItemStruct->itemID), rc.TopLeft(), ILD_NORMAL);

					rc.left += rc.Height() + IMAGE_MARGIN;
				}
			}
			
			pdc->DrawText(str, str.GetLength(), rc, 
						  DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		}

		if (lpDrawItemStruct->itemAction == ODA_SELECT ||
			lpDrawItemStruct->itemAction == ODA_FOCUS)
		{
			return;
		}
		rcFocus.left = rc.right + 1;
	}
	
	rcFocus.right = rectListCtrl.right;
	//  We don't highlight all the way to the right side anymore.
	//pdc->FillSolidRect(rcFocus, clrBackground);
	pdc->FillSolidRect(rcFocus, GetSysColor(COLOR_WINDOW));

	//Draw the focus rect, shall we?
/*	if (lpDrawItemStruct->itemState & ODS_FOCUS)
	{
		rcFocus.left = 0;
		pdc->DrawFocusRect(rcFocus);
	}
*/
}

//This is the key to having the s draw all the way across.
void CDownloadListCtrl::OnPaint() 
{
	// in full row select mode, we need to extend the clipping region
	// so we can paint a selection all the way to the right
	CRect rcAllLabels;
	CRect rcListCtrl;
	GetItemRect(0, rcAllLabels, LVIR_BOUNDS);
	GetWindowRect(&rcListCtrl);

	if(rcAllLabels.right < rcListCtrl.right)
	{
		// need to call BeginPaint (in CPaintDC c-tor)
		// to get correct clipping rect
		CPaintDC dc(this);

		CRect rcClip;
		dc.GetClipBox(rcClip);

		rcClip.left = min(rcAllLabels.right-1, rcClip.left);
		rcClip.right = rcListCtrl.right;

		InvalidateRect(rcClip, FALSE);
		// EndPaint will be called in CPaintDC d-tor
	}

	CListCtrl::OnPaint();
}

BOOL CDownloadListCtrl::DeleteItem(int nItem)
{
	CString* pStrPW = (CString*)GetItemData(nItem);
	if (pStrPW)
	{
		delete pStrPW;
		pStrPW = NULL;
		SetItemData(nItem, NULL);
	}

	return CListCtrl::DeleteItem(nItem);
}

BOOL CDownloadListCtrl::DeleteAllItems()
{
	while (GetItemCount() > 0)
	{
		DeleteItem(0);
	}
	return CListCtrl::DeleteAllItems();
}

void CDownloadListCtrl::OnDestroy() 
{	
	DeleteAllItems();
	CListCtrl::OnDestroy();	
}

void CDownloadListCtrl::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Show the Properties for the selected file 
	// The parent checks if only one, of course
	if (m_pParentDDDialog)
	{
		CDownloadDemonDlg* pDDDialog = (CDownloadDemonDlg*)m_pParentDDDialog;
		pDDDialog->ShowItemProperties();
	}
	
	*pResult = 0;
}
