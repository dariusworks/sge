/////////////////////////////////////////////////////////////////////////////
// $Id$

#if !defined(INCLUDED_DDXEXT_H)
#define INCLUDED_DDXEXT_H

#include <atlddx.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cWinDataExchangeEx
//

template <typename T>
class cWinDataExchangeEx : public CWinDataExchange<T>
{
public:
   template <typename INDEX>
   BOOL DDX_Combo_Index(UINT id, INDEX & index, BOOL bSigned, BOOL bSave, 
      BOOL bValidate = FALSE, INDEX minIndex = 0, INDEX maxIndex = 0)
   {
      T* pT = static_cast<T*>(this);
      BOOL bSuccess = TRUE;

      if (bSave)
      {
         index = ::SendDlgItemMessage(pT->m_hWnd, id, CB_GETCURSEL, 0, 0);
         bSuccess = (index != CB_ERR);	
      }
      else
      {
         Assert(!bValidate || index >= minIndex && index <= maxIndex);
         int result = ::SendDlgItemMessage(pT->m_hWnd, id, CB_SETCURSEL, index, 0);
         // Return value of CB_ERR just means the selection was cleared which
         // is not really an error, so bSuccess is TRUE always.
         bSuccess = TRUE;
      }

      if (!bSuccess)
      {
         pT->OnDataExchangeError(id, bSave);
      }
      else if (bSave && bValidate)
      {
         Assert(minIndex != maxIndex);
         if (index < minIndex || index > maxIndex)
         {
            _XData data;
            data.nDataType = ddxDataInt;
            data.intData.nVal = (long)index;
            data.intData.nMin = (long)minIndex;
            data.intData.nMax = (long)maxIndex;
            pT->OnDataValidateError(id, bSave, data);
            bSuccess = FALSE;
         }
      }

      return bSuccess;
   }
};

/////////////////////////////////////////////////////////////////////////////

#define DDX_COMBO_INDEX(nID, var) \
   if (nCtlID == (UINT)-1 || nCtlID == nID) \
   { \
      if (!DDX_Combo_Index(nID, var, TRUE, bSaveAndValidate)) \
         return FALSE; \
   }

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(INCLUDED_DDXEXT_H)
