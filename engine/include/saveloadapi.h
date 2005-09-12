////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SAVELOADAPI_H
#define INCLUDED_SAVELOADAPI_H

/// @file saveloadapi.h
/// Interface definitions for a simple serialization system

#include "enginedll.h"
#include "comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(ISaveLoadManager);
F_DECLARE_INTERFACE(ISaveLoadParticipant);

F_DECLARE_INTERFACE(IWriter);
F_DECLARE_INTERFACE(IReader);

////////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ISaveLoadManager
//

interface ISaveLoadManager : IUnknown
{
   virtual tResult RegisterSaveLoadParticipant(REFGUID id, int version, ISaveLoadParticipant *) = 0;
   virtual tResult RevokeSaveLoadParticipant(REFGUID id, int version) = 0;

   virtual tResult Save(IWriter * pWriter) = 0;
   virtual tResult Load(IReader * pReader) = 0;
};

///////////////////////////////////////

ENGINE_API tResult SaveLoadManagerCreate();

////////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: ISaveLoadParticipant
//

interface ISaveLoadParticipant : IUnknown
{
   virtual tResult Save(IWriter * pWriter) = 0;
   virtual tResult Load(IReader * pReader, int version) = 0;
};

////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SAVELOADAPI_H
