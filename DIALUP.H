/**************************************************************************

   File:          DialUp.h
   
   Description:   

**************************************************************************/

#include <shlobj.h>

typedef BOOL (*LPENUMDUITEMS)(LPCTSTR, LPARAM);

#ifdef __cplusplus
extern "C" {
#endif

void EnumDialUpConnections(LPENUMDUITEMS, LPARAM);
BOOL CreateDialUpLink(LPTSTR, LPTSTR);
BOOL ExecuteDialUpConnection(LPCTSTR);
BOOL CreateNewConnectionLink(LPTSTR);
BOOL ExecuteNewConnection(void);

#ifdef __cplusplus
}
#endif

