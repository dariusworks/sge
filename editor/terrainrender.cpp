/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "terrainrender.h"
#include "editorapi.h"
#include "engineapi.h"

#include "imagedata.h"
#include "resourceapi.h"
#include "globalobj.h"
#include "filespec.h"

#include <algorithm>
#include <functional>
#include <map>

#include <GL/glew.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// REFERENCES
// http://cbloom.com/3d/techdocs/splatting.txt
// http://oss.sgi.com/projects/ogl-sample/registry/ARB/texture_env_combine.txt
// http://www.gamedev.net/community/forums/topic.asp?topic_id=330331

const uint kNoIndex = ~0;

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainRenderer
//

////////////////////////////////////////

tResult TerrainRendererCreate()
{
   cAutoIPtr<ITerrainRenderer> p(new cTerrainRenderer);
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_ITerrainRenderer, static_cast<ITerrainRenderer*>(p));
}

////////////////////////////////////////

cTerrainRenderer::cTerrainRenderer()
 : m_terrainModelListener(this),
   m_nTilesPerChunk(TerrainRendererDefaults::kTerrainTilesPerChunk),
   m_bEnableBlending(true),
   m_bTerrainChanged(false)
{
}

////////////////////////////////////////

cTerrainRenderer::~cTerrainRenderer()
{
}

////////////////////////////////////////

BEGIN_CONSTRAINTS(cTerrainRenderer)
   AFTER_GUID(IID_ITerrainModel)
END_CONSTRAINTS()

////////////////////////////////////////

tResult cTerrainRenderer::Init()
{
   UseGlobal(TerrainModel);
   pTerrainModel->AddTerrainModelListener(&m_terrainModelListener);

   return S_OK;
}

////////////////////////////////////////

tResult cTerrainRenderer::Term()
{
   UseGlobal(TerrainModel);
   pTerrainModel->RemoveTerrainModelListener(&m_terrainModelListener);

   ClearChunks();

   return S_OK;
}

////////////////////////////////////////

void cTerrainRenderer::SetTilesPerChunk(uint tilesPerChunk)
{
   // TODO: some sort of validation on the input
   m_nTilesPerChunk = tilesPerChunk;
   RegenerateChunks();
}

////////////////////////////////////////

uint cTerrainRenderer::GetTilesPerChunk() const
{
   return m_nTilesPerChunk;
}

////////////////////////////////////////

tResult cTerrainRenderer::EnableBlending(bool bEnable)
{
   m_bEnableBlending = bEnable;
   if (bEnable && m_bTerrainChanged)
   {
      RegenerateChunks();
   }
   return S_OK;
}

////////////////////////////////////////

void cTerrainRenderer::RegenerateChunks()
{
   if (!m_bEnableBlending)
   {
      return;
   }

   UseGlobal(TerrainModel);

   for (tChunks::iterator iter = m_chunks.begin(); iter != m_chunks.end(); iter++)
   {
      delete *iter;
   }
   m_chunks.clear();

   cTerrainSettings terrainSettings;
   Verify(pTerrainModel->GetTerrainSettings(&terrainSettings) == S_OK);

   uint nChunksX = terrainSettings.GetTileCountX() / GetTilesPerChunk();
   uint nChunksZ = terrainSettings.GetTileCountZ() / GetTilesPerChunk();

   cAutoIPtr<IEditorTileSet> pTileSet;
   pTerrainModel->GetTileSet(&pTileSet);

   for (uint iz = 0; iz < nChunksZ; iz++)
   {
      cRange<uint> zr(iz * GetTilesPerChunk(), (iz+1) * GetTilesPerChunk());

      for (uint ix = 0; ix < nChunksX; ix++)
      {
         cRange<uint> xr(ix * GetTilesPerChunk(), (ix+1) * GetTilesPerChunk());

         cTerrainChunk * pChunk = NULL;
         if (cTerrainChunkBlended::Create(xr, zr, &pChunk) == S_OK)
         {
            m_chunks.push_back(pChunk);
         }
      }
   }

   m_bTerrainChanged = false;
}

////////////////////////////////////////

void cTerrainRenderer::ClearChunks()
{
   for (tChunks::iterator iter = m_chunks.begin(); iter != m_chunks.end(); iter++)
   {
      delete *iter;
   }
   m_chunks.clear();
}

////////////////////////////////////////

// VC6 requires explicit instantiation of mem_fun_t for void return type
#if _MSC_VER <= 1200
template <>
class std::mem_fun_t<void, cTerrainChunk>
{
public:
   mem_fun_t(void (cTerrainChunk::*pfn)())
      : m_pfn(pfn) {}
	void operator()(cTerrainChunk * pChunk) const
   {
      ((pChunk->*m_pfn)());
   }
private:
	void (cTerrainChunk::*m_pfn)();
};

template <>
class std::mem_fun1_t<void, cTerrainChunk, IEditorTileSet*>
{
public:
   mem_fun1_t(void (cTerrainChunk::*pfn)(IEditorTileSet*))
      : m_pfn(pfn) {}
	void operator()(cTerrainChunk * pChunk, IEditorTileSet * pTileSet) const
   {
      ((pChunk->*m_pfn)(pTileSet));
   }
private:
	void (cTerrainChunk::*m_pfn)(IEditorTileSet*);
};
#endif

////////////////////////////////////////

void cTerrainRenderer::Render()
{
   UseGlobal(TerrainModel);
   cAutoIPtr<IEditorTileSet> pTileSet;
   if (pTerrainModel->GetTileSet(&pTileSet) != S_OK)
   {
      return;
   }

   if (m_bEnableBlending)
   {
      if (!m_chunks.empty())
      {
         std::for_each(m_chunks.begin(), m_chunks.end(),
            std::bind2nd(std::mem_fun1<void, cTerrainChunk, IEditorTileSet*>(&cTerrainChunk::Render),
               static_cast<IEditorTileSet*>(pTileSet)));
      }
   }
   else
   {
      glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);

      glDisableClientState(GL_EDGE_FLAG_ARRAY);
      glDisableClientState(GL_INDEX_ARRAY);
      glDisableClientState(GL_VERTEX_ARRAY);
      glDisableClientState(GL_NORMAL_ARRAY);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      glDisableClientState(GL_COLOR_ARRAY);

//      glEnable(GL_COLOR_MATERIAL);

      UseGlobal(ResourceManager);

      tTerrainQuads::const_iterator iter = pTerrainModel->BeginTerrainQuads();
      tTerrainQuads::const_iterator end = pTerrainModel->EndTerrainQuads();
      for (; iter != end; iter++)
      {
         const sTerrainVertex * pVertices = iter->verts;

         cStr texture;
         cAutoIPtr<IEditorTileSet> pEditorTileSet;
         if (pTerrainModel->GetTileSet(&pEditorTileSet) == S_OK
            && pEditorTileSet->GetTileTexture(iter->tile, &texture) == S_OK)
         {
            GLuint tex;
            if (pResourceManager->Load(texture.c_str(), kRT_GlTexture, NULL, (void**)&tex) == S_OK)
            {
               glEnable(GL_TEXTURE_2D);
               glBindTexture(GL_TEXTURE_2D, tex);
            }
         }

         glBegin(GL_QUADS);

         glNormal3f(1,1,1);
         glColor4f(1,1,1,1);

         glTexCoord2fv(pVertices[0].uv1.v);
         glVertex3fv(pVertices[0].pos.v);

         glTexCoord2fv(pVertices[3].uv1.v);
         glVertex3fv(pVertices[3].pos.v);

         glTexCoord2fv(pVertices[2].uv1.v);
         glVertex3fv(pVertices[2].pos.v);

         glTexCoord2fv(pVertices[1].uv1.v);
         glVertex3fv(pVertices[1].pos.v);

         glEnd();
      }

      glPopAttrib();
   }
}

////////////////////////////////////////

cTerrainRenderer::cTerrainModelListener::cTerrainModelListener(cTerrainRenderer * pOuter)
 : m_pOuter(pOuter)
{
}

////////////////////////////////////////

void cTerrainRenderer::cTerrainModelListener::OnTerrainInitialize()
{
   if (m_pOuter != NULL)
   {
      m_pOuter->m_bTerrainChanged = true;
      m_pOuter->RegenerateChunks();
   }
}

////////////////////////////////////////

void cTerrainRenderer::cTerrainModelListener::OnTerrainClear()
{
   if (m_pOuter != NULL)
   {
      m_pOuter->m_bTerrainChanged = true;
      m_pOuter->ClearChunks();
   }
}

////////////////////////////////////////

void cTerrainRenderer::cTerrainModelListener::OnTerrainChange()
{
   if (m_pOuter != NULL)
   {
      m_pOuter->m_bTerrainChanged = true;
      m_pOuter->RegenerateChunks();
   }
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainChunk
//

////////////////////////////////////////

cTerrainChunk::cTerrainChunk()
{
}

////////////////////////////////////////

cTerrainChunk::~cTerrainChunk()
{
}


/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSplatBuilder
//

////////////////////////////////////////

cSplatBuilder::cSplatBuilder(uint tile)
 : m_tile(tile),
   m_alphaMapId(0)
{
}

////////////////////////////////////////

cSplatBuilder::~cSplatBuilder()
{
}

////////////////////////////////////////

tResult cSplatBuilder::GetGlTexture(IEditorTileSet * pTileSet, uint * pTexId)
{
   cStr tileTex;
   if (pTileSet->GetTileTexture(m_tile, &tileTex) == S_OK)
   {
      UseGlobal(ResourceManager);
      return pResourceManager->Load(tileTex.c_str(), kRT_GlTexture, NULL, (void**)pTexId);
   }
   return E_FAIL;
}

////////////////////////////////////////

tResult cSplatBuilder::GetAlphaMap(uint * pAlphaMapId)
{
   if (pAlphaMapId == NULL)
   {
      return E_POINTER;
   }
   *pAlphaMapId = m_alphaMapId;
   return S_OK;
}

////////////////////////////////////////

void cSplatBuilder::AddTriangle(uint i0, uint i1, uint i2)
{
   m_indices.push_back(i0);
   m_indices.push_back(i1);
   m_indices.push_back(i2);
}

////////////////////////////////////////

size_t cSplatBuilder::GetIndexCount() const
{
   return m_indices.size();
}

////////////////////////////////////////

const uint * cSplatBuilder::GetIndexPtr() const
{
   return &m_indices[0];
}

////////////////////////////////////////

static float SplatTexelWeight(const tVec2 & pt1, const tVec2 & pt2)
{
   static const float kOneOver175Sqr = 1.0f / (1.75f * 1.75f);
   return 1 - (Vec2DistanceSqr(pt1, pt2) * kOneOver175Sqr);
}

template <typename T>
inline T Clamp(T value, T rangeFirst, T rangeLast)
{
   if (value < rangeFirst)
   {
      return rangeFirst;
   }
   else if (value > rangeLast)
   {
      return rangeLast;
   }
   else
   {
      return value;
   }
}

inline float STW(const tVec2 & pt1, const tVec2 & pt2)
{
   return Clamp(SplatTexelWeight(pt1,pt2), 0.f, 1.f);
}

static const float g_texelWeights[4][8] =
{
   {
      // from upper left texel (-.25, -.25)
      STW(tVec2(-.25,-.25), tVec2(-1,-1)), // top left
      STW(tVec2(-.25,-.25), tVec2( 0,-1)), // top mid
      STW(tVec2(-.25,-.25), tVec2( 1,-1)), // top right
      STW(tVec2(-.25,-.25), tVec2(-1, 0)), // left
      STW(tVec2(-.25,-.25), tVec2( 1, 0)), // right
      STW(tVec2(-.25,-.25), tVec2(-1, 1)), // bottom left
      STW(tVec2(-.25,-.25), tVec2( 0, 1)), // bottom mid
      STW(tVec2(-.25,-.25), tVec2( 1, 1)), // bottom right (could be zero)
   },
   {
      // from upper right texel (+.25, -.25)
      STW(tVec2(.25,-.25), tVec2(-1,-1)), // top left
      STW(tVec2(.25,-.25), tVec2( 0,-1)), // top mid
      STW(tVec2(.25,-.25), tVec2( 1,-1)), // top right
      STW(tVec2(.25,-.25), tVec2(-1, 0)), // left
      STW(tVec2(.25,-.25), tVec2( 1, 0)), // right
      STW(tVec2(.25,-.25), tVec2(-1, 1)), // bottom left (could be zero)
      STW(tVec2(.25,-.25), tVec2( 0, 1)), // bottom mid
      STW(tVec2(.25,-.25), tVec2( 1, 1)), // bottom right
   },
   {
      // from lower left texel (-.25, +.25)
      STW(tVec2(-.25,.25), tVec2(-1,-1)), // top left
      STW(tVec2(-.25,.25), tVec2( 0,-1)), // top mid
      STW(tVec2(-.25,.25), tVec2( 1,-1)), // top right (could be zero)
      STW(tVec2(-.25,.25), tVec2(-1, 0)), // left
      STW(tVec2(-.25,.25), tVec2( 1, 0)), // right
      STW(tVec2(-.25,.25), tVec2(-1, 1)), // bottom left
      STW(tVec2(-.25,.25), tVec2( 0, 1)), // bottom mid
      STW(tVec2(-.25,.25), tVec2( 1, 1)), // bottom right
   },
   {
      // from lower right texel (+.25, +.25)
      STW(tVec2(.25,.25), tVec2(-1,-1)), // top left (could be zero)
      STW(tVec2(.25,.25), tVec2( 0,-1)), // top mid
      STW(tVec2(.25,.25), tVec2( 1,-1)), // top right
      STW(tVec2(.25,.25), tVec2(-1, 0)), // left
      STW(tVec2(.25,.25), tVec2( 1, 0)), // right
      STW(tVec2(.25,.25), tVec2(-1, 1)), // bottom left
      STW(tVec2(.25,.25), tVec2( 0, 1)), // bottom mid
      STW(tVec2(.25,.25), tVec2( 1, 1)), // bottom right
   },
};

void cSplatBuilder::BuildAlphaMap(const cRange<uint> xRange, const cRange<uint> zRange)
{
   UseGlobal(TerrainModel);
   UseGlobal(TerrainRenderer);

   cTerrainSettings terrainSettings;
   Verify(pTerrainModel->GetTerrainSettings(&terrainSettings) == S_OK);

   BITMAPINFO bmi = {0};
   bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
   bmi.bmiHeader.biWidth = pTerrainRenderer->GetTilesPerChunk() * 2;
   bmi.bmiHeader.biHeight = -(static_cast<int>(pTerrainRenderer->GetTilesPerChunk()) * 2);
   bmi.bmiHeader.biPlanes = 1;
   bmi.bmiHeader.biCompression = BI_RGB;
   bmi.bmiHeader.biBitCount = 32;
   byte * pBitmapBits = NULL;
   HBITMAP hBitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&pBitmapBits, NULL, 0);
   if (hBitmap == NULL)
   {
      return;
   }

   DIBSECTION dibSection = {0};
   Verify(GetObject(hBitmap, sizeof(dibSection), &dibSection));

   uint redMask = dibSection.dsBitfields[0];
   uint greenMask = dibSection.dsBitfields[1];
   uint blueMask = dibSection.dsBitfields[2];

   for (uint z = zRange.GetStart(); z < zRange.GetEnd(); z++)
   {
      uint zPrev = kNoIndex;
      if (z > zRange.GetStart())
      {
         zPrev = z - 1;
      }
      else if ((z == zRange.GetStart()) && (zRange.GetStart() > 0))
      {
         zPrev = zRange.GetStart() - 1;
      }

      uint zNext = kNoIndex;
      if (z < (zRange.GetEnd() - 1))
      {
         zNext = z + 1;
      }
      else if ((z == (zRange.GetEnd() - 1)) && (zRange.GetEnd() < terrainSettings.GetTileCountZ()))
      {
         zNext = zRange.GetEnd();
      }

      Assert(zPrev == zRange.GetPrev(z, 0, kNoIndex));
      Assert(zNext == zRange.GetNext(z, terrainSettings.GetTileCountZ(), kNoIndex));

      for (uint x = xRange.GetStart(); x < xRange.GetEnd(); x++)
      {
         uint xPrev = kNoIndex;
         if (x > xRange.GetStart())
         {
            xPrev = x - 1;
         }
         else if ((x == xRange.GetStart()) && (xRange.GetStart() > 0))
         {
            xPrev = xRange.GetStart() - 1;
         }

         uint xNext = kNoIndex;
         if (x < (xRange.GetEnd() - 1))
         {
            xNext = x + 1;
         }
         else if ((x == (xRange.GetEnd() - 1)) && (xRange.GetEnd() < terrainSettings.GetTileCountX()))
         {
            xNext = xRange.GetEnd();
         }

         Assert(xPrev == xRange.GetPrev(x, 0, kNoIndex));
         Assert(xNext == xRange.GetNext(x, terrainSettings.GetTileCountX(), kNoIndex));

         const uint neighborCoords[8][2] =
         {
            {xPrev, zPrev},
            {x,     zPrev},
            {xNext, zPrev},
            {xPrev, z,   },
            {xNext, z,   },
            {xPrev, zNext},
            {x,     zNext},
            {xNext, zNext},
         };

         float texelWeights[4];
         int texelDivisors[4];

         memset(texelWeights, 0, sizeof(texelWeights));
         memset(texelDivisors, 0, sizeof(texelDivisors));

         for (int i = 0; i < _countof(neighborCoords); i++)
         {
            uint tile, nx = neighborCoords[i][0], nz = neighborCoords[i][1];
            if (pTerrainModel->GetQuadTile(nx, nz, &tile) == S_OK)
            {
               if (tile == m_tile)
               {
                  for (int j = 0; j < _countof(texelWeights); j++)
                  {
                     texelWeights[j] += g_texelWeights[j][i];
                     texelDivisors[j] += 1;
                  }
               }
            }
         }

//         if (texelDivisors[0] != 0) texelWeights[0] /= texelDivisors[0];
//         if (texelDivisors[1] != 0) texelWeights[1] /= texelDivisors[1];
//         if (texelDivisors[2] != 0) texelWeights[2] /= texelDivisors[2];
//         if (texelDivisors[3] != 0) texelWeights[3] /= texelDivisors[3];

         texelWeights[0] = Clamp(texelWeights[0], 0.f, 1.f);
         texelWeights[1] = Clamp(texelWeights[1], 0.f, 1.f);
         texelWeights[2] = Clamp(texelWeights[2], 0.f, 1.f);
         texelWeights[3] = Clamp(texelWeights[3], 0.f, 1.f);

         uint iz = (z - zRange.GetStart()) * 2;
         uint ix = (x - xRange.GetStart()) * 2;

         uint m = bmi.bmiHeader.biBitCount / 8;

         byte * p0 = pBitmapBits + (iz * bmi.bmiHeader.biWidth * m) + (ix * m);
         byte * p1 = pBitmapBits + (iz * bmi.bmiHeader.biWidth * m) + ((ix+1) * m);
         byte * p2 = pBitmapBits + ((iz+1) * bmi.bmiHeader.biWidth * m) + (ix * m);
         byte * p3 = pBitmapBits + ((iz+1) * bmi.bmiHeader.biWidth * m) + ((ix+1) * m);

         p0[0] = p0[1] = p0[2] = p0[3] = (byte)(255 * texelWeights[0]);
         p1[0] = p1[1] = p1[2] = p1[3] = (byte)(255 * texelWeights[1]);
         p2[0] = p2[1] = p2[2] = p2[3] = (byte)(255 * texelWeights[2]);
         p3[0] = p3[1] = p3[2] = p3[3] = (byte)(255 * texelWeights[3]);
      }
   }

   cImageData * pImage = new cImageData;
   if (pImage != NULL)
   {
      if (pImage->Create(bmi.bmiHeader.biWidth, abs(bmi.bmiHeader.biHeight), kPF_RGBA8888, pBitmapBits))
      {
#if 0 && defined(_DEBUG)
         {
            cStr temp, file;
            cFileSpec(m_tileTexture.c_str()).GetFileNameNoExt(&temp);
            file.Format("%sAlpha%d%d.bmp", temp.c_str(), iChunkX, iChunkZ);
            cAutoIPtr<IWriter> pWriter(FileCreateWriter(cFileSpec(file.c_str())));
            BmpWrite(pImage, pWriter);
         }
#endif

         GlTextureCreate(pImage, &m_alphaMapId);
      }

      delete pImage;
      pImage = NULL;
   }

   DeleteObject(hBitmap), hBitmap = NULL;
}

/////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainChunkBlended
//

////////////////////////////////////////

cTerrainChunkBlended::cTerrainChunkBlended()
{
}

////////////////////////////////////////

cTerrainChunkBlended::~cTerrainChunkBlended()
{
   tSplatBuilders::iterator iter = m_splats.begin();
   tSplatBuilders::iterator end = m_splats.end();
   for (; iter != end; iter++)
   {
      delete *iter;
   }
   m_splats.clear();
}

////////////////////////////////////////

tResult cTerrainChunkBlended::Create(const cRange<uint> xRange, const cRange<uint> zRange,
                                     cTerrainChunk * * ppChunk)
{
   if (ppChunk == NULL)
   {
      return E_POINTER;
   }

   cTerrainChunkBlended * pChunk = new cTerrainChunkBlended;
   if (!pChunk)
   {
      return E_OUTOFMEMORY;
   }

   UseGlobal(TerrainModel);
   UseGlobal(TerrainRenderer);

   pChunk->m_vertices.resize(xRange.GetLength() * zRange.GetLength() * 4);

   typedef std::map<uint, cSplatBuilder *> tSplatBuilderMap;
   tSplatBuilderMap splatBuilders;

   float d = 1.0f / pTerrainRenderer->GetTilesPerChunk();

   for (uint z = zRange.GetStart(); z < zRange.GetEnd(); z++)
   {
      for (uint x = xRange.GetStart(); x < xRange.GetEnd(); x++)
      {
         uint tile;
         Verify(pTerrainModel->GetQuadTile(x, z, &tile) == S_OK);

         sTerrainVertex verts[4];
         pTerrainModel->GetQuadVertices(x, z, verts);

         cSplatBuilder * pSplatBuilder = NULL;

         if (splatBuilders.find(tile) == splatBuilders.end())
         {
            pSplatBuilder = new cSplatBuilder(tile);
            if (pSplatBuilder != NULL)
            {
               splatBuilders[tile] = pSplatBuilder;
            }
         }
         else
         {
            pSplatBuilder = splatBuilders[tile];
         }

         uint iVert = ((z - zRange.GetStart()) * zRange.GetLength() + x - xRange.GetStart()) * 4;

         if (pSplatBuilder != NULL)
         {
            pSplatBuilder->AddTriangle(iVert+2,iVert+1,iVert+0);
            pSplatBuilder->AddTriangle(iVert+0,iVert+3,iVert+2);
         }

         pChunk->m_vertices[iVert+0] = verts[0];
         pChunk->m_vertices[iVert+1] = verts[1];
         pChunk->m_vertices[iVert+2] = verts[2];
         pChunk->m_vertices[iVert+3] = verts[3];

         float u = d * (x - xRange.GetStart());
         float v = d * (z - zRange.GetStart());

         pChunk->m_vertices[iVert+0].uv2 = tVec2(u, v);
         pChunk->m_vertices[iVert+1].uv2 = tVec2(u+d, v);
         pChunk->m_vertices[iVert+2].uv2 = tVec2(u+d, v+d);
         pChunk->m_vertices[iVert+3].uv2 = tVec2(u, v+d);
      }
   }

   tSplatBuilderMap::iterator iter = splatBuilders.begin();
   tSplatBuilderMap::iterator end = splatBuilders.end();
   for (; iter != end; iter++)
   {
      iter->second->BuildAlphaMap(xRange, zRange);
      pChunk->m_splats.push_back(iter->second);
   }
   splatBuilders.clear();

   *ppChunk = pChunk;

   return S_OK;
}

////////////////////////////////////////

void cTerrainChunkBlended::Render(IEditorTileSet * pTileSet)
{
   glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);
   glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

   glDisableClientState(GL_EDGE_FLAG_ARRAY);
   glDisableClientState(GL_INDEX_ARRAY);
   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_NORMAL_ARRAY);
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   glDisableClientState(GL_COLOR_ARRAY);

   const byte * pVertexData = (const byte *)(sTerrainVertex *)&m_vertices[0];

   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(3, GL_FLOAT, sizeof(sTerrainVertex), pVertexData + offsetof(sTerrainVertex, pos));

//   glColor4f(1,1,1,1);

   tSplatBuilders::iterator iter = m_splats.begin();
   tSplatBuilders::iterator end = m_splats.end();
   for (; iter != end; iter++)
   {
      glDisable(GL_BLEND);
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);

      GLuint alphaMapId;
      if ((*iter)->GetAlphaMap(&alphaMapId) == S_OK)
      {
         glTexCoordPointer(2, GL_FLOAT, sizeof(sTerrainVertex), pVertexData + offsetof(sTerrainVertex, uv2));
         glEnableClientState(GL_TEXTURE_COORD_ARRAY);
         glBindTexture(GL_TEXTURE_2D, alphaMapId);
         glEnable(GL_TEXTURE_2D);
//         glBlendFunc(GL_SRC_COLOR, GL_ZERO);
         glDrawElements(GL_TRIANGLES, (*iter)->GetIndexCount(), GL_UNSIGNED_INT, (*iter)->GetIndexPtr());
      }

      glEnable(GL_BLEND);
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

      GLuint texId;
      if ((*iter)->GetGlTexture(pTileSet, &texId) == S_OK)
      {
         glTexCoordPointer(2, GL_FLOAT, sizeof(sTerrainVertex), pVertexData + offsetof(sTerrainVertex, uv1));
         glEnableClientState(GL_TEXTURE_COORD_ARRAY);
         glBindTexture(GL_TEXTURE_2D, texId);
         glEnable(GL_TEXTURE_2D);
//         glBlendFunc(GL_SRC_COLOR, GL_DST_ALPHA);
         glDrawElements(GL_TRIANGLES, (*iter)->GetIndexCount(), GL_UNSIGNED_INT, (*iter)->GetIndexPtr());
      }
   }

   glPopClientAttrib();
   glPopAttrib();
}

/////////////////////////////////////////////////////////////////////////////
