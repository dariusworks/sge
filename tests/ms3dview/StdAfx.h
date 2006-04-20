// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__5DF74147_C03C_48A7_9FA0_7330FF82E565__INCLUDED_)
#define AFX_STDAFX_H__5DF74147_C03C_48A7_9FA0_7330FF82E565__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

// Use the inline function versions of these from <combase.h>
#undef SUCCEEDED
#undef FAILED

#include "techtypes.h"
#include "techassert.h"
#include "techlog.h"

#pragma warning(disable:4355) // 'this' used in base member initializer list

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__5DF74147_C03C_48A7_9FA0_7330FF82E565__INCLUDED_)
