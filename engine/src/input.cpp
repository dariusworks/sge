///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "input.h"
#include "keys.h"
#include "scriptapi.h"

#include <cstring>
#include <cstdlib>
#include <locale>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

#if !defined(NDEBUG) && defined(_MSC_VER)
EXTERN_C CRT_IMPORT int CDECL _CrtIsValidHeapPointer(const void * userData);
#else
#define _CrtIsValidHeapPointer(userData) ((int)1)
#endif

LOG_DEFINE_CHANNEL(InputEvents);

///////////////////////////////////////////////////////////////////////////////

static bool ScriptExecString(const char * pszCode)
{
   UseGlobal(ScriptInterpreter);
   return (pScriptInterpreter->ExecString(pszCode) == S_OK);
}

///////////////////////////////////////////////////////////////////////////////

inline long KeyGetBindable(long key)
{
   // not allowed to bind uppercase keys
   if (isalpha(key))
   {
      return tolower(key);
   }
   else
   {
      return key;
   }
}

///////////////////////////////////////////////////////////////////////////////

extern const char * Key2Name(long key); // from cmds.cpp
extern long Name2Key(const char * pszKeyName); // from cmds.cpp

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cInput
//

///////////////////////////////////////

cInput::cInput()
 : m_windowSink(this),
   m_oldMouseState(0)
{
   memset(m_keyRepeats, 0, sizeof(m_keyRepeats));
   memset(m_keyDownBindings, 0, sizeof(m_keyDownBindings));
   memset(m_keyUpBindings, 0, sizeof(m_keyUpBindings));
}

///////////////////////////////////////

tResult cInput::Init()
{
#if !defined(NDEBUG) && defined(_WIN32)
   Assert(_countof(m_keyDownBindings) == kMaxKeys);
   Assert(_countof(m_keyDownBindings) == _countof(m_keyUpBindings));
   for (int i = 0; i < _countof(m_keyDownBindings); i++)
   {
      Assert(_CrtIsValidHeapPointer(m_keyDownBindings[i]) || m_keyDownBindings[i] == NULL);
      Assert(_CrtIsValidHeapPointer(m_keyUpBindings[i]) || m_keyUpBindings[i] == NULL);
   }
#endif
   return S_OK;
}

///////////////////////////////////////

tResult cInput::Term()
{
   Assert(_countof(m_keyDownBindings) == _countof(m_keyUpBindings));
   for (int i = 0; i < _countof(m_keyDownBindings); i++)
   {
      delete [] m_keyDownBindings[i];
      m_keyDownBindings[i] = NULL;
      delete [] m_keyUpBindings[i];
      m_keyUpBindings[i] = NULL;
   }
   return S_OK;
}

///////////////////////////////////////

bool cInput::KeyIsDown(long key)
{
   if (key > -1 && key < kMaxKeys)
      return m_keyRepeats[key] > 0;
   else
      return false;
}

///////////////////////////////////////

void cInput::KeyBind(long key, const char * pszDownCmd, const char * pszUpCmd)
{
   Assert(key > -1 && key < kMaxKeys);

   Assert(KeyGetDownBinding(toupper(key)) == NULL);
   key = KeyGetBindable(key);

   KeyUnbind(key);

   if (pszDownCmd != NULL && *pszDownCmd != 0)
   {
      m_keyDownBindings[key] = new char[strlen(pszDownCmd) + 1];
      strcpy(m_keyDownBindings[key], pszDownCmd);
   }

   if (pszUpCmd != NULL && *pszUpCmd != 0)
   {
      m_keyUpBindings[key] = new char[strlen(pszUpCmd) + 1];
      strcpy(m_keyUpBindings[key], pszUpCmd);
   }
}

///////////////////////////////////////

void cInput::KeyUnbind(long key)
{
   Assert(key > -1 && key < kMaxKeys);

   key = KeyGetBindable(key);

   delete [] m_keyDownBindings[key];
   m_keyDownBindings[key] = NULL;

   delete [] m_keyUpBindings[key];
   m_keyUpBindings[key] = NULL;
}

///////////////////////////////////////

tResult cInput::AddWindow(IWindow * pWindow)
{
   Assert(pWindow != NULL);
   return pWindow->Connect(&m_windowSink);
}

///////////////////////////////////////

tResult cInput::RemoveWindow(IWindow * pWindow)
{
   Assert(pWindow != NULL);
   return pWindow->Disconnect(&m_windowSink);
}

///////////////////////////////////////

bool cInput::DispatchInputEvent(int x, int y, long key, bool down, double time)
{
#ifndef NDEBUG
   const char * pszKey = Key2Name(key);
   if (pszKey != NULL)
   {
      DebugMsgEx3(InputEvents, "%s %s %f\n", pszKey, down ? "down" : "up", time);
   }
   else
   {
      DebugMsgEx3(InputEvents, "%d %s %f\n", key, down ? "down" : "up", time);
   }
#endif

   Assert((key >= 0) && (key < kMaxKeys));

   if (down)
   {
      m_keyRepeats[key]++;
   }
   else
   {
      m_keyRepeats[key] = 0;
   }

   sInputEvent event;
   event.key = key;
   event.down = down;
   event.point = tVec2(static_cast<float>(x), static_cast<float>(y));
   event.time = time;

   // iterate in reverse order so the most recently added listener gets first crack
   tSinks::reverse_iterator iter;
   for (iter = AccessSinks().rbegin(); iter != AccessSinks().rend(); iter++)
   {
      if ((*iter)->OnInputEvent(&event))
      {
         return true; // do no further processing for this key event
      }
   }

   return false;
}

///////////////////////////////////////

void cInput::HandleKeyEvent(long key, bool down, double time)
{
   // TODO: Provide mouse position even for key events
   if (DispatchInputEvent(0, 0, key, down, time))
   {
      return;
   }

   key = KeyGetBindable(key);

   if (down)
   {
      const char * p = KeyGetDownBinding(key);
      if (p != NULL)
      {
         ScriptExecString(p);
      }
   }
   else
   {
      const char * p = KeyGetUpBinding(key);
      if (p != NULL)
      {
         ScriptExecString(p);
      }
   }
}

///////////////////////////////////////

void cInput::HandleMouseEvent(int x, int y, uint mouseState, double time)
{
   // Up/down doesn't matter for mouse motion. Use false so that the
   // repeat count doesn't get incremented wildly.
   DispatchInputEvent(x, y, kMouseMove, false, time);

   if ((mouseState & kLMouseDown) && !(m_oldMouseState & kLMouseDown))
      DispatchInputEvent(x, y, kMouseLeft, true, time);
   else if (!(mouseState & kLMouseDown) && (m_oldMouseState & kLMouseDown))
      DispatchInputEvent(x, y, kMouseLeft, false, time);

   if ((mouseState & kRMouseDown) && !(m_oldMouseState & kRMouseDown))
      DispatchInputEvent(x, y, kMouseRight, true, time);
   else if (!(mouseState & kRMouseDown) && (m_oldMouseState & kRMouseDown))
      DispatchInputEvent(x, y, kMouseRight, false, time);

   if ((mouseState & kMMouseDown) && !(m_oldMouseState & kMMouseDown))
      DispatchInputEvent(x, y, kMouseMiddle, true, time);
   else if (!(mouseState & kMMouseDown) && (m_oldMouseState & kMMouseDown))
      DispatchInputEvent(x, y, kMouseMiddle, false, time);

   m_oldMouseState = mouseState;
}

///////////////////////////////////////

cInput::cWindowSink::cWindowSink(cInput * pOuter)
 : m_pOuter(pOuter)
{
}

///////////////////////////////////////

void cInput::cWindowSink::OnKeyEvent(long key, bool down, double time)
{
   Assert(m_pOuter != NULL);
   m_pOuter->HandleKeyEvent(key, down, time);
}

///////////////////////////////////////

void cInput::cWindowSink::OnMouseEvent(int x, int y, uint mouseState, double time)
{
   Assert(m_pOuter != NULL);
   m_pOuter->HandleMouseEvent(x, y, mouseState, time);
}

///////////////////////////////////////

void InputCreate()
{
   cAutoIPtr<IInput> p(new cInput);
}

///////////////////////////////////////////////////////////////////////////////
