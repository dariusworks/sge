///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ai/aiagentapi.h"

#include "engine/entityapi.h"
#include "engine/cameraapi.h"
#include "engine/terrainapi.h"

#include "platform/inputapi.h"
#include "platform/keys.h"

#include "render/renderapi.h"

#include "tech/globalobj.h"
#include "tech/multivar.h"
#include "tech/ray.inl"

#include "tech/dbgalloc.h" // must be last header


/////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(EntityCmds);

#define LocalMsg(msg)            DebugMsgEx(EntityCmds,msg)
#define LocalMsg1(msg,a)         DebugMsgEx1(EntityCmds,msg,(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx2(EntityCmds,msg,(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx3(EntityCmds,msg,(a),(b),(c))
#define LocalMsg4(msg,a,b,c,d)   DebugMsgEx4(EntityCmds,msg,(a),(b),(c),(d))


///////////////////////////////////////////////////////////////////////////////

tResult EntityCommandSpawn(IEntity * pEntity, const cMultiVar * pArgs, uint nArgs)
{
   if (pArgs == NULL)
   {
      return E_POINTER;
   }

   if (nArgs != 1 || !pArgs[0].IsString())
   {
      return E_INVALIDARG;
   }

   cAutoIPtr<IEntitySpawnComponent> pSpawnComponent;
   if (pEntity->GetComponent(IID_IEntitySpawnComponent, &pSpawnComponent) == S_OK)
   {
      tVec3 rallyPoint;
      if (pSpawnComponent->GetRallyPoint(&rallyPoint) == S_OK)
      {
         UseGlobal(EntityManager);
         pEntityManager->SpawnEntity(pArgs[0].ToString(), rallyPoint);
      }
   }

   WarnMsgIf(!pSpawnComponent, "Attempting to spawn from entity with no spawn component\n");

   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////

static bool GetTerrainLocation(const cRay<float> & ray, cPoint3<float> * pLocation)
{
   HTERRAINQUAD hQuad;
   UseGlobal(TerrainModel);
   if (pTerrainModel->GetQuadFromHitTest(ray, &hQuad) == S_OK)
   {
      cPoint3<float> corners[4];
      if (pTerrainModel->GetQuadCorners(hQuad, corners) == S_OK)
      {
         if (ray.IntersectsTriangle(corners[0], corners[3], corners[2], pLocation)
            || ray.IntersectsTriangle(corners[2], corners[1], corners[0], pLocation))
         {
            return true;
         }
      }
   }
   return false;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTerrainClickInputMode
//
// Base class for input modes that require the player to choose
// a point on the terrain (move, attack, set rally point, etc.)

class cTerrainClickInputMode : public cComObject<IMPLEMENTS(IInputModalListener)>
{
public:
   cTerrainClickInputMode(IEntity * pEntity);
   virtual ~cTerrainClickInputMode();

   // TODO: Over-ride QI to support IInputListener too

   virtual bool OnInputEvent(const sInputEvent * pEvent);
   virtual void CancelMode();

protected:
   IEntity * AccessEntity() { return m_pEntity; }
   const IEntity * AccessEntity() const { return m_pEntity; }

   virtual tResult OnTerrainClick(const cPoint3<float> & location);

private:
   cAutoIPtr<IEntity> m_pEntity;
};

////////////////////////////////////////

cTerrainClickInputMode::cTerrainClickInputMode(IEntity * pEntity)
 : m_pEntity(CTAddRef(pEntity))
{
}

////////////////////////////////////////

cTerrainClickInputMode::~cTerrainClickInputMode()
{
}

////////////////////////////////////////

bool cTerrainClickInputMode::OnInputEvent(const sInputEvent * pEvent)
{
   Assert(pEvent != NULL);

   bool bResult = false;

   if (pEvent->key == kMouseLeft && pEvent->down)
   {
      UseGlobal(Renderer);

      cAutoIPtr<IRenderCamera> pCamera;
      cRay<float> pickRay;
      if (pRenderer->GetCamera(&pCamera) == S_OK
         && pCamera->GenerateScreenPickRay(pEvent->point.x, pEvent->point.y, &pickRay) == S_OK)
      {
         cPoint3<float> location;
         if (GetTerrainLocation(pickRay, &location))
         {
            UseGlobal(Input);
            pInput->PopModalListener();

            Assert(!!m_pEntity);

            bResult = (OnTerrainClick(location) == S_OK);
         }
      }
   }

   return bResult;
}

////////////////////////////////////////

void cTerrainClickInputMode::CancelMode()
{
}

////////////////////////////////////////

tResult cTerrainClickInputMode::OnTerrainClick(const cPoint3<float> & location)
{
   return S_FALSE;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSetRallyPointMode
//

class cSetRallyPointMode : public cTerrainClickInputMode
{
public:
   cSetRallyPointMode(IEntity * pEntity);
   ~cSetRallyPointMode();

protected:
   virtual tResult OnTerrainClick(const tVec3 & location);
};

////////////////////////////////////////

cSetRallyPointMode::cSetRallyPointMode(IEntity * pEntity)
 : cTerrainClickInputMode(pEntity)
{
}

////////////////////////////////////////

cSetRallyPointMode::~cSetRallyPointMode()
{
}

////////////////////////////////////////

tResult cSetRallyPointMode::OnTerrainClick(const tVec3 & location)
{
   LocalMsg3("Set rally point to (%f, %f, %f)\n", location.x, location.y, location.z);

   cAutoIPtr<IEntitySpawnComponent> pSpawnComponent;
   if (AccessEntity()->GetComponent(IID_IEntitySpawnComponent, &pSpawnComponent) == S_OK)
   {
      pSpawnComponent->SetRallyPoint(location);
      return S_OK;
   }

   WarnMsgIf(!pSpawnComponent, "Attempting to set rally point for entity with no spawn component\n");

   return S_FALSE;
}

///////////////////////////////////////////////////////////////////////////////

tResult EntityCommandSetRallyPoint(IEntity * pEntity, const cMultiVar * pArgs, uint nArgs)
{
   cAutoIPtr<IInputModalListener> pMode(new cSetRallyPointMode(pEntity));
   if (!pMode)
   {
      return E_OUTOFMEMORY;
   }
   UseGlobal(Input);
   pInput->PushModalListener(pMode);
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMoveMode
//

class cMoveMode : public cTerrainClickInputMode
{
public:
   cMoveMode(IEntity * pEntity);
   ~cMoveMode();

protected:
   virtual tResult OnTerrainClick(const cPoint3<float> & location);
};

////////////////////////////////////////

cMoveMode::cMoveMode(IEntity * pEntity)
 : cTerrainClickInputMode(pEntity)
{
}

////////////////////////////////////////

cMoveMode::~cMoveMode()
{
}

////////////////////////////////////////

tResult cMoveMode::OnTerrainClick(const cPoint3<float> & location)
{
   LocalMsg3("Issue move orders to (%f, %f, %f)\n", location.x, location.y, location.z);

   cMultiVar args[3] =
   {
      cMultiVar(location.x),
      cMultiVar(location.y),
      cMultiVar(location.z),
   };

   UseGlobal(AIAgentMessageRouter);
   pAIAgentMessageRouter->SendMessage(AccessEntity()->GetId(), 0, kAIAMT_OrderMoveTo, _countof(args), &args[0]);

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

tResult EntityCommandMove(IEntity * pEntity, const cMultiVar * pArgs, uint nArgs)
{
   cAutoIPtr<IInputModalListener> pMode(new cMoveMode(pEntity));
   if (!pMode)
   {
      return E_OUTOFMEMORY;
   }
   UseGlobal(Input);
   pInput->PushModalListener(pMode);
   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////

tResult EntityCommandStop(IEntity * pEntity, const cMultiVar * pArgs, uint nArgs)
{
   if (pEntity == NULL)
   {
      return E_POINTER;
   }

   UseGlobal(AIAgentMessageRouter);
   pAIAgentMessageRouter->SendMessage(pEntity->GetId(), 0, kAIAMT_OrderStop, 0, NULL);

   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////

void RegisterBuiltinEntityCommands()
{
   UseGlobal(EntityCommandManager);
   pEntityCommandManager->RegisterCommand(_T("Spawn"), EntityCommandSpawn);
   pEntityCommandManager->RegisterCommand(_T("SetRallyPoint"), EntityCommandSetRallyPoint);
   pEntityCommandManager->RegisterCommand(_T("Move"), EntityCommandMove);
   pEntityCommandManager->RegisterCommand(_T("Stop"), EntityCommandStop);
}

///////////////////////////////////////////////////////////////////////////////
