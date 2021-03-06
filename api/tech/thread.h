///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_THREAD_H
#define INCLUDED_THREAD_H

#include "techdll.h"

#ifndef _WIN32
#include <pthread.h>
#endif

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
typedef ulong tThreadId;
#else
typedef pthread_t tThreadId;
#endif

const uint kInfiniteTimeout = ~0u;

TECH_API void ThreadSleep(uint milliseconds);

TECH_API tThreadId ThreadGetCurrentId();

TECH_API void ThreadSetName(tThreadId threadId, const char * pszName);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cThread
//

enum eThreadPriority
{
   kTP_Lowest = 0,
   kTP_Highest = 15,
   kTP_Normal = (kTP_Lowest + kTP_Highest) / 2,
};

class TECH_API cThread
{
protected:
   cThread(const cThread &);
   const cThread & operator =(const cThread &);

public:
   cThread();
   virtual ~cThread();

   bool Create(int priority = kTP_Normal, uint stackSize = 0);

   tThreadId GetThreadId() const;

   void Join();

   bool Terminate();

protected:
   virtual int Run() = 0;

private:
   tThreadId m_threadId;
#ifdef _WIN32
   static uint STDCALL ThreadEntry(void * param);
   HANDLE m_hThread;
#else
   static void * ThreadEntry(void * param);
   pthread_t m_thread;
   bool m_bJoined;
#endif
};

////////////////////////////////////////

inline tThreadId cThread::GetThreadId() const
{
   return m_threadId;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cThreadEvent
//

class TECH_API cThreadEvent
{
public:
   cThreadEvent();
   ~cThreadEvent();

   bool Create();
   void Destroy();

   bool Wait();
   bool Wait(uint timeout);

   // Unblock a single thread waiting on the event
   bool Signal();

private:
#ifdef _WIN32
   HANDLE m_hEvent;
#else
   pthread_cond_t m_cond;
   pthread_mutex_t m_mutex;
   bool m_bInitialized;
#endif
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cThreadMutex
//

class TECH_API cThreadMutex
{
public:
   cThreadMutex();
   ~cThreadMutex();

   bool Create();

   bool Acquire(uint timeout = kInfiniteTimeout);
   bool Release();

private:
#ifdef _WIN32
   HANDLE m_hMutex;
#else
   pthread_mutex_t m_mutex;
#endif
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMutexLock
//

class TECH_API cMutexLock
{
public:
   cMutexLock(cThreadMutex * pMutex);
   ~cMutexLock();

   bool Acquire(uint timeout = kInfiniteTimeout);
   void Release();

private:
   cThreadMutex * m_pMutex;
   bool m_bLocked;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_THREAD_H
