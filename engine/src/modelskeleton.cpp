///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "modelskeleton.h"

#include <algorithm>
#include <cfloat>
#include <stack>

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelSkeleton
//

///////////////////////////////////////

cModelSkeleton::cModelSkeleton()
{
}

///////////////////////////////////////

cModelSkeleton::cModelSkeleton(const cModelSkeleton & other)
{
   operator =(other);
}

///////////////////////////////////////

cModelSkeleton::cModelSkeleton(const tModelJoints & joints)
 : m_joints(joints.size())
{
   if (!joints.empty())
   {
      std::copy(joints.begin(), joints.end(), m_joints.begin());
   }
}

///////////////////////////////////////

cModelSkeleton::~cModelSkeleton()
{
   std::multimap<eModelAnimationType, IModelAnimation *>::iterator iter = m_anims.begin();
   for (; iter != m_anims.end(); iter++)
   {
      iter->second->Release();
   }
}

///////////////////////////////////////

const cModelSkeleton & cModelSkeleton::operator =(const cModelSkeleton & other)
{
   m_joints.resize(other.m_joints.size());
   std::copy(other.m_joints.begin(), other.m_joints.end(), m_joints.begin());
   tAnimMap::const_iterator iter = other.m_anims.begin();
   for (; iter != other.m_anims.end(); iter++)
   {
      m_anims.insert(std::make_pair(iter->first, CTAddRef(iter->second)));
   }
   return *this;
}

///////////////////////////////////////

tResult cModelSkeleton::GetJointCount(size_t * pJointCount) const
{
   if (pJointCount == NULL)
   {
      return E_POINTER;
   }
   *pJointCount = m_joints.size();
   return S_OK;
}

///////////////////////////////////////

tResult cModelSkeleton::GetJoint(size_t iJoint, sModelJoint * pJoint) const
{
   if (iJoint >= m_joints.size())
   {
      return E_INVALIDARG;
   }
   if (pJoint == NULL)
   {
      return E_POINTER;
   }
   *pJoint = m_joints[iJoint];
   return S_OK;
}

///////////////////////////////////////

tResult cModelSkeleton::GetBindMatrices(size_t nMaxMatrices, tMatrix4 * pMatrices) const
{
   if (nMaxMatrices < m_joints.size())
   {
      return E_INVALIDARG;
   }

   if (pMatrices == NULL)
   {
      return E_POINTER;
   }

   uint i;
   int iRootJoint = -1;
   std::multimap<int, int> jointChildMap;
   tModelJoints::const_iterator iter = m_joints.begin();
   for (i = 0; iter != m_joints.end(); iter++, i++)
   {
      int iParent = iter->parentIndex;
      if (iParent >= 0)
      {
         jointChildMap.insert(std::make_pair(iParent, i));
      }
      else
      {
         Assert(iRootJoint == -1);
         iRootJoint = i;
      }
   }

   if (iRootJoint < 0)
   {
      ErrorMsg("Bad set of joints: no root\n");
      return E_FAIL;
   }

   tMatrices absolutes(m_joints.size(), tMatrix4::GetIdentity());

   std::stack<int> s;
   s.push(iRootJoint);
   while (!s.empty())
   {
      int iJoint = s.top();
      s.pop();

      int iParent = m_joints[iJoint].parentIndex;
      if (iParent == -1)
      {
         absolutes[iJoint] = m_joints[iJoint].localTransform;
      }
      else
      {
         absolutes[iParent].Multiply(m_joints[iJoint].localTransform, &absolutes[iJoint]);
      }

      std::multimap<int, int>::iterator iter = jointChildMap.lower_bound(iJoint);
      std::multimap<int, int>::iterator end = jointChildMap.upper_bound(iJoint);
      for (; iter != end; iter++)
      {
         s.push(iter->second);
      }
   }

   for (i = 0; i < m_joints.size(); i++)
   {
      MatrixInvert(absolutes[i].m, pMatrices[i].m);
   }

   return S_OK;
}

///////////////////////////////////////

void cModelSkeleton::InterpolateMatrices(IModelAnimation * pAnim, double time, tMatrices * pMatrices) const
{
   pMatrices->resize(m_joints.size());

   tModelJoints::const_iterator iter = m_joints.begin();
   tModelJoints::const_iterator end = m_joints.end();
   for (uint i = 0; iter != end; iter++, i++)
   {
      tVec3 position;
      tQuat rotation;
      if (pAnim->Interpolate(i, time, &position, &rotation) == S_OK)
      {
         tMatrix4 mt, mr;

         rotation.ToMatrix(&mr);
         MatrixTranslate(position.x, position.y, position.z, &mt);

         tMatrix4 temp;
         mt.Multiply(mr, &temp);

         tMatrix4 mf;
         iter->localTransform.Multiply(temp, &mf);

         int iParent = iter->parentIndex;
         if (iParent < 0)
         {
            temp = mf;
         }
         else
         {
            (*pMatrices)[iParent].Multiply(mf, &temp);
         }

         (*pMatrices)[i] = temp;
      }
   }
}

///////////////////////////////////////

tResult cModelSkeleton::AddAnimation(eModelAnimationType type,
                                     IModelAnimation * pAnim)
{
   if (pAnim == NULL)
   {
      return E_POINTER;
   }
   m_anims.insert(std::make_pair(type, CTAddRef(pAnim)));
   return S_OK;
}

///////////////////////////////////////

tResult cModelSkeleton::GetAnimation(eModelAnimationType type,
                                     IModelAnimation * * ppAnim) const
{
   if (ppAnim == NULL)
   {
      return E_POINTER;
   }
   
   tAnimMap::const_iterator first = m_anims.lower_bound(type);
   if (first == m_anims.end())
   {
      return S_FALSE;
   }

   tAnimMap::const_iterator last = m_anims.upper_bound(type);

   if (first == last)
   {
      *ppAnim = CTAddRef(first->second);
      return S_OK;
   }
   else
   {
      tAnimMap::difference_type nAnims = 0;
      tAnimMap::const_iterator iter = first;
      for (; iter != last; iter++)
      {
         nAnims++;
      }
      uint i, iAnim = Rand() % nAnims;
      for (i = 0, iter = first; iter != last; i++, iter++)
      {
         if (i == iAnim)
         {
            break;
         }
      }
      *ppAnim = CTAddRef(iter->second);
      return S_OK;
   }

   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////

tResult ModelSkeletonCreate(const tModelJoints & joints, IModelSkeleton * * ppSkeleton)
{
   if (ppSkeleton == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IModelSkeleton> pSkeleton(static_cast<IModelSkeleton*>(new cModelSkeleton(joints)));
   if (!pSkeleton)
   {
      return E_OUTOFMEMORY;
   }

   return pSkeleton.GetPointer(ppSkeleton);
}

///////////////////////////////////////////////////////////////////////////////
