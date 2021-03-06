/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_TOOLPALETTE_H)
#define INCLUDED_TOOLPALETTE_H

#include "TrackMouseEvent.h"

#include <atlgdi.h>
#include <atlcrack.h>

#include <string>
#include <vector>
#include <map>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class cToolItem;
class cToolGroup;

///////////////////////////////////////////////////////////////////////////////

DECLARE_HANDLE(HTOOLGROUP);
DECLARE_HANDLE(HTOOLITEM);

///////////////////////////////////////////////////////////////////////////////

const int kMaxName = 50;

enum eToolPaletteToolState
{
   kTPTS_None = 0,
   kTPTS_Disabled = (1<<0),
   kTPTS_Checked = (1<<1),
};

struct sToolPaletteItem
{
   char szName[kMaxName];
   uint state;
   int iImage;
   void * pUserData;
};

///////////////////////////////////////////////////////////////////////////////

enum eToolPaletteNotifyCodes
{
   kTPN_ItemClick       = 0xD100,
   kTPN_ItemDestroy     = 0xD101,
   kTPN_ItemCheck       = 0xD102,
   kTPN_ItemUncheck     = 0xD103,
};

///////////////////////////////////////////////////////////////////////////////
//
// STRUCT: sNMToolPaletteItem
//
// Info struct for most item-based notification messages

struct sNMToolPaletteItem
{
   NMHDR hdr;
   POINT pt;
   HTOOLITEM hTool;
   void * pUserData;
};

typedef sNMToolPaletteItem sNMToolPaletteItemClick;
typedef sNMToolPaletteItem sNMToolPaletteItemDestroy;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolItem
//

class cToolItem
{
   cToolItem(const cToolItem &);
   const cToolItem & operator =(const cToolItem &);

public:
   cToolItem(cToolGroup * pGroup, const tChar * pszName, int iImage, void * pUserData);
   ~cToolItem();

   void SetState(uint mask, uint state);
   void ToggleChecked();

   cToolGroup * GetGroup() const;
   const tChar * GetName() const;
   uint GetState() const;
   int GetImageIndex() const;
   void * GetUserData() const;

   bool IsDisabled() const;
   bool IsChecked() const;

private:
   cToolGroup * m_pGroup;
   std::string m_name;
   uint m_state;
   int m_iImage;
   void * m_pUserData;
};

////////////////////////////////////////

inline cToolGroup * cToolItem::GetGroup() const
{
   return m_pGroup;
}

////////////////////////////////////////

inline const tChar * cToolItem::GetName() const
{
   return m_name.c_str();
}

////////////////////////////////////////

inline uint cToolItem::GetState() const
{
   return m_state;
}

////////////////////////////////////////

inline int cToolItem::GetImageIndex() const
{
   return m_iImage;
}

////////////////////////////////////////

inline void * cToolItem::GetUserData() const
{
   return m_pUserData;
}

////////////////////////////////////////

inline bool cToolItem::IsDisabled() const
{
   return (m_state & kTPTS_Disabled) == kTPTS_Disabled;
}

////////////////////////////////////////

inline bool cToolItem::IsChecked() const
{
   return (m_state & kTPTS_Checked) == kTPTS_Checked;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolGroup
//

class cToolGroup
{
   cToolGroup(const cToolGroup &);
   const cToolGroup & operator =(const cToolGroup &);

public:
   cToolGroup(const tChar * pszName, HIMAGELIST hImageList);
   ~cToolGroup();

   const tChar * GetName() const;
   HIMAGELIST GetImageList() const;
   uint GetToolCount() const;
   cToolItem * GetTool(uint index) const;

   bool IsCollapsed() const;
   void ToggleExpandCollapse();

   HTOOLITEM AddTool(const tChar * pszTool, int iImage, void * pUserData);
   bool RemoveTool(HTOOLITEM hTool);
   HTOOLITEM FindTool(const tChar * pszTool);
   bool IsTool(HTOOLITEM hTool) const;
   void Clear();

private:
   std::string m_name;
   HIMAGELIST m_hImageList;

   typedef std::vector<cToolItem *> tTools;
   tTools m_tools;

   bool m_bCollapsed;
};

////////////////////////////////////////

typedef std::vector<cToolGroup *> tGroups;

////////////////////////////////////////

inline const tChar * cToolGroup::GetName() const
{
   return m_name.c_str();
}

////////////////////////////////////////

inline HIMAGELIST cToolGroup::GetImageList() const
{
   return m_hImageList;
}

////////////////////////////////////////

inline uint cToolGroup::GetToolCount() const
{
   return m_tools.size();
}

////////////////////////////////////////

inline cToolItem * cToolGroup::GetTool(uint index) const
{
   if (index < m_tools.size())
   {
      return m_tools[index];
   }
   return NULL;
}

////////////////////////////////////////

inline bool cToolGroup::IsCollapsed() const
{
   return m_bCollapsed;
}

////////////////////////////////////////

inline void cToolGroup::ToggleExpandCollapse()
{
   m_bCollapsed = !m_bCollapsed;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolPaletteRenderer
//
// Handles painting for cToolPaletteImpl. Called in a loop for each group to be
// rendered.

class cToolPaletteRenderer
{
   cToolPaletteRenderer(const cToolPaletteRenderer &);
   void operator =(const cToolPaletteRenderer &);

public:
   cToolPaletteRenderer();
   ~cToolPaletteRenderer();

   void Render(WTL::CDCHandle dc, const CRect & itemRect, const cToolGroup * pToolGroup) const;
   void Render(WTL::CDCHandle dc, const CRect & itemRect, const cToolItem * pToolItem, bool bMouseOver) const;

private:
   mutable HBRUSH m_hCheckedItemBrush;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolPaletteImpl
//

template <class T, class TRenderer>
class ATL_NO_VTABLE cToolPaletteImpl : public ATL::CWindowImpl<T>,
                                       public cTrackMouseEvent<T>
{
   typedef ATL::CWindowImpl<T> tBase;

public:
	DECLARE_WND_SUPERCLASS(NULL, tBase::GetWndClassName())

   cToolPaletteImpl();
   ~cToolPaletteImpl();

   HTOOLGROUP AddGroup(const tChar * pszGroup, HIMAGELIST hImageList);
   bool RemoveGroup(HTOOLGROUP hGroup);
   uint GetGroupCount() const;
   HTOOLGROUP FindGroup(const tChar * pszGroup);
   bool IsGroup(HTOOLGROUP hGroup);
   bool IsTool(HTOOLITEM hTool);
   void Clear();
   HTOOLITEM AddTool(HTOOLGROUP hGroup, const tChar * pszTool, int iImage, void * pUserData = NULL);
   HTOOLITEM AddTool(HTOOLGROUP hGroup, const sToolPaletteItem * pTPI);
   bool GetToolText(HTOOLITEM hTool, std::string * pText);
   bool GetTool(HTOOLITEM hTool, sToolPaletteItem * pTPI);
   bool RemoveTool(HTOOLITEM hTool);
   bool EnableTool(HTOOLITEM hTool, bool bEnable);

protected:
   BEGIN_MSG_MAP_EX(cToolPaletteImpl)
      CHAIN_MSG_MAP(cTrackMouseEvent<T>)
      MSG_WM_DESTROY(OnDestroy)
      MSG_WM_SIZE(OnSize)
      MSG_WM_PAINT(OnPaint)
      MSG_WM_SETFONT(OnSetFont)
      MSG_WM_ERASEBKGND(OnEraseBkgnd)
      MSG_WM_CANCELMODE(OnCancelMode)
      MSG_WM_MOUSELEAVE(OnMouseLeave)
      MSG_WM_MOUSEMOVE(OnMouseMove)
      MSG_WM_LBUTTONDOWN(OnLButtonDown)
      MSG_WM_LBUTTONUP(OnLButtonUp)
   END_MSG_MAP()

   void DoPaint(WTL::CDCHandle dc);

   void OnDestroy();
   void OnSize(UINT nType, CSize size);
   void OnPaint(HDC hDC);
   void OnSetFont(HFONT hFont, BOOL bRedraw);
   LRESULT OnEraseBkgnd(WTL::CDCHandle dc);
   void OnCancelMode();
   void OnMouseLeave();
   void OnMouseMove(UINT flags, CPoint point);
   void OnLButtonDown(UINT flags, CPoint point);
   void OnLButtonUp(UINT flags, CPoint point);

private:
   void GetCheckedItems(std::vector<HTOOLITEM> * pCheckedItems);

   HANDLE GetHitItem(const CPoint & point) const;

   void SetMouseOverItem(HANDLE hItem);

   void DoClick(HANDLE hItem, CPoint point);
   void DoGroupClick(HTOOLGROUP hGroup, CPoint point);
   void DoItemClick(HTOOLITEM hTool, CPoint point);
   LRESULT DoNotify(cToolItem * pTool, int code, CPoint point = CPoint());

   bool ExclusiveCheck() const
   {
      return m_bExclusiveCheck;
   }

   ////////////////////////////////////

   enum { kTextGap = 2 };

   bool m_bExclusiveCheck;

   tGroups m_groups;

   TRenderer m_renderer;

   typedef std::map<const void *, CRect> tCachedRects;
   tCachedRects m_cachedRects;

   HFONT m_hFont;

   HANDLE m_hMouseOverItem;
   HANDLE m_hClickCandidateItem;
};

////////////////////////////////////////

template <class T, class TRenderer>
cToolPaletteImpl<T, TRenderer>::cToolPaletteImpl()
 : m_bExclusiveCheck(true)
 , m_hFont(NULL)
 , m_hMouseOverItem(NULL)
 , m_hClickCandidateItem(NULL)
{
}

////////////////////////////////////////

template <class T, class TRenderer>
cToolPaletteImpl<T, TRenderer>::~cToolPaletteImpl()
{
   Assert(m_hFont == NULL);
}

////////////////////////////////////////

template <class T, class TRenderer>
HTOOLGROUP cToolPaletteImpl<T, TRenderer>::AddGroup(const tChar * pszGroup, HIMAGELIST hImageList)
{
   if (pszGroup != NULL)
   {
      HTOOLGROUP hGroup = FindGroup(pszGroup);
      if (hGroup != NULL)
      {
         return hGroup;
      }

      cToolGroup * pGroup = new cToolGroup(pszGroup, hImageList);
      if (pGroup != NULL)
      {
         m_groups.push_back(pGroup);
         return reinterpret_cast<HTOOLGROUP>(pGroup);
      }
   }

   return NULL;
}

////////////////////////////////////////

template <class T, class TRenderer>
bool cToolPaletteImpl<T, TRenderer>::RemoveGroup(HTOOLGROUP hGroup)
{
   if (hGroup != NULL)
   {
      cToolGroup * pRmGroup = reinterpret_cast<cToolGroup *>(hGroup);
      tGroups::iterator iter = m_groups.begin();
      tGroups::iterator end = m_groups.end();
      for (; iter != end; iter++)
      {
         if (*iter == pRmGroup)
         {
            uint nTools = pRmGroup->GetToolCount();
            for (uint i = 0; i < nTools; i++)
            {
               DoNotify(pRmGroup->GetTool(i), kTPN_ItemDestroy);
            }
            delete pRmGroup;
            m_groups.erase(iter);
            m_cachedRects.clear();
            return true;
         }
      }
   }
   return false;
}

////////////////////////////////////////

template <class T, class TRenderer>
uint cToolPaletteImpl<T, TRenderer>::GetGroupCount() const
{
   return m_groups.size();
}

////////////////////////////////////////

template <class T, class TRenderer>
HTOOLGROUP cToolPaletteImpl<T, TRenderer>::FindGroup(const tChar * pszGroup)
{
   if (pszGroup != NULL)
   {
      tGroups::iterator iter = m_groups.begin();
      tGroups::iterator end = m_groups.end();
      for (; iter != end; iter++)
      {
         if (lstrcmp((*iter)->GetName(), pszGroup) == 0)
         {
            return reinterpret_cast<HTOOLGROUP>(*iter);
         }
      }
   }
   return NULL;
}

////////////////////////////////////////

template <class T, class TRenderer>
bool cToolPaletteImpl<T, TRenderer>::IsGroup(HTOOLGROUP hGroup)
{
   if (hGroup != NULL)
   {
      cToolGroup * pGroup = reinterpret_cast<cToolGroup *>(hGroup);
      tGroups::iterator iter = m_groups.begin();
      tGroups::iterator end = m_groups.end();
      for (; iter != end; iter++)
      {
         if (*iter == pGroup)
         {
            return true;
         }
      }
   }
   return false;
}

////////////////////////////////////////

template <class T, class TRenderer>
bool cToolPaletteImpl<T, TRenderer>::IsTool(HTOOLITEM hTool)
{
   if (hTool != NULL)
   {
      tGroups::iterator iter = m_groups.begin();
      tGroups::iterator end = m_groups.end();
      for (; iter != end; iter++)
      {
         if ((*iter)->IsTool(hTool))
         {
            return true;
         }
      }
   }
   return false;
}

////////////////////////////////////////

template <class T, class TRenderer>
void cToolPaletteImpl<T, TRenderer>::Clear()
{
   // Call RemoveGroup() for each so that all the item destroy notifications happen
   while (!m_groups.empty())
   {
      RemoveGroup(reinterpret_cast<HTOOLGROUP>(m_groups.front()));
   }

   // Still important to let the STL container do any internal cleanup
   m_groups.clear();
}

////////////////////////////////////////

template <class T, class TRenderer>
HTOOLITEM cToolPaletteImpl<T, TRenderer>::AddTool(HTOOLGROUP hGroup, const tChar * pszTool, int iImage, void * pUserData)
{
   sToolPaletteItem tpi = {0};
   lstrcpyn(tpi.szName, pszTool, _countof(tpi.szName));
   tpi.iImage = iImage;
   tpi.pUserData = pUserData;
   return AddTool(hGroup, &tpi);
}

////////////////////////////////////////

template <class T, class TRenderer>
HTOOLITEM cToolPaletteImpl<T, TRenderer>::AddTool(HTOOLGROUP hGroup, const sToolPaletteItem * pTPI)
{
   if (IsGroup(hGroup) && (pTPI != NULL))
   {
      cToolGroup * pGroup = reinterpret_cast<cToolGroup *>(hGroup);
      HTOOLITEM hItem = pGroup->AddTool(pTPI->szName, pTPI->iImage, pTPI->pUserData);
      if (hItem != NULL)
      {
         Invalidate();
         return hItem;
      }
   }

   return NULL;
}

////////////////////////////////////////

template <class T, class TRenderer>
bool cToolPaletteImpl<T, TRenderer>::GetToolText(HTOOLITEM hTool, std::string * pText)
{
   if (IsTool(hTool))
   {
      cToolItem * pTool = reinterpret_cast<cToolItem *>(hTool);
      Assert(IsGroup(reinterpret_cast<HTOOLGROUP>(pTool->GetGroup())));

      if (pText != NULL)
      {
         *pText = pTool->GetName();
      }

      return true;
   }

   return false;
}

////////////////////////////////////////

template <class T, class TRenderer>
bool cToolPaletteImpl<T, TRenderer>::GetTool(HTOOLITEM hTool, sToolPaletteItem * pTPI)
{
   if (IsTool(hTool))
   {
      cToolItem * pTool = reinterpret_cast<cToolItem *>(hTool);
      Assert(IsGroup(reinterpret_cast<HTOOLGROUP>(pTool->GetGroup())));

      if (pTPI != NULL)
      {
         lstrcpyn(pTPI->szName, pTool->GetName(), _countof(pTPI->szName));
         pTPI->iImage = pTool->GetImageIndex();
         pTPI->state = 0;
         pTPI->pUserData = pTool->GetUserData();
         return true;
      }
   }

   return false;
}

////////////////////////////////////////

template <class T, class TRenderer>
bool cToolPaletteImpl<T, TRenderer>::RemoveTool(HTOOLITEM hTool)
{
   // TODO
   WarnMsg("Unsupported function called: cToolPalette::RemoveTool\n");
   return false;
}

////////////////////////////////////////

template <class T, class TRenderer>
bool cToolPaletteImpl<T, TRenderer>::EnableTool(HTOOLITEM hTool, bool bEnable)
{
   if (IsTool(hTool))
   {
      cToolItem * pTool = reinterpret_cast<cToolItem *>(hTool);
      Assert(IsGroup(reinterpret_cast<HTOOLGROUP>(pTool->GetGroup())));

      pTool->SetState(kTPTS_Disabled, bEnable ? 0 : kTPTS_Disabled);

      return true;
   }

   return false;
}

////////////////////////////////////////

template <class T, class TRenderer>
void cToolPaletteImpl<T, TRenderer>::DoPaint(WTL::CDCHandle dc)
{
   HFONT hOldFont = dc.SelectFont((m_hFont != NULL) ? m_hFont : WTL::AtlGetDefaultGuiFont());

   const POINT * pMousePos = NULL;
   POINT mousePos;
   if (GetCursorPos(&mousePos) && ::MapWindowPoints(NULL, m_hWnd, &mousePos, 1))
   {
      pMousePos = &mousePos;
   }

   TEXTMETRIC tm = {0};
   Verify(dc.GetTextMetrics(&tm));

   int itemHeight = tm.tmHeight + tm.tmExternalLeading + (2 * kTextGap);

   CRect itemRect;
   Verify(GetClientRect(&itemRect));
   itemRect.bottom = itemRect.top + itemHeight;

   tGroups::const_iterator iter = m_groups.begin();
   for (; iter != m_groups.end(); iter++)
   {
      const cToolGroup * pToolGroup = (*iter);
      Assert(pToolGroup != NULL);

      if (lstrlen(pToolGroup->GetName()) > 0)
      {
         m_renderer.Render(dc, itemRect, pToolGroup);
         m_cachedRects[pToolGroup] = itemRect;
         itemRect.OffsetRect(0, itemHeight);
      }

      if (!pToolGroup->IsCollapsed())
      {
         uint nTools = pToolGroup->GetToolCount();
         for (uint i = 0; i < nTools; i++)
         {
            const cToolItem * pToolItem = pToolGroup->GetTool(i);
            Assert(pToolItem != NULL);

            m_renderer.Render(dc, itemRect, pToolItem,
               pMousePos != NULL && itemRect.PtInRect(*pMousePos));
            m_cachedRects[pToolItem] = itemRect;
            itemRect.OffsetRect(0, itemHeight);
         }
      }
   }

   dc.FillSolidRect(itemRect.left, itemRect.top, itemRect.right, 9999, GetSysColor(COLOR_3DFACE));

   dc.SelectFont(hOldFont);
}

////////////////////////////////////////

template <class T, class TRenderer>
void cToolPaletteImpl<T, TRenderer>::OnDestroy()
{
   Clear();
   SetFont(NULL, FALSE);
   m_cachedRects.clear();
}

////////////////////////////////////////

template <class T, class TRenderer>
void cToolPaletteImpl<T, TRenderer>::OnSize(UINT nType, CSize size)
{
   m_cachedRects.clear();
}

////////////////////////////////////////

template <class T, class TRenderer>
void cToolPaletteImpl<T, TRenderer>::OnPaint(HDC hDC)
{
   if (hDC != NULL)
   {
      DoPaint(hDC);
   }
   else
   {
      PAINTSTRUCT ps = {0};
      hDC = ::BeginPaint(m_hWnd, &ps);
      DoPaint(hDC);
      ::EndPaint(m_hWnd, &ps);
   }
}

////////////////////////////////////////

template <class T, class TRenderer>
void cToolPaletteImpl<T, TRenderer>::OnSetFont(HFONT hFont, BOOL bRedraw)
{
   if (m_hFont != NULL)
   {
      Verify(DeleteObject(m_hFont));
      m_hFont = NULL;
   }

   if (hFont != NULL)
   {
      LOGFONT logFont = {0};
      if (GetObject(hFont, sizeof(LOGFONT), &logFont))
      {
         m_cachedRects.clear();
         m_hFont = CreateFontIndirect(&logFont);
      }
   }

   if (bRedraw)
   {
      Invalidate();
      UpdateWindow();
   }
}

////////////////////////////////////////

template <class T, class TRenderer>
LRESULT cToolPaletteImpl<T, TRenderer>::OnEraseBkgnd(WTL::CDCHandle dc)
{
   return TRUE;
}

////////////////////////////////////////

template <class T, class TRenderer>
void cToolPaletteImpl<T, TRenderer>::OnCancelMode()
{
   m_hClickCandidateItem = NULL;
}

////////////////////////////////////////

template <class T, class TRenderer>
void cToolPaletteImpl<T, TRenderer>::OnMouseLeave()
{
   SetMouseOverItem(NULL);
}

////////////////////////////////////////

template <class T, class TRenderer>
void cToolPaletteImpl<T, TRenderer>::OnMouseMove(UINT flags, CPoint point)
{
   SetMouseOverItem(GetHitItem(point));
}

////////////////////////////////////////

template <class T, class TRenderer>
void cToolPaletteImpl<T, TRenderer>::OnLButtonDown(UINT flags, CPoint point)
{
   Assert(m_hClickCandidateItem == NULL);
   m_hClickCandidateItem = GetHitItem(point);
   if (m_hClickCandidateItem != NULL)
   {
      SetCapture();
   }
}

////////////////////////////////////////

template <class T, class TRenderer>
void cToolPaletteImpl<T, TRenderer>::OnLButtonUp(UINT flags, CPoint point)
{
   if (m_hClickCandidateItem != NULL)
   {
      ReleaseCapture();
      if (m_hClickCandidateItem == GetHitItem(point))
      {
         DoClick(m_hClickCandidateItem, point);
      }
      m_hClickCandidateItem = NULL;
   }
}

////////////////////////////////////////

template <class T, class TRenderer>
void cToolPaletteImpl<T, TRenderer>::GetCheckedItems(std::vector<HTOOLITEM> * pCheckedItems)
{
   if (pCheckedItems != NULL)
   {
      pCheckedItems->clear();

      tGroups::const_iterator iter = m_groups.begin();
      for (; iter != m_groups.end(); iter++)
      {
         uint nTools = (*iter)->GetToolCount();
         for (uint i = 0; i < nTools; i++)
         {
            cToolItem * pTool = (*iter)->GetTool(i);
            if (pTool->IsChecked())
            {
               pCheckedItems->push_back(reinterpret_cast<HTOOLITEM>(pTool));
            }
         }
      }
   }
}

////////////////////////////////////////

template <class T, class TRenderer>
HANDLE cToolPaletteImpl<T, TRenderer>::GetHitItem(const CPoint & point) const
{
   int itemHeight = 0;

   {
      HDC hDC = ::GetDC(m_hWnd);
      if (hDC != NULL)
      {
         HGDIOBJ hOldFont = SelectObject(hDC, (m_hFont != NULL) ? m_hFont : WTL::AtlGetDefaultGuiFont());

         TEXTMETRIC tm = {0};
         Verify(GetTextMetrics(hDC, &tm));

         SelectObject(hDC, hOldFont);

         ::ReleaseDC(m_hWnd, hDC);

         itemHeight = tm.tmHeight + tm.tmExternalLeading + (2 * kTextGap);
      }
   }

   if (itemHeight == 0)
   {
      return NULL;
   }

   CRect itemRect;
   Verify(GetClientRect(&itemRect));
   itemRect.bottom = itemRect.top + itemHeight;

   HANDLE hHitItem = NULL;

   tGroups::const_iterator iter = m_groups.begin();
   for (; (iter != m_groups.end()) && (hHitItem == NULL); iter++)
   {
      const cToolGroup * pToolGroup = (*iter);
      Assert(pToolGroup != NULL);

      if (lstrlen(pToolGroup->GetName()) > 0)
      {
         if (itemRect.PtInRect(point))
         {
            hHitItem = (HANDLE)pToolGroup;
            break;
         }

         itemRect.OffsetRect(0, itemHeight);
      }

      if (!pToolGroup->IsCollapsed())
      {
         uint nTools = pToolGroup->GetToolCount();
         for (uint i = 0; i < nTools; i++)
         {
            const cToolItem * pToolItem = pToolGroup->GetTool(i);
            Assert(pToolItem != NULL);

            if (itemRect.PtInRect(point))
            {
               hHitItem = (HANDLE)pToolItem;
               break;
            }

            itemRect.OffsetRect(0, itemHeight);
         }
      }
   }

   return hHitItem;
}

////////////////////////////////////////

template <class T, class TRenderer>
void cToolPaletteImpl<T, TRenderer>::SetMouseOverItem(HANDLE hItem)
{
   if (hItem != m_hMouseOverItem)
   {
      // remove this if-statement if group headings ever have a mouse-over effect
      if (!IsGroup(reinterpret_cast<HTOOLGROUP>(m_hMouseOverItem)))
      {
         std::map<const void *, CRect>::iterator f = m_cachedRects.find(m_hMouseOverItem);
         if (f != m_cachedRects.end())
         {
            InvalidateRect(f->second, TRUE);
         }
      }

      m_hMouseOverItem = hItem;

      // remove this if-statement if group headings ever have a mouse-over effect
      if (!IsGroup(reinterpret_cast<HTOOLGROUP>(hItem)))
      {
         std::map<const void *, CRect>::iterator f = m_cachedRects.find(hItem);
         if (f != m_cachedRects.end())
         {
            InvalidateRect(f->second, FALSE);
         }
      }
   }
}

////////////////////////////////////////

template <class T, class TRenderer>
void cToolPaletteImpl<T, TRenderer>::DoClick(HANDLE hItem, CPoint point)
{
   if (hItem != NULL)
   {
      if (IsGroup(reinterpret_cast<HTOOLGROUP>(hItem)))
      {
         DoGroupClick(reinterpret_cast<HTOOLGROUP>(hItem), point);
      }
      else
      {
         DoItemClick(reinterpret_cast<HTOOLITEM>(hItem), point);
      }
   }
}

////////////////////////////////////////

template <class T, class TRenderer>
void cToolPaletteImpl<T, TRenderer>::DoGroupClick(HTOOLGROUP hGroup, CPoint point)
{
   cToolGroup * pGroup = reinterpret_cast<cToolGroup *>(hGroup);
   pGroup->ToggleExpandCollapse();

   tCachedRects::iterator f = m_cachedRects.find(hGroup);
   if (f != m_cachedRects.end())
   {
      RECT invalidRect;
      GetClientRect(&invalidRect);
      invalidRect.top = f->second.top;
      InvalidateRect(&invalidRect);
   }
   else
   {
      Invalidate();
   }

   m_cachedRects.clear();
}

////////////////////////////////////////

template <class T, class TRenderer>
void cToolPaletteImpl<T, TRenderer>::DoItemClick(HTOOLITEM hTool, CPoint point)
{
   cToolItem * pTool = reinterpret_cast<cToolItem *>(hTool);
   Assert(IsGroup(reinterpret_cast<HTOOLGROUP>(pTool->GetGroup())));
   if (!pTool->IsDisabled())
   {
      DoNotify(pTool, kTPN_ItemClick, point);
      if (ExclusiveCheck())
      {
         std::vector<HTOOLITEM> checkedItems;
         GetCheckedItems(&checkedItems);
         Assert(checkedItems.size() <= 1);
         if (!checkedItems.empty())
         {
            if (checkedItems[0] != hTool)
            {
               cToolItem * pUncheckTool = reinterpret_cast<cToolItem *>(checkedItems[0]);

               DoNotify(pUncheckTool, kTPN_ItemUncheck);
               pUncheckTool->SetState(kTPTS_Checked, 0);

               std::map<const void *, CRect>::const_iterator f = m_cachedRects.find(checkedItems[0]);
               if (f != m_cachedRects.end())
               {
                  InvalidateRect(f->second);
               }
               else
               {
                  Invalidate();
               }
            }
         }

         if (!pTool->IsChecked())
         {
            DoNotify(pTool, kTPN_ItemCheck);
            pTool->SetState(kTPTS_Checked, kTPTS_Checked);
         }
      }
      else
      {
         DoNotify(pTool, pTool->IsChecked()? kTPN_ItemUncheck : kTPN_ItemCheck);
         pTool->ToggleChecked();
      }

      std::map<const void *, CRect>::const_iterator f = m_cachedRects.find(hTool);
      if (f != m_cachedRects.end())
      {
         InvalidateRect(f->second);
      }
      else
      {
         Invalidate();
      }
   }
}

////////////////////////////////////////

template <class T, class TRenderer>
LRESULT cToolPaletteImpl<T, TRenderer>::DoNotify(cToolItem * pTool, int code, CPoint point)
{
   Assert(code == kTPN_ItemClick || code == kTPN_ItemDestroy
      || code == kTPN_ItemCheck || code == kTPN_ItemUncheck);
   if (pTool != NULL)
   {
      HWND hWndParent = GetParent();
      if (::IsWindow(hWndParent))
      {
         sNMToolPaletteItem nm = {0};
         nm.hdr.hwndFrom = m_hWnd;
         nm.hdr.code = code;
         nm.hdr.idFrom = GetDlgCtrlID();
         nm.pt = point;
         nm.hTool = reinterpret_cast<HTOOLITEM>(pTool);
         nm.pUserData = pTool->GetUserData();
         return ::SendMessage(hWndParent, WM_NOTIFY, nm.hdr.idFrom, reinterpret_cast<LPARAM>(&nm));
      }
   }
   return -1;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolPalette
//

class cToolPalette : public cToolPaletteImpl<cToolPalette, cToolPaletteRenderer>
{
public:
	DECLARE_WND_SUPERCLASS(_T("ToolPalette"), GetWndClassName())
};


/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_TOOLPALETTE_H)
