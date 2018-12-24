// CDownloadDemonDropTarget.h : header file
//

#if !defined(CDownloadDemonDropTargetHeader)
#define CDownloadDemonDropTargetHeader

#include "RetrieveItem.h"
#include "DownloadListCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CDownloadDemonDropTarget

class CDownloadDemonDropTarget : public COleDropTarget
{
public:
	CDownloadDemonDropTarget(CLIPFORMAT cfObjectDescriptor);
	virtual BOOL Register(CDownloadListCtrl* pWnd);
	void AllowAdditional(BOOL bAllowAdditional);

protected:
	BOOL m_bInDrag;
	BOOL m_bAllowDrop;
	CPoint m_dragPoint;
	CSize m_dragSize;
	CSize m_dragOffset;
	DROPEFFECT m_prevDropEffect;
	CLIPFORMAT m_cfObjectDescriptor;


	BOOL ObjectIsLink(COleDataObject* pDataObject, CRetrieveItem& cRetrieveItem);

	// drag drop implementation
	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject,
								   DWORD grfKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject,
								  DWORD grfKeyState, CPoint point);
	virtual void OnDragLeave(CWnd* pWnd);	
	virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
						DROPEFFECT dropEffect, CPoint point);	

private:
	CDownloadListCtrl* m_pDownloadListCtrl;
	BOOL m_bAllowAdditional;
};

#endif // !defined(CDownloadDemonDropTargetHeader)