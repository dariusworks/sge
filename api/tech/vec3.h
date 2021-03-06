///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_VEC3_H
#define INCLUDED_VEC3_H

#include "techmath.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cVec3
//

template <typename T>
class cVec3
{
public:
   typedef T value_type;
   typedef const cVec3<T> & const_reference;

   cVec3();
   cVec3(value_type xx, value_type yy, value_type zz);
   cVec3(const value_type v[3]);
   cVec3(const_reference other);

   const_reference operator =(const_reference other);
   const_reference operator +=(const_reference other);
   const_reference operator -=(const_reference other);
   const_reference operator *=(value_type scale);
   const_reference operator /=(value_type divisor);

   cVec3 operator -() const;

   value_type Length() const;
   value_type LengthSqr() const;
   void Normalize();
   value_type Dot(const_reference other) const;
   cVec3 Cross(const_reference other) const;

   union
   {
      struct
      {
         value_type x, y, z;
      };
      value_type v[3];
   };
};

///////////////////////////////////////

template <typename T>
inline cVec3<T>::cVec3()
{
}

///////////////////////////////////////

template <typename T>
inline cVec3<T>::cVec3(value_type xx, value_type yy, value_type zz)
: x(xx)
, y(yy)
, z(zz)
{
}

///////////////////////////////////////

template <typename T>
inline cVec3<T>::cVec3(const value_type v[3])
: x(v[0])
, y(v[1])
, z(v[2])
{
}

///////////////////////////////////////

template <typename T>
inline cVec3<T>::cVec3(const_reference other)
: x(other.x)
, y(other.y)
, z(other.z)
{
}

///////////////////////////////////////

template <typename T>
inline typename cVec3<T>::const_reference cVec3<T>::operator =(const_reference other)
{
   x = other.x;
   y = other.y;
   z = other.z;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline typename cVec3<T>::const_reference cVec3<T>::operator +=(const_reference other)
{
   x += other.x;
   y += other.y;
   z += other.z;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline typename cVec3<T>::const_reference cVec3<T>::operator -=(const_reference other)
{
   x -= other.x;
   y -= other.y;
   z -= other.z;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline typename cVec3<T>::const_reference cVec3<T>::operator *=(value_type scale)
{
   x *= scale;
   y *= scale;
   z *= scale;
   return *this;
}

///////////////////////////////////////

template <typename T>
inline typename cVec3<T>::const_reference cVec3<T>::operator /=(value_type divisor)
{
   x /= divisor;
   y /= divisor;
   z /= divisor;
   return *this;
}

///////////////////////////////////////

template <typename T>
cVec3<T> cVec3<T>::operator -() const
{
   return cVec3<T>(-x, -y, -z);
}

///////////////////////////////////////

template <typename T>
inline typename cVec3<T>::value_type cVec3<T>::Length() const
{
   return sqrt(LengthSqr());
}

template <>
inline float cVec3<float>::Length() const
{
   return sqrtf(LengthSqr());
}

///////////////////////////////////////

template <typename T>
inline typename cVec3<T>::value_type cVec3<T>::LengthSqr() const
{
   return x * x + y * y + z * z;
}

///////////////////////////////////////

template <typename T>
inline void cVec3<T>::Normalize()
{
   value_type m = Length();
   if (m > 0)
      m = 1.0f / m;
   else
      m = 0;
   operator *=(m);
}

///////////////////////////////////////

template <typename T>
inline typename cVec3<T>::value_type cVec3<T>::Dot(const_reference other) const
{
   return x * other.x + y * other.y + z * other.z;
}

///////////////////////////////////////

template <typename T>
inline cVec3<T> cVec3<T>::Cross(const_reference other) const
{
   return cVec3(y * other.z - z * other.y,
                z * other.x - x * other.z,
                x * other.y - y * other.x);
}

///////////////////////////////////////

template <typename T>
inline cVec3<T> operator -(const cVec3<T> & a, const cVec3<T> & b)
{
   return cVec3<T>(a.x - b.x, a.y - b.y, a.z - b.z);
}

///////////////////////////////////////

template <typename T>
inline cVec3<T> operator +(const cVec3<T> & a, const cVec3<T> & b)
{
   return cVec3<T>(a.x + b.x, a.y + b.y, a.z + b.z);
}

///////////////////////////////////////

template <typename T>
inline cVec3<T> operator *(const cVec3<T> & a, typename cVec3<T>::value_type scale)
{
   return cVec3<T>(a.x * scale, a.y * scale, a.z * scale);
}

///////////////////////////////////////

template <typename T>
inline cVec3<T> operator /(const cVec3<T> & a, typename cVec3<T>::value_type divisor)
{
   return cVec3<T>(a.x / divisor, a.y / divisor, a.z / divisor);
}

///////////////////////////////////////
// Assumes 'u' is in the range [0..1]

template <typename T>
inline cVec3<T> Lerp(const cVec3<T> & v1, const cVec3<T> & v2, T u)
{
   return (v1 * (1 - u)) + (v2 * u);
}

///////////////////////////////////////

template <typename T>
inline bool AlmostEqual(const cVec3<T> & v1, const cVec3<T> & v2, int maxUnitsLastPlace)
{
   return AlmostEqual(v1.x, v2.x, maxUnitsLastPlace)
      && AlmostEqual(v1.y, v2.y, maxUnitsLastPlace)
      && AlmostEqual(v1.z, v2.z, maxUnitsLastPlace);
}

///////////////////////////////////////

inline bool AlmostEqual(const cVec3<float> & v1, const cVec3<float> & v2,
                        int maxUnitsLastPlace = kFloatMaxUnitsLastPlace)
{
   return AlmostEqual(v1.x, v2.x, maxUnitsLastPlace)
      && AlmostEqual(v1.y, v2.y, maxUnitsLastPlace)
      && AlmostEqual(v1.z, v2.z, maxUnitsLastPlace);
}

///////////////////////////////////////

inline bool AlmostEqual(const cVec3<double> & v1, const cVec3<double> & v2,
                        int maxUnitsLastPlace = kDoubleMaxUnitsLastPlace)
{
   return AlmostEqual(v1.x, v2.x, maxUnitsLastPlace)
      && AlmostEqual(v1.y, v2.y, maxUnitsLastPlace)
      && AlmostEqual(v1.z, v2.z, maxUnitsLastPlace);
}

///////////////////////////////////////

#ifndef NO_DEFAULT_VEC3
typedef class TECH_API cVec3<float> tVec3;
#endif

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_VEC3_H

