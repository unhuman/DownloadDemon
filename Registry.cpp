// ==========================================================================
// 					Class Implementation : COXRegistry
// ==========================================================================

// Source file : registry.cpp

// Source : Periphere NV (adaptions by F.Melendez)

// ****************************************************************************
// 			Original source by
// 				Samuel R. Blackburn
// 				CI$: 76300,326
// 				Internet: sammy@sed.csc.com
// ****************************************************************************

// Creation Date : 	   27 November 1995
// Last Modification : 15th July 1996
                          
// //////////////////////////////////////////////////////////////////////////

#include "stdafx.h"		// standard MFC include
#include "registry.h"	// class specification
#include <WINDOWSX.H>

#if defined( _DEBUG )
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC( COXRegistry, CObject );

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// Definition of static members

const HKEY COXRegistry::keyLocalMachine = HKEY_LOCAL_MACHINE;
const HKEY COXRegistry::keyClassesRoot  = HKEY_CLASSES_ROOT;
const HKEY COXRegistry::keyUsers        = HKEY_USERS;
const HKEY COXRegistry::keyCurrentUser  = HKEY_CURRENT_USER;

#if(WINVER >= 0x0400)
 	// Win 95 extra	registries
	const HKEY COXRegistry::keyCurrentConfig	= HKEY_CURRENT_CONFIG;
	const HKEY COXRegistry::keyDynamicData  	= HKEY_DYN_DATA;
#endif

// Data members -------------------------------------------------------------
// protected:

	// HKEY m_KeyHandle;
	// --- Handle of the current open registry key

	// HKEY m_RegistryHandle;
	// --- Handle of the current open MAIN registry database
	
	// LONG m_ErrorCode;
	// --- the last error that occurred after executing a command

	// CString m_ClassName;
	// --- Name of the class of the current open key

	// CString m_ComputerName;
	// --- Name of the computer whose registry we opened

	// CString m_sKeyName;
	// --- Name of the current open key

	// CString m_sRegistryName;
	// --- Name of the current open registry database
	
	// DWORD   m_NumberOfSubkeys;
	// --- Number of subkeys of the current open key
	
	// DWORD   m_NumberOfValues;
	// --- Number of values of the current open key

	/*
	** Data items filled in by QueryInfo
	*/

	// DWORD    m_LongestSubkeyNameLength;
	// --- Length of the name of the longest subkey of the current open key

	// DWORD    m_LongestClassNameLength;
	// --- Length of the name of the longest class of the current open key

	// DWORD    m_LongestValueNameLength;
	// --- Length of the name of the longest value of the current open key

	// DWORD    m_LongestValueDataLength;
	// --- Length of the data of the longest value of the current open key

	// DWORD    m_SecurityDescriptorLength;
	// --- Length of the security descriptor of the current open key

	// FILETIME m_LastWriteTime;
	// --- the last time the open key or one of its value entries was modified

// private:
	
// Member functions ---------------------------------------------------------
// public:

COXRegistry::COXRegistry()
	{
	m_Initialize();
	}

COXRegistry::~COXRegistry()
	{
	if ( m_RegistryHandle != (HKEY) NULL )
		{
		Disconnect();
		}

	m_Initialize();
	}

BOOL COXRegistry::IsWin95()
	// --- In  : 
	// --- Out : 
	// --- Returns :
	// --- Effect : Determine the environment the app is executing in.
	{
	OSVERSIONINFO info;
	
	info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&info);

	return (info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);
	}

void COXRegistry::m_Initialize()
	{
	ASSERT_VALID( this );

	/*
	** Make sure everything is zeroed out
	*/

	m_sClassName.Empty();
	m_sComputerName.Empty();
	m_sKeyName.Empty();
	m_sRegistryName.Empty();

	m_RegistryHandle               = (HKEY) NULL;
	m_KeyHandle                    = (HKEY) NULL;
	m_ErrorCode                    = 0L;
	m_NumberOfSubkeys              = 0;
	m_LongestSubkeyNameLength      = 0;
	m_LongestClassNameLength       = 0;
	m_NumberOfValues               = 0;
	m_LongestValueNameLength       = 0;
	m_LongestValueDataLength       = 0;
	m_SecurityDescriptorLength     = 0;
	m_LastWriteTime.dwLowDateTime  = 0;
	m_LastWriteTime.dwHighDateTime = 0;

	m_bWin95 = IsWin95();
	}

BOOL COXRegistry::Disconnect()
	{
	ASSERT_VALID( this );

	if ( m_RegistryHandle == (HKEY) NULL )
		{
		return( TRUE );
		}

	m_ErrorCode = ::RegCloseKey( m_RegistryHandle );

	if ( m_ErrorCode == ERROR_SUCCESS )
		{
		m_RegistryHandle = (HKEY) NULL;
		m_Initialize();

		return( TRUE );
		}
	else
		{
		return( FALSE );
		}
	}

BOOL COXRegistry::Connect( HKEY hKeyToOpen, LPCTSTR pszNameOfComputer )
	{
	ASSERT_VALID( this );

	/*
	** name_of_computer can be NULL
	*/

	if (m_RegistryHandle != NULL)
		{
		TRACE0("In COXRegistry::Connect : Trying to connect while previous registry still connected\n");
		TRACE0("In COXRegistry::Connect : Disconnecting previous Registry connection\n");
		if (!Disconnect())
			return( FALSE );
		}

	if ( hKeyToOpen == keyClassesRoot || hKeyToOpen == keyCurrentUser )
		{
		if ( pszNameOfComputer == NULL )
			{
			 /*
			 ** NT won't allow you to connect to these hives via RegConnectRegistry so we'll just skip that step
			 */

			 m_RegistryHandle = hKeyToOpen;
			 m_ErrorCode      = ERROR_SUCCESS;
			}
		else
			{
			 m_ErrorCode = ERROR_INVALID_HANDLE;
			}
		}
	else
		{
		m_ErrorCode = ::RegConnectRegistry( (LPTSTR) pszNameOfComputer, hKeyToOpen, &m_RegistryHandle );
		}

	if ( m_ErrorCode == ERROR_SUCCESS )
		{
		if ( pszNameOfComputer == NULL )
			{
			TCHAR computer_name[ MAX_PATH ] = _T("");
			DWORD size = MAX_PATH;

			if ( ::GetComputerName( computer_name, &size ) == TRUE )
				{
				m_sComputerName = computer_name;
				}
			else
				{
				m_sComputerName.Empty();
				}
			}
		else
			{
			 m_sComputerName = pszNameOfComputer;
			}

		/*
		** It would be nice to use a switch statement here but I get a "not integral" error!
		*/

		if ( hKeyToOpen == HKEY_LOCAL_MACHINE )
			{
			m_sRegistryName = "HKEY_LOCAL_MACHINE";
			m_KeyHandle = hKeyToOpen;
			}
		else if ( hKeyToOpen == HKEY_CLASSES_ROOT )
			{
			m_sRegistryName = "HKEY_CLASSES_ROOT";
			m_KeyHandle = hKeyToOpen;
			}
		else if ( hKeyToOpen == HKEY_USERS )
			{
			m_sRegistryName = "HKEY_USERS";
			m_KeyHandle = hKeyToOpen;
			}
		else if ( hKeyToOpen == HKEY_CURRENT_USER )
			{
			m_sRegistryName = "HKEY_CURRENT_USER";
			m_KeyHandle = hKeyToOpen;
			}
		else
			{
			TRACE0("In COXRegistry::Connect : Registry name unknown\n");
			m_sRegistryName = "Unknown";
			}

		return( TRUE );
		}
	else
		{
		TRACE1("In COXRegistry::Connect : Registry connection Failed code = %lu\n", m_ErrorCode);
		return( FALSE );
		}
	}

BOOL COXRegistry::CreateKey( LPCTSTR          pszNameOfSubkey, 
                        LPCTSTR               pszNameOfClass,
                        CreateOptions         options, 
                        CreatePermissions     permissions, 
                        LPSECURITY_ATTRIBUTES pSecurityAttributes,
                        CreationDisposition*  pDisposition )
	{
	ASSERT_VALID( this );
	ASSERT( pszNameOfSubkey != NULL );

	if ( pszNameOfSubkey == NULL )
		{
		TRACE0("In COXRegistry::Create : Invalid parameter\n");
		m_ErrorCode = ERROR_INVALID_PARAMETER;
		return( FALSE );
		}

	DWORD disposition = 0;

	if ( pszNameOfClass == NULL )
		{
		pszNameOfClass = _T("");
		}

	m_ErrorCode = ::RegCreateKeyEx( m_RegistryHandle, pszNameOfSubkey,
									(DWORD) 0,
									(LPTSTR) pszNameOfClass,
									options,
									permissions,
									pSecurityAttributes,
									&m_KeyHandle,
									&disposition );

	if ( m_ErrorCode == ERROR_SUCCESS )
		{
		if ( pDisposition != NULL )
			{
			*pDisposition = (CreationDisposition) disposition;
			}

		m_sKeyName = pszNameOfSubkey;

		return( TRUE );
		}
	else
		{
		TRACE1("In COXRegistry::Create : Subkey Creation Failed code = %lu\n", m_ErrorCode);
		return( FALSE );
		}
	}

BOOL COXRegistry::DeleteKey( LPCTSTR pszNameOfKeyToDelete )
	{
	ASSERT_VALID( this );
	ASSERT( pszNameOfKeyToDelete != NULL );

	if ( pszNameOfKeyToDelete == NULL )
		{
		TRACE0("In COXRegistry::DeleteKey : Invalid parameter\n");
		m_ErrorCode = ERROR_INVALID_PARAMETER;
		return( FALSE );
		}

	/*
	** You can't delete a key given a full path. What you have to do is back up one level and then do a delete
	*/

	CString full_key_name = pszNameOfKeyToDelete;

	if ( full_key_name.Find( _T('\\') ) == (-1) )
		{
		/*
		** User had not given us a full path so assume the name of the key he passed us
		** is a key off of the current key
		*/
		if (m_bWin95)
			m_ErrorCode = ::RegDeleteKey( m_KeyHandle, pszNameOfKeyToDelete );
		else
			m_ErrorCode = DeleteRegistryKey( m_KeyHandle, pszNameOfKeyToDelete );
		}
	else
		{
		int last_back_slash_location = full_key_name.GetLength() - 1;

		/*
		** We know this loop will succeed because a back slash was found in the above if statement
		*/

		while( full_key_name[ last_back_slash_location ] != _T('\\') )
			{
			last_back_slash_location--;
			}

		CString currently_opened_key_name = m_sKeyName;

		CString parent_key_name = full_key_name.Left( last_back_slash_location );
		CString child_key_name  = full_key_name.Right( ( full_key_name.GetLength() - last_back_slash_location ) - 1 );

		/*
		** Now we open the parent key and delete the child
		*/

		if ( OpenKey( parent_key_name ) == TRUE )
			{
			if (m_bWin95)
				m_ErrorCode = ::RegDeleteKey( m_KeyHandle, child_key_name );
			else
				m_ErrorCode = DeleteRegistryKey( m_KeyHandle, child_key_name );
			}
		else
			{
			TRACE1("In COXRegistry::DeleteKey : Subkey Deletion Failed code = %lu\n", m_ErrorCode);
			m_sKeyName = currently_opened_key_name;
			return( FALSE );
			}
		}

	if ( m_ErrorCode == ERROR_SUCCESS )
		{
		return( TRUE );
		}
	else
		{
		TRACE1("In COXRegistry::DeleteKey : Subkey Deletion Failed code = %lu\n", m_ErrorCode);
		return( FALSE );
		}
	} 

LONG COXRegistry::DeleteRegistryKey(HKEY hParentKey, LPCTSTR pszNameOfKeyToDelete)
	{
	DWORD dwValueSize, dwKey = 0;
	CString sValueName;
	long lStatus, lStatus2;
	HKEY hKeyToDelete;

	lStatus2 = ::RegOpenKeyEx(hParentKey, pszNameOfKeyToDelete, NULL, permissionAllAccess, &hKeyToDelete);
	if (lStatus2 != ERROR_SUCCESS)
		return lStatus2;

	//  Start with the highest index of Values
	lStatus = ::RegQueryInfoKey(hKeyToDelete, NULL, NULL, NULL, &dwKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	if (lStatus != ERROR_SUCCESS)
		return lStatus;

	//  Start looking for subkeys now
	FILETIME FileTime;
	do
		{
		dwValueSize = 512;					//  Tell system how much buffer we have
		
		//  Get "next" key
		lStatus = ::RegEnumKeyEx(hKeyToDelete, --dwKey, sValueName.GetBuffer(dwValueSize), &dwValueSize,
			NULL, NULL, NULL, &FileTime);
		sValueName.ReleaseBuffer();
		
		if( lStatus == ERROR_SUCCESS)						//  Was a valid key found?
			DeleteRegistryKey(hKeyToDelete, sValueName);
		}
	while(lStatus == ERROR_SUCCESS);             //  Loop until key enum fails
			
	::RegCloseKey(hKeyToDelete);						//  Close key to delete
 
	return ::RegDeleteKey(hParentKey, pszNameOfKeyToDelete); //  Delete key
	}

BOOL COXRegistry::DeleteValue( LPCTSTR pszNameOfValueToDelete )
	{
	ASSERT_VALID( this );

	/*
	** name_of_value_to_delete can be NULL
	*/

	m_ErrorCode = ::RegDeleteValue( m_KeyHandle, (LPTSTR) pszNameOfValueToDelete );

	if ( m_ErrorCode == ERROR_SUCCESS )
		{
		return( TRUE );
		}
	else
		{
		TRACE1("In COXRegistry::DeleteValue : Value Deletion Failed code = %lu\n", m_ErrorCode);
		return( FALSE );
		}
	}

#if defined( _DEBUG )

void COXRegistry::Dump( CDumpContext& dump_context ) const
	{
	CObject::Dump( dump_context );

	dump_context << "m_KeyHandle = "                << m_KeyHandle                << "\n";
	dump_context << "m_RegistryHandle = "           << m_RegistryHandle           << "\n";
	dump_context << "m_sClassName = \""              << m_sClassName              << "\"\n";
	dump_context << "m_sComputerName = \""           << m_sComputerName           << "\"\n";
	dump_context << "m_sKeyName = \""                << m_sKeyName                << "\"\n";
	dump_context << "m_sRegistryName = \""           << m_sRegistryName           << "\"\n";
	dump_context << "m_NumberOfSubkeys = "          << m_NumberOfSubkeys          << "\n";
	dump_context << "m_NumberOfValues = "           << m_NumberOfValues           << "\n";
	dump_context << "m_LongestSubkeyNameLength = "  << m_LongestSubkeyNameLength  << "\n";
	dump_context << "m_LongestClassNameLength = "   << m_LongestClassNameLength   << "\n";
	dump_context << "m_LongestValueNameLength = "   << m_LongestValueNameLength   << "\n";
	dump_context << "m_LongestValueDataLength = "   << m_LongestValueDataLength   << "\n";
	dump_context << "m_SecurityDescriptorLength = " << m_SecurityDescriptorLength << "\n";
	dump_context << "m_LastWriteTime = "            << CTime(m_LastWriteTime)     << "\n";
	}

void COXRegistry::AssertValid() const
	{
	CObject::AssertValid();
	}

#endif // _DEBUG

BOOL COXRegistry::EnumerateKeys( const DWORD dwSubkeyIndex, CString& sSubkeyName, CString& sClassName )
	{
	ASSERT_VALID( this );

	TCHAR subkey_name_string[ 2048 ];
	TCHAR class_name_string[ 2048 ];

	DWORD size_of_subkey_name_string = sizeof( subkey_name_string ) - 1;
	DWORD size_of_class_name_string  = sizeof( class_name_string  ) - 1;

	::ZeroMemory( subkey_name_string, sizeof( subkey_name_string ) );
	::ZeroMemory( class_name_string,  sizeof( class_name_string  ) );

	m_ErrorCode = ::RegEnumKeyEx( m_KeyHandle, 
					             dwSubkeyIndex, 
					             subkey_name_string, 
					            &size_of_subkey_name_string,
					             NULL,
					             class_name_string,
					            &size_of_class_name_string,
					            &m_LastWriteTime );

	if ( m_ErrorCode == ERROR_SUCCESS )
		{
		sSubkeyName = subkey_name_string;
		sClassName  = class_name_string;

		return( TRUE );
		}
	else
		{
		TRACE1("In COXRegistry::EnumerateKeys : Enumeration Failed code = %lu\n", m_ErrorCode);
		return( FALSE );
		}
	}

BOOL COXRegistry::EnumerateValues( const DWORD    dwValueIndex, 
                                 CString&       sNameOfValue, 
                                 KeyValueTypes& eTypeCode,
                                 LPBYTE         pbDataBuffer,
                                 DWORD&         dwSizeOfDataBuffer )
	{
	ASSERT_VALID( this );

	/*
	** data_buffer and size_of_data_buffer can be NULL
	*/

	DWORD temp_type_code = eTypeCode;

	TCHAR temp_name[ 2048 ];

	::ZeroMemory( temp_name, sizeof( temp_name ) );

	DWORD temp_name_size = sizeof( temp_name );

	m_ErrorCode = ::RegEnumValue( m_KeyHandle,
		                         dwValueIndex,
		                         temp_name,
		                        &temp_name_size,
		                         NULL,
		                        &temp_type_code,
		                         pbDataBuffer,
		                        &dwSizeOfDataBuffer );

	if ( m_ErrorCode == ERROR_SUCCESS )
		{
		eTypeCode     = (KeyValueTypes) temp_type_code;
		sNameOfValue = temp_name;

		return( TRUE );
		}
	else
		{
		TRACE1("In COXRegistry::EnumerateValues : Enumeration Failed code = %lu\n", m_ErrorCode);
		return( FALSE );
		}
	}

BOOL COXRegistry::FlushRegistry()
	{
	ASSERT_VALID( this );

	m_ErrorCode = ::RegFlushKey( m_KeyHandle );

	if ( m_ErrorCode == ERROR_SUCCESS )
		{
		return( TRUE );
		}
	else
		{
		TRACE1("In COXRegistry::Flush : Flush Failed code = %lu\n", m_ErrorCode);
		return( FALSE );
		}
	}

BOOL COXRegistry::GetBinaryValue( LPCTSTR pszNameOfValue, CByteArray& abReturnArray )
	{
	ASSERT_VALID( this );
	ASSERT( pszNameOfValue != NULL );

	if ( pszNameOfValue == NULL )
		{
		TRACE0("In COXRegistry::GetBinaryValue : Invalid parameter\n");
		m_ErrorCode = ERROR_INVALID_PARAMETER;
		return( FALSE );
		}

	DWORD size_of_buffer = m_LongestValueDataLength;

	LPBYTE memory_buffer = (LPBYTE) new BYTE[ size_of_buffer ];

	if ( memory_buffer == NULL )
		{
		m_ErrorCode = ::GetLastError();
		return( FALSE );
		}

	BOOL return_value = TRUE;

	KeyValueTypes type = typeBinary;

	if ( QueryValue( pszNameOfValue, type, memory_buffer, size_of_buffer ) == TRUE &&
		type == typeBinary)
		{
		/*
		** We've got data so give it back to the caller
		*/

		abReturnArray.RemoveAll();

		DWORD index = 0;

		while( index < size_of_buffer )
			{
			abReturnArray.Add( memory_buffer[ index ] );
			index++;
			}

		return_value = TRUE;
		}
	else
		{
		return_value = FALSE;
		}

	delete [] memory_buffer;

	return( return_value );
	}

void COXRegistry::GetClassName( CString& sClassName ) const
	{
	sClassName = m_sClassName;
	}

void COXRegistry::GetComputerName( CString& sComputerName ) const
	{
	sComputerName = m_sComputerName;
	}

BOOL COXRegistry::GetDoubleWordValue( LPCTSTR pszNameOfValue, DWORD& dwReturnValue )
	{
	ASSERT_VALID( this );
	ASSERT( pszNameOfValue != NULL );

	if ( pszNameOfValue == NULL )
		{
		TRACE0("In COXRegistry::GetDoubleWordValue : Invalid parameter\n");
		m_ErrorCode = ERROR_INVALID_PARAMETER;
		return( FALSE );
		}

	DWORD size_of_buffer = sizeof( DWORD );

	KeyValueTypes type = typeDoubleWord;

	return( QueryValue( pszNameOfValue, type, (LPBYTE) &dwReturnValue, size_of_buffer ) && type == typeDoubleWord);
	}

LONG COXRegistry::GetErrorCode() const
	{
	ASSERT_VALID( this );
	return( m_ErrorCode );
	}

void COXRegistry::GetKeyName( CString& sKeyName ) const
	{
	sKeyName = m_sKeyName;
	}

DWORD COXRegistry::GetNumberOfSubkeys() const
	{
	return( m_NumberOfSubkeys );
	}

DWORD COXRegistry::GetNumberOfValues() const
	{
	return( m_NumberOfValues );
	}

void COXRegistry::GetRegistryName( CString& sRegistryName ) const
	{
	sRegistryName = m_sRegistryName;
	}

BOOL COXRegistry::GetSecurity( const SECURITY_INFORMATION whatYouWantToKnow,
                             PSECURITY_DESCRIPTOR       pDataBuffer,
                             DWORD&                     dwSizeOfDataBuffer )
	{
	ASSERT_VALID( this );
	ASSERT( pDataBuffer != NULL );

	if ( pDataBuffer == NULL )
		{
		TRACE0("In COXRegistry::GetSecurity : Invalid parameter\n");
		m_ErrorCode = ERROR_INVALID_PARAMETER;
		return( FALSE );
		}

	m_ErrorCode = ::RegGetKeySecurity( m_KeyHandle,
		                              whatYouWantToKnow,
		                              pDataBuffer,
		                             &dwSizeOfDataBuffer );

	if ( m_ErrorCode == ERROR_SUCCESS )
		{
		return( TRUE );
		}
	else
		{
		return( FALSE );
		}
	}

BOOL COXRegistry::GetStringValue( LPCTSTR pszNameOfValue, CString& sReturnString )
	{
	ASSERT_VALID( this );
	ASSERT( pszNameOfValue != NULL );

	if ( pszNameOfValue == NULL )
		{
		TRACE0("In COXRegistry::GetStringValue : Invalid parameter\n");
		m_ErrorCode = ERROR_INVALID_PARAMETER;
		return( FALSE );
		}

	TCHAR temp_string[ 2048 ];
	DWORD size_of_buffer = 2048;

	::ZeroMemory( temp_string, sizeof( temp_string ) );

	KeyValueTypes type = typeString;

	if ( QueryValue( pszNameOfValue, type, (LPBYTE) temp_string, size_of_buffer ) == TRUE &&
		((type == typeString) || (type == typeUnexpandedString)) )
		{
		sReturnString = temp_string;
		return( TRUE );
		}
	else
		{
		sReturnString.Empty();
		return( FALSE );
		}
	}

BOOL COXRegistry::GetStringArrayValue( LPCTSTR pszNameOfValue, CStringArray& asReturnArray )
	{
	ASSERT_VALID( this );
	ASSERT( pszNameOfValue != NULL );

	if ( pszNameOfValue == NULL )
		{
		TRACE0("In COXRegistry::GetStringArrayValue : Invalid parameter\n");
		m_ErrorCode = ERROR_INVALID_PARAMETER;
		return( FALSE );
		}

	DWORD size_of_buffer = m_LongestValueDataLength;

	LPBYTE memory_buffer = (LPBYTE) new BYTE[ size_of_buffer ];

	if ( memory_buffer == NULL )
		{
		m_ErrorCode = ::GetLastError();
		return( FALSE );
		}

	BOOL return_value = TRUE;

	KeyValueTypes type = typeMultipleString; // A double NULL terminated string

	if ( QueryValue( pszNameOfValue, type, memory_buffer, size_of_buffer ) == TRUE &&
		type == typeMultipleString)
		{
		/*
		** We've got data so give it back to the caller
		*/

		LPTSTR strings = (LPTSTR) memory_buffer;

		asReturnArray.RemoveAll();

		while( strings[ 0 ] != 0x00 )
			{
			asReturnArray.Add( (LPCTSTR) strings );
			strings += (_tcslen( (LPCTSTR) strings ) + 1 );
			}

		return_value = TRUE;
		}
	else
		{
		return_value = FALSE;
		}

	delete [] memory_buffer;

	return( return_value );
	}

BOOL COXRegistry::GetValue( LPCTSTR pszNameOfValue, CByteArray& abReturnArray )
	{
	ASSERT_VALID( this );
	ASSERT( pszNameOfValue != NULL );

	if ( pszNameOfValue == NULL )
		{
		TRACE0("In COXRegistry::GetValue : Invalid parameter\n");
		m_ErrorCode = ERROR_INVALID_PARAMETER;
		return( FALSE );
		}

	return( GetBinaryValue( pszNameOfValue, abReturnArray ) );
	}

BOOL COXRegistry::GetValue( LPCTSTR pszNameOfValue, DWORD& dwReturnValue )
	{
	ASSERT_VALID( this );
	ASSERT( pszNameOfValue != NULL );

	if ( pszNameOfValue == NULL )
		{
		TRACE0("In COXRegistry::GetValue : Invalid parameter\n");
		m_ErrorCode = ERROR_INVALID_PARAMETER;
		return( FALSE );
		}

	return( GetDoubleWordValue( pszNameOfValue, dwReturnValue ) );
	}

BOOL COXRegistry::GetValue( LPCTSTR pszNameOfValue, CString& sReturnString )
	{
	ASSERT_VALID( this );
	ASSERT( pszNameOfValue != NULL );

	if ( pszNameOfValue == NULL )
		{
		TRACE0("In COXRegistry::GetValue : Invalid parameter\n");
		m_ErrorCode = ERROR_INVALID_PARAMETER;
		return( FALSE );
		}

	return( GetStringValue( pszNameOfValue, sReturnString ) );
	}

BOOL COXRegistry::GetValue( LPCTSTR pszNameOfValue, CStringArray& asReturnArray )
	{
	ASSERT_VALID( this );
	ASSERT( pszNameOfValue != NULL );

	if ( pszNameOfValue == NULL )
		{
		TRACE0("In COXRegistry::GetValue : Invalid parameter\n");
		m_ErrorCode = ERROR_INVALID_PARAMETER;
		return( FALSE );
		}

	return( GetStringArrayValue( pszNameOfValue, asReturnArray ) );
	}

BOOL COXRegistry::LoadFromFile( LPCTSTR pszNameOfSubkey, LPCTSTR pszNameOfFileContainingInformation )
	{
	ASSERT_VALID( this );
	ASSERT( pszNameOfSubkey != NULL );
	ASSERT( pszNameOfFileContainingInformation != NULL );

	if ( pszNameOfSubkey == NULL || pszNameOfFileContainingInformation == NULL )
		{
		TRACE0("In COXRegistry::Load : Invalid parameter\n");
		m_ErrorCode = ERROR_INVALID_PARAMETER;
		return( FALSE );
		}

	m_ErrorCode = ::RegLoadKey( m_RegistryHandle, pszNameOfSubkey, pszNameOfFileContainingInformation );

	if ( m_ErrorCode == ERROR_SUCCESS )
		{
		return( TRUE );
		}
	else
		{
		TRACE1("In COXRegistry::Load : Load Failed code = %lu\n", m_ErrorCode);
		return( FALSE );
		}
	}

BOOL COXRegistry::NotifyChange( const HANDLE             hEventHandle, 
                              const NotifyChangeFilter eChangesToBeReported,
                              const BOOL               bThisSubkeyOrAllSubkeys,
                              const BOOL               bWaitForChangeOrSignalEvent )
	{
	ASSERT_VALID( this );

	m_ErrorCode = ::RegNotifyChangeKeyValue( m_KeyHandle,
		                                    bThisSubkeyOrAllSubkeys,
		                                    eChangesToBeReported,
		                                    hEventHandle,
		                                    bWaitForChangeOrSignalEvent );

	if ( m_ErrorCode == ERROR_SUCCESS )
		{
		return( TRUE );
		}
	else
		{
		return( FALSE );
		}
	}

BOOL COXRegistry::OpenKey( LPCTSTR pszNameOfSubkeyToOpen, const CreatePermissions eSecurityAccessMask )
	{
	ASSERT_VALID( this );

	/*
	** name_of_subkey_to_open can be NULL
	*/

	m_ErrorCode = ::RegOpenKeyEx( m_RegistryHandle, pszNameOfSubkeyToOpen, NULL, eSecurityAccessMask, &m_KeyHandle );

	if ( m_ErrorCode == ERROR_SUCCESS )
		{
		QueryInfo();
		m_sKeyName = pszNameOfSubkeyToOpen;

		return( TRUE );
		}
	else
		{
		TRACE1("In COXRegistry::Open : Open Failed code = %lu\n", m_ErrorCode);
		return( FALSE );
		}
	}

BOOL COXRegistry::QueryInfo()
	{
	ASSERT_VALID( this );

	TCHAR class_name[ 2048 ];

	DWORD size_of_class_name = sizeof( class_name ) - 1;

	::ZeroMemory( class_name, sizeof( class_name ) );

	m_ErrorCode = ::RegQueryInfoKey( m_KeyHandle,
		                            class_name,
		                           &size_of_class_name,
				                  (LPDWORD) NULL,
		                           &m_NumberOfSubkeys,
		                           &m_LongestSubkeyNameLength,
		                           &m_LongestClassNameLength,
		                           &m_NumberOfValues,
		                           &m_LongestValueNameLength,
		                           &m_LongestValueDataLength,
		                           &m_SecurityDescriptorLength,
		                           &m_LastWriteTime );

	if ( m_ErrorCode == ERROR_SUCCESS )
		{
		m_sClassName = class_name;

		return( TRUE );
		}
	else
		{
		TRACE1("In COXRegistry::QueryInfo : QueryInfo Failed code = %lu\n", m_ErrorCode);
		return( FALSE );
		}
	}

BOOL COXRegistry::QueryValue( LPCTSTR        pszNameOfValue, 
                            KeyValueTypes&   eValueType, 
                            LPBYTE           pbAddressOfBuffer, 
                            DWORD&           dwSizeOfBuffer )
	{
	ASSERT_VALID( this );
	ASSERT( pszNameOfValue != NULL );

	/*
	** address_of_buffer and size_of_buffer can be NULL
	*/

	if ( pszNameOfValue == NULL )
		{
		TRACE0("In COXRegistry::QueryValue : Invalid parameter\n");
		m_ErrorCode = ERROR_INVALID_PARAMETER;
		return( FALSE );
		}

	DWORD temp_data_type = (DWORD) eValueType;

	m_ErrorCode = ::RegQueryValueEx( m_KeyHandle,
		                   (LPTSTR) pszNameOfValue,
		                            NULL,
		                           &temp_data_type,
		                            pbAddressOfBuffer,
		                           &dwSizeOfBuffer );

	if ( m_ErrorCode == ERROR_SUCCESS )
		{
		eValueType = (KeyValueTypes) temp_data_type;
		return( TRUE );
		}
	else
		{
		TRACE1("In COXRegistry::QueryValue : QueryValue Failed code = %lu\n", m_ErrorCode);
		return( FALSE );
		}
	}

BOOL COXRegistry::ReplaceFile( LPCTSTR pszNameOfSubkey,
                         LPCTSTR pszNameOfFileWithNewData,
                         LPCTSTR pszNameOfBackupFile )
	{
	ASSERT_VALID( this );
	ASSERT( pszNameOfSubkey             != NULL );
	ASSERT( pszNameOfFileWithNewData != NULL );
	ASSERT( pszNameOfBackupFile        != NULL );

	if ( pszNameOfSubkey             == NULL ||
		pszNameOfFileWithNewData == NULL ||
		pszNameOfBackupFile        == NULL )
		{
		TRACE0("In COXRegistry::Replace : Invalid parameter\n");
		m_ErrorCode = ERROR_INVALID_PARAMETER;
		return( FALSE );
		}

	m_ErrorCode = ::RegReplaceKey( m_KeyHandle, 
	                          pszNameOfSubkey,
	                          pszNameOfFileWithNewData,
	                          pszNameOfBackupFile );

	if ( m_ErrorCode == ERROR_SUCCESS )
		{
		return( TRUE );
		}
	else
		{
		TRACE1("In COXRegistry::Replace : Replace Failed code = %lu\n", m_ErrorCode);
		return( FALSE );
		}
	}

BOOL COXRegistry::RestoreFromFile( LPCTSTR pszNameOfFileHoldingSavedTree, const DWORD dwVolatilityFlags )
	{
	ASSERT_VALID( this );
	ASSERT( pszNameOfFileHoldingSavedTree != NULL );

	if ( pszNameOfFileHoldingSavedTree == NULL )
		{
		TRACE0("In COXRegistry::Restore : Invalid parameter\n");
		m_ErrorCode = ERROR_INVALID_PARAMETER;
		return( FALSE );
		}

	m_ErrorCode = ::RegRestoreKey( m_KeyHandle,
		                          pszNameOfFileHoldingSavedTree,
		                          dwVolatilityFlags );

	if ( m_ErrorCode == ERROR_SUCCESS )
		{
		return( TRUE );
		}
	else
		{
		TRACE1("In COXRegistry::Restore : Restore Failed code = %lu\n", m_ErrorCode);
		return( FALSE );
		}
	}

BOOL COXRegistry::SaveToFile( LPCTSTR pszNameOfFileToHoldTree, LPSECURITY_ATTRIBUTES pSecurityAttributes )
	{
	ASSERT_VALID( this );
	ASSERT( pszNameOfFileToHoldTree != NULL );

	if ( pszNameOfFileToHoldTree == NULL )
		{
		TRACE0("In COXRegistry::Save : Invalid parameter\n");
		m_ErrorCode = ERROR_INVALID_PARAMETER;
		return( FALSE );
		}

	m_ErrorCode = ::RegSaveKey( m_KeyHandle, pszNameOfFileToHoldTree, pSecurityAttributes );

	if ( m_ErrorCode == ERROR_SUCCESS )
		{
		return( TRUE );
		}
	else
		{
		TRACE1("In COXRegistry::Save : Save Failed code = %lu\n", m_ErrorCode);
		return( FALSE );
		}
	}

BOOL COXRegistry::SetBinaryValue( LPCTSTR pszNameOfValue, const CByteArray& abBytesToWrite )
	{
	ASSERT_VALID( this );
	ASSERT( pszNameOfValue != NULL );

	if ( pszNameOfValue == NULL )
		{
		TRACE0("In COXRegistry::SetBinaryValue : Invalid parameter\n");
		m_ErrorCode = ERROR_INVALID_PARAMETER;
		return( FALSE );
		}

	DWORD size_of_buffer = abBytesToWrite.GetSize();

	LPBYTE memory_buffer = new BYTE[ size_of_buffer ];

	if ( memory_buffer == NULL )
		{
		m_ErrorCode = ::GetLastError();
		return( FALSE );
		}

	DWORD index = 0;

	while( index < size_of_buffer )
		{
		memory_buffer[ index ] = abBytesToWrite[ index ];
		index++;
		}

	BOOL return_value = SetValue( pszNameOfValue, typeBinary, memory_buffer, size_of_buffer );

	delete [] memory_buffer;

	return( return_value );
	}

BOOL COXRegistry::SetDoubleWordValue( LPCTSTR pszNameOfValue, DWORD dwValueToWrite )
	{
	ASSERT_VALID( this );
	ASSERT( pszNameOfValue != NULL );

	if ( pszNameOfValue == NULL )
		{
		TRACE0("In COXRegistry::SetDoubleWordValue : Invalid parameter\n");
		m_ErrorCode = ERROR_INVALID_PARAMETER;
		return( FALSE );
		}

	return( SetValue( pszNameOfValue, typeDoubleWord, (const PBYTE) &dwValueToWrite, sizeof( DWORD ) ) );
	}

BOOL COXRegistry::SetSecurity( const SECURITY_INFORMATION& securityInformation,
                             const PSECURITY_DESCRIPTOR  pSecurityDescriptor )
	{
	ASSERT_VALID( this );
	ASSERT( pSecurityDescriptor != NULL );

	if ( pSecurityDescriptor == NULL )
		{
		TRACE0("In COXRegistry::SetSecurity : Invalid parameter\n");
		m_ErrorCode = ERROR_INVALID_PARAMETER;
		return( FALSE );
		}

	m_ErrorCode = ::RegSetKeySecurity( m_KeyHandle, securityInformation, pSecurityDescriptor );

	if ( m_ErrorCode == ERROR_SUCCESS )
		{
		return( TRUE );
		}
	else
		{
		return( FALSE );
		}
	}

BOOL COXRegistry::SetStringValue( LPCTSTR pszNameOfValue, const CString& sStringValue,
	BOOL bTypeUnexpandedString /* = FALSE */)
	{
	ASSERT_VALID( this );
	ASSERT( pszNameOfValue != NULL );

	if ( pszNameOfValue == NULL )
		{
		TRACE0("In COXRegistry::SetStringValue : Invalid parameter\n");
		m_ErrorCode = ERROR_INVALID_PARAMETER;
		return( FALSE );
		}

	return( SetValue( pszNameOfValue, bTypeUnexpandedString ? typeUnexpandedString : typeString, 
		(const PBYTE) (LPCTSTR) sStringValue, (sStringValue.GetLength() + 1 ) * sizeof(TCHAR) ) );
	}

BOOL COXRegistry::SetStringArrayValue( LPCTSTR pszNameOfValue, const CStringArray& asStringArray )
	{
	ASSERT_VALID( this );
	ASSERT( pszNameOfValue != NULL );

	if ( pszNameOfValue == NULL )
		{
		TRACE0("In COXRegistry::SetStringArrayValue : Invalid parameter\n");
		m_ErrorCode = ERROR_INVALID_PARAMETER;
		return( FALSE );
		}

	DWORD size_of_buffer = 0;

	/*
	** Find out how big our buffer needs to be...
	*/

	int index             = 0;
	int number_of_strings = asStringArray.GetSize();

	CString temp_string;

	while( index < number_of_strings )
		{
		temp_string = asStringArray[ index ];
		size_of_buffer += temp_string.GetLength() + 1;
		index++;
		}

	/*
	** Don't forget the second NULL needed for double null terminated strings...
	*/

	size_of_buffer++;

	LPBYTE memory_buffer = new BYTE[ size_of_buffer ];

	if ( memory_buffer == NULL )
		{
		m_ErrorCode = ::GetLastError();
		return( FALSE );
		}

	::ZeroMemory( memory_buffer, size_of_buffer );

	/*
	** OK, now add the strings to the memory buffer
	*/

	LPTSTR string = (LPTSTR) memory_buffer;

	index             = 0;
	int string_length = 0;

	while( index < number_of_strings )
		{
		temp_string = asStringArray[ index ];
		_tcscpy( &string[ string_length ], temp_string );
		string_length += temp_string.GetLength() + 1;

		index++;
		}

	string_length++;

	BOOL return_value = TRUE;

	KeyValueTypes type = typeMultipleString; // A double NULL terminated string

	if ( SetValue( pszNameOfValue, type, memory_buffer, size_of_buffer ) != TRUE )
		{
		return_value = FALSE;
		}

	delete [] memory_buffer;

	return( return_value );
	}

BOOL COXRegistry::SetValue( LPCTSTR pszNameOfValue, const CByteArray& abBytesToWrite )
	{
	ASSERT_VALID( this );
	ASSERT( pszNameOfValue != NULL );

	if ( pszNameOfValue == NULL )
		{
		TRACE0("In COXRegistry::SetValue : Invalid parameter\n");
		m_ErrorCode = ERROR_INVALID_PARAMETER;
		return( FALSE );
		}

	return( SetBinaryValue( pszNameOfValue, abBytesToWrite ) );
	}

BOOL COXRegistry::SetValue( LPCTSTR pszNameOfValue, DWORD dwValue )
	{
	ASSERT_VALID( this );
	ASSERT( pszNameOfValue != NULL );

	if ( pszNameOfValue == NULL )
		{
		TRACE0("In COXRegistry::SetValue : Invalid parameter\n");
		m_ErrorCode = ERROR_INVALID_PARAMETER;
		return( FALSE );
		}

	return( SetDoubleWordValue( pszNameOfValue, dwValue ) );
	}

BOOL COXRegistry::SetValue( LPCTSTR pszNameOfValue, const CStringArray& asStringsToWrite )
	{
	ASSERT_VALID( this );
	ASSERT( pszNameOfValue != NULL );

	if ( pszNameOfValue == NULL )
		{
		TRACE0("In COXRegistry::SetValue : Invalid parameter\n");
		m_ErrorCode = ERROR_INVALID_PARAMETER;
		return( FALSE );
		}

	return( SetStringArrayValue( pszNameOfValue, asStringsToWrite ) );
	}

BOOL COXRegistry::SetValue( LPCTSTR pszNameOfValue, const CString& sStringToWrite,
		BOOL bTypeUnexpandedString /* = FALSE */)
	{
	ASSERT_VALID( this );
	ASSERT( pszNameOfValue != NULL );

	if ( pszNameOfValue == NULL )
		{
		TRACE0("In COXRegistry::SetValue : Invalid parameter\n");
		m_ErrorCode = ERROR_INVALID_PARAMETER;
		return( FALSE );
		}

	return( SetStringValue( pszNameOfValue, sStringToWrite, bTypeUnexpandedString ) );
	}

BOOL COXRegistry::SetValue( LPCTSTR             pszNameOfSubkey, 
                          const KeyValueTypes eTypeOfValueToSet, 
                          const PBYTE         pbAddressOfValueData, 
                          const DWORD         dwSizeOfData )
	{
	ASSERT_VALID( this );
	ASSERT( pszNameOfSubkey         != NULL );
	ASSERT( pbAddressOfValueData != NULL );

	if ( pszNameOfSubkey == NULL || pbAddressOfValueData == NULL )
		{
		TRACE0("In COXRegistry::SetValue : Invalid parameter\n");
		m_ErrorCode = ERROR_INVALID_PARAMETER;
		return( FALSE );
		}

	m_ErrorCode = ::RegSetValueEx( m_KeyHandle,
	                          pszNameOfSubkey,
	                          0,
	                          eTypeOfValueToSet,
	                          pbAddressOfValueData,
	                          dwSizeOfData );

	if ( m_ErrorCode == ERROR_SUCCESS )
		{
		return( TRUE );
		}
	else
		{
		return( FALSE );
		}
	}

BOOL COXRegistry::UnLoadKey( LPCTSTR pszNameOfSubkeyToUnload )
	{
	ASSERT_VALID( this );
	ASSERT( pszNameOfSubkeyToUnload != NULL );

	if ( pszNameOfSubkeyToUnload == NULL )
		{
		TRACE0("In COXRegistry::UnLoad : Invalid parameter\n");
		m_ErrorCode = ERROR_INVALID_PARAMETER;
		return( FALSE );
		}

	m_ErrorCode = ::RegUnLoadKey( m_KeyHandle, pszNameOfSubkeyToUnload );

	if ( m_ErrorCode == ERROR_SUCCESS )
		{
		return( TRUE );
		}
	else
		{
		TRACE1("In COXRegistry::UnLoad : UnLoad Failed code = %lu\n", m_ErrorCode);
		return( FALSE );
		}
	}

BOOL COXRegistry::RenameRegistryKey(LPCTSTR pszToKey)
	{
	BOOL bSuccess = CopyRegistryKey(pszToKey, TRUE);         //  Copy "from"->"to"
	if (!bSuccess)
		return FALSE;

	DeleteKey(m_sKeyName);							//  Delete last top-level key
	
	return(OpenKey(pszToKey));
	}                                               //  End of RenameRegistryItem()

BOOL COXRegistry::CopyRegistryKey(LPCTSTR pszToKey, BOOL bReplace /* = FALSE */)
	{
	HKEY hToKeyHandle;
	long ErrorCode = ::RegCreateKeyEx(m_RegistryHandle, pszToKey,
									(DWORD) 0,
									(LPTSTR) NULL,
									optionsNonVolatile,
									permissionAllAccess,
									NULL,
									&hToKeyHandle,
									NULL);

	if (ErrorCode != ERROR_SUCCESS)
		{
		TRACE1("In COXRegistry::CopyRegistryKey : Subkey Creation Failed code = %lu\n", ErrorCode);
		return(FALSE);
		}
	
	return(CopyRegistryKey(m_KeyHandle, hToKeyHandle, bReplace));    //  to "walk the tree"
	}

BOOL COXRegistry::CopyRegistryKey(HKEY hFromKey, HKEY hToKey, BOOL bReplace)
	{
	DWORD dwValue, dwKey;
	long lStatus, lStatus2;
	DWORD dwValueSize, dwBufferSize, dwType;
	CString sValueName;
	BYTE* pbBuffer;
	HKEY hNewTo, hNewFrom;
	
	pbBuffer = (BYTE*)(GlobalAllocPtr(GHND, 8192));      //  Need some buffer space
	if(pbBuffer == NULL)
		return (FALSE);
	
	//  Start with the highest index of Values
	lStatus = ::RegQueryInfoKey(hFromKey, NULL, NULL, NULL, &dwKey, NULL, NULL, &dwValue, NULL, NULL, NULL, NULL);
	if (lStatus != ERROR_SUCCESS)
		return FALSE;

	do 
		{
		dwValueSize = 512;							//  Tell system how much buffer we have
		dwBufferSize = 8192;

		//  and enumerate data values at current key
		lStatus = ::RegEnumValue(hFromKey, --dwValue, sValueName.GetBuffer(dwValueSize), &dwValueSize,
			NULL, &dwType, pbBuffer, &dwBufferSize);
		sValueName.ReleaseBuffer();
		
		if(lStatus == ERROR_SUCCESS)           //  Move each value to new place
			{
			lStatus2 = ::RegSetValueEx(hToKey, (LPCTSTR)sValueName, 0, dwType, pbBuffer,
				dwBufferSize);

			if (bReplace)
				::RegDeleteValue(hFromKey, (LPCTSTR)sValueName);
			}
		} 
	while(lStatus == ERROR_SUCCESS && lStatus2 == ERROR_SUCCESS);			//  Loop until all values found
		
	//  Start over, looking for keys now instead of values
	FILETIME FileTime;
	do
		{
		dwValueSize = 512;					//  Tell system how much buffer we have
		dwBufferSize = 8192;				//  Tell system about the buffer size
		
		//  Get "next" key
		lStatus = ::RegEnumKeyEx(hFromKey, --dwKey, sValueName.GetBuffer(dwValueSize), &dwValueSize,
			NULL, NULL, NULL, &FileTime);
		sValueName.ReleaseBuffer();
		
		if( lStatus == ERROR_SUCCESS)						//  Was a valid key found?
			{                                               //  Open the key if found
			lStatus2 = ::RegCreateKeyEx(hToKey, sValueName,
										(DWORD) 0,
										(LPTSTR) NULL,
										optionsNonVolatile,
										permissionAllAccess,
										NULL,
										&hNewTo,
										NULL);

			if(lStatus2 == ERROR_SUCCESS)					//  If the key opened...
				{											//  Create new key of old name
				lStatus2 = ::RegOpenKeyEx(hFromKey, sValueName, NULL, permissionAllAccess, &hNewFrom);
				if(lStatus2 == ERROR_SUCCESS)
					{													//  If that worked, recurse back here
					CopyRegistryKey(hNewFrom, hNewTo, bReplace);		//  to "walk the tree"
					::RegCloseKey(hNewFrom);							//  Close each new key
					if (bReplace)
						::RegDeleteKey(hFromKey, sValueName);   //  Delete old key
					}

				::RegCloseKey(hNewTo);                       //  Close each old key
				}
			}
		}
	while(lStatus == ERROR_SUCCESS && lStatus2 == ERROR_SUCCESS);             //  Loop until key enum fails
			
	GlobalFreePtr(pbBuffer);                      //  Free buffer and exit
	return (lStatus2 == ERROR_SUCCESS);
	}