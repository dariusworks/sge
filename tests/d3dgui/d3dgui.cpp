///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guiapi.h"
#include "simapi.h"
#include "inputapi.h"
#include "engineapi.h"
#include "scriptapi.h"
#include "sys.h"

#include "resourceapi.h"
#include "configapi.h"
#include "filespec.h"
#include "filepath.h"
#include "imageapi.h"
#include "techstring.h"
#include "globalobj.h"
#include "threadcallapi.h"

#include <ctime>

#define VC_EXTRALEAN
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>

#include "dbgalloc.h" // must be last header

#pragma warning(disable:4355) // 'this' used in base member initializer list

#pragma comment(lib, "d3d9")
#ifdef D3D_DEBUG_INFO
#pragma comment(lib, "d3dx9d")
#else
#pragma comment(lib, "d3dx9")
#endif

// The following definitions are required for WinMain
F_DECLARE_HANDLE(HINSTANCE);
typedef char * LPSTR;

static tResult d3dguiframe();

///////////////////////////////////////////////////////////////////////////////

#define kDefaultWidth   800
#define kDefaultHeight  600
#define kDefaultBpp     16
#define kDefaultFov     70

static const int kDefStatsX = 25;
static const int kDefStatsY = 25;
static const cColor kDefStatsColor(1,1,1,1);

///////////////////////////////////////////////////////////////////////////////

cAutoIPtr<IGUIFont> g_pFont;

HWND g_hWnd = NULL;


///////////////////////////////////////////////////////////////////////////////

static void RegisterGlobalObjects()
{
   InputCreate();
   SimCreate();
   ResourceManagerCreate();
   ScriptInterpreterCreate();
   GUIContextCreate();
   GUIFactoryCreate();
   GUIFontFactoryCreate();
   ThreadCallerCreate();
}


///////////////////////////////////////////////////////////////////////////////

static tResult InitGlobalConfig(int argc, tChar * argv[])
{
   Assert(argc > 0);

   cFileSpec cfgFile(argv[0]);
   cfgFile.SetFileExt(_T("cfg"));

   cAutoIPtr<IDictionaryStore> pStore = DictionaryStoreCreate(cfgFile);
   if (!pStore)
   {
      return E_OUTOFMEMORY;
   }
   pStore->Load(g_pConfig);

   ParseCommandLine(argc, argv, g_pConfig);

   return S_OK;
}

static bool d3dguiinit(int argc, tChar * argv[])
{
   if (InitGlobalConfig(argc, argv) != S_OK)
   {
      return false;
   }

   srand(time(NULL));
   SeedRand(time(NULL));

   RegisterGlobalObjects();
   if (FAILED(StartGlobalObjects()))
   {
      DebugMsg("One or more application-level services failed to start!\n");
      return false;
   }

   ImageRegisterResourceFormats();
   TextFormatRegister(NULL);
   EngineRegisterResourceFormats();
   EngineRegisterScriptFunctions();

   cStr temp;
   if (ConfigGet(_T("data"), &temp) == S_OK)
   {
      UseGlobal(ResourceManager);
      pResourceManager->AddDirectoryTreeFlattened(temp.c_str());
   }

   cStr script("autoexec.lua");
   ConfigGet(_T("autoexec_script"), &script);
   if (!script.empty())
   {
      char * pszCode = NULL;
      UseGlobal(ResourceManager);
      if (pResourceManager->Load(script.c_str(), kRT_Text, NULL, (void**)&pszCode) == S_OK)
      {
         UseGlobal(ScriptInterpreter);
         pScriptInterpreter->ExecString(pszCode);
      }
   }

   int width = kDefaultWidth;
   int height = kDefaultHeight;
   int bpp = kDefaultBpp;
   ConfigGet(_T("screen_width"), &width);
   ConfigGet(_T("screen_height"), &height);
   ConfigGet(_T("screen_bpp"), &bpp);

   g_hWnd = reinterpret_cast<HWND>(SysCreateWindow(_T("D3D GUI Test"), width, height, kDirect3D9));
   if (g_hWnd == NULL)
   {
      return false;
   }

   cAutoIPtr<IDirect3DDevice9> pD3dDevice;
   if (SysGetDirect3DDevice9(&pD3dDevice) == S_OK)
   {
      UseGlobal(GUIContext);
      cAutoIPtr<IGUIRenderDeviceContext> pGuiRenderDevice;
      if (GUIRenderDeviceCreateD3D(&pGuiRenderDevice) == S_OK)
      {
         pGUIContext->SetRenderDeviceContext(pGuiRenderDevice);
      }
   }

   UseGlobal(ThreadCaller);
   if (FAILED(pThreadCaller->ThreadInit()))
   {
      return false;
   }

   UseGlobal(GUIContext);
   if (FAILED(pGUIContext->GetDefaultFont(&g_pFont)))
   {
      WarnMsg("Failed to get a default font interface pointer for showing frame stats\n");
      return false;
   }

   pGUIContext->PushPage("start.xml");

   SysSetFrameCallback(d3dguiframe);
   SysAppActivate(true);

   UseGlobal(Sim);
   pSim->Go();

   if (FAILED(SysRunUnitTests()))
   {
      return false;
   }

   return true;
}

///////////////////////////////////////////////////////////////////////////////

static void d3dguiterm()
{
   UseGlobal(Sim);
   pSim->Stop();

   UseGlobal(ThreadCaller);
   pThreadCaller->ThreadTerm();

   SafeRelease(g_pFont);

   // This will make sure the GL context is destroyed
   SysQuit();

   StopGlobalObjects();
}

///////////////////////////////////////////////////////////////////////////////

static tResult d3dguiframe()
{
   UseGlobal(Sim);
   pSim->NextFrame();

   cAutoIPtr<IDirect3DDevice9> pD3dDevice;
   if (SysGetDirect3DDevice9(&pD3dDevice) != S_OK)
   {
      // D3D device not initialized yet?
      return E_FAIL;
   }

   pD3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1, 0);

   if (pD3dDevice->BeginScene() == D3D_OK)
   {
      UseGlobal(GUIContext);
      cAutoIPtr<IGUIRenderDeviceContext> pRenderDeviceContext;
      if (pGUIContext->GetRenderDeviceContext(&pRenderDeviceContext) == S_OK)
      {
         pRenderDeviceContext->Begin2D();

         pGUIContext->RenderGUI();

         if (!!g_pFont)
         {
            char szStats[100];
            SysReportFrameStats(szStats, _countof(szStats));

            tRect rect(kDefStatsX, kDefStatsY, 0, 0);
            g_pFont->RenderText(szStats, strlen(szStats), &rect, kRT_NoClip | kRT_DropShadow, kDefStatsColor);
         }

         pRenderDeviceContext->End2D();
      }

      pD3dDevice->EndScene();
      pD3dDevice->Present(NULL, NULL, NULL, NULL);
   }

   SysSwapBuffers();

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

#if defined(_WIN32) && !defined(__CYGWIN__)
int STDCALL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine, int nShowCmd)
{
   if (!d3dguiinit(__argc, __targv))
   {
      d3dguiterm();
      return -1;
   }

   int result = SysEventLoop(d3dguiframe);

   d3dguiterm();

   return result;
}
#endif

///////////////////////////////////////////////////////////////////////////////

#if defined(__CYGWIN__)
int main(int argc, char * argv[])
{
   if (!d3dguiinit(argc, argv))
   {
      d3dguiterm();
      return EXIT_FAILURE;
   }

   int result = SysEventLoop(NULL);

   d3dguiterm();

   return result;
}
#endif

///////////////////////////////////////////////////////////////////////////////
