///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guibutton.h"
#include "guielementbasetem.h"
#include "guirender.h"

#include "font.h"
#include "color.h"
#include "render.h"

#include "globalobj.h"

#include <tinyxml.h>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIButtonElement
//

///////////////////////////////////////

cGUIButtonElement::cGUIButtonElement()
 : m_bArmed(false),
   m_bMouseOver(false),
   m_text("")
{
}

///////////////////////////////////////

cGUIButtonElement::~cGUIButtonElement()
{
}

///////////////////////////////////////

tResult cGUIButtonElement::OnEvent(IGUIEvent * pEvent)
{
   Assert(pEvent != NULL);

   tGUIEventCode eventCode;
   Verify(pEvent->GetEventCode(&eventCode) == S_OK);

   if (eventCode == kGUIEventMouseEnter)
   {
      DebugMsg("Mouse enter button\n");
      SetMouseOver(true);
   }
   else if (eventCode == kGUIEventMouseLeave)
   {
      DebugMsg("Mouse leave button\n");
      SetMouseOver(false);
   }
   else if (eventCode == kGUIEventMouseDown)
   {
      DebugMsg("Mouse down button\n");
      UseGlobal(GUIContext);
      pGUIContext->SetCapture(this);
      SetArmed(true);
   }
   else if (eventCode == kGUIEventMouseUp)
   {
      DebugMsg("Mouse up button\n");
      UseGlobal(GUIContext);
      cAutoIPtr<IGUIElement> pCapture;
      if (pGUIContext->GetCapture(&pCapture) == S_OK)
      {
         if (CTIsSameObject(this, pCapture))
         {
            pGUIContext->SetCapture(NULL);
         }
      }
      SetArmed(false);
   }
   else if (eventCode == kGUIEventClick)
   {
      DebugMsg("Mouse click button\n");
   }

   return S_OK;
}

///////////////////////////////////////

tResult cGUIButtonElement::GetRendererClass(tGUIString * pRendererClass)
{
   if (pRendererClass == NULL)
      return E_POINTER;
   *pRendererClass = "button";
   return S_OK;
}

///////////////////////////////////////

bool cGUIButtonElement::IsArmed() const
{
   return m_bArmed;
}

///////////////////////////////////////

void cGUIButtonElement::SetArmed(bool bArmed)
{
   m_bArmed = bArmed;
}

///////////////////////////////////////

bool cGUIButtonElement::IsMouseOver() const
{
   return m_bMouseOver;
}

///////////////////////////////////////

void cGUIButtonElement::SetMouseOver(bool bMouseOver)
{
   m_bMouseOver = bMouseOver;
}

///////////////////////////////////////

const char * cGUIButtonElement::GetText() const
{
   return m_text;
}

///////////////////////////////////////

void cGUIButtonElement::SetText(const char * pszText)
{
   m_text = pszText;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIButtonElementFactory
//

AUTOREGISTER_GUIELEMENTFACTORY(button, cGUIButtonElementFactory);

tResult cGUIButtonElementFactory::CreateElement(const TiXmlElement * pXmlElement, IGUIElement * * ppElement)
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   if (pXmlElement != NULL)
   {
      if (strcmp(pXmlElement->Value(), "button") == 0)
      {
         cAutoIPtr<IGUIButtonElement> pButton = static_cast<IGUIButtonElement *>(new cGUIButtonElement);
         if (!!pButton)
         {
            if (pXmlElement->Attribute("text"))
            {
               pButton->SetText(pXmlElement->Attribute("text"));
            }

            if (pXmlElement->Attribute("style"))
            {
               cAutoIPtr<IGUIStyle> pStyle;
               if (GUIStyleParse(pXmlElement->Attribute("style"), &pStyle) == S_OK)
               {
                  pButton->SetStyle(pStyle);
               }
            }

            *ppElement = CTAddRef(pButton);
            return S_OK;
         }
      }
   }
   else
   {
      *ppElement = static_cast<IGUIButtonElement *>(new cGUIButtonElement);
      return (*ppElement != NULL) ? S_OK : E_FAIL;
   }

   return E_FAIL;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIButtonRenderer
//

///////////////////////////////////////

cGUIButtonRenderer::cGUIButtonRenderer()
 : m_pFont(FontCreateDefault())
{
}

///////////////////////////////////////

cGUIButtonRenderer::~cGUIButtonRenderer()
{
}

///////////////////////////////////////

static const tGUIColor kGray(0.75,0.75,0.75);
static const tGUIColor kDarkGray(0.5,0.5,0.5);
static const tGUIColor kLightGray(0.87f,0.87f,0.87f);

tResult cGUIButtonRenderer::Render(IGUIElement * pElement, IRenderDevice * pRenderDevice)
{
   if (pElement == NULL || pRenderDevice == NULL)
   {
      return E_POINTER;
   }

   static const int g_bevel = 2;

   cAutoIPtr<IGUIButtonElement> pButton;
   if (pElement->QueryInterface(IID_IGUIButtonElement, (void**)&pButton) == S_OK)
   {
      tGUIPoint pos = pButton->GetPosition();
      tGUISize size = pButton->GetSize();

      tGUIPoint textOffset(0,0);

      tRect rect(pos.x, pos.y, pos.x + size.width, pos.y + size.height);
      tGUIRect rect2(pos.x, pos.y, pos.x + size.width, pos.y + size.height);

      UseGlobal(GUIRenderingTools);

      if (pButton->IsArmed() && pButton->IsMouseOver())
      {
         pGUIRenderingTools->Render3dRect(rect2, g_bevel, kDarkGray, kLightGray, kGray);
         textOffset = tGUIPoint(g_bevel,g_bevel);
      }
      else
      {
         pGUIRenderingTools->Render3dRect(rect2, g_bevel, kLightGray, kDarkGray, kGray);
      }

      rect.left += textOffset.x;
      rect.top += textOffset.y;
      m_pFont->DrawText(pButton->GetText(), -1, kDT_Center | kDT_VCenter | kDT_SingleLine, &rect, tGUIColor::White);

      return S_OK;
   }

   return E_FAIL;
}

///////////////////////////////////////

tGUISize cGUIButtonRenderer::GetPreferredSize(IGUIElement * pElement)
{
   if (pElement != NULL)
   {
      cAutoIPtr<IGUIButtonElement> pButton;
      if (pElement->QueryInterface(IID_IGUIButtonElement, (void**)&pButton) == S_OK)
      {
         tRect rect(0,0,0,0);
         m_pFont->DrawText(pButton->GetText(), -1, kDT_CalcRect, &rect, tGUIColor::White);
         return tGUISize(rect.GetWidth() + rect.GetHeight(), rect.GetHeight() * 1.5);
      }
   }
   return tGUISize(0,0);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIButtonRendererFactory
//

AUTOREGISTER_GUIELEMENTRENDERERFACTORY(button, cGUIButtonRendererFactory);

tResult cGUIButtonRendererFactory::CreateRenderer(IGUIElement * /*pElement*/, IGUIElementRenderer * * ppRenderer)
{
   if (ppRenderer == NULL)
   {
      return E_POINTER;
   }

   *ppRenderer = static_cast<IGUIElementRenderer *>(new cGUIButtonRenderer);
   return (*ppRenderer != NULL) ? S_OK : E_OUTOFMEMORY;
}

///////////////////////////////////////////////////////////////////////////////
