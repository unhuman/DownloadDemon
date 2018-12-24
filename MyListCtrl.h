#ifndef DOWNLOADLISTCTRL_H
#define DOWNLOADLISTCTRL_H

#include "RetrieveItem.h"

/////////////////////////////////////////////////////////////////////////////
// CMyListCtrl window
class CMyListCtrl : public CListCtrl
{
// Construction
public:
	CMyListCtrl()
	{
		m_nHighlight = HIGHLIGHT_ROW;
		m_pParentDDDialog = NULL;
	}

// Attributes
public:
	enum EHighlight {HIGHLIGHT_NORMAL, HIGHLIGHT_ALLCOLUMNS, HIGHLIGHT_ROW};

// Operations
public:
	BOOL ProcessSaveLoad(CString strFilename, BOOL bSave, BOOL bPromptOverwrite);
	enum Status {WAIT, CONNECT, TRANSFER, FINISHED, STOPPED, 
				 FAILED, TIMEDOUT, LOOKING, NOTFOUND,
				 FILEOPENERR, FILEWRITEERR, SKIPPED};
	void DeleteAllItemsWithStatus(Status statDelete);
	void DeleteSelectedItems();
	int /*Status*/ GetItemStatus(int iItem);
	void AddColumnHeadings(BOOL bUseExistingConfig);
	void SaveColHeadingSizes();
	BOOL AddItem(CString strLink, int iMode, CString strStrip, BOOL bInformErr,
				 CString strDestDir, CString strUserName, CString strPassword);
	BOOL RetrieveItem(int iItemRet, int& /*Status&*/ iImage, 
					  CString& strFullPath, int& iMode,
					  CString& strStrip, CString& strDestDir,
					  CString& strUser, CString& strPW);
	BOOL UpdateStatus(int iItemUpdate, int /*Status*/ iImage, CString strStatus);
	BOOL UpdateItem(int iUpdateItem, CString strFullPath, int iMode,
					CString strStrip, CString strDestDir, 
					CString strUser, CString strPW);
	void SetParent(void* pParent)
	{
		m_pParentDDDialog = pParent;
	}

	int GetCountForStatus(int /*Status*/ iStatus);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMyListCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMyListCtrl()
	{
	}
	BOOL DeleteItem(int nItem);
	BOOL DeleteAllItems();	

	// Generated message map functions
protected:
	CImageList m_cImageList;
	int  m_nHighlight;		// Indicate type of selection highlighting

	//{{AFX_MSG(CMyListCtrl)
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );	

	DECLARE_MESSAGE_MAP()
private:
	void* m_pParentDDDialog; // This will map to Download Demon Dialog
	int GetRegistryColSize(BOOL bGet, int iCol, int iDefault = 0);	
};

/////////////////////////////////////////////////////////////////////////////
#endif //ndef DOWNLOADLISTCTRL_H
