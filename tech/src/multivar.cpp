///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"
#include "multivar.h"

#include "techmath.h"
#include "techstring.h"

#ifdef HAVE_CPPUNITLITE2
#include "CppUnitLite2.h"
#endif

#include <cstdio>
#include <cstdlib>

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMultiVar
//

////////////////////////////////////////

cMultiVar::cMultiVar()
 : m_pTempBuffer(NULL)
 , m_pConversionBuffer(NULL)
 , m_type(kMVT_Empty)
{
}

////////////////////////////////////////

cMultiVar::cMultiVar(const cMultiVar & other)
 : m_pTempBuffer(NULL)
 , m_pConversionBuffer(NULL)
 , m_type(kMVT_Empty)
{
   Assign(other);
}

////////////////////////////////////////

cMultiVar::cMultiVar(int i)
 : m_pTempBuffer(NULL)
 , m_pConversionBuffer(NULL)
 , m_type(kMVT_Int)
{
   m_value.i = i;
}

////////////////////////////////////////

cMultiVar::cMultiVar(float f)
 : m_pTempBuffer(NULL)
 , m_pConversionBuffer(NULL)
 , m_type(kMVT_Float)
{
   m_value.f = f;
}

////////////////////////////////////////

cMultiVar::cMultiVar(double d)
 : m_pTempBuffer(NULL)
 , m_pConversionBuffer(NULL)
 , m_type(kMVT_Double)
{
   m_value.d = d;
}

////////////////////////////////////////

cMultiVar::cMultiVar(const char * psz)
 : m_pTempBuffer(NULL)
 , m_pConversionBuffer(NULL)
{
   Assign(psz);
}

////////////////////////////////////////

cMultiVar::cMultiVar(const wchar_t * pwsz)
 : m_pTempBuffer(NULL)
 , m_pConversionBuffer(NULL)
{
   Assign(pwsz);
}

////////////////////////////////////////

cMultiVar::cMultiVar(IUnknown * pUnk)
 : m_pTempBuffer(NULL)
 , m_pConversionBuffer(NULL)
 , m_type(kMVT_Interface)
{
   m_value.pUnk = CTAddRef(pUnk);
}

////////////////////////////////////////

cMultiVar::~cMultiVar()
{
   Clear();
}

////////////////////////////////////////

void cMultiVar::Assign(const cMultiVar & multiVar)
{
   if (GetType() != multiVar.GetType())
   {
      Clear();
   }
   if (multiVar.IsInt())
   {
      Assign(multiVar.ToInt());
   }
   else if (multiVar.IsFloat())
   {
      Assign(multiVar.ToFloat());
   }
   else if (multiVar.IsDouble())
   {
      Assign(multiVar.ToDouble());
   }
   else if (multiVar.IsString())
   {
      Assign(multiVar.m_value.psz);
   }
   else if (multiVar.IsInterface())
   {
      m_type = kMVT_Interface;
      m_value.pUnk = CTAddRef(multiVar.m_value.pUnk);
   }
}

////////////////////////////////////////

void cMultiVar::Assign(int value)
{
   if (GetType() != kMVT_Int)
   {
      Clear();
      m_type = kMVT_Int;
   }
   m_value.i = value;
}

////////////////////////////////////////

void cMultiVar::Assign(float value)
{
   if (GetType() != kMVT_Float)
   {
      Clear();
      m_type = kMVT_Float;
   }
   m_value.f = value;
}

////////////////////////////////////////

void cMultiVar::Assign(double value)
{
   if (GetType() != kMVT_Double)
   {
      Clear();
      m_type = kMVT_Double;
   }
   m_value.d = value;
}

////////////////////////////////////////

void cMultiVar::Assign(const char * pszValue)
{
   if (GetType() != kMVT_String)
   {
      Clear();
      m_type = kMVT_String;
   }
   if (pszValue != NULL)
   {
      uint length = strlen(pszValue);
      uint bufferSize = (length + 1) * sizeof(char);
      m_pTempBuffer = realloc(m_pTempBuffer, bufferSize);
      m_value.psz = reinterpret_cast<char*>(m_pTempBuffer);
      strcpy(m_value.psz, pszValue);
   }
}

////////////////////////////////////////

void cMultiVar::Assign(const wchar_t * pwszValue)
{
   if (GetType() != kMVT_String)
   {
      Clear();
      m_type = kMVT_String;
   }
   if (pwszValue != NULL)
   {
      uint length = wcslen(pwszValue);
      uint bufferSize = (length + 1) * sizeof(wchar_t);
      m_pTempBuffer = realloc(m_pTempBuffer, bufferSize);
      m_value.psz = reinterpret_cast<char*>(m_pTempBuffer);
      wcstombs(m_value.psz, pwszValue, bufferSize);
   }
}

////////////////////////////////////////

void cMultiVar::Assign(IUnknown * pUnk)
{
   if (GetType() != kMVT_String)
   {
      Clear();
      m_type = kMVT_Interface;
   }
   m_value.pUnk = CTAddRef(pUnk);
}

////////////////////////////////////////

int cMultiVar::ToInt() const
{
   if (m_type == kMVT_Int)
   {
      return m_value.i;
   }
   else if (m_type == kMVT_Float)
   {
      return FloatToInt(m_value.f);
   }
   else if (m_type == kMVT_Double)
   {
      return DoubleToInt(m_value.d);
   }
   else if (m_type == kMVT_String)
   {
      return DoubleToInt(strtod(m_value.psz, NULL));
   }
   else
   {
      ErrorMsg("Attempt to access incompatible cMultiVar as an integer\n");
      return 0;
   }
}

////////////////////////////////////////

float cMultiVar::ToFloat() const
{
   if (m_type == kMVT_Int)
   {
      return static_cast<float>(m_value.i);
   }
   else if (m_type == kMVT_Float)
   {
      return m_value.f;
   }
   else if (m_type == kMVT_Double)
   {
      return static_cast<float>(m_value.d);
   }
   else if (m_type == kMVT_String)
   {
      return static_cast<float>(strtod(m_value.psz, NULL));
   }
   else
   {
      ErrorMsg("Attempt to access incompatible cMultiVar as a float\n");
      return 0;
   }
}

////////////////////////////////////////

double cMultiVar::ToDouble() const
{
   if (m_type == kMVT_Int)
   {
      return static_cast<double>(m_value.i);
   }
   else if (m_type == kMVT_Float)
   {
      return static_cast<double>(m_value.f);
   }
   else if (m_type == kMVT_Double)
   {
      return m_value.d;
   }
   else if (m_type == kMVT_String)
   {
      return strtod(m_value.psz, NULL);
   }
   else
   {
      ErrorMsg("Attempt to access incompatible cMultiVar as a double\n");
      return 0;
   }
}

////////////////////////////////////////

const char * cMultiVar::ToAsciiString() const
{
   if (m_type == kMVT_Int)
   {
      if (m_pConversionBuffer == NULL)
      {
         std::string temp;
         Sprintf(&temp, "%d", m_value.i);
         uint length = temp.length();
         uint bufferSize = (length + 1) * sizeof(char);
         m_pConversionBuffer = realloc(m_pConversionBuffer, bufferSize);
         strcpy(reinterpret_cast<char*>(m_pConversionBuffer), temp.c_str());
      }
      return reinterpret_cast<const char*>(m_pConversionBuffer);
   }
   else if (m_type == kMVT_Float || m_type == kMVT_Double)
   {
      if (m_pConversionBuffer == NULL)
      {
         std::string temp;
         Sprintf(&temp, "%f", (m_type == kMVT_Float) ? m_value.f : m_value.d);
         uint length = temp.length();
         uint bufferSize = (length + 1) * sizeof(char);
         m_pConversionBuffer = realloc(m_pConversionBuffer, bufferSize);
         strcpy(reinterpret_cast<char*>(m_pConversionBuffer), temp.c_str());
      }
      return reinterpret_cast<const char*>(m_pConversionBuffer);
   }
   else if (m_type == kMVT_String)
   {
      return m_value.psz;
   }
   else
   {
      ErrorMsg("Attempt to access incompatible cMultiVar as a string\n");
      return NULL;
   }
}

////////////////////////////////////////

const wchar_t * cMultiVar::ToWideString() const
{
   if (m_type == kMVT_Int)
   {
      if (m_pConversionBuffer == NULL)
      {
         cStr temp;
         Sprintf(&temp, _T("%d"), m_value.i);
         uint length = temp.length();
         uint bufferSize = (length + 1) * sizeof(char);
         m_pConversionBuffer = realloc(m_pConversionBuffer, bufferSize);
#ifdef _UNICODE
         wcscpy(reinterpret_cast<wchar_t*>(m_pConversionBuffer), temp.c_str());
#else
         mbstowcs(reinterpret_cast<wchar_t*>(m_pConversionBuffer), temp.c_str(), bufferSize);
#endif
      }
      return reinterpret_cast<const wchar_t*>(m_pConversionBuffer);
   }
   else if (m_type == kMVT_Float || m_type == kMVT_Double)
   {
      if (m_pConversionBuffer == NULL)
      {
         cStr temp;
         Sprintf(&temp, _T("%f"), (m_type == kMVT_Float) ? m_value.f : m_value.d);
         uint length = temp.length();
         uint bufferSize = (length + 1) * sizeof(wchar_t);
         m_pConversionBuffer = realloc(m_pConversionBuffer, bufferSize);
#ifdef _UNICODE
         wcscpy(reinterpret_cast<wchar_t*>(m_pConversionBuffer), temp.c_str());
#else
         mbstowcs(reinterpret_cast<wchar_t*>(m_pConversionBuffer), temp.c_str(), bufferSize);
#endif
      }
      return reinterpret_cast<const wchar_t*>(m_pConversionBuffer);
   }
   else if (m_type == kMVT_String)
   {
      if (m_pConversionBuffer == NULL)
      {
         uint length = strlen(m_value.psz);
         uint bufferSize = (length + 1) * sizeof(wchar_t);
         m_pConversionBuffer = realloc(m_pConversionBuffer, bufferSize);
         mbstowcs(reinterpret_cast<wchar_t*>(m_pConversionBuffer), m_value.psz, bufferSize);
      }
      return reinterpret_cast<const wchar_t*>(m_pConversionBuffer);
   }
   else
   {
      ErrorMsg("Attempt to access incompatible cMultiVar as a wide-character string\n");
      return NULL;
   }
}

////////////////////////////////////////

cMultiVar::operator IUnknown *() const
{
   if (m_type == kMVT_Interface)
   {
      return m_value.pUnk;
   }
   return NULL;
}

////////////////////////////////////////

void cMultiVar::Clear()
{
   if (m_type == kMVT_Interface)
   {
      SafeRelease(m_value.pUnk);
   }
   if (m_pTempBuffer != NULL)
   {
      free(m_pTempBuffer);
      m_pTempBuffer = NULL;
   }
   if (m_pConversionBuffer != NULL)
   {
      free(m_pConversionBuffer);
      m_pConversionBuffer = NULL;
   }
   m_type = kMVT_Empty;
   memset(&m_value, 0, sizeof(m_value));
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNITLITE2

////////////////////////////////////////

TEST(MultiVarConstructors)
{
   {
      cMultiVar multiVarInt(9999);
      CHECK(multiVarInt.GetType() == kMVT_Int);
      CHECK(multiVarInt.IsInt());
      CHECK(multiVarInt.ToInt() == 9999);
   }

   {
      cMultiVar multiVarFloat(kPi);
      CHECK(multiVarFloat.GetType() == kMVT_Float);
      CHECK(multiVarFloat.IsFloat());
      CHECK(multiVarFloat.ToInt() == 3);
      CHECK(multiVarFloat.ToFloat() == kPi);
   }

   {
      double d = 2 * 3.14159;
      cMultiVar multiVarDouble(d);
      CHECK(multiVarDouble.GetType() == kMVT_Double);
      CHECK(multiVarDouble.IsDouble());
      CHECK(multiVarDouble.ToInt() == 6);
      CHECK(multiVarDouble.ToDouble() == d);
   }

   {
      cMultiVar multiVarString("Single-byte string");
      CHECK(multiVarString.GetType() == kMVT_String);
      CHECK(multiVarString.IsString());
      CHECK(strcmp(multiVarString.ToString(), "Single-byte string") == 0);
   }

   {
      cMultiVar multiVarWideString(L"Wide-character string");
      CHECK(multiVarWideString.GetType() == kMVT_String);
      CHECK(multiVarWideString.IsString());
      CHECK(wcscmp(multiVarWideString.ToWideString(), L"Wide-character string") == 0);
   }

   {
      cMultiVar multiVarInterface(static_cast<IUnknown*>(NULL));
      CHECK(multiVarInterface.GetType() == kMVT_Interface);
      CHECK(multiVarInterface.IsInterface());
   }

   {
      cMultiVar multiVarEmpty;
      CHECK(multiVarEmpty.GetType() == kMVT_Empty);
      CHECK(multiVarEmpty.IsEmpty());
   }
}

////////////////////////////////////////

TEST(MultiVarAssignment)
{
   cMultiVar test;
   CHECK_EQUAL((double)(test = 3.1415), 3.1415);
   CHECK(strcmp((test = "3.1415"), "3.1415") == 0);
}

////////////////////////////////////////

TEST(MultiVarCastDouble)
{
   CHECK_EQUAL((double)cMultiVar(3.1415), 3.1415);
   CHECK_EQUAL((double)cMultiVar("3.1415"), 3.1415);
}

////////////////////////////////////////

TEST(MultiVarCastString)
{
   CHECK(strcmp((const char *)cMultiVar(3.1415), "3.141500") == 0);
   CHECK(strcmp((const char *)cMultiVar("3.1415"), "3.1415") == 0);
}

////////////////////////////////////////

TEST(MultiVarCastWideCharString)
{
   CHECK(wcscmp((const wchar_t *)cMultiVar(3.1415), L"3.141500") == 0);
   CHECK(wcscmp((const wchar_t *)cMultiVar("3.1415"), L"3.1415") == 0);
}

////////////////////////////////////////

TEST(MultiVarChangeType)
{
   cMultiVar v(kPi);
   CHECK(v.ToFloat() == kPi);
   v = L"3.141500";
   CHECK(wcscmp((const wchar_t *)cMultiVar(3.1415), L"3.141500") == 0);
}

#endif // HAVE_CPPUNITLITE2

///////////////////////////////////////////////////////////////////////////////
