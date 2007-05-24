///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MS3DMODEL_H
#define INCLUDED_MS3DMODEL_H

#include "ms3dmodeldll.h"

#include "ms3dgroup.h"
#include "ms3djoint.h"
#include "ms3dmaterial.h"
#include "ms3dtriangle.h"
#include "ms3dvertex.h"

#include "tech/comtools.h"

#include <string>
#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

struct sModelVertex;

F_DECLARE_INTERFACE(IModel);
F_DECLARE_INTERFACE(IReader);

template class MS3DMODEL_API std::allocator<cMs3dVertex>;
template class MS3DMODEL_API std::allocator<cMs3dTriangle>;
template class MS3DMODEL_API std::allocator<cMs3dGroup>;
template class MS3DMODEL_API std::allocator<cMs3dMaterial>;
template class MS3DMODEL_API std::allocator<cMs3dJoint>;
template class MS3DMODEL_API std::allocator<std::string>;

template class MS3DMODEL_API std::vector<cMs3dVertex>;
template class MS3DMODEL_API std::vector<cMs3dTriangle>;
template class MS3DMODEL_API std::vector<cMs3dGroup>;
template class MS3DMODEL_API std::vector<cMs3dMaterial>;
template class MS3DMODEL_API std::vector<cMs3dJoint>;
template class MS3DMODEL_API std::vector<std::string>;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dModel
//

class MS3DMODEL_API cMs3dModel
{
   cMs3dModel(const cMs3dModel &);
   const cMs3dModel & operator =(const cMs3dModel &);

public:
   cMs3dModel();
   ~cMs3dModel();

   IModel * Read(IReader * pReader);

   static void * Load(IReader * pReader);
   static void Unload(void * pData);

private:
   std::vector<cMs3dVertex> ms3dVerts;
   std::vector<cMs3dTriangle> ms3dTris;
   std::vector<cMs3dGroup> ms3dGroups;
   std::vector<cMs3dMaterial> ms3dMaterials;
   float m_animationFPS;
   float m_currentTime;
   int m_nTotalFrames;
   std::vector<cMs3dJoint> ms3dJoints;
   std::vector<std::string> m_groupComments;
   std::vector<std::string> m_materialComments;
   std::vector<std::string> m_jointComments;
   std::string m_modelComment;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MS3DMODEL_H
