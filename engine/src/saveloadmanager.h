////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SAVELOADMANAGER_H
#define INCLUDED_SAVELOADMANAGER_H

#include "saveloadapi.h"
#include "globalobjdef.h"
#include "readwriteapi.h"

#include <map>

#ifdef _MSC_VER
#pragma once
#endif

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cVersionedParticipant
//

class cVersionedParticipant
{
   cVersionedParticipant(const cVersionedParticipant &);
   void operator =(const cVersionedParticipant &);

public:
   cVersionedParticipant();
   ~cVersionedParticipant();

   tResult AddVersion(int, ISaveLoadParticipant *);
   tResult RemoveVersion(int);

   tResult GetMostRecentVersion(int * pVersion) const;
   tResult GetParticipant(int version, ISaveLoadParticipant * *);

private:
   typedef std::map<int, ISaveLoadParticipant *> tVersionMap;
   tVersionMap m_versionMap;
};


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cSaveLoadManager
//

class cSaveLoadManager : public cComObject2<IMPLEMENTS(ISaveLoadManager), IMPLEMENTS(IGlobalObject)>
{
public:
   cSaveLoadManager();
   ~cSaveLoadManager();

   DECLARE_NAME(SaveLoadManager)
   DECLARE_NO_CONSTRAINTS()

   virtual tResult Init();
   virtual tResult Term();

   virtual tResult RegisterSaveLoadParticipant(REFGUID id, int version, ISaveLoadParticipant *);
   virtual tResult RevokeSaveLoadParticipant(REFGUID id, int version);

   virtual tResult Save(IWriter *);
   virtual tResult Load(IReader *);

private:
   struct sLessGuid
   {
      bool operator()(const GUID * pLhs, const GUID * pRhs) const
      {
         return (memcmp(pLhs, pRhs, sizeof(GUID)) < 0);
      }
   };

   typedef std::map<const GUID *, cVersionedParticipant *, sLessGuid> tParticipantMap;
   tParticipantMap m_participantMap;

   struct sFileEntry
   {
      GUID id;
      int version;
      ulong offset;
      ulong length;
   };

   friend class cReadWriteOps<sFileEntry>;
};

////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SAVELOADMANAGER_H
