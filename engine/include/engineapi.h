///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENGINEAPI_H
#define INCLUDED_ENGINEAPI_H

#include "enginedll.h"
#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

class cImageData;

///////////////////////////////////////////////////////////////////////////////

#define kRT_CgProgram   _T("CgProgram")
#define kRT_CgEffect    _T("CgEffect")
#define kRT_GlTexture   _T("GlTexture")
#define kRT_TiXml       _T("TiXml")

ENGINE_API tResult EngineRegisterResourceFormats();

ENGINE_API tResult GlTextureCreate(const cImageData * pImageData, uint * pTexId);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENGINEAPI_H
