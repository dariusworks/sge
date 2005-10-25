///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIBASICRENDERER_H
#define INCLUDED_GUIBASICRENDERER_H

#include "guiapi.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIBasicRenderer
//

class cGUIBasicRenderer : public cComObject<IMPLEMENTS(IGUIElementRenderer)>
{
public:
   cGUIBasicRenderer();
   ~cGUIBasicRenderer();

   virtual tResult Render(IGUIElement * pElement, IGUIRenderDevice * pRenderDevice);
   tGUISize GetPreferredSize(IGUIElement * pElement);
   virtual tResult GetPreferredSize(IGUIElement * pElement, tGUISize * pSize);
   virtual tResult ComputeClientArea(IGUIElement * pElement, tGUIRect * pClientArea);
   virtual tResult GetFont(IGUIElement * pElement, IGUIFont * * ppFont);

private:
   tResult Render(IGUIButtonElement * pButtonElement, IGUIRenderDevice * pRenderDevice);
   tResult Render(IGUIDialogElement * pDialogElement, IGUIRenderDevice * pRenderDevice);
   tResult Render(IGUILabelElement * pLabelElement, IGUIRenderDevice * pRenderDevice);
   tResult Render(IGUIPanelElement * pPanelElement, IGUIRenderDevice * pRenderDevice);
   tResult Render(IGUITextEditElement * pTextEditElement, IGUIRenderDevice * pRenderDevice);

   tGUISize GetPreferredSize(IGUIButtonElement * pButtonElement);
   tGUISize GetPreferredSize(IGUIDialogElement * pDialogElement);
   tGUISize GetPreferredSize(IGUILabelElement * pLabelElement);
   tGUISize GetPreferredSize(IGUIPanelElement * pPanelElement);
   tGUISize GetPreferredSize(IGUITextEditElement * pTextEditElement);
   tGUISize GetPreferredSize(IGUIContainerElement * pContainerElement);
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIBasicRendererFactory
//

class cGUIBasicRendererFactory : public cComObject<IMPLEMENTS(IGUIElementRendererFactory)>
{
public:
   virtual tResult CreateRenderer(IGUIElement * pElement, IGUIElementRenderer * * ppRenderer);

private:
   cAutoIPtr<IGUIElementRenderer> m_pStatelessBasicRenderer;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIBASICRENDERER_H
