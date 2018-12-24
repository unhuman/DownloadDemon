/**************************************************************************

   File:          DialUp.cpp
   
   Description:   

**************************************************************************/

/**************************************************************************
   #include statements
**************************************************************************/

#include "stdafx.h"
#include <windows.h>
#include "DialUp.h"

/**************************************************************************
   private function prototypes
**************************************************************************/

LPITEMIDLIST GetDialUpNetworkingPidl(LPSHELLFOLDER);
LPITEMIDLIST GetDialUpItem(LPCTSTR, LPMALLOC);
LPITEMIDLIST GetNewConnectionItem(LPMALLOC);
BOOL ExecuteDialUpConnection(LPCTSTR);
BOOL GetItemIdName(LPSHELLFOLDER, LPITEMIDLIST, DWORD, LPTSTR, UINT);
LPITEMIDLIST Pidl_Concatenate(LPMALLOC, LPCITEMIDLIST, LPCITEMIDLIST);
UINT Pidl_GetSize(LPCITEMIDLIST);
LPITEMIDLIST Pidl_Create(LPMALLOC, UINT);
inline LPITEMIDLIST Pidl_GetNextItem(LPCITEMIDLIST);

/**************************************************************************
   global variables
**************************************************************************/

/**************************************************************************

   EnumDialUpConnections()
   
   Parameters:
   
   Returns - 

**************************************************************************/

void EnumDialUpConnections(LPENUMDUITEMS lpProc, LPARAM lParam)
{
HRESULT        hr;
LPSHELLFOLDER  pDesktop,
               pDialUp;
LPITEMIDLIST   pidlTemp,
               pidlDialUp;
LPENUMIDLIST   pEnum;
DWORD          dwRetrieved;
TCHAR          szTemp[MAX_PATH];
int            i = 0;

if(FAILED(SHGetDesktopFolder(&pDesktop)))
   return;

pidlDialUp = GetDialUpNetworkingPidl(pDesktop);

if(pidlDialUp)
   {
   //get the IShellFolder for Dial-Up Networking
   hr = pDesktop->BindToObject(  pidlDialUp, 
                                 NULL, 
                                 IID_IShellFolder, 
                                 (LPVOID*)&pDialUp);

   if(SUCCEEDED(hr)) 
      {
      if(SUCCEEDED(pDialUp->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &pEnum)))
         {
         //enumerate the item's PIDLs, looking for the one requested
         do
            {
            hr = pEnum->Next(1, &pidlTemp, &dwRetrieved);

            //check it's name
            GetItemIdName(pDialUp, pidlTemp, SHGDN_NORMAL, szTemp, sizeof(szTemp));

            if(i && SUCCEEDED(hr) && dwRetrieved)
               {
               if(!lpProc(szTemp, lParam))
                  break;
               }   
            i = 1;
            }
         while(SUCCEEDED(hr) && dwRetrieved);

         pEnum->Release();
         }

      pDialUp->Release();
      }
   }

pDesktop->Release();

}

/**************************************************************************

   CreateDialUpLink()
   
   Parameters:

      LPTSTR lpszConnectionName - The display name of the existing dial-up 
                                 connection to create the shortcut for.
      LPTSTR lpszLinkName -       The name of the shortcut. The .LNK 
                                 extension will be added to the end of 
                                 this name if it is not present.
   
   Returns - Boolean indicating succes or failure.

**************************************************************************/

BOOL CreateDialUpLink(LPTSTR lpszConnectionName, LPTSTR lpszLinkName)
{
HRESULT        hr;
IShellLink     *pShellLink; 
BOOL           bReturn;
LPITEMIDLIST   pidlConnection;
LPMALLOC       pMalloc;

if(FAILED(SHGetMalloc(&pMalloc)))
   return FALSE;

pidlConnection = GetDialUpItem(lpszConnectionName, pMalloc);

if(!pidlConnection)
   {
   return FALSE;
   }

CoInitialize(NULL);

//create a new IShellLink interface
hr = CoCreateInstance(  CLSID_ShellLink, 
                        NULL, 
                        CLSCTX_INPROC_SERVER, 
                        IID_IShellLink, 
                        (LPVOID*)&pShellLink); 

bReturn = FALSE;

if(SUCCEEDED(hr)) 
   { 
   IPersistFile* pPersistFile; 

   //set the PIDL for the shortcut target
   hr = pShellLink->SetIDList(pidlConnection); 

   if(SUCCEEDED(hr)) 
      {
      // Query IShellLink for the IPersistFile interface for saving the 
      // shortcut in persistent storage. 
      hr = pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile); 

      if(SUCCEEDED(hr)) 
         { 
         WCHAR wsz[MAX_PATH]; 

         //make sure that the link name ends with a ".LNK" extension
         if(lstrcmpi(lpszLinkName + lstrlen(lpszLinkName) - 4, ".lnk"))
            {
            //add the proper extension
            lstrcat(lpszLinkName, ".lnk");
            }

         // convert the string to a wide character string
         MultiByteToWideChar( CP_ACP, 
                              0, 
                              lpszLinkName, 
                              -1, 
                              wsz, 
                              MAX_PATH); 

         // Save the link by calling IPersistFile::Save. 
         if(SUCCEEDED(pPersistFile->Save(wsz, TRUE)))
            bReturn = TRUE;
    
         pPersistFile->Release(); 
         } 

      pShellLink->Release(); 
      }
   } 

pMalloc->Free(pidlConnection);
   
return bReturn;
}

/**************************************************************************

   ExecuteDialUpConnection()
   
   Parameters:
   
   Returns - 

**************************************************************************/

BOOL ExecuteDialUpConnection(LPCTSTR lpszConnectionName)
{
	LPMALLOC          pMalloc;
	LPITEMIDLIST      pidlConnection;
	SHELLEXECUTEINFO  sei;

	if(FAILED(SHGetMalloc(&pMalloc)))
	   return FALSE;

	pidlConnection = GetDialUpItem(lpszConnectionName, pMalloc);

	if(!pidlConnection)
	{
	   return FALSE;
	}

	ZeroMemory(&sei, sizeof(sei));
	sei.cbSize     = sizeof(SHELLEXECUTEINFO);
	sei.fMask      = SEE_MASK_INVOKEIDLIST | SEE_MASK_FLAG_NO_UI;
	sei.lpIDList   = pidlConnection;
	sei.nShow      = SW_SHOWDEFAULT;

	ShellExecuteEx(&sei);

	pMalloc->Free(pidlConnection);
   
	return TRUE;
}

/**************************************************************************

   CreateNewConnectionLink()
   
   Parameters:

      LPTSTR lpszLinkName -       The name of the shortcut. The .LNK 
                                 extension will be added to the end of 
                                 this name if it is not present.
   
   Returns - Boolean indicating succes or failure.

**************************************************************************/

BOOL CreateNewConnectionLink(LPTSTR lpszLinkName)
{
	HRESULT        hr;
	IShellLink     *pShellLink; 
	BOOL           bReturn;
	LPITEMIDLIST   pidlConnection;
	LPMALLOC       pMalloc;

	if(FAILED(SHGetMalloc(&pMalloc)))
	   return FALSE;

	pidlConnection = GetNewConnectionItem(pMalloc);

	if(!pidlConnection)
	   {
	   return FALSE;
	   }

	CoInitialize(NULL);

	//create a new IShellLink interface
	hr = CoCreateInstance(  CLSID_ShellLink, 
							NULL, 
							CLSCTX_INPROC_SERVER, 
							IID_IShellLink, 
							(LPVOID*)&pShellLink); 

	bReturn = FALSE;

	if(SUCCEEDED(hr)) 
	{ 
	   IPersistFile* pPersistFile; 

	   //set the PIDL for the shortcut target
	   hr = pShellLink->SetIDList(pidlConnection); 

	   if(SUCCEEDED(hr)) 
	   {
		  // Query IShellLink for the IPersistFile interface for saving the 
		  // shortcut in persistent storage. 
		  hr = pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile); 

		  if(SUCCEEDED(hr)) 
		  { 
			 WCHAR wsz[MAX_PATH]; 

			 //make sure that the link name ends with a ".LNK" extension
			 if(lstrcmpi(lpszLinkName + lstrlen(lpszLinkName) - 4, ".lnk"))
				{
				//add the proper extension
				lstrcat(lpszLinkName, ".lnk");
				}

			 // convert the string to a wide character string
			 MultiByteToWideChar( CP_ACP, 
								  0, 
								  lpszLinkName, 
								  -1, 
								  wsz, 
								  MAX_PATH); 

			 // Save the link by calling IPersistFile::Save. 
			 if(SUCCEEDED(pPersistFile->Save(wsz, TRUE)))
				bReturn = TRUE;
    
			 pPersistFile->Release(); 
		  } 

		  pShellLink->Release(); 
	   }
	} 

	pMalloc->Free(pidlConnection);
   
	return bReturn;
}

/**************************************************************************

   ExecuteNewConnection()
   
   Parameters:
   
   Returns - 

**************************************************************************/

BOOL ExecuteNewConnection(void)
{
	LPMALLOC          pMalloc;
	LPITEMIDLIST      pidlConnection;
	SHELLEXECUTEINFO  sei;

	if(FAILED(SHGetMalloc(&pMalloc)))
	   return FALSE;

	pidlConnection = GetNewConnectionItem(pMalloc);

	if(!pidlConnection)
	{
	   return FALSE;
	}

	ZeroMemory(&sei, sizeof(sei));
	sei.cbSize     = sizeof(SHELLEXECUTEINFO);
	sei.fMask      = SEE_MASK_INVOKEIDLIST | SEE_MASK_FLAG_NO_UI;
	sei.lpIDList   = pidlConnection;
	sei.nShow      = SW_SHOWDEFAULT;

	ShellExecuteEx(&sei);

	pMalloc->Free(pidlConnection);
   
	return TRUE;
}

/**************************************************************************

   GetDialUpNetworkingPidl()
   
   Returns - 

**************************************************************************/

LPITEMIDLIST GetDialUpNetworkingPidl(LPSHELLFOLDER pDesktop)
{
	HRESULT        hr;
	LPITEMIDLIST   pidlDialUp;

	if(!pDesktop)
	   return NULL;

	//get the pidl for Dial-Up Networking
	hr = pDesktop->ParseDisplayName( NULL,
									 NULL,
									 L"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{992CFFA0-F557-101A-88EC-00DD010CCC48}",
									 NULL,
									 &pidlDialUp,
									 NULL);

	if(FAILED(hr)) 
	   return NULL;

	return pidlDialUp;
}

/**************************************************************************

   GetNewConnectionItem()
   
   Parameters:
   
   Returns - 

**************************************************************************/

LPITEMIDLIST GetNewConnectionItem(LPMALLOC pMalloc)
{
HRESULT        hr;
LPSHELLFOLDER  pDesktop,
               pDialUp;
LPITEMIDLIST   pidlTarget = NULL,
               pidlTemp,
               pidlDialUp;
LPENUMIDLIST   pEnum;
DWORD          dwRetrieved;

if(!pMalloc)
   return NULL;

if(FAILED(SHGetDesktopFolder(&pDesktop)))
   return NULL;

pidlDialUp = GetDialUpNetworkingPidl(pDesktop);

if(pidlDialUp)
   {
   //get the IShellFolder for Dial-Up Networking
   hr = pDesktop->BindToObject(  pidlDialUp, 
                                 NULL, 
                                 IID_IShellFolder, 
                                 (LPVOID*)&pDialUp);

   if(SUCCEEDED(hr)) 
      {
      if(SUCCEEDED(pDialUp->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &pEnum)))
         {
         //get the first item's PIDL, this will be the item for creating a new connection
         hr = pEnum->Next(1, &pidlTemp, &dwRetrieved);

         if(dwRetrieved && SUCCEEDED(hr)) 
            {
            //we need to concatenate the item's PIDL onto the Dial-Up Networking PIDL
            pidlTarget = Pidl_Concatenate(pMalloc, pidlDialUp, pidlTemp);
            }

         pEnum->Release();
         }
      pDialUp->Release();
      }
   }

pDesktop->Release();

return pidlTarget;
}

/**************************************************************************

   GetDialUpItem()
   
   Parameters:
   
   Returns - 

**************************************************************************/

LPITEMIDLIST GetDialUpItem(LPCTSTR lpszName, LPMALLOC pMalloc)
{
HRESULT        hr;
LPSHELLFOLDER  pDesktop,
               pDialUp;
LPITEMIDLIST   pidlTarget,
               pidlTemp,
               pidlDialUp;
LPENUMIDLIST   pEnum;
DWORD          dwRetrieved;
char           szTemp[MAX_PATH];

if(!pMalloc)
   return NULL;

if(FAILED(SHGetDesktopFolder(&pDesktop)))
   return NULL;

pidlDialUp = GetDialUpNetworkingPidl(pDesktop);

if(pidlDialUp)
   {
   //get the IShellFolder for Dial-Up Networking
   hr = pDesktop->BindToObject(  pidlDialUp, 
                                 NULL, 
                                 IID_IShellFolder, 
                                 (LPVOID*)&pDialUp);

   if(SUCCEEDED(hr)) 
      {
      if(SUCCEEDED(pDialUp->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &pEnum)))
         {
         //enumerate the item's PIDLs, looking for the one requested
         do
            {
            hr = pEnum->Next(1, &pidlTemp, &dwRetrieved);

            //check it's name
            GetItemIdName(pDialUp, pidlTemp, SHGDN_NORMAL, szTemp, sizeof(szTemp));
            }
         while(SUCCEEDED(hr) && dwRetrieved && (lstrcmpi(szTemp, lpszName)));


         if(dwRetrieved && SUCCEEDED(hr)) 
            {
            //we need to concatenate the item's PIDL onto the Dial-Up Networking PIDL
            pidlTarget = Pidl_Concatenate(pMalloc, pidlDialUp, pidlTemp);
            }

         pEnum->Release();
         }
      pDialUp->Release();
      }
   }

pDesktop->Release();

return pidlTarget;
}

/**************************************************************************

   GetItemIdName()
   
**************************************************************************/

BOOL GetItemIdName(  LPSHELLFOLDER pFolder, 
                     LPITEMIDLIST pidl, 
                     DWORD dwFlags, 
                     LPTSTR pszName, 
                     UINT cchMax)
{
BOOL     fSuccess = TRUE;
STRRET   str;

if(SUCCEEDED(pFolder->GetDisplayNameOf(pidl, dwFlags, &str))) 
   {
   switch (str.uType) 
      {
      case STRRET_WSTR:
         WideCharToMultiByte( CP_ACP,
                                0,
                                str.pOleStr,
                                -1,
                                pszName,
                                cchMax,
                                NULL,
                                NULL);
         break;

      case STRRET_OFFSET:
         lstrcpyn(pszName, (LPTSTR) pidl + str.uOffset, cchMax);
         break;

      case STRRET_CSTR:
         lstrcpyn(pszName, (LPTSTR) str.cStr, cchMax);
         break;

      default:
         fSuccess = FALSE;
         break;
      }
   } 
else 
   {
   fSuccess = FALSE;
   }

return (fSuccess);
}

//
//  FUNCTION:   Pidl_Concatenate
//
//  PURPOSE:    Creates a new ITEMIDLIST with pidl2 appended to pidl1.
//
//  PARAMETERS:
//		piMalloc - Pointer to the allocator interface that should create the
//				   new ITEMIDLIST.
//      pidl1 	 - Pointer to an ITEMIDLIST that contains the root.
//		pidl2    - Pointer to an ITEMIDLIST that contains what should be 
//				   appended to the root.
//
//  RETURN VALUE:
//  	Returns a new ITEMIDLIST if successful, NULL otherwise.  
//

LPITEMIDLIST Pidl_Concatenate(   LPMALLOC piMalloc, 
                                 LPCITEMIDLIST pidl1,
							            LPCITEMIDLIST pidl2)
{
	LPITEMIDLIST pidlNew;
	UINT cb1, cb2 = 0;

	//
	// Pidl1 can possibly be NULL if it points to the desktop.  Since we only
	// need a single NULL terminator, we remove the extra 2 bytes from the
	// size of the first ITEMIDLIST.
	//
	if (pidl1)
		cb1 = Pidl_GetSize(pidl1) - (2 * sizeof(BYTE));

	cb2 = Pidl_GetSize(pidl2);

	//
	// Create a new ITEMIDLIST that is the size of both pidl1 and pidl2, then
	// copy pidl1 and pidl2 to the new list.
	//
	pidlNew = Pidl_Create(piMalloc, cb1 + cb2);
	if (pidlNew)
	{
		if (pidl1)	
			CopyMemory(pidlNew, pidl1, cb1);

		CopyMemory(((LPBYTE)pidlNew) + cb1, pidl2, cb2);
	}

	return (pidlNew);
}


//
//  FUNCTION:   Pidl_GetSize 
//
//  PURPOSE:    Returns the total number of bytes in an ITEMIDLIST.
//
//  PARAMETERS:
//      pidl - Pointer to the ITEMIDLIST that you want the size of.
//

UINT Pidl_GetSize(LPCITEMIDLIST pidl)
{
	UINT cbTotal = 0;
	LPITEMIDLIST pidlTemp = (LPITEMIDLIST) pidl;

	if (pidlTemp)
	{
		while (pidlTemp->mkid.cb)
		{
			cbTotal += pidlTemp->mkid.cb;
			pidlTemp = Pidl_GetNextItem(pidlTemp);
		}	

		// Requires a 16 bit zero value for the NULL terminator
		cbTotal += 2 * sizeof(BYTE);
	}

	return (cbTotal);
}

//
//  FUNCTION:   Pidl_Create 
//
//  PURPOSE:    Creates a new ITEMIDLIST of the specified size.
//
//  PARAMETERS:
//      piMalloc - Pointer to the allocator interface that should allocate
//				   memory.
//		cbSize   - Size of the ITEMIDLIST to create.
//
//  RETURN VALUE:
//      Returns a pointer to the new ITEMIDLIST, or NULL if a problem occured.
//

LPITEMIDLIST Pidl_Create(LPMALLOC piMalloc, UINT cbSize)
{
	LPITEMIDLIST pidl = NULL;

	pidl = (LPITEMIDLIST) piMalloc->Alloc(cbSize);
	if (pidl)
		ZeroMemory(pidl, cbSize);

	return (pidl);
}


//
//  FUNCTION:   Pidl_GetNextItem 
//
//  PURPOSE:    Returns a pointer to the next item in the ITEMIDLIST.
//
//  PARAMETERS:
//      pidl - Pointer to an ITEMIDLIST to walk through
//

inline LPITEMIDLIST Pidl_GetNextItem(LPCITEMIDLIST pidl)
{
	if (pidl)
		return (LPITEMIDLIST) (LPBYTE)(((LPBYTE)pidl) + pidl->mkid.cb);
	else
		return (NULL);
}
