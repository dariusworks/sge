///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdafx.h"

#include "ms3dmesh.h"

#include "filespec.h"
#include "readwriteapi.h"
#include "techmath.h"
#include "animation.h"
#include "vec4.h"
#include "str.h"

#include "render.h"
#include "material.h"
#include "image.h"
#include "color.h"

#include <cfloat>
#include <map>
#include <algorithm>

#include <GL/gl.h>
#include "GL/glext.h"

#include <Cg/cg.h>
#include <Cg/cgGL.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// REFERENCES
// http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=31
// http://rsn.gamedev.net/tutorials/ms3danim.asp

extern PFNGLVERTEXWEIGHTFEXTPROC glVertexWeightfEXT;
extern PFNGLVERTEXWEIGHTFVEXTPROC glVertexWeightfvEXT;
extern PFNGLVERTEXWEIGHTPOINTEREXTPROC glVertexWeightPointerEXT;

///////////////////////////////////////////////////////////////////////////////

CGcontext g_cgContext = NULL;
ulong g_nCgContextRefs = 0;

CGprofile g_cgProfile = CG_PROFILE_UNKNOWN;

static CGcontext GetCgContext()
{
   if (g_cgContext == NULL)
   {
      g_cgContext = cgCreateContext();
   }
   ++g_nCgContextRefs;
   return g_cgContext;
}

static void ReleaseCgContext()
{
   if (--g_nCgContextRefs == 0)
   {
      cgDestroyContext(g_cgContext);
      g_cgContext = NULL;
   }
}

// used as the 4th dimension when a 3D vector is passed to a 4D function
static const tVec4::value_type k4thDimension = 1;

static const char g_MS3D[] = "MS3D000000";

///////////////////////////////////////////////////////////////////////////////

template <>
std::vector<IKeyFrameInterpolator *>::~vector()
{
   std::for_each(begin(), end(), CTInterfaceMethodRef(&IUnknown::Release));
   clear();
}

template <>
std::vector<IMaterial *>::~vector()
{
   std::for_each(begin(), end(), CTInterfaceMethodRef(&IUnknown::Release));
   clear();
}

///////////////////////////////////////////////////////////////////////////////

void cgErrorCallback()
{
   CGerror lastError = cgGetError();

   if (lastError)
   {
      DebugPrintf(NULL, 0, cgGetErrorString(lastError));

      const char * pszListing = cgGetLastListing(g_cgContext);
      if (pszListing != NULL)
      {
         DebugPrintf(NULL, 0, "   %s\n", pszListing);
      }
   }
}

///////////////////////////////////////////////////////////////////////////////

static void MatrixFromAngles(tVec3 angles, tMatrix4 * pMatrix)
{
   tMatrix4 rotX, rotY, rotZ, temp1, temp2;
   MatrixRotateX(Rad2Deg(angles.x), &rotX);
   MatrixRotateY(Rad2Deg(angles.y), &rotY);
   MatrixRotateZ(Rad2Deg(angles.z), &rotZ);
   temp1 = rotZ;
   temp2 = temp1 * rotY;
   *pMatrix = temp2 * rotX;
}

///////////////////////////////////////////////////////////////////////////////

static char * GetResource(const char * pResId, const char * pResType)
{
   HRSRC hR = FindResource(AfxGetInstanceHandle(), pResId, pResType);
   if (hR)
   {
      uint resSize = SizeofResource(AfxGetInstanceHandle(), hR);
      HGLOBAL hG = LoadResource(AfxGetInstanceHandle(), hR);
      if (hG)
      {
         void * pResData = LockResource(hG);
         if (pResData)
         {
            char * pszContents = new char[resSize + 1];
            strcpy(pszContents, (const char *)pResData);
            return pszContents;
         }
      }
   }
   return NULL;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dGroup
//

///////////////////////////////////////

cMs3dGroup::cMs3dGroup()
 : materialIndex(-1)
{
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cReadWriteOps<cMs3dGroup>
//

template <>
class cReadWriteOps<cMs3dGroup>
{
public:
   static tResult Read(IReader * pReader, cMs3dGroup * pGroup);
};

///////////////////////////////////////

tResult cReadWriteOps<cMs3dGroup>::Read(IReader * pReader, cMs3dGroup * pGroup)
{
   Assert(pReader != NULL);
   Assert(pGroup != NULL);

   tResult result = E_FAIL;

   do
   {
      byte flags; // SELECTED | HIDDEN
      if (pReader->Read(&flags, sizeof(flags)) != S_OK)
         break;

      if (pReader->Read(pGroup->name, sizeof(pGroup->name)) != S_OK)
         break;

      uint16 nTriangles;
      if (pReader->Read(&nTriangles, sizeof(nTriangles)) != S_OK)
         break;

      pGroup->triangleIndices.resize(nTriangles);

      if (pReader->Read(&pGroup->triangleIndices[0], pGroup->triangleIndices.size() * sizeof(uint16)) != S_OK)
         break;

      if (pReader->Read(&pGroup->materialIndex, sizeof(pGroup->materialIndex)) != S_OK)
         break;

      result = S_OK;
   }
   while (0);

   return result;
}


///////////////////////////////////////////////////////////////////////////////

static tResult ReadKeyFrames(IReader * pReader, 
                             std::vector<sKeyFrameVec3> * pTranslationFrames,
                             std::vector<sKeyFrameQuat> * pRotationFrames)
{
   Assert(pReader != NULL);
   Assert(pTranslationFrames != NULL);
   Assert(pRotationFrames != NULL);

   tResult result = E_FAIL;

   do
   {
      uint16 nKeyFramesRot, nKeyFramesTrans;

      if (pReader->Read(&nKeyFramesRot, sizeof(nKeyFramesRot)) != S_OK)
         break;
      if (pReader->Read(&nKeyFramesTrans, sizeof(nKeyFramesTrans)) != S_OK)
         break;

      if (nKeyFramesRot != nKeyFramesTrans)
      {
         DebugMsg2("Have %d rotation and %d translation key frames\n", nKeyFramesRot, nKeyFramesTrans);
         break;
      }

      std::vector<ms3d_keyframe_rot_t> rotationKeys(nKeyFramesRot);
      if (pReader->Read(&rotationKeys[0], rotationKeys.size() * sizeof(ms3d_keyframe_rot_t)) != S_OK)
         break;

      std::vector<ms3d_keyframe_pos_t> translationKeys(nKeyFramesTrans);
      if (pReader->Read(&translationKeys[0], translationKeys.size() * sizeof(ms3d_keyframe_pos_t)) != S_OK)
         break;

      pTranslationFrames->resize(nKeyFramesTrans);
      for (unsigned i = 0; i < nKeyFramesTrans; i++)
      {
         (*pTranslationFrames)[i].time = translationKeys[i].time;
         (*pTranslationFrames)[i].value = tVec3(translationKeys[i].position);
      }

      pRotationFrames->resize(nKeyFramesRot);
      for (i = 0; i < nKeyFramesRot; i++)
      {
         (*pRotationFrames)[i].time = rotationKeys[i].time;
         (*pRotationFrames)[i].value = QuatFromEulerAngles(tVec3(rotationKeys[i].rotation));
      }

      result = S_OK;
   }
   while (0);

   return result;
}


///////////////////////////////////////////////////////////////////////////////

struct sMs3dBoneInfo
{
   char name[kMaxBoneName];
   char parentName[kMaxBoneName];
   float rotation[3];
   float position[3];
};

template <>
class cReadWriteOps<sMs3dBoneInfo>
{
public:
   static tResult Read(IReader * pReader, sMs3dBoneInfo * pBoneInfo);
};

tResult cReadWriteOps<sMs3dBoneInfo>::Read(IReader * pReader, sMs3dBoneInfo * pBoneInfo)
{
   Assert(pReader != NULL);
   Assert(pBoneInfo != NULL);

   byte flags; // SELECTED | DIRTY

   if (pReader->Read(&flags, sizeof(flags)) != S_OK
      || pReader->Read(pBoneInfo->name, sizeof(pBoneInfo->name)) != S_OK
      || pReader->Read(pBoneInfo->parentName, sizeof(pBoneInfo->parentName)) != S_OK
      || pReader->Read(pBoneInfo->rotation, sizeof(pBoneInfo->rotation)) != S_OK
      || pReader->Read(pBoneInfo->position, sizeof(pBoneInfo->position)) != S_OK)
   {
      return E_FAIL;
   }

   return S_OK;
}

static tResult ReadSkeleton(IReader * pReader, 
                            std::vector<sBoneInfo> * pBones,
                            std::vector<IKeyFrameInterpolator *> * pInterpolators)
{
   Assert(pReader != NULL);
   Assert(pBones != NULL);
   Assert(pInterpolators != NULL);

   tResult result = E_FAIL;

   do
   {
      float animationFPS;
      float currentTime;
      int nTotalFrames;
      uint16 nJoints;
      if (pReader->Read(&animationFPS, sizeof(animationFPS)) != S_OK
         || pReader->Read(&currentTime, sizeof(currentTime)) != S_OK
         || pReader->Read(&nTotalFrames, sizeof(nTotalFrames)) != S_OK
         || pReader->Read(&nJoints, sizeof(nJoints)) != S_OK)
      {
         break;
      }

      std::map<cStr, int> boneNames; // map bone names to indices

      std::vector<sMs3dBoneInfo> boneInfo(nJoints);

      pInterpolators->resize(nJoints);

      uint i;
      for (i = 0; i < nJoints; i++)
      {
         if (pReader->Read(&boneInfo[i]) != S_OK)
            break;

         boneNames.insert(std::make_pair(cStr(boneInfo[i].name), i));

         std::vector<sKeyFrameVec3> translationFrames;
         std::vector<sKeyFrameQuat> rotationFrames;
         if (ReadKeyFrames(pReader, &translationFrames, &rotationFrames) != S_OK)
         {
            break;
         }

         IKeyFrameInterpolator * pInterpolator = NULL;
         if (KeyFrameInterpolatorCreate(boneInfo[i].name, NULL, 0,
            &rotationFrames[0], rotationFrames.size(),
            &translationFrames[0], rotationFrames.size(),
            &pInterpolator) != S_OK)
         {
            SafeRelease(pInterpolator);
            break;
         }

         (*pInterpolators)[i] = pInterpolator;
      }

      if (i < nJoints)
         break;

      pBones->resize(nJoints);

      for (i = 0; i < nJoints; i++)
      {
         strcpy((*pBones)[i].name, boneInfo[i].name);

         if (strlen(boneInfo[i].parentName) > 0)
         {
            std::map<cStr, int>::iterator n = boneNames.find(boneInfo[i].parentName);
            if (n != boneNames.end())
            {
               Assert(strcmp(boneInfo[n->second].name, boneInfo[i].parentName) == 0);
               (*pBones)[i].parentIndex = n->second;
            }
         }

         tMatrix4 mt, mr;
         MatrixTranslate(boneInfo[i].position[0], boneInfo[i].position[1], boneInfo[i].position[2], &mt);
         MatrixFromAngles(tVec3(boneInfo[i].rotation), &mr);

         (*pBones)[i].localTransform = mt * mr;
      }

      result = S_OK;
   }
   while (0);

   return result;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dMesh
//

cMs3dMesh::cMs3dMesh()
 : m_pfnRender(RenderSoftware),
   m_bCalculatedAABB(false),
   m_program(NULL),
   m_modelViewProjParam(NULL),
   m_pSkeleton(NULL)
{
}

cMs3dMesh::~cMs3dMesh()
{
   SafeRelease(m_pSkeleton);

   m_vertices.clear();
   m_triangles.clear();
   m_groups.clear();

   std::for_each(m_materials.begin(), m_materials.end(), CTInterfaceMethodRef(&IUnknown::Release));
   m_materials.clear();

   m_bCalculatedAABB = false;

   if (m_program != NULL)
   {
      cgDestroyProgram(m_program);
      m_program = NULL;
   }

   ReleaseCgContext();
}

void cMs3dMesh::GetAABB(tVec3 * pMaxs, tVec3 * pMins) const
{
   if (!m_bCalculatedAABB)
   {
      m_maxs = tVec3(FLT_MIN, FLT_MIN, FLT_MIN);
      m_mins = tVec3(FLT_MAX, FLT_MAX, FLT_MAX);

      tVertices::const_iterator iter;
      for (iter = m_vertices.begin(); iter != m_vertices.end(); iter++)
      {
         if (m_maxs.x < iter->vertex[0])
            m_maxs.x = iter->vertex[0];
         if (m_mins.x > iter->vertex[0])
            m_mins.x = iter->vertex[0];

         if (m_maxs.y < iter->vertex[1])
            m_maxs.y = iter->vertex[1];
         if (m_mins.y > iter->vertex[1])
            m_mins.y = iter->vertex[1];

         if (m_maxs.z < iter->vertex[2])
            m_maxs.z = iter->vertex[2];
         if (m_mins.z > iter->vertex[2])
            m_mins.z = iter->vertex[2];
      }

      m_bCalculatedAABB = true;
   }

   Assert(pMaxs && pMins);
   *pMaxs = m_maxs;
   *pMins = m_mins;
}

void cMs3dMesh::Render(IRenderDevice * pRenderDevice) const
{
   Assert(m_pfnRender != NULL);
   (this->*m_pfnRender)();
}

tResult cMs3dMesh::AddMaterial(IMaterial * pMaterial)
{
   return E_NOTIMPL;
}

tResult cMs3dMesh::FindMaterial(const char * pszName, IMaterial * * ppMaterial) const
{
   return E_NOTIMPL;
}

tResult cMs3dMesh::AddSubMesh(ISubMesh * pSubMesh)
{
   return E_NOTIMPL;
}

tResult cMs3dMesh::AttachSkeleton(ISkeleton * pSkeleton)
{
   SafeRelease(m_pSkeleton);
   m_pSkeleton = pSkeleton;
   if (m_pSkeleton)
   {
      m_pSkeleton->AddRef();
   }
   return S_OK;
}

tResult cMs3dMesh::GetSkeleton(ISkeleton * * ppSkeleton)
{
   if (ppSkeleton != NULL)
   {
      *ppSkeleton = static_cast<ISkeleton *>(m_pSkeleton);
      if (*ppSkeleton != NULL)
      {
         (*ppSkeleton)->AddRef();
         return S_OK;
      }
      else
      {
         return S_FALSE;
      }
   }
   return E_FAIL;
}

tResult cMs3dMesh::Read(IReader * pReader, IRenderDevice * pRenderDevice, IResourceManager * pResourceManager)
{
   Assert(m_vertices.empty());
   Assert(m_triangles.empty());
   Assert(m_groups.empty());
   Assert(m_materials.empty());
   Assert(!m_pSkeleton);

   ms3d_header_t header;
   if (pReader->Read(&header, sizeof(header)) != S_OK ||
      memcmp(g_MS3D, header.id, _countof(header.id)) != 0)
      return E_FAIL;

   uint16 nVertices;
   if (pReader->Read(&nVertices, sizeof(nVertices)) != S_OK
      || nVertices == 0)
      return E_FAIL;

   m_vertices.resize(nVertices);
   if (pReader->Read(&m_vertices[0], m_vertices.size() * sizeof(ms3d_vertex_t)) != S_OK)
      return E_FAIL;

   uint16 nTriangles;
   if (pReader->Read(&nTriangles, sizeof(nTriangles)) != S_OK
      || nTriangles == 0)
      return E_FAIL;

   m_triangles.resize(nTriangles);
   if (pReader->Read(&m_triangles[0], m_triangles.size() * sizeof(ms3d_triangle_t)) != S_OK)
      return E_FAIL;

   uint16 nGroups;
   if (pReader->Read(&nGroups, sizeof(nGroups)) != S_OK)
      return E_FAIL;

   uint i;

   m_groups.resize(nGroups);
   for (i = 0; i < nGroups; i++)
   {
      if (pReader->Read(&m_groups[i]) != S_OK)
         return E_FAIL;
   }

   uint16 nMaterials;
   if (pReader->Read(&nMaterials, sizeof(nMaterials)) != S_OK)
      return E_FAIL;

   for (i = 0; i < nMaterials; i++)
   {
      ms3d_material_t material;
      if (pReader->Read(&material, sizeof(material)) != S_OK)
         return E_FAIL;

      cAutoIPtr<ITexture> pTexture;

      if (material.texture[0] != 0)
      {
         cImage * pTextureImage = ImageLoad(pResourceManager, material.texture);
         if (pTextureImage == NULL)
         {
            DebugMsg1("Could not load texture image %s\n", material.texture);
            return E_FAIL;
         }

         if (pRenderDevice->CreateTexture(pTextureImage, &pTexture) != S_OK)
         {
            DebugMsg1("Could not create device texture for %s\n", material.texture);
            delete pTextureImage;
            return E_FAIL;
         }

         delete pTextureImage;
      }

      IMaterial * pMaterial = MaterialCreate();

      if (pMaterial != NULL)
      {
         pMaterial->SetName(material.name);
         pMaterial->SetAmbient(cColor(material.ambient));
         pMaterial->SetDiffuse(cColor(material.diffuse));
         pMaterial->SetSpecular(cColor(material.specular));
         pMaterial->SetEmissive(cColor(material.emissive));
         pMaterial->SetShininess(material.shininess);
         pMaterial->SetTexture(0, pTexture);

         m_materials.push_back(pMaterial);
      }
   }

   Assert(m_materials.size() == nMaterials);

   std::vector<sBoneInfo> bones;
   std::vector<IKeyFrameInterpolator *> interpolators;

   if (ReadSkeleton(pReader, &bones, &interpolators) != S_OK)
      return E_FAIL;

   cAutoIPtr<ISkeleton> pSkeleton;
   if (SkeletonCreate(&bones[0], bones.size(), &interpolators[0], interpolators.size(), &pSkeleton) == S_OK)
   {
      AttachSkeleton(pSkeleton);

      m_boneMatrices.resize(pSkeleton->GetBoneCount());

      tMatrices inverses(pSkeleton->GetBoneCount());

      for (int i = 0; i < inverses.size(); i++)
      {
         MatrixInvert(pSkeleton->GetBoneWorldTransform(i), &inverses[i]);
      }

      // transform all vertices by the inverse of the affecting bone's absolute matrix
      tVertices::iterator vertIter;
      for (vertIter = m_vertices.begin(); vertIter != m_vertices.end(); vertIter++)
      {
         if (vertIter->boneId != -1)
         {
            const tMatrix4 & m = inverses[vertIter->boneId];
            tVec4 v2, v(vertIter->vertex[0], vertIter->vertex[1], vertIter->vertex[2], k4thDimension);
            v2 = m.Transform(v);
            vertIter->vertex[0] = v2.x;
            vertIter->vertex[1] = v2.y;
            vertIter->vertex[2] = v2.z;
         }
      }

      // transform the vertex normals as well
      tTriangles::iterator triIter;
      for (triIter = m_triangles.begin(); triIter != m_triangles.end(); triIter++)
      {
         for (int i = 0; i < 3; i++)
         {
            ms3d_vertex_t & v = m_vertices[triIter->vertexIndices[i]];
            if (v.boneId != -1)
            {
               const tMatrix4 & m = inverses[v.boneId];
               tVec4 n(
                  triIter->vertexNormals[i][0],
                  triIter->vertexNormals[i][1],
                  triIter->vertexNormals[i][2],
                  k4thDimension);
               tVec4 nprime;
               nprime = m.Transform(n);
               triIter->vertexNormals[i][0] = nprime.x;
               triIter->vertexNormals[i][1] = nprime.y;
               triIter->vertexNormals[i][2] = nprime.z;
            }
         }
      }

      SetFrame(0);

      cgSetErrorCallback(cgErrorCallback);

      g_cgProfile = cgGLGetLatestProfile(CG_GL_VERTEX);

      if (g_cgProfile != CG_PROFILE_UNKNOWN)
      {
         char * pszProgram = GetResource("ms3dmeshanim.cg", "CG");

         if (pszProgram != NULL)
         {
            m_program = cgCreateProgram(GetCgContext(), CG_SOURCE, pszProgram,
               g_cgProfile, NULL, NULL);

            delete [] pszProgram;

            if (m_program != NULL)
            {
               cgGLLoadProgram(m_program);

               m_modelViewProjParam = cgGetNamedParameter(m_program, "modelViewProj");

               m_pfnRender = RenderVertexProgram;
            }
         }
      }
   }

   return S_OK;
}

void cMs3dMesh::SetFrame(float percent)
{
   if (!!m_pSkeleton)
   {
      m_pSkeleton->GetBoneMatrices(percent, &m_boneMatrices);
   }
}

void cMs3dMesh::RenderSoftware() const
{
   tGroups::const_iterator iter;
   for (iter = m_groups.begin(); iter != m_groups.end(); iter++)
   {
      glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);

      if (iter->GetMaterialIndex() > -1)
      {
         IMaterial * pMaterial = const_cast<IMaterial *>(m_materials[iter->GetMaterialIndex()]);
         GlMaterial(pMaterial);
      }

      glBegin(GL_TRIANGLES);

      const std::vector<uint16> & tris = iter->GetTriangleIndices();
      std::vector<uint16>::const_iterator iterTris;
      for (iterTris = tris.begin(); iterTris != tris.end(); iterTris++)
      {
         const ms3d_triangle_t & tri = m_triangles[*iterTris];

         for (int k = 0; k < 3; k++)
         {
            glTexCoord2f(tri.s[k], 1.0f - tri.t[k]);
            const ms3d_vertex_t & vk = m_vertices[tri.vertexIndices[k]];
            if (vk.boneId == -1)
            {
               glNormal3fv(tri.vertexNormals[k]);
               glVertex3fv(vk.vertex);
            }
            else
            {
               const tMatrix4 & m = m_boneMatrices[vk.boneId];

               tVec4 nprime, n(tri.vertexNormals[k][0], tri.vertexNormals[k][1], tri.vertexNormals[k][2], k4thDimension);
               nprime = m.Transform(n);
               glNormal3fv(nprime.v);

               tVec4 vprime, v(vk.vertex[0], vk.vertex[1], vk.vertex[2], k4thDimension);
               vprime = m.Transform(v);
               glVertex3fv(vprime.v);
            }
         }
      }

      glEnd();

      glPopAttrib();
   }
}

void cMs3dMesh::RenderVertexProgram() const
{
   cgGLBindProgram(m_program);

   cgGLEnableProfile(g_cgProfile);

   cgGLSetStateMatrixParameter(m_modelViewProjParam, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);

   tGroups::const_iterator iter;
   for (iter = m_groups.begin(); iter != m_groups.end(); iter++)
   {
      glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);

      if (iter->GetMaterialIndex() > -1)
      {
         IMaterial * pMaterial = const_cast<IMaterial *>(m_materials[iter->GetMaterialIndex()]);
         GlMaterial(pMaterial);
      }

      glBegin(GL_TRIANGLES);

      const std::vector<uint16> & tris = iter->GetTriangleIndices();
      std::vector<uint16>::const_iterator iterTris;
      for (iterTris = tris.begin(); iterTris != tris.end(); iterTris++)
      {
         const ms3d_triangle_t & tri = m_triangles[*iterTris];

         for (int k = 0; k < 3; k++)
         {
            const ms3d_vertex_t & vk = m_vertices[tri.vertexIndices[k]];
            if (vk.boneId != -1)
            {
               //const tMatrix4 & m = m_joints[vk.boneId].GetFinalMatrix();

               glTexCoord2f(tri.s[k], 1.0f - tri.t[k]);
               glNormal3fv(tri.vertexNormals[k]);
               glVertex3fv(vk.vertex);
            }
         }
      }

      glEnd();

      glPopAttrib();
   }

   cgGLDisableProfile(g_cgProfile);
}

///////////////////////////////////////////////////////////////////////////////
