/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"

#include "ms3dTreeView.h"
#include "ms3dviewDoc.h"

#include "material.h"
#include "color.h"

#include "resource.h"       // main symbols

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// cMs3dTreeView

IMPLEMENT_DYNCREATE(cMs3dTreeView, CTreeView)

cMs3dTreeView::cMs3dTreeView()
{
}

cMs3dTreeView::~cMs3dTreeView()
{
}


BEGIN_MESSAGE_MAP(cMs3dTreeView, CTreeView)
	//{{AFX_MSG_MAP(cMs3dTreeView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cMs3dTreeView drawing

void cMs3dTreeView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// cMs3dTreeView diagnostics

#ifdef _DEBUG
void cMs3dTreeView::AssertValid() const
{
	CTreeView::AssertValid();
}

void cMs3dTreeView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}

CMs3dviewDoc* cMs3dTreeView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMs3dviewDoc)));
	return (CMs3dviewDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// cMs3dTreeView message handlers

void cMs3dTreeView::OnInitialUpdate() 
{
	CTreeView::OnInitialUpdate();
	
	// TODO: Add your specialized code here and/or call the base class

	CMs3dviewDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);

   VERIFY(GetTreeCtrl().DeleteAllItems());

   if (pDoc && pDoc->GetModel())
   {
      int nMaterials = pDoc->GetModel()->GetMaterialCount();
      if (nMaterials > 0)
      {
         HTREEITEM hMaterials = GetTreeCtrl().InsertItem("Materials");
         if (hMaterials != NULL)
         {
            for (int i = 0; i < nMaterials; i++)
            {
               IMaterial * pMaterial = pDoc->GetModel()->AccessMaterial(i);
               if (pMaterial != NULL)
               {
                  HTREEITEM hMaterial = GetTreeCtrl().InsertItem(pMaterial->GetName(), hMaterials);
                  if (hMaterial != NULL)
                  {
                     float s;
                     CString str;
                     cColor color;
                     if (pMaterial->GetAmbient(&color) == S_OK)
                     {
                        str.Format("Ambient: %.2f, %.2f, %.2f, %.2f",
                           color.GetPointer()[0], color.GetPointer()[1], color.GetPointer()[2], color.GetPointer()[3]);
                        GetTreeCtrl().InsertItem(str, hMaterial);
                     }
                     if (pMaterial->GetDiffuse(&color) == S_OK)
                     {
                        str.Format("Diffuse: %.2f, %.2f, %.2f, %.2f",
                           color.GetPointer()[0], color.GetPointer()[1], color.GetPointer()[2], color.GetPointer()[3]);
                        GetTreeCtrl().InsertItem(str, hMaterial);
                     }
                     if (pMaterial->GetSpecular(&color) == S_OK)
                     {
                        str.Format("Specular: %.2f, %.2f, %.2f, %.2f",
                           color.GetPointer()[0], color.GetPointer()[1], color.GetPointer()[2], color.GetPointer()[3]);
                        GetTreeCtrl().InsertItem(str, hMaterial);
                     }
                     if (pMaterial->GetEmissive(&color) == S_OK)
                     {
                        str.Format("Emissive: %.2f, %.2f, %.2f, %.2f",
                           color.GetPointer()[0], color.GetPointer()[1], color.GetPointer()[2], color.GetPointer()[3]);
                        GetTreeCtrl().InsertItem(str, hMaterial);
                     }
                     if (pMaterial->GetShininess(&s) == S_OK)
                     {
                        str.Format("Shininess: %.2f", s);
                        GetTreeCtrl().InsertItem(str, hMaterial);
                     }
                  }
               }
            }
         }
      }

      int nGroups = pDoc->GetModel()->GetGroupCount();
      if (nGroups > 0)
      {
         HTREEITEM hGroups = GetTreeCtrl().InsertItem("Groups");
         if (hGroups != NULL)
         {
            for (int i = 0; i < nGroups; i++)
            {
               const cMs3dGroup & g = pDoc->GetModel()->GetGroup(i);
               HTREEITEM hGroup = GetTreeCtrl().InsertItem(g.GetName(), hGroups);
               if (hGroup != NULL)
               {
                  CString str;
                  str.Format("Material index: %d", g.GetMaterialIndex());
                  GetTreeCtrl().InsertItem(str, hGroup);
                  str.Format("Triangle count: %d", g.GetTriangleIndices().size());
                  GetTreeCtrl().InsertItem(str, hGroup);
               }
            }
         }
      }
   }
}

void cMs3dTreeView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	
}

BOOL cMs3dTreeView::PreCreateWindow(CREATESTRUCT& cs) 
{
   cs.style |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT;
	
	return CTreeView::PreCreateWindow(cs);
}
