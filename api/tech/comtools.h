///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_COMTOOLS_H
#define INCLUDED_COMTOOLS_H

#include "techdll.h"
#include "combase.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

inline bool CTIsEqualGUID(REFGUID rguid1, REFGUID rguid2)
{
   return (
	  ((long*) &rguid1)[0] == ((long*) &rguid2)[0] &&
	  ((long*) &rguid1)[1] == ((long*) &rguid2)[1] &&
	  ((long*) &rguid1)[2] == ((long*) &rguid2)[2] &&
	  ((long*) &rguid1)[3] == ((long*) &rguid2)[3]);
}

inline bool CTIsEqualUnknown(REFGUID rguid)
{
   return CTIsEqualGUID(rguid, IID_IUnknown);
}

///////////////////////////////////////////////////////////////////////////////

template <class T>
ulong SafeRelease(T & p)
{
   if (!!p)
   {
      ulong result = p->Release();
      (void * &)p = NULL;
      return result;
   }
   return (ulong)-1;
}

///////////////////////////////////////

#define CTAddRef(p) (((p) != NULL) ? ((p)->AddRef(), (p)) : (p))

///////////////////////////////////////////////////////////////////////////////

TECH_API bool CTIsSameObject(IUnknown * pUnk1, IUnknown * pUnk2);

///////////////////////////////////////////////////////////////////////////////

class TECH_API cCTLessInterface
{
public:
   bool operator()(IUnknown * pUnk1, IUnknown * pUnk2) const;
};

///////////////////////////////////////////////////////////////////////////////

struct sQIPair
{
   IUnknown * pUnk;
   const IID * pIID;
};

inline tResult CTQueryInterface(const sQIPair * pPairs, int nPairs,
                                REFGUID iid, void * * ppvObject)
{
   for (int i = 0; i < nPairs; i++)
   {
      if (CTIsEqualGUID(iid, *pPairs[i].pIID))
      {
         pPairs[i].pUnk->AddRef();
         *ppvObject = pPairs[i].pUnk;
         return S_OK;
      }
   }
   if (CTIsEqualUnknown(iid))
   {
      pPairs[0].pUnk->AddRef();
      *ppvObject = pPairs[0].pUnk;
      return S_OK;
   }
   *ppvObject = NULL;
   return E_NOINTERFACE;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cNonDelegatingUnknown
//

class cNonDelegatingUnknown
{
protected:
   cNonDelegatingUnknown()
    : m_cRef(1)
   {
   }

   virtual ~cNonDelegatingUnknown()
   {
   }

   tResult DoQueryInterface(const sQIPair * pPairs, int nPairs,
                            REFGUID iid, void * * ppvObject)
   {
      return CTQueryInterface(pPairs, nPairs, iid, ppvObject);
   }

   ulong DoAddRef()
   {
      return ++m_cRef;
   }

   ulong DoRelease()
   {
      if (--m_cRef)
      {
         return m_cRef;
      }
      OnFinalRelease();
      DeleteThis();
      return 0;
   }

   virtual void OnFinalRelease()
   {
   }

   virtual void DeleteThis()
   {
      delete this;
   }

private:
   ulong m_cRef;
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cDelegatingUnknown
//

class cDelegatingUnknown
{
protected:
   cDelegatingUnknown(IUnknown * pUnkOuter)
    : m_pUnkOuter(pUnkOuter)
   {
   }

   virtual ~cDelegatingUnknown()
   {
   }

   tResult DoQueryInterface(REFGUID iid, void * * ppvObject)
   {
      return m_pUnkOuter->QueryInterface(iid, ppvObject);
   }

   ulong DoAddRef()
   {
      return m_pUnkOuter->AddRef();
   }

   ulong DoRelease()
   {
      return m_pUnkOuter->Release();
   }

private:
   IUnknown * m_pUnkOuter;
};


///////////////////////////////////////////////////////////////////////////////
//
// COM object base classes
//
// Example:
//    class cFoo : public cComObject<IMPLEMENTS(IFoo)>
//    {
//    public:
//    };

#define IMPLEMENTS(Interface) Interface, &IID_##Interface

template <class INTRFC, const IID * PIID, 
          class UNKNOWN = cNonDelegatingUnknown>
class cComObject : public INTRFC, public UNKNOWN
{
public:
   virtual ~cComObject() {}
   virtual ulong STDMETHODCALLTYPE AddRef() { return UNKNOWN::DoAddRef(); }
   virtual ulong STDMETHODCALLTYPE Release() { return UNKNOWN::DoRelease(); }
   virtual tResult STDMETHODCALLTYPE QueryInterface(REFGUID iid,
                                                    void * * ppvObject)
   {
      const struct sQIPair pairs[] =
      {
         { static_cast<INTRFC *>(this), PIID },
      };
      return UNKNOWN::DoQueryInterface(pairs, _countof(pairs), iid, ppvObject);
   }
};

template <class INTRFC1, const IID * PIID1, 
          class INTRFC2, const IID * PIID2,
          class UNKNOWN = cNonDelegatingUnknown>
class cComObject2 : public INTRFC1, public INTRFC2, public UNKNOWN
{
public:
   virtual ~cComObject2() {}
   virtual ulong STDMETHODCALLTYPE AddRef() { return UNKNOWN::DoAddRef(); }
   virtual ulong STDMETHODCALLTYPE Release() { return UNKNOWN::DoRelease(); }
   virtual tResult STDMETHODCALLTYPE QueryInterface(REFGUID iid,
                                                    void * * ppvObject)
   {
      const struct sQIPair pairs[] =
      {
         { static_cast<INTRFC1 *>(this), PIID1 },
         { static_cast<INTRFC2 *>(this), PIID2 },
      };
      return UNKNOWN::DoQueryInterface(pairs, _countof(pairs), iid, ppvObject);
   }
};

template <class INTRFC1, const IID * PIID1, 
          class INTRFC2, const IID * PIID2,
          class INTRFC3, const IID * PIID3,
          class UNKNOWN = cNonDelegatingUnknown>
class cComObject3 : public INTRFC1, public INTRFC2, public INTRFC3, public UNKNOWN
{
public:
   virtual ~cComObject3() {}
   virtual ulong STDMETHODCALLTYPE AddRef() { return UNKNOWN::DoAddRef(); }
   virtual ulong STDMETHODCALLTYPE Release() { return UNKNOWN::DoRelease(); }
   virtual tResult STDMETHODCALLTYPE QueryInterface(REFGUID iid,
                                                    void * * ppvObject)
   {
      const struct sQIPair pairs[] =
      {
         { static_cast<INTRFC1 *>(this), PIID1 },
         { static_cast<INTRFC2 *>(this), PIID2 },
         { static_cast<INTRFC3 *>(this), PIID3 },
      };
      return UNKNOWN::DoQueryInterface(pairs, _countof(pairs), iid, ppvObject);
   }
};

template <class INTRFC1, const IID * PIID1, 
          class INTRFC2, const IID * PIID2,
          class INTRFC3, const IID * PIID3,
          class INTRFC4, const IID * PIID4,
          class UNKNOWN = cNonDelegatingUnknown>
class cComObject4 : public INTRFC1, public INTRFC2, public INTRFC3, public INTRFC4, public UNKNOWN
{
public:
   virtual ~cComObject4() {}
   virtual ulong STDMETHODCALLTYPE AddRef() { return UNKNOWN::DoAddRef(); }
   virtual ulong STDMETHODCALLTYPE Release() { return UNKNOWN::DoRelease(); }
   virtual tResult STDMETHODCALLTYPE QueryInterface(REFGUID iid,
                                                    void * * ppvObject)
   {
      const struct sQIPair pairs[] =
      {
         { static_cast<INTRFC1 *>(this), PIID1 },
         { static_cast<INTRFC2 *>(this), PIID2 },
         { static_cast<INTRFC3 *>(this), PIID3 },
         { static_cast<INTRFC4 *>(this), PIID4 },
      };
      return UNKNOWN::DoQueryInterface(pairs, _countof(pairs), iid, ppvObject);
   }
};

template <class INTRFC, const IID * PIID, 
          class UNKNOWN = cDelegatingUnknown>
class cComAggregableObject : public INTRFC, public UNKNOWN
{
public:
   cComAggregableObject(IUnknown * pUnkOuter);
   virtual ~cComAggregableObject();
   virtual ulong STDMETHODCALLTYPE AddRef() { return UNKNOWN::DoAddRef(); }
   virtual ulong STDMETHODCALLTYPE Release() { return UNKNOWN::DoRelease(); }
   virtual tResult STDMETHODCALLTYPE QueryInterface(REFGUID iid,
                                                    void * * ppvObject)
   {
      return UNKNOWN::DoQueryInterface(iid, ppvObject);
   }
protected:
   IUnknown * AccessInnerUnknown() { return &m_innerUnknown; }
private:
   class cInnerUnknown : public cComObject<IMPLEMENTS(IUnknown), cNonDelegatingUnknown>
   {
   public:
      cInnerUnknown(cComAggregableObject<INTRFC, PIID, UNKNOWN> * pOuter) : m_pOuter(pOuter) {}
      ~cInnerUnknown() {}
      virtual tResult STDMETHODCALLTYPE QueryInterface(REFGUID iid, void * * ppvObject)
      {
         const struct sQIPair pairs[] =
         {
            { static_cast<IUnknown *>(this), &IID_IUnknown },
            { static_cast<INTRFC *>(m_pOuter), PIID },
         };
         return cNonDelegatingUnknown::DoQueryInterface(pairs, _countof(pairs), iid, ppvObject);
      }
      virtual void DeleteThis() { delete m_pOuter; }
   private:
      cComAggregableObject<INTRFC, PIID, UNKNOWN> * m_pOuter;
   };
   friend class cInnerUnknown;
   cInnerUnknown m_innerUnknown;
};

////////////////////////////////////////

#pragma warning(push)
#pragma warning(disable:4355) // 'this' : used in base member initializer list

template <class INTRFC, const IID * PIID, class UNKNOWN>
cComAggregableObject<INTRFC, PIID, UNKNOWN>::cComAggregableObject(IUnknown * pUnkOuter)
 : UNKNOWN((pUnkOuter != NULL) ? pUnkOuter : &m_innerUnknown)
 , m_innerUnknown(this)
{
}

#pragma warning(pop)

////////////////////////////////////////

template <class INTRFC, const IID * PIID, class UNKNOWN>
cComAggregableObject<INTRFC, PIID, UNKNOWN>::~cComAggregableObject()
{
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cAutoIPtr
//

template <class INTRFC>
class cAutoIPtr
{
#ifdef __GNUC__
   typedef void * NullType;
#else
   typedef int NullType;
#endif

public:
   cAutoIPtr();
   cAutoIPtr(INTRFC * pI);
   cAutoIPtr(const cAutoIPtr<INTRFC> & p);

   ~cAutoIPtr();

   const cAutoIPtr<INTRFC> & operator =(NullType); // assign to NULL
   const cAutoIPtr<INTRFC> & operator =(INTRFC * pI);
   const cAutoIPtr<INTRFC> & operator =(const cAutoIPtr<INTRFC> & p);

   INTRFC ** operator &();
   INTRFC * operator ->();
   const INTRFC * operator ->() const;
   operator INTRFC *();
   operator const INTRFC *() const;
   bool operator !() const;
   bool operator !=(NullType) const; // if (p != NULL) ...

   INTRFC * AccessPointer() const;
   tResult GetPointer(INTRFC * * ppInterface) const;

private:
   INTRFC * m_pInterface;
};

///////////////////////////////////////

template <class INTRFC>
inline cAutoIPtr<INTRFC>::cAutoIPtr()
 : m_pInterface(NULL)
{
}

///////////////////////////////////////
// when receiving raw interface pointer, assume ownership of existing reference

template <class INTRFC>
inline cAutoIPtr<INTRFC>::cAutoIPtr(INTRFC * pI)
 : m_pInterface(pI)
{
}

///////////////////////////////////////
// when copying smart pointer, obtain own reference

template <class INTRFC>
inline cAutoIPtr<INTRFC>::cAutoIPtr(const cAutoIPtr<INTRFC> & p)
 : m_pInterface(p.m_pInterface)
{
   if (m_pInterface != NULL)
      m_pInterface->AddRef();
}

///////////////////////////////////////

template <class INTRFC>
inline cAutoIPtr<INTRFC>::~cAutoIPtr()
{
   SafeRelease(m_pInterface);
}

///////////////////////////////////////

template <class INTRFC>
inline const cAutoIPtr<INTRFC> & cAutoIPtr<INTRFC>::operator =(NullType null)
{
   Assert(null == NULL);
   m_pInterface = NULL;
   return *this;
}

///////////////////////////////////////
// when receiving raw interface pointer, assume ownership of existing reference

template <class INTRFC>
inline const cAutoIPtr<INTRFC> & cAutoIPtr<INTRFC>::operator =(INTRFC * pI)
{
   INTRFC * pFormer = m_pInterface;
   m_pInterface = pI;
   SafeRelease(pFormer);
   return *this;
}

///////////////////////////////////////
// when copying smart pointer, obtain own reference

template <class INTRFC>
inline const cAutoIPtr<INTRFC> & cAutoIPtr<INTRFC>::operator =(const cAutoIPtr<INTRFC> & p)
{
   INTRFC * pFormer = m_pInterface;
   m_pInterface = p.m_pInterface;
   if (m_pInterface != NULL)
      m_pInterface->AddRef();
   SafeRelease(pFormer);
   return *this;
}

///////////////////////////////////////

template <class INTRFC>
inline INTRFC * * cAutoIPtr<INTRFC>::operator &()
{
   // Taking the address of the inner pointer most likely means that the pointer
   // is about to be filled in a la QueryInterface. Better not have a managed
   // pointer in that case because it could get lost.
   Assert(m_pInterface == NULL);
   return &m_pInterface;
}

///////////////////////////////////////

template <class INTRFC>
inline INTRFC * cAutoIPtr<INTRFC>::operator ->()
{
   Assert(m_pInterface != NULL);
   return m_pInterface;
}

///////////////////////////////////////

template <class INTRFC>
inline const INTRFC * cAutoIPtr<INTRFC>::operator ->() const
{
   Assert(m_pInterface != NULL);
   return m_pInterface;
}

///////////////////////////////////////

template <class INTRFC>
inline cAutoIPtr<INTRFC>::operator INTRFC *()
{
   return m_pInterface;
}

///////////////////////////////////////

template <class INTRFC>
inline cAutoIPtr<INTRFC>::operator const INTRFC *() const
{
   return m_pInterface;
}

///////////////////////////////////////

template <class INTRFC>
inline bool cAutoIPtr<INTRFC>::operator !() const
{
   return (m_pInterface == NULL);
}

///////////////////////////////////////

template <class INTRFC>
inline bool cAutoIPtr<INTRFC>::operator !=(NullType null) const
{
   Assert(null == NULL);
   return (m_pInterface != NULL);
}

///////////////////////////////////////

template <class INTRFC>
inline INTRFC * cAutoIPtr<INTRFC>::AccessPointer() const
{
   return m_pInterface;
}

///////////////////////////////////////

template <class INTRFC>
inline tResult cAutoIPtr<INTRFC>::GetPointer(INTRFC * * ppInterface) const
{
   if (ppInterface == NULL)
      return E_POINTER;
   if (m_pInterface == NULL)
      return S_FALSE;
   *ppInterface = m_pInterface;
   m_pInterface->AddRef();
   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_COMTOOLS_H
