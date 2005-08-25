///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_READWRITEFILE_H
#define INCLUDED_READWRITEFILE_H

#include "readwriteapi.h"
#include <stdio.h>

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFileReader
//

class cFileReader : public cComObject<IMPLEMENTS(IReader)>
{
public:
   cFileReader(const cFileSpec & file);
   virtual ~cFileReader();

   virtual void OnFinalRelease();

   virtual tResult Tell(ulong * pPos);
   virtual tResult Seek(long pos, eSeekOrigin origin);

   virtual tResult Read(cStr * pValue, tChar stop);
   virtual tResult Read(void * pv, size_t cb, size_t * pcbRead = NULL);

private:
   FILE * m_fp;
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cFileWriter
//

class cFileWriter : public cComObject<IMPLEMENTS(IWriter)>
{
public:
   cFileWriter(const cFileSpec & file);
   virtual ~cFileWriter();

   virtual void OnFinalRelease();

   virtual tResult Write(const char * value);
   virtual tResult Write(void * pv, size_t cb, size_t * pcbWritten = NULL);

private:
   FILE * m_fp;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_READWRITEFILE_H
