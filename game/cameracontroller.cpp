///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "cameracontroller.h"
#include "scenecamera.h"
#include "globalobj.h"
#include "boundingvolume.h"
#include "raycast.h"
#include "ggl.h"
#include "scenegroup.h"

#include "vec4.h"
#include "configapi.h"
#include "keys.h"

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

extern cSceneGroup * g_pGameGroup; // HACK

///////////////////////////////////////////////////////////////////////////////

class cPickNodeVisitor : public cSceneNodeVisitor
{
public:
   cPickNodeVisitor(const tVec3 & rayDir, const tVec3 & rayOrigin);

   virtual void VisitSceneNode(cSceneNode * pNode);

   tVec3 m_rayDir, m_rayOrigin;
   std::vector<cSceneNode *> m_hitNodes;
};

cPickNodeVisitor::cPickNodeVisitor(const tVec3 & rayDir, const tVec3 & rayOrigin)
 : m_rayDir(rayDir),
   m_rayOrigin(rayOrigin)
{
}

void cPickNodeVisitor::VisitSceneNode(cSceneNode * pNode)
{
   Assert(pNode != NULL);

   if (pNode->IsPickable())
   {
      const cBoundingVolume * pBounds = pNode->GetBoundingVolume();
      if (pBounds != NULL)
      {
         if (pBounds->Intersects(m_rayOrigin, m_rayDir))
         {
            pNode->Hit();
            m_hitNodes.push_back(pNode);
         }
         else
         {
            pNode->ClearHitState();
         }
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGameCameraController
//

///////////////////////////////////////

cGameCameraController::cGameCameraController(cSceneCamera * pCamera)
 : m_pitch(kDefaultPitch),
   m_oneOverTangentPitch(0),
   m_elevation(kDefaultElevation),
   m_focus(0,0,0),
   m_velocity(0,0,0),
   m_pCamera(pCamera)
{
   ConfigGet("view_elevation", &m_elevation);
   ConfigGet("view_pitch", &m_pitch);
   MatrixRotateX(m_pitch, &m_rotation);
   m_oneOverTangentPitch = 1.0f / tanf(m_pitch);
}

///////////////////////////////////////

cGameCameraController::~cGameCameraController()
{
}

///////////////////////////////////////

void cGameCameraController::Connect()
{
   UseGlobal(Sim);
   pSim->Connect(this);
   InputAddListener(this);
}

///////////////////////////////////////

void cGameCameraController::Disconnect()
{
   InputRemoveListener(this);
   UseGlobal(Sim);
   pSim->Disconnect(this);
}

///////////////////////////////////////

void cGameCameraController::OnFrame(double elapsedTime)
{
   m_focus += m_velocity * (float)elapsedTime;

   float zOffset = m_elevation * m_oneOverTangentPitch;

   m_eye = tVec3(m_focus.x, m_focus.y + m_elevation, m_focus.z + zOffset);

   // Very simple third-person camera model. Always looking down the -z axis
   // and slightly pitched over the x axis.
   sMatrix4 mt;
   MatrixTranslate(-m_eye.x, -m_eye.y, -m_eye.z, &mt);

   sMatrix4 newModelView = m_rotation * mt;
   m_pCamera->SetModelViewMatrix(newModelView);
}

///////////////////////////////////////

bool cGameCameraController::OnMouseEvent(int x, int y, uint mouseState, double time)
{
   if (mouseState & kLMouseDown)
   {
      tVec3 dir;
      if (BuildPickRay(x, y, &dir))
      {
         cPickNodeVisitor pickVisitor(dir, GetEyePosition());
         g_pGameGroup->Traverse(&pickVisitor);

         if (pickVisitor.m_hitNodes.empty())
         {
            tVec3 intersect;
            if (RayIntersectPlane(GetEyePosition(), dir, tVec3(0,1,0), 0, &intersect))
            {
               DebugMsg3("Hit the ground at approximately (%.1f,%.1f,%.1f)\n",
                  intersect.x, intersect.y, intersect.z);
            }
         }
      }

      return true;
   }

   return false;
}

///////////////////////////////////////

bool cGameCameraController::OnKeyEvent(long key, bool down, double time)
{
   bool bUpdateCamera = false;

   switch (key)
   {
      case kLeft:
      {
         m_velocity.x = down ? -kDefaultSpeed : 0;
         bUpdateCamera = true;
         break;
      }

      case kRight:
      {
         m_velocity.x = down ? kDefaultSpeed : 0;
         bUpdateCamera = true;
         break;
      }

      case kUp:
      {
         m_velocity.z = down ? -kDefaultSpeed : 0;
         bUpdateCamera = true;
         break;
      }

      case kDown:
      {
         m_velocity.z = down ? kDefaultSpeed : 0;
         bUpdateCamera = true;
         break;
      }

      case kMouseWheelUp:
      {
         m_elevation++;
         bUpdateCamera = true;
         break;
      }

      case kMouseWheelDown:
      {
         m_elevation--;
         bUpdateCamera = true;
         break;
      }
   }

   return bUpdateCamera;
}

///////////////////////////////////////

void cGameCameraController::LookAtPoint(float x, float z)
{
   m_focus = tVec3(x, 0, z);
}

///////////////////////////////////////

bool cGameCameraController::BuildPickRay(int x, int y, tVec3 * pRay)
{
   Assert(m_pCamera != NULL);

   int viewport[4];
   glGetIntegerv(GL_VIEWPORT, viewport);

   y = viewport[3] - y;

   // convert screen coords to normalized (origin at center, [-1..1])
   float normx = (float)(x - viewport[0]) * 2.f / viewport[2] - 1.f;
   float normy = (float)(y - viewport[1]) * 2.f / viewport[3] - 1.f;

   const sMatrix4 & m = m_pCamera->GetModelViewProjectionInverseMatrix();

   tVec4 n = m.Transform(tVec4(normx, normy, -1, 1));
   if (n.w == 0.0f)
      return false;
   n.x /= n.w;
   n.y /= n.w;
   n.z /= n.w;

   tVec4 f = m.Transform(tVec4(normx, normy, 1, 1));
   if (f.w == 0.0f)
      return false;
   f.x /= f.w;
   f.y /= f.w;
   f.z /= f.w;

   Assert(pRay != NULL);
   *pRay = tVec3(f.x - n.x, f.y - n.y, f.z - n.z);
   pRay->Normalize();

   return true;
}

///////////////////////////////////////////////////////////////////////////////
