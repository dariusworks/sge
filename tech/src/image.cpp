///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "image.h"

#include "color.h"
#include "globalobj.h"
#include "resourceapi.h"

#include <cstring>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

uint BytesPerPixel(ePixelFormat pixelFormat)
{
   if (pixelFormat <= kPF_ERROR || pixelFormat >= kPF_NumPixelFormats)
   {
      return 0;
   }

   static const uint bytesPerPixel[] =
   {
      1, // kPF_Grayscale
      1, // kPF_ColorMapped
      2, // kPF_RGB555
      2, // kPF_BGR555
      2, // kPF_RGB565
      2, // kPF_BGR565
      2, // kPF_RGBA1555
      2, // kPF_BGRA1555
      3, // kPF_RGB888
      3, // kPF_BGR888
      4, // kPF_RGBA8888
      4, // kPF_BGRA8888
   };

   Assert(_countof(bytesPerPixel) == kPF_NumPixelFormats);

   return bytesPerPixel[pixelFormat];
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cImage
//

///////////////////////////////////////

cImage::cImage(uint width, uint height, ePixelFormat pixelFormat, byte * pData)
 : m_width(width)
 , m_height(height)
 , m_pixelFormat(pixelFormat)
 , m_pData(pData)
{
}

///////////////////////////////////////

cImage::~cImage()
{
   // This is the memory allocated in ImageCreate() below
   delete [] m_pData;
   m_pData = NULL;
}

///////////////////////////////////////

uint cImage::GetWidth() const
{
   return m_width;
}

///////////////////////////////////////

uint cImage::GetHeight() const
{
   return m_height;
}

///////////////////////////////////////

ePixelFormat cImage::GetPixelFormat() const
{
   return m_pixelFormat;
}

///////////////////////////////////////

const void * cImage::GetData() const
{
   return m_pData;
}

///////////////////////////////////////

tResult cImage::GetPixel(uint x, uint y, cColor * pPixel) const
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cImage::SetPixel(uint x, uint y, const cColor & pixel)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cImage::Clone(IImage * * ppImage)
{
   return ImageCreate(GetWidth(), GetHeight(), GetPixelFormat(), GetData(), ppImage);
}

///////////////////////////////////////////////////////////////////////////////

extern void * TargaLoad(IReader * pReader);
extern void * BmpLoad(IReader * pReader);

///////////////////////////////////////

void ImageUnload(void * pData)
{
   reinterpret_cast<IImage*>(pData)->Release();
}

///////////////////////////////////////

tResult ImageRegisterResourceFormats()
{
   UseGlobal(ResourceManager);
   if (!!pResourceManager)
   {
      if (pResourceManager->RegisterFormat(kRT_Image, _T("tga"), TargaLoad, NULL, ImageUnload) == S_OK
         && pResourceManager->RegisterFormat(kRT_Image, _T("bmp"), BmpLoad, NULL, ImageUnload) == S_OK)
      {
         return S_OK;
      }
   }

   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////

tResult ImageCreate(uint width, uint height, ePixelFormat pixelFormat, const void * pData, IImage * * ppImage)
{
   if (width == 0 || height == 0)
   {
      return E_INVALIDARG;
   }

   if (pixelFormat <= kPF_ERROR || pixelFormat >= kPF_NumPixelFormats)
   {
      return E_INVALIDARG;
   }

   if (ppImage == NULL)
   {
      return E_POINTER;
   }

   uint memSize = BytesPerPixel(pixelFormat) * width * height;
   if (memSize == 0)
   {
      WarnMsg1("Invalid pixel format %d\n", pixelFormat);
      return E_FAIL;
   }

   byte * pImageData = new byte[memSize];
   if (pImageData == NULL)
   {
      return E_OUTOFMEMORY;
   }

   if (pData != NULL)
   {
      memcpy(pImageData, pData, memSize);
   }
   else
   {
      memset(pImageData, 0, memSize);
   }

   cAutoIPtr<cImage> pImage(new cImage(width, height, pixelFormat, pImageData));
   if (!pImage)
   {
      return E_OUTOFMEMORY;
   }

   *ppImage = CTAddRef(static_cast<IImage*>(pImage));
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
