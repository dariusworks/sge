///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_COLOR_H
#define INCLUDED_COLOR_H

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cColor
//

class cColor
{
public:
   cColor();
   cColor(float r, float g, float b);
   cColor(float r, float g, float b, float a);
   cColor(const float rgba[4]);
   cColor(const cColor & other);
   const cColor & operator =(const cColor & other);

   bool operator !=(const cColor & other);
   bool operator ==(const cColor & other);

   float GetRed() const;
   float GetGreen() const;
   float GetBlue() const;
   float GetAlpha() const;

   uint GetA8R8G8B8();

   const float * GetPointer() const;

private:
   float m_rgba[4];
   mutable uint m_a8r8g8b8;
};

///////////////////////////////////////

inline cColor::cColor()
 : m_a8r8g8b8(0)
{
}

///////////////////////////////////////

inline cColor::cColor(float r, float g, float b)
 : m_a8r8g8b8(0)
{
   m_rgba[0] = r;
   m_rgba[1] = g;
   m_rgba[2] = b;
   m_rgba[3] = 1;
}

///////////////////////////////////////

inline cColor::cColor(float r, float g, float b, float a)
 : m_a8r8g8b8(0)
{
   m_rgba[0] = r;
   m_rgba[1] = g;
   m_rgba[2] = b;
   m_rgba[3] = a;
}

///////////////////////////////////////

inline cColor::cColor(const float rgba[4])
 : m_a8r8g8b8(0)
{
   m_rgba[0] = rgba[0];
   m_rgba[1] = rgba[1];
   m_rgba[2] = rgba[2];
   m_rgba[3] = rgba[3];
}

///////////////////////////////////////

inline cColor::cColor(const cColor & other)
 : m_a8r8g8b8(other.m_a8r8g8b8)
{
   m_rgba[0] = other.m_rgba[0];
   m_rgba[1] = other.m_rgba[1];
   m_rgba[2] = other.m_rgba[2];
   m_rgba[3] = other.m_rgba[3];
}

///////////////////////////////////////

inline const cColor & cColor::operator =(const cColor & other)
{
   m_rgba[0] = other.m_rgba[0];
   m_rgba[1] = other.m_rgba[1];
   m_rgba[2] = other.m_rgba[2];
   m_rgba[3] = other.m_rgba[3];
   m_a8r8g8b8 = other.m_a8r8g8b8;
   return *this;
}

///////////////////////////////////////

inline bool cColor::operator !=(const cColor & other)
{
   return
      (m_rgba[0] != other.m_rgba[0]) &&
      (m_rgba[1] != other.m_rgba[1]) &&
      (m_rgba[2] != other.m_rgba[2]) &&
      (m_rgba[3] != other.m_rgba[3]);
}

///////////////////////////////////////

inline bool cColor::operator ==(const cColor & other)
{
   return
      (m_rgba[0] == other.m_rgba[0]) &&
      (m_rgba[1] == other.m_rgba[1]) &&
      (m_rgba[2] == other.m_rgba[2]) &&
      (m_rgba[3] == other.m_rgba[3]);
}

///////////////////////////////////////

inline float cColor::GetRed() const
{
   return m_rgba[0];
}

///////////////////////////////////////

inline float cColor::GetGreen() const
{
   return m_rgba[1];
}

///////////////////////////////////////

inline float cColor::GetBlue() const
{
   return m_rgba[2];
}

///////////////////////////////////////

inline float cColor::GetAlpha() const
{
   return m_rgba[3];
}

///////////////////////////////////////

inline uint cColor::GetA8R8G8B8()
{
   if (m_a8r8g8b8 == 0)
   {
      m_a8r8g8b8 = (((byte)(GetAlpha() * 255) & 0xFF) << 24)
         | (((byte)(GetRed() * 255) & 0xFF) << 16)
         | (((byte)(GetGreen() * 255) & 0xFF) << 8)
         | ((byte)(GetBlue() * 255) & 0xFF);
   }
   return m_a8r8g8b8;
}

///////////////////////////////////////

inline const float * cColor::GetPointer() const
{
   return m_rgba;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_COLOR_H
