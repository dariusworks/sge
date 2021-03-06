/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_EDITORVIEW_H)
#define INCLUDED_EDITORVIEW_H

#include "editorapi.h"
#include "afxcomtools.h"

#include "tech/schedulerapi.h"

#include "tech/matrix4.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class cEditorDoc;

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEditorView
//

class cEditorView : public CScrollView,
                    public cComObject4<IMPLEMENTS(IEditorView),
                                       IMPLEMENTS(IEditorAppListener),
                                       IMPLEMENTS(IEditorToolStateListener),
                                       IMPLEMENTS(ITask),
                                       cAfxComServices<cEditorView> >
{
protected: // create from serialization only
	cEditorView();
	DECLARE_DYNCREATE(cEditorView)

// Attributes
public:
	cEditorDoc * GetDocument();

// Operations
public:
   // IEditorView

   // IEditorAppListener
   virtual tResult OnDefaultTileSetChange(const tChar * pszTileSet);

   // IEditorToolStateListener
   virtual tResult OnActiveToolChange(IEditorTool * pNewTool, IEditorTool * pFormerTool);

   // ITask
   virtual tResult Execute(double time);

   void RenderGL();
   void RenderD3D();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(cEditorView)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual void OnInitialUpdate();
   virtual void OnFinalRelease();
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
   virtual void PostNcDestroy();
	//}}AFX_VIRTUAL
   virtual void PreSubclassWindow();
   virtual int OnToolHitTest(CPoint point, TOOLINFO* pToolInfo) const;

// Implementation
public:
	virtual ~cEditorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(cEditorView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	afx_msg void OnToolsCameraSettings();
   afx_msg void OnToolTipShow(UINT id, NMHDR * pNMHDR, LRESULT * pResult);
   afx_msg void OnToolTipPop(UINT id, NMHDR * pNMHDR, LRESULT * pResult);

	DECLARE_MESSAGE_MAP()

private:
   float m_cameraFov, m_cameraZNear, m_cameraZFar;

   bool m_bInPostNcDestroy;

   CWnd m_toolTipGuard;
};

#ifndef _DEBUG  // debug version in editorView.cpp
inline cEditorDoc* cEditorView::GetDocument()
   { return (cEditorDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(INCLUDED_EDITORVIEW_H)
