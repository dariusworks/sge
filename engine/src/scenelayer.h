///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SCENELAYER_H
#define INCLUDED_SCENELAYER_H

#include "sceneapi.h"

#include <list>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSceneLayer
//

typedef std::list<ISceneEntity *> tSceneEntityList;

class cSceneLayer
{
public:
   cSceneLayer();
   ~cSceneLayer();

   tResult AddEntity(ISceneEntity * pEntity);
   tResult RemoveEntity(ISceneEntity * pEntity);
   tResult HasEntity(ISceneEntity * pEntity) const;

   tResult SetCamera(ISceneCamera * pCamera);
   tResult GetCamera(ISceneCamera * * ppCamera);

   void Clear();

   tResult AddInputListener(IInputListener * pListener);
   tResult RemoveInputListener(IInputListener * pListener);

   tResult Render(IRenderDevice * pRenderDevice);

   tResult Query(const cRay & ray, tSceneEntityList * pEntities);

   bool HandleMouseEvent(int x, int y, uint mouseState, double time);
   bool HandleKeyEvent(long key, bool down, double time);

private:
   tSceneEntityList m_entities;
   cAutoIPtr<ISceneCamera> m_pCamera;

   typedef std::list<IInputListener *> tInputListeners;
   tInputListeners m_inputListeners;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SCENELAYER_H
