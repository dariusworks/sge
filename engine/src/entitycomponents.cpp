///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entitycomponents.h"

#include "renderapi.h"

#include "color.h"
#include "globalobj.h"
#include "multivar.h"
#include "resourceapi.h"

#include <tinyxml.h>

#include <GL/glew.h>

#include <algorithm>
#include <cfloat>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

#define IsFlagSet(f, b) (((f)&(b))==(b))

extern tResult ModelEntityCreate(tEntityId id, const tChar * pszModel, const tVec3 & position, IEntity * * ppEntity);


///////////////////////////////////////////////////////////////////////////////

const sVertexElement g_modelVert[] =
{
   { kVEU_TexCoord,  kVET_Float2,   0, 0 },
   { kVEU_Normal,    kVET_Float3,   0, 2 * sizeof(float) },
   { kVEU_Position,  kVET_Float3,   0, 5 * sizeof(float) },
   { kVEU_Index,     kVET_Float1,   0, 8 * sizeof(float) },
};

const sVertexElement g_blendedVert[] =
{
   { kVEU_TexCoord,  kVET_Float2,   0, 0 },
   { kVEU_Normal,    kVET_Float3,   0, 2 * sizeof(float) },
   { kVEU_Position,  kVET_Float3,   0, 5 * sizeof(float) },
};


////////////////////////////////////////////////////////////////////////////////

static void CalculateBBox(const tModelVertices & vertices, tAxisAlignedBox * pBBox)
{
   tVec3 mins(FLT_MAX, FLT_MAX, FLT_MAX), maxs(-FLT_MAX, -FLT_MAX, -FLT_MAX);
   tModelVertices::const_iterator iter = vertices.begin();
   for (; iter != vertices.end(); iter++)
   {
      if (iter->pos.x < mins.x)
      {
         mins.x = iter->pos.x;
      }
      if (iter->pos.y < mins.y)
      {
         mins.y = iter->pos.y;
      }
      if (iter->pos.z < mins.z)
      {
         mins.z = iter->pos.z;
      }
      if (iter->pos.x > maxs.x)
      {
         maxs.x = iter->pos.x;
      }
      if (iter->pos.y > maxs.y)
      {
         maxs.y = iter->pos.y;
      }
      if (iter->pos.z > maxs.z)
      {
         maxs.z = iter->pos.z;
      }
   }
   *pBBox = tAxisAlignedBox(mins, maxs);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityRenderComponent
//

///////////////////////////////////////

cEntityRenderComponent::cEntityRenderComponent(const tChar * pszModel)
 : m_model(pszModel ? pszModel : _T(""))
 , m_pModel(NULL)
{
}

///////////////////////////////////////

cEntityRenderComponent::~cEntityRenderComponent()
{
}

///////////////////////////////////////

tResult cEntityRenderComponent::GetModel(cStr * pModel) const
{
   if (pModel == NULL)
   {
      return E_POINTER;
   }
   if (m_model.empty())
   {
      return S_FALSE;
   }
   *pModel = m_model;
   return S_OK;
}

///////////////////////////////////////

tResult cEntityRenderComponent::GetBoundingBox(tAxisAlignedBox * pBBox) const
{
   if (pBBox == NULL)
   {
      return E_POINTER;
   }
   *pBBox = m_bbox;
   return S_OK;
}

///////////////////////////////////////

void cEntityRenderComponent::Update(double elapsedTime)
{
   // TODO: When the resource manager is fast enough, this if test should be removed
   // to get a new cModel pointer whenever the resource changes, e.g., by re-saving
   // from a 3D modelling program (3DS Max, or MilkShape, or something).
#if 1
   if (m_pModel == NULL)
#endif
   {
      UseGlobal(ResourceManager);
      cModel * pModel = NULL;
      if (pResourceManager->Load(m_model.c_str(), kRT_Model, NULL, (void**)&pModel) != S_OK)
      {
         m_pModel = NULL;
         return;
      }

      AssertMsg(pModel != NULL, _T("Should have returned by now if ResourceManager->Load failed\n"));

      if (pModel != m_pModel)
      {
         m_pModel = pModel;

         cAutoIPtr<IModelSkeleton> pSkeleton;
         if (m_pModel->GetSkeleton(&pSkeleton) == S_OK)
         {
            size_t nJoints = 0;
            if (pSkeleton->GetJointCount(&nJoints) == S_OK && nJoints > 0)
            {
               ModelAnimationControllerCreate(pSkeleton, &m_pAnimController);
            }
         }

         CalculateBBox(m_pModel->GetVertices(), &m_bbox);
      }
   }

   if (!!m_pAnimController)
   {
      m_pAnimController->Advance(elapsedTime);
      m_pModel->ApplyJointMatrices(m_pAnimController->GetBlendMatrices(), &m_blendedVerts);
   }
}

///////////////////////////////////////

void cEntityRenderComponent::Render()
{
   // TODO: When the resource manager is fast enough, this code should be used to
   // get a new cModel pointer in case the resource was changed, e.g., by re-saving
   // from a 3D modelling program (3DS Max, or MilkShape, or something).
#if 0
   UseGlobal(ResourceManager);
   cModel * pModel = NULL;
   if (pResourceManager->Load(m_model.c_str(), kRT_Model, NULL, (void**)&pModel) != S_OK
      || (pModel != m_pModel))
   {
      m_pModel = NULL;
      return;
   }
#else
   if (m_pModel == NULL)
   {
      return;
   }
#endif

   UseGlobal(Renderer);

   if (!m_blendedVerts.empty())
   {
      pRenderer->SetVertexFormat(g_blendedVert, _countof(g_blendedVert));
      pRenderer->SubmitVertices(&m_blendedVerts[0], m_blendedVerts.size());
   }
   else
   {
      pRenderer->SetVertexFormat(g_modelVert, _countof(g_modelVert));
      const tModelVertices & verts = m_pModel->GetVertices();
      pRenderer->SubmitVertices(const_cast<sModelVertex *>(&verts[0]), verts.size());
   }

   pRenderer->SetIndexFormat(kIF_16Bit);

   tModelMeshes::const_iterator iter = m_pModel->BeginMeshses();
   tModelMeshes::const_iterator end = m_pModel->EndMeshses();
   for (; iter != end; iter++)
   {
      int iMaterial = iter->GetMaterialIndex();
      if (iMaterial >= 0)
      {
         m_pModel->GetMaterial(iMaterial).GlDiffuseAndTexture();
      }
      pRenderer->Render(iter->GetPrimitiveType(), const_cast<uint16*>(iter->GetIndexData()), iter->GetIndexCount());
   }
}

///////////////////////////////////////

tResult EntityRenderComponentFactory(const TiXmlElement * pTiXmlElement,
                                     IEntity * pEntity, IEntityComponent * * ppComponent)
{
   if (pTiXmlElement == NULL || pEntity == NULL || ppComponent == NULL)
   {
      return E_POINTER;
   }

   if (_stricmp(pTiXmlElement->Value(), "render") != 0)
   {
      return E_INVALIDARG;
   }

   if (pTiXmlElement->Attribute("model") == NULL)
   {
      return E_FAIL;
   }

#ifdef _UNICODE
   cMultiVar temp(pTiXmlElement->Attribute("model"));
   const wchar_t * pszModel = temp.ToWideString();
#else
   const char * pszModel = pTiXmlElement->Attribute("model");
#endif

   cAutoIPtr<cEntityRenderComponent> pRender = new cEntityRenderComponent(pszModel);
   if (!pRender)
   {
      return E_OUTOFMEMORY;
   }

   if (pEntity->AddComponent(IID_IEntityRenderComponent, static_cast<IEntityComponent*>(pRender)) != S_OK)
   {
      return E_FAIL;
   }

   *ppComponent = CTAddRef(static_cast<IEntityComponent*>(pRender));
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntitySpawnComponent
//

///////////////////////////////////////

cEntitySpawnComponent::cEntitySpawnComponent()
 : m_maxQueueSize(0)
{
}

///////////////////////////////////////

cEntitySpawnComponent::~cEntitySpawnComponent()
{
}

///////////////////////////////////////

uint cEntitySpawnComponent::GetMaxQueueSize() const
{
   return m_maxQueueSize;
}

///////////////////////////////////////

tResult cEntitySpawnComponent::SetRallyPoint(const tVec3 & rallyPoint)
{
   m_rallyPoint = rallyPoint;
   return S_OK;
}

///////////////////////////////////////

tResult cEntitySpawnComponent::GetRallyPoint(tVec3 * pRallyPoint) const
{
   if (pRallyPoint == NULL)
   {
      return E_POINTER;
   }
   *pRallyPoint = m_rallyPoint;
   return S_OK;
}

///////////////////////////////////////

tResult cEntitySpawnComponent::Spawn(const tChar * pszEntity)
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult EntitySpawnComponentFactory(const TiXmlElement * pTiXmlElement,
                                    IEntity * pEntity, IEntityComponent * * ppComponent)
{
   if (pTiXmlElement == NULL || pEntity == NULL || ppComponent == NULL)
   {
      return E_POINTER;
   }

//   if (pEntity->AddComponent(componentId, pComponent) != S_OK)
   {
   }

   return E_NOTIMPL;
}


///////////////////////////////////////////////////////////////////////////////

void RegisterBuiltInComponents()
{
   UseGlobal(EntityManager);
   Verify(pEntityManager->RegisterComponentFactory(_T("render"), EntityRenderComponentFactory) == S_OK);
   Verify(pEntityManager->RegisterComponentFactory(_T("spawn"), EntitySpawnComponentFactory) == S_OK);
}


///////////////////////////////////////////////////////////////////////////////
