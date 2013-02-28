///////////////////////////////////////////////////////////////////////////////
// $Id$

// The following ifdef block is the standard way of creating macros which make
// exporting from a DLL simpler. All files within this DLL are compiled with
// the MS3DMODEL_EXPORTS symbol defined on the command line. this symbol should not
// be defined on any project that uses this DLL. This way any other project
// whose source files include this file see MS3DMODEL_API functions as being
// imported from a DLL, whereas this DLL sees symbols defined with this macro
// as being exported.
#ifdef NO_AUTO_EXPORTS
#define MS3DMODEL_API
#else
#ifdef MS3DMODEL_EXPORTS
#define MS3DMODEL_API __declspec(dllexport)
#else
#define MS3DMODEL_API __declspec(dllimport)
#endif
#endif
