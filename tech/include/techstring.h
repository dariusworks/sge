///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_TECHSTRING_H
#define INCLUDED_TECHSTRING_H

#include "techdll.h"

#include <cstdarg>
#include <string>
#include <vector>

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////

TECH_API int SprintfLengthEstimate(const tChar * pszFormat, va_list args);


///////////////////////////////////////////////////////////////////////////////

template <typename STRING>
const STRING & CDECL Sprintf(STRING * pString, const tChar * pszFormat, ...)
{
   Assert(pString != NULL);
   va_list args;
   va_start(args, pszFormat);
   int length = SprintfLengthEstimate(pszFormat, args) + 1; // plus one for null terminator
   size_t nBytes = length * sizeof(typename STRING::value_type);
   typename STRING::value_type * pszTemp = reinterpret_cast<typename STRING::value_type *>(alloca(nBytes));
   int result = _vsntprintf(pszTemp, length, pszFormat, args);
   va_end(args);
   pString->assign(pszTemp);
   return *pString;
}


///////////////////////////////////////////////////////////////////////////////

typedef void (* tParseTupleCallbackFn)(const tChar * pszToken, uint_ptr userData);

TECH_API int ParseTuple(const tChar * pszIn, const tChar * pszDelims,
                        tParseTupleCallbackFn pfnCallback, uint_ptr userData);


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cToken
//
// An accessory class to cTokenizer used to create typed tokens from
// a string representation

template <typename TOKEN, typename CHAR = tChar>
class cToken
{
public:
   static TOKEN Token(const CHAR * pszToken)
   {
      Assert(!"Cannot use default token convert function!");
      return TOKEN();
   }
};

///////////////////////////////////////

template <>
std::string cToken<std::string, tChar>::Token(const tChar * pszToken)
{
   return std::string(pszToken);
}

///////////////////////////////////////

template <>
double cToken<double, tChar>::Token(const tChar * pszToken)
{
#ifdef _UNICODE
   return _wtof(pszToken);
#else
   return atof(pszToken);
#endif
}

///////////////////////////////////////

template <>
float cToken<float, tChar>::Token(const tChar * pszToken)
{
#ifdef _UNICODE
   return static_cast<float>(_wtof(pszToken));
#else
   return static_cast<float>(atof(pszToken));
#endif
}

///////////////////////////////////////

template <>
int cToken<int, tChar>::Token(const tChar * pszToken)
{
#ifdef __GNUC__
   return Round(strtod(pszToken, NULL));
#else
   return _ttoi(pszToken);
#endif
}


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cTokenizer
//
// Really just a helper class that wraps the ParseTuple function

template <typename TOKEN, typename CONTAINER = std::vector<TOKEN>, typename CHAR = tChar>
class cTokenizer
{
public:
   int Tokenize(const CHAR * pszIn, const CHAR * pszDelims = NULL)
   {
      tParseTupleCallbackFn pfn = &cTokenizer<TOKEN, CONTAINER, CHAR>::FillContainer;
      return ::ParseTuple(pszIn, pszDelims, pfn, (uint_ptr)&m_tokens);
   }

   static void FillContainer(const CHAR * pszToken, uint_ptr userData)
   {
      CONTAINER * pContainer = (CONTAINER *)userData;
      if (pszToken && pContainer)
      {
         pContainer->push_back(cToken<TOKEN>::Token(pszToken));
      }
   }

   CONTAINER m_tokens;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cStr
//

#ifdef _UNICODE
typedef std::wstring cStrBase;
#else
typedef std::string cStrBase;
#endif

class TECH_API cStr : public cStrBase
{
public:
   cStr();
   cStr(const tChar * psz);
   cStr(const tChar * psz, uint length);

   const cStr & operator =(const cStr & other);
   bool operator ==(const cStr & other) const;
   bool operator !=(const cStr & other) const;
   const cStr & operator =(const tChar * psz);
   bool operator ==(const tChar * psz) const;
   bool operator !=(const tChar * psz) const;

   const tChar * Get() const;
   uint GetLength() const;
   void Empty();
   bool IsEmpty() const;

   void Append(const tChar * psz);
   void TrimLeadingSpace();
   void TrimTrailingSpace();

   int ToInt() const;
   float ToFloat() const;
   double ToDouble() const;

   static const cStr gm_whitespace;
};

///////////////////////////////////////

inline cStr::cStr()
{
}

///////////////////////////////////////

inline cStr::cStr(const tChar * psz)
 : cStrBase(psz)
{
}

///////////////////////////////////////

inline cStr::cStr(const tChar * psz, uint length)
 : cStrBase(psz, length)
{
}

///////////////////////////////////////

inline const cStr & cStr::operator =(const cStr & other)
{
   assign(other);
   return *this;
}

///////////////////////////////////////

inline bool cStr::operator ==(const cStr & other) const
{
   return compare(other) == 0;
}

///////////////////////////////////////

inline bool cStr::operator !=(const cStr & other) const
{
   return compare(other) != 0;
}

///////////////////////////////////////

inline const cStr & cStr::operator =(const tChar * psz)
{
   assign(psz);
   return *this;
}

///////////////////////////////////////

inline bool cStr::operator ==(const tChar * psz) const
{
   return compare(psz) == 0;
}

///////////////////////////////////////

inline bool cStr::operator !=(const tChar * psz) const
{
   return compare(psz) != 0;
}

///////////////////////////////////////

inline const tChar * cStr::Get() const
{
   return c_str();
}

///////////////////////////////////////

inline uint cStr::GetLength() const
{
   return length();
}

///////////////////////////////////////

inline void cStr::Empty()
{
   erase();
}

///////////////////////////////////////

inline bool cStr::IsEmpty() const
{
   return empty();
}

///////////////////////////////////////

inline void cStr::Append(const tChar * psz)
{
   *this += psz;
}

///////////////////////////////////////

inline void cStr::TrimLeadingSpace()
{
   cStrBase::size_type index = find_first_not_of(gm_whitespace);
   if (index != cStrBase::npos)
   {
      erase(0, index);
   }
}

///////////////////////////////////////

inline void cStr::TrimTrailingSpace()
{
   cStrBase::size_type index = find_last_not_of(gm_whitespace);
   if (index != cStrBase::npos)
   {
      erase(index + 1);
   }
}

///////////////////////////////////////

template <>
cStr cToken<cStr, tChar>::Token(const tChar * pszToken)
{
   return cStr(pszToken);
}


///////////////////////////////////////////////////////////////////////////////

typedef struct _GUID GUID;
typedef const GUID & REFGUID;
TECH_API const cStr & GUIDToString(REFGUID guid, cStr * pStr);


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cStrLessNoCase
//

class cStrLessNoCase
{
public:
   bool operator()(const cStr & lhs, const cStr & rhs) const;
};

///////////////////////////////////////

inline bool cStrLessNoCase::operator()(const cStr & lhs, const cStr & rhs) const
{
   return (_tcsicmp(lhs.c_str(), rhs.c_str()) < 0) ? true : false;
}

///////////////////////////////////////////////////////////////////////////////

int filepathcmp(const cStr & f1, const cStr & f2);
int filepathicmp(const cStr & f1, const cStr & f2);

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_TECHSTRING_H
