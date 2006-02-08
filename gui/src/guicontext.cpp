///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guicontext.h"
#include "guievent.h"
#include "guielementenum.h"
#include "guipage.h"
#include "guistyleapi.h"

#include "sys.h"
#include "engineapi.h"

#include "keys.h"
#include "multivar.h"
#include "resourceapi.h"
#include "configapi.h"

#include "guieventroutertem.h"

#include <tinyxml.h>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(GUIContext);

#define LocalMsg(msg)                  DebugMsgEx(GUIContext,(msg))
#define LocalMsg1(msg,a1)              DebugMsgEx1(GUIContext,(msg),(a1))
#define LocalMsg2(msg,a1,a2)           DebugMsgEx2(GUIContext,(msg),(a1),(a2))

#define LocalMsgIf(cond,msg)           DebugMsgIfEx(GUIContext,(cond),(msg))
#define LocalMsgIf1(cond,msg,a1)       DebugMsgIfEx1(GUIContext,(cond),(msg),(a1))
#define LocalMsgIf2(cond,msg,a1,a2)    DebugMsgIfEx2(GUIContext,(cond),(msg),(a1),(a2))

///////////////////////////////////////////////////////////////////////////////

inline bool IsNumber(const tScriptVar & scriptVar)
{
   return scriptVar.IsInt() || scriptVar.IsFloat() || scriptVar.IsDouble();
}

///////////////////////////////////////////////////////////////////////////////

static bool g_bExitModalLoop = false;
tSysFrameFn g_pfnOuterFrameHandler = NULL;

static tResult GUIModalLoopFrameHandler()
{
   if (g_bExitModalLoop)
   {
      g_pfnOuterFrameHandler = NULL;
      return S_FALSE;
   }
   return (g_pfnOuterFrameHandler != NULL) ? (*g_pfnOuterFrameHandler)() : S_OK;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIModalLoopEventListener
//

////////////////////////////////////////

class cGUIModalLoopEventListener : public cComObject<IMPLEMENTS(IGUIEventListener)>
{
public:
   cGUIModalLoopEventListener(tResult * pResult);
   ~cGUIModalLoopEventListener();

   virtual tResult OnEvent(IGUIEvent * pEvent);

private:
   tResult * m_pResult;
};

////////////////////////////////////////

cGUIModalLoopEventListener::cGUIModalLoopEventListener(tResult * pResult)
 : m_pResult(pResult)
{
}

////////////////////////////////////////

cGUIModalLoopEventListener::~cGUIModalLoopEventListener()
{
}

////////////////////////////////////////

tResult cGUIModalLoopEventListener::OnEvent(IGUIEvent * pEvent)
{
   bool bEatEvent = false;

   tGUIEventCode eventCode;
   Verify(pEvent->GetEventCode(&eventCode) == S_OK);

   long keyCode;
   Verify(pEvent->GetKeyCode(&keyCode) == S_OK);

   switch (eventCode)
   {
      case kGUIEventKeyUp:
      {
         if (keyCode == kEnter)
         {
            *m_pResult = S_OK;
            g_bExitModalLoop = true;
            bEatEvent = true;
         }
         break;
      }
      case kGUIEventKeyDown:
      {
         if (keyCode == kEscape)
         {
            *m_pResult = S_FALSE;
            g_bExitModalLoop = true;
            bEatEvent = true;
         }
         break;
      }
      case kGUIEventClick:
      {
         cAutoIPtr<IGUIElement> pSrcElement;
         if (pEvent->GetSourceElement(&pSrcElement) == S_OK)
         {
            cAutoIPtr<IGUIButtonElement> pButtonElement;
            if (pSrcElement->QueryInterface(IID_IGUIButtonElement, (void**)&pButtonElement) == S_OK)
            {
               if (GUIElementIdMatch(pButtonElement, "ok"))
               {
                  *m_pResult = S_OK;
                  g_bExitModalLoop = true;
                  bEatEvent = true;
               }
               else if (GUIElementIdMatch(pButtonElement, "cancel"))
               {
                  *m_pResult = S_FALSE;
                  g_bExitModalLoop = true;
                  bEatEvent = true;
               }
            }
         }
         break;
      }
   }

   return bEatEvent ? S_FALSE : S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIContext
//

///////////////////////////////////////

BEGIN_CONSTRAINTS(cGUIContext)
   AFTER_GUID(IID_IInput)
   AFTER_GUID(IID_IScriptInterpreter)
END_CONSTRAINTS()

///////////////////////////////////////

cGUIContext::cGUIContext(const tChar * pszScriptName)
 : m_inputListener(this)
 , m_bShowingModalDialog(false)
 , m_scriptName((pszScriptName != NULL) ? pszScriptName : _T(""))
#ifdef GUI_DEBUG
 , m_bShowDebugInfo(false)
 , m_debugInfoPlacement(0,0)
 , m_debugInfoTextColor(GUIStandardColors::White)
 , m_lastMousePos(0,0)
#endif
{
}

///////////////////////////////////////

cGUIContext::~cGUIContext()
{
   Assert(m_pages.empty());
}

///////////////////////////////////////

tResult cGUIContext::Init()
{
   UseGlobal(Input);
   pInput->AddInputListener(&m_inputListener, kILP_GUI);

   GUILayoutRegisterBuiltInTypes();

   UseGlobal(ScriptInterpreter);
   pScriptInterpreter->AddNamedItem(m_scriptName.empty() ? GetName() : m_scriptName.c_str(),
      static_cast<IScriptable*>(this));

   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::Term()
{
   UseGlobal(Input);
   pInput->RemoveInputListener(&m_inputListener);

   std::list<cGUIPage*>::iterator iter = m_pages.begin();
   for (; iter != m_pages.end(); iter++)
   {
      delete *iter;
   }
   m_pages.clear();

   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::Invoke(const char * pszMethodName,
                            int argc, const tScriptVar * argv,
                            int nMaxResults, tScriptVar * pResults)
{
   if (pszMethodName == NULL)
   {
      return E_POINTER;
   }

   typedef tResult (cGUIContext::*tInvokeMethod)(int argc, const tScriptVar * argv,
                                                 int nMaxResults, tScriptVar * pResults);

   static const struct
   {
      const tChar * pszMethodName;
      tInvokeMethod pfnMethod;
   }
   invokeMethods[] =
   {
      { "ShowModalDialog",    &cGUIContext::InvokeShowModalDialog },
      { "PushPage",           &cGUIContext::InvokePushPage },
      { "PopPage",            &cGUIContext::InvokePopPage },
      { "ToggleDebugInfo",    &cGUIContext::InvokeToggleDebugInfo },
      { "GetElement",         &cGUIContext::InvokeGetElement },
   };

   for (int i = 0; i < _countof(invokeMethods); i++)
   {
      if (strcmp(invokeMethods[i].pszMethodName, pszMethodName) == 0)
      {
         return (this->*invokeMethods[i].pfnMethod)(argc, argv, nMaxResults, pResults);
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIContext::InvokeShowModalDialog(int argc, const tScriptVar * argv,
                                           int nMaxResults, tScriptVar * pResults)
{
   Assert(nMaxResults >= 1);

   if (argc != 1 || !argv[0].IsString())
   {
      return E_INVALIDARG;
   }

   tResult result = ShowModalDialog(argv[0]);
   if (result == S_OK)
   {
      *pResults = tScriptVar(true);
      result = 1; // # of return values
   }
   else if (result == S_FALSE)
   {
      *pResults = tScriptVar();
      result = 1; // # of return values
   }
   return result;
}

///////////////////////////////////////

tResult cGUIContext::InvokePushPage(int argc, const tScriptVar * argv,
                                    int nMaxResults, tScriptVar * pResults)
{
   if (argc == 1 && argv[0].IsString())
   {
      if (PushPage(argv[0]) == S_OK)
      {
         LocalMsg1("Loading GUI definitions from %s\n", argv[0].ToString());
         return S_OK;
      }
   }
   else
   {
      return E_INVALIDARG;
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIContext::InvokePopPage(int argc, const tScriptVar * argv,
                                   int nMaxResults, tScriptVar * pResults)
{
   if (argc != 0)
   {
      return E_INVALIDARG;
   }

   if (PopPage() == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIContext::InvokeToggleDebugInfo(int argc, const tScriptVar * argv,
                                           int nMaxResults, tScriptVar * pResults)
{
   tGUIPoint placement(0,0);
   cAutoIPtr<IGUIStyle> pStyle;

   if (argc == 2 
      && IsNumber(argv[0]) 
      && IsNumber(argv[1]))
   {
      placement = tGUIPoint(argv[0], argv[1]);
   }
   else if (argc == 3 
      && IsNumber(argv[0]) 
      && IsNumber(argv[1])
      && argv[2].IsString())
   {
      placement = tGUIPoint(argv[0], argv[1]);
      if (GUIStyleParse(argv[2], -1, &pStyle) != S_OK)
      {
         SafeRelease(pStyle);
      }
   }
   else
   {
      return E_FAIL;
   }

   if (ShowDebugInfo(placement, pStyle) == S_FALSE)
   {
      HideDebugInfo();
   }

   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::InvokeGetElement(int argc, const tScriptVar * argv,
                                      int nMaxResults, tScriptVar * pResults)
{
   if (argc != 1 || !argv[0].IsString())
   {
      return E_INVALIDARG;
   }

   cAutoIPtr<IGUIElement> pElement;
   if (GetElementById(argv[0], &pElement) == S_OK)
   {
      cAutoIPtr<IScriptable> pScriptable;
      if (pElement->QueryInterface(IID_IScriptable, (void**)&pScriptable) == S_OK)
      {
         pResults[0] = pScriptable;
         return 1;
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

class cBoolSetter
{
public:
   cBoolSetter(bool * pBool, bool initialValue = true) : m_pBool(pBool)
   {
      Assert(pBool != NULL);
      *m_pBool = initialValue;
   }
   ~cBoolSetter()
   {
      *m_pBool = !*m_pBool;
   }
private:
   bool * m_pBool;
};

tResult cGUIContext::ShowModalDialog(const tChar * pszDialog)
{
   if (pszDialog == NULL)
   {
      return E_POINTER;
   }

   // Only one at a time supported now
   if (m_bShowingModalDialog)
   {
      ErrorMsg("Attempt to show multiple modal dialogs\n");
      return E_FAIL;
   }

   tResult result = E_FAIL;

   if (PushPage(pszDialog) == S_OK)
   {
      if (!GetCurrentPage()->IsModalDialogPage())
      {
         PopPage();
         return E_FAIL;
      }

      cBoolSetter boolSetter(&m_bShowingModalDialog, true);

      GetCurrentPage()->SetOverlay(true);

      cGUIModalLoopEventListener listener(&result);
      AddEventListener(&listener);

      g_bExitModalLoop = false;
      g_pfnOuterFrameHandler = SysGetFrameCallback();

      // This function won't return until the modal loop is ended by
      // some user action (Enter, Esc, OK button click, etc.)
      SysEventLoop(GUIModalLoopFrameHandler);

      RemoveEventListener(&listener);

      PopPage();
   }

   return result;
}

///////////////////////////////////////

tResult cGUIContext::PushPage(const tChar * pszPage)
{
   if (pszPage == NULL)
   {
      return E_POINTER;
   }

   TiXmlBase::SetCondenseWhiteSpace(false);

   TiXmlDocument doc;
   TiXmlDocument * pTiXmlDoc = &doc;

   doc.Parse(pszPage);
   int errorId = doc.ErrorId();

   if (errorId != TiXmlBase::TIXML_NO_ERROR && errorId != TiXmlBase::TIXML_ERROR_DOCUMENT_EMPTY)
   {
      ErrorMsg1("TiXml parse error: %s\n", doc.ErrorDesc());
      return E_FAIL;
   }
   else if (errorId == TiXmlBase::TIXML_ERROR_DOCUMENT_EMPTY)
   {
      UseGlobal(ResourceManager);
      if (pResourceManager->Load(pszPage, kRT_TiXml, NULL, (void**)&pTiXmlDoc) != S_OK)
      {
         ErrorMsg("Error loading TiXml document as a resource\n");
         return E_FAIL;
      }
   }

   cGUIPage * pPage = NULL;
   if (cGUIPage::Create(pTiXmlDoc, &pPage) == S_OK)
   {
      m_pages.push_back(pPage);
      SetFocus(NULL);
      SetMouseOver(NULL);
      SetDrag(NULL);
      pPage->Activate();
      return S_OK;
   }

   return S_FALSE;
}

///////////////////////////////////////

tResult cGUIContext::PopPage()
{
   if (m_pages.empty())
   {
      return E_FAIL;
   }

   cGUIPage * pLastPage = m_pages.back();
   m_pages.pop_back();
   pLastPage->Deactivate();
   delete pLastPage, pLastPage = NULL;

   SetFocus(NULL);
   SetMouseOver(NULL);
   SetDrag(NULL);

   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::GetElementById(const tChar * pszId, IGUIElement * * ppElement)
{
   if (pszId == NULL || ppElement == NULL)
   {
      return E_POINTER;
   }

   cGUIPage * pPage = GetCurrentPage();
   if (pPage != NULL)
   {
      return pPage->GetElement(pszId, ppElement);
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIContext::RenderGUI()
{
   cAutoIPtr<IGUIRenderDeviceContext> pRenderDeviceContext;
   if (GetRenderDeviceContext(&pRenderDeviceContext) != S_OK)
   {
      return E_FAIL;
   }

   uint width, height;
   if (pRenderDeviceContext->GetViewportSize(&width, &height) != S_OK)
   {
      return E_FAIL;
   }

   std::list<cGUIPage*> renderPages;

   std::list<cGUIPage *>::reverse_iterator iter = m_pages.rbegin();
   for (; iter != m_pages.rend(); iter++)
   {
      renderPages.push_back(*iter);
      if (!(*iter)->IsOverlay())
      {
         break;
      }
   }

   iter = renderPages.rbegin();
   for (; iter != renderPages.rend(); iter++)
   {
      (*iter)->UpdateLayout(tGUIRect(0,0,width,height));
      (*iter)->Render(static_cast<IGUIRenderDevice*>(pRenderDeviceContext));
   }

#ifdef GUI_DEBUG
   RenderDebugInfo();
#endif

   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::GetRenderDeviceContext(IGUIRenderDeviceContext * * ppRenderDeviceContext)
{
   // Wait as long as possible to create the GUI rendering device to
   // ensure there is a GL context or a D3D device or whatever.
   if (!m_pRenderDeviceContext)
   {
      if (GUIRenderDeviceCreateD3D(&m_pRenderDeviceContext) != S_OK)
      {
         if (FAILED(GUIRenderDeviceCreateGL(&m_pRenderDeviceContext)))
         {
            return E_FAIL;
         }
      }
   }

   return m_pRenderDeviceContext.GetPointer(ppRenderDeviceContext);
}

///////////////////////////////////////

tResult cGUIContext::SetRenderDeviceContext(IGUIRenderDeviceContext * pRenderDeviceContext)
{
   SafeRelease(m_pRenderDeviceContext);
   m_pRenderDeviceContext = CTAddRef(pRenderDeviceContext);
   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::GetDefaultFont(IGUIFont * * ppFont)
{
   if (!m_pDefaultFont)
   {
      tChar szTypeFace[32];
      memset(szTypeFace, 0, sizeof(szTypeFace));
      if (ConfigGetString("default_font_win32", szTypeFace, _countof(szTypeFace)) != S_OK)
      {
         ConfigGetString("default_font", szTypeFace, _countof(szTypeFace));
      }

      int pointSize = 10;
      if (ConfigGet("default_font_size_win32", &pointSize) != S_OK)
      {
         ConfigGet("default_font_size", &pointSize);
      }

      int effects = kGFE_None;
      if (ConfigGet("default_font_effects_win32", &effects) != S_OK)
      {
         ConfigGet("default_font_effects", &effects);
      }

      UseGlobal(GUIFontFactory);
      pGUIFontFactory->CreateFont(cGUIFontDesc(szTypeFace, pointSize, effects), &m_pDefaultFont);
   }

   return m_pDefaultFont.GetPointer(ppFont);
}

///////////////////////////////////////

tResult cGUIContext::ShowDebugInfo(const tGUIPoint & placement, IGUIStyle * pStyle)
{
#ifdef GUI_DEBUG
   if (!m_bShowDebugInfo)
   {
      m_debugInfoPlacement = placement;

      if (pStyle != NULL)
      {
         pStyle->GetForegroundColor(&m_debugInfoTextColor);

         cGUIFontDesc debugFontDesc;
         if (pStyle->GetFontDesc(&debugFontDesc) == S_OK)
         {
            SafeRelease(m_pDebugFont);
            UseGlobal(GUIFontFactory);
            pGUIFontFactory->CreateFont(debugFontDesc, &m_pDebugFont);
         }
      }

      m_bShowDebugInfo = true;
      return S_OK;
   }
#endif
   return S_FALSE;
}

///////////////////////////////////////

tResult cGUIContext::HideDebugInfo()
{
#ifdef GUI_DEBUG
   if (m_bShowDebugInfo)
   {
      m_bShowDebugInfo = false;
      return S_OK;
   }
#endif
   return S_FALSE;
}

///////////////////////////////////////

tResult cGUIContext::GetHitElement(const tGUIPoint & point, IGUIElement * * ppElement) const
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   const cGUIPage * pPage = GetCurrentPage();
   if (pPage != NULL)
   {
      std::list<IGUIElement*> hitElements;
      if (pPage->GetHitElements(point, &hitElements) == S_OK)
      {
         Assert(!hitElements.empty());
         *ppElement = CTAddRef(hitElements.front());
         std::for_each(hitElements.begin(), hitElements.end(), CTInterfaceMethod(&IGUIElement::Release));
         return S_OK;
      }
   }

   return S_FALSE;
}

///////////////////////////////////////

tResult cGUIContext::GetActiveModalDialog(IGUIDialogElement * * ppModalDialog)
{
   if (ppModalDialog == NULL)
   {
      return E_POINTER;
   }

   cGUIPage * pPage = GetCurrentPage();
   if (pPage != NULL)
   {
      return pPage->GetActiveModalDialog(ppModalDialog);
   }

   return S_FALSE;
}

///////////////////////////////////////

#ifdef GUI_DEBUG
tResult cGUIContext::GetDebugFont(IGUIFont * * ppFont)
{
   if (!!m_pDebugFont)
   {
      return m_pDebugFont.GetPointer(ppFont);
   }
   else
   {
      return GetDefaultFont(ppFont);
   }
}
#endif

///////////////////////////////////////

#ifdef GUI_DEBUG
void cGUIContext::RenderDebugInfo()
{
   if (!m_bShowDebugInfo)
   {
      return;
   }

   cGUIPage * pPage = GetCurrentPage();
   if (pPage == NULL)
   {
      return;
   }

   cAutoIPtr<IGUIFont> pFont;
   if (GetDebugFont(&pFont) == S_OK)
   {
      tGUIRect rect(0,0,0,0);
      pFont->RenderText("Xy", -1, &rect, kRT_CalcRect, m_debugInfoTextColor);

      const int lineHeight = rect.GetHeight();

      rect = tGUIRect(Round(m_debugInfoPlacement.x), Round(m_debugInfoPlacement.y), 0, 0);

      cStr temp;
      temp.Format("Mouse: (%d, %d)", Round(m_lastMousePos.x), Round(m_lastMousePos.y));
      pFont->RenderText(temp.c_str(), temp.length(), &rect, kRT_NoClip, m_debugInfoTextColor);
      rect.Offset(0, lineHeight);

      tGUIElementList hitElements;
      if (pPage->GetHitElements(m_lastMousePos, &hitElements) == S_OK)
      {
         tGUIElementList::reverse_iterator iter = hitElements.rbegin();
         for (int index = 0; iter != hitElements.rend(); iter++, index++)
         {
            tGUIString type(GUIElementType(*iter)), temp;
            temp.Format("Element %d: %s", index, type.c_str());
            pFont->RenderText(temp.c_str(), temp.length(), &rect, kRT_NoClip, m_debugInfoTextColor);
            rect.Offset(0, lineHeight);

            rect.left += lineHeight;

            const tGUISize size((*iter)->GetSize());
            temp.Format("Size: %d x %d", Round(size.width), Round(size.height));
            pFont->RenderText(temp.c_str(), temp.length(), &rect, kRT_NoClip, m_debugInfoTextColor);
            rect.Offset(0, lineHeight);

            const tGUIPoint pos((*iter)->GetPosition());
            temp.Format("Position: (%d, %d)", Round(pos.x), Round(pos.y));
            pFont->RenderText(temp.c_str(), temp.length(), &rect, kRT_NoClip, m_debugInfoTextColor);
            rect.Offset(0, lineHeight);

            uint nParents = 0;
            tGUIPoint absPos(GUIElementAbsolutePosition(*iter, &nParents));
            temp.Format("Absolute Position: (%d, %d)", Round(absPos.x), Round(absPos.y));
            pFont->RenderText(temp.c_str(), temp.length(), &rect, kRT_NoClip, m_debugInfoTextColor);
            rect.Offset(0, lineHeight);

            temp.Format("# Parents: %d", nParents);
            pFont->RenderText(temp.c_str(), temp.length(), &rect, kRT_NoClip, m_debugInfoTextColor);
            rect.Offset(0, lineHeight);

            const tGUIPoint relPoint(m_lastMousePos - absPos);
            Assert((*iter)->Contains(relPoint));
            temp.Format("Mouse (relative): (%d, %d)", Round(relPoint.x), Round(relPoint.y));
            pFont->RenderText(temp.c_str(), temp.length(), &rect, kRT_NoClip, m_debugInfoTextColor);
            rect.Offset(0, lineHeight);

            rect.left -= lineHeight;
         }

         std::for_each(hitElements.begin(), hitElements.end(), CTInterfaceMethod(&IGUIElement::Release));
      }
   }
}
#endif

///////////////////////////////////////

#ifdef GUI_DEBUG
bool cGUIContext::HandleInputEvent(const sInputEvent * pEvent)
{
   Assert(pEvent != NULL);

   if (pEvent->key == kMouseMove)
   {
      m_lastMousePos = pEvent->point;
   }

   return cGUIEventRouter<cGUIContext, IGUIContext>::HandleInputEvent(pEvent);
}
#endif

///////////////////////////////////////

cGUIContext::cInputListener::cInputListener(cGUIContext * pOuter)
 : m_pOuter(pOuter)
{
}

///////////////////////////////////////

bool cGUIContext::cInputListener::OnInputEvent(const sInputEvent * pEvent)
{
   Assert(m_pOuter != NULL);
   return m_pOuter->HandleInputEvent(pEvent);
}

///////////////////////////////////////

tResult GUIContextCreate(const tChar * pszScriptName)
{
   cAutoIPtr<IGUIContext> p(static_cast<IGUIContext*>(new cGUIContext(pszScriptName)));
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_IGUIContext, p);
}

///////////////////////////////////////////////////////////////////////////////
