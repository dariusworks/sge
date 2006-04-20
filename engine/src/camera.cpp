///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "camera.h"

#include "configapi.h"
#include "keys.h"
#include "ray.h"
#include "vec4.h"

#include <algorithm>

#include <GL/glew.h>

#include "dbgalloc.h" // must be last header


// REFERENCES
// http://www.opengl.org/resources/faq/technical/viewing.htm

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cCamera
//

///////////////////////////////////////

cCamera::cCamera()
 : m_bUpdateCompositeMatrices(true)
{
}

///////////////////////////////////////

cCamera::~cCamera()
{
}

///////////////////////////////////////

tResult cCamera::Init()
{
   return S_OK;
}

///////////////////////////////////////

tResult cCamera::Term()
{
   return S_OK;
}

///////////////////////////////////////

void cCamera::SetPerspective(float fov, float aspect, float znear, float zfar)
{
   MatrixPerspective(fov, aspect, znear, zfar, &m_proj);
   m_bUpdateCompositeMatrices = true;
}

///////////////////////////////////////

void cCamera::SetOrtho(float left, float right, float bottom, float top, float znear, float zfar)
{
   MatrixOrtho(left, right, bottom, top, znear, zfar, &m_proj);
   m_bUpdateCompositeMatrices = true;
}

///////////////////////////////////////

tResult cCamera::GeneratePickRay(float ndx, float ndy, cRay * pRay) const
{
   if (pRay == NULL)
   {
      return E_POINTER;
   }

   const tMatrix4 & m = GetViewProjectionInverseMatrix();

   tVec4 n;
   m.Transform(tVec4(ndx, ndy, -1, 1), &n);
   if (n.w == 0.0f)
   {
      return E_FAIL;
   }
   n.x /= n.w;
   n.y /= n.w;
   n.z /= n.w;

   tVec4 f;
   m.Transform(tVec4(ndx, ndy, 1, 1), &f);
   if (f.w == 0.0f)
   {
      return E_FAIL;
   }
   f.x /= f.w;
   f.y /= f.w;
   f.z /= f.w;

   tVec4 eye;
   m_viewInv.Transform(tVec4(0,0,0,1), &eye);

   tVec3 dir(f.x - n.x, f.y - n.y, f.z - n.z);
   dir.Normalize();

   *pRay = cRay(tVec3(eye.x,eye.y,eye.z), dir);

   return S_OK;
}

///////////////////////////////////////

void cCamera::SetGLState()
{
   glMatrixMode(GL_PROJECTION);
   glLoadMatrixf(GetProjectionMatrix().m);
   glMatrixMode(GL_MODELVIEW);
   glLoadMatrixf(GetViewMatrix().m);
}

///////////////////////////////////////

void cCamera::UpdateCompositeMatrices() const
{
   if (m_bUpdateCompositeMatrices)
   {
      m_bUpdateCompositeMatrices = false;
      MatrixInvert(m_view.m, m_viewInv.m);
      GetProjectionMatrix().Multiply(GetViewMatrix(), &m_viewProj);
      MatrixInvert(GetViewProjectionMatrix().m, m_viewProjInv.m);
      m_frustum.ExtractPlanes(GetViewProjectionMatrix());
   }
}

///////////////////////////////////////

tResult CameraCreate()
{
   cAutoIPtr<ICamera> p(static_cast<ICamera*>(new cCamera));
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_ICamera, p);
}

///////////////////////////////////////////////////////////////////////////////

tResult ScreenToNormalizedDeviceCoords(int sx, int sy, float * pndx, float * pndy)
{
   if (pndx == NULL || pndy == NULL)
   {
      return E_POINTER;
   }

   int viewport[4];
   glGetIntegerv(GL_VIEWPORT, viewport);

   sy = viewport[3] - sy;

   // convert screen coords to normalized (origin at center, [-1..1])
   float normx = (float)(sx - viewport[0]) * 2.f / viewport[2] - 1.f;
   float normy = (float)(sy - viewport[1]) * 2.f / viewport[3] - 1.f;

   *pndx = normx;
   *pndy = normy;

   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cCameraControl
//

static const float kElevationDefault = 100;
static const float kElevationMin = 25;
static const float kElevationMax = 200;
static const float kElevationStep = 10;
static const float kElevationSpeed = 15;
static const float kDefaultPitch = 70;
static const float kDefaultSpeed = 50;

///////////////////////////////////////

cCameraControl::cCameraControl()
 : m_moveCameraTask(this)
 , m_cameraMove(kCameraMoveNone)
 , m_pitch(kDefaultPitch)
 , m_oneOverTangentPitch(0)
 , m_elevation(kElevationDefault)
 , m_focus(0,0,0)
 , m_elevationLerp(kElevationDefault, kElevationDefault, 1)
{
   ConfigGet(_T("view_elevation"), &m_elevation);
   ConfigGet(_T("view_pitch"), &m_pitch);
   MatrixRotateX(m_pitch, &m_rotation);
   m_oneOverTangentPitch = 1.0f / tanf(m_pitch);
}

///////////////////////////////////////

cCameraControl::~cCameraControl()
{
}

///////////////////////////////////////

tResult cCameraControl::Init()
{
   UseGlobal(Scheduler);
   pScheduler->AddFrameTask(&m_moveCameraTask, 0, 1, 0);

   UseGlobal(Input);
   pInput->AddInputListener(static_cast<IInputListener*>(this));

   return S_OK;
}

///////////////////////////////////////

tResult cCameraControl::Term()
{
   UseGlobal(Input);
   pInput->RemoveInputListener(static_cast<IInputListener*>(this));

   UseGlobal(Scheduler);
   pScheduler->RemoveFrameTask(&m_moveCameraTask);

   return S_OK;
}

///////////////////////////////////////

bool cCameraControl::OnInputEvent(const sInputEvent * pEvent)
{
   switch (pEvent->key)
   {
      case kLeft:
      {
         if (pEvent->down)
         {
            m_cameraMove |= kCameraMoveLeft;
         }
         else
         {
            m_cameraMove &= ~kCameraMoveLeft;
         }
         break;
      }

      case kRight:
      {
         if (pEvent->down)
         {
            m_cameraMove |= kCameraMoveRight;
         }
         else
         {
            m_cameraMove &= ~kCameraMoveRight;
         }
         break;
      }

      case kUp:
      {
         if (pEvent->down)
         {
            m_cameraMove |= kCameraMoveForward;
         }
         else
         {
            m_cameraMove &= ~kCameraMoveForward;
         }
         break;
      }

      case kDown:
      {
         if (pEvent->down)
         {
            m_cameraMove |= kCameraMoveBack;
         }
         else
         {
            m_cameraMove &= ~kCameraMoveBack;
         }
         break;
      }

      case kMouseWheelUp:
      {
         if (pEvent->down)
         {
            Raise();
         }
         break;
      }

      case kMouseWheelDown:
      {
         if (pEvent->down)
         {
            Lower();
         }
         break;
      }
   }

   return false;
}

///////////////////////////////////////

static const uint kCameraMoveLeftRight = (kCameraMoveLeft | kCameraMoveRight);
static const uint kCameraMoveForwardBack = (kCameraMoveForward | kCameraMoveBack);

void cCameraControl::SimFrame(double elapsedTime)
{
   if (m_cameraMove != kCameraMoveNone)
   {
      float focusVelX = 0, focusVelZ = 0;

      if ((m_cameraMove & kCameraMoveLeftRight) == kCameraMoveLeft)
      {
         focusVelX = -kDefaultSpeed;
      }
      else if ((m_cameraMove & kCameraMoveLeftRight) == kCameraMoveRight)
      {
         focusVelX = kDefaultSpeed;
      }

      if ((m_cameraMove & kCameraMoveForwardBack) == kCameraMoveForward)
      {
         focusVelZ = -kDefaultSpeed;
      }
      else if ((m_cameraMove & kCameraMoveForwardBack) == kCameraMoveBack)
      {
         focusVelZ = kDefaultSpeed;
      }

      m_focus.x += static_cast<float>(focusVelX * elapsedTime);
      m_focus.z += static_cast<float>(focusVelZ * elapsedTime);
   }

   m_elevation = m_elevationLerp.Update(elapsedTime);

   float zOffset = m_elevation * m_oneOverTangentPitch;

   m_eye = tVec3(m_focus.x, m_focus.y + m_elevation, m_focus.z + zOffset);

   tMatrix4 mt;
   MatrixTranslate(-m_eye.x, -m_eye.y, -m_eye.z, &mt);

   tMatrix4 newModelView;
   m_rotation.Multiply(mt, &newModelView);

   UseGlobal(Camera);
   pCamera->SetViewMatrix(newModelView);
}

///////////////////////////////////////

tResult cCameraControl::GetElevation(float * pElevation) const
{
   if (pElevation == NULL)
   {
      return E_POINTER;
   }
   *pElevation = m_elevation;
   return S_OK;
}

///////////////////////////////////////

tResult cCameraControl::SetElevation(float elevation)
{
   m_elevation = elevation;
   m_elevationLerp.Restart(m_elevation, m_elevation, 1);
   return S_OK;
}

///////////////////////////////////////

tResult cCameraControl::GetPitch(float * pPitch) const
{
   if (pPitch == NULL)
   {
      return E_POINTER;
   }
   *pPitch = m_pitch;
   return S_OK;
}

///////////////////////////////////////

tResult cCameraControl::SetPitch(float pitch)
{
   m_pitch = pitch;
   return S_OK;
}

///////////////////////////////////////

tResult cCameraControl::LookAtPoint(float x, float z)
{
   m_focus = tVec3(x, 0, z);
   return S_OK;
}

///////////////////////////////////////

void cCameraControl::SetMovement(uint mask, uint flag)
{
   m_cameraMove = (m_cameraMove & ~mask) | (flag & mask);
}

///////////////////////////////////////

tResult cCameraControl::Raise()
{
   m_elevationLerp.Restart(m_elevation, m_elevation + kElevationStep, kElevationSpeed);
   return S_OK;
}

///////////////////////////////////////

tResult cCameraControl::Lower()
{
   m_elevationLerp.Restart(m_elevation, m_elevation - kElevationStep, kElevationSpeed);
   return S_OK;
}

///////////////////////////////////////

cCameraControl::cMoveCameraTask::cMoveCameraTask(cCameraControl * pOuter)
 : m_pOuter(pOuter)
{
}

///////////////////////////////////////

void cCameraControl::cMoveCameraTask::Execute(double time)
{
   double elapsed = fabs(time - m_lastTime);
   m_pOuter->SimFrame(elapsed);
   m_lastTime = time;
}

///////////////////////////////////////

tResult CameraControlCreate()
{
   cAutoIPtr<ICameraControl> pCameraControl(static_cast<ICameraControl*>(new cCameraControl));
   if (!pCameraControl)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_ICameraControl, pCameraControl);
}

///////////////////////////////////////////////////////////////////////////////
