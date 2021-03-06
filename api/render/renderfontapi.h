///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_RENDERFONTAPI_H
#define INCLUDED_RENDERFONTAPI_H

#include "renderdll.h"
#include "tech/comtools.h"

#include "tech/rect.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE_GUID(IRenderFont, "D82266F8-FD93-49f7-904B-74D7AF37DA24");
F_DECLARE_INTERFACE_GUID(IRenderFontFactory, "747C41BD-488B-4349-95C9-78342D132976");

enum
{
   kASCIIGlyphFirst = 32,
   kASCIIGlyphLast = 128,
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IRenderFont
//

enum eRenderFontFlags
{
   kRFF_None      = (0<<0),
   kRFF_Bold      = (1<<0),
   kRFF_Italic    = (1<<1),
   kRFF_Shadow    = (1<<2),
   kRFF_Outline   = (1<<3),
};

enum eRenderTextFlags
{
   kRT_Center        = (1<<0),
   kRT_VCenter       = (1<<1),
   kRT_NoClip        = (1<<2),
   kRT_CalcRect      = (1<<3),
   kRT_SingleLine    = (1<<4),
   kRT_Bottom        = (1<<5),
   kRT_NoBlend       = (1<<6),
   kRT_DropShadow    = (1<<7),
};

interface IRenderFont : IUnknown
{
   virtual tResult RenderText(const tChar * pszText, int textLength, tRecti * pRect,
                              uint flags, const float color[4]) const = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IRenderFontFactory
//

interface IRenderFontFactory : IUnknown
{
   virtual tResult CreateFont(const tChar * pszFont, int fontPointSize, uint flags, IRenderFont * * ppFont) = 0;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_RENDERFONTAPI_H
