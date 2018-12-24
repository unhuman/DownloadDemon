// ==========================================================================
// 					Class Specification : COXRegistry
// ==========================================================================

// Header file : registry.h

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

// Properties:
//	NO	Abstract class (does not have any objects)
//	YES	Derived from CObject

//	NO	Is a Cwnd.                     
//	NO	Two stage creation (constructor & Create())
//	NO	Has a message map
//	NO 	Needs a resource (template)

//	NO	Persistent objects (saveable on disk)      
//	NO	Uses exceptions

// //////////////////////////////////////////////////////////////////////////

// Desciption :         
//   This class makes the reading and writing to the registry of
//   Windows NT and Win95 very easy

// Remark:

// Prerequisites (necessary conditions):
//		***

/////////////////////////////////////////////////////////////////////////////

#ifndef __REGISTRY_H__
#define __REGISTRY_H__

#include <winreg.h>

#define THIS_SUB_KEY FALSE
#define ALL_SUB_KEYS TRUE

#define SIGNAL_EVENT    TRUE
#define WAIT_FOR_CHANGE FALSE

class COXRegistry : public CObject
{
DECLARE_DYNAMIC( COXRegistry )
// Data members -------------------------------------------------------------
public:
	static const HKEY keyLocalMachine;
	static const HKEY keyClassesRoot;
	static const HKEY keyUsers;
	static const HKEY keyCurrentUser;

#if(WINVER >= 0x0400)
 	// Win 95 extra	registries
	static const HKEY keyCurrentConfig;
	static const HKEY keyDynamicData;
#endif

	enum KeyValueTypes
	{
	 typeBinary                 = REG_BINARY,
	 typeDoubleWord             = REG_DWORD,
	 typeDoubleWordLittleEndian = REG_DWORD_LITTLE_ENDIAN,
	 typeDoubleWordBigEndian    = REG_DWORD_BIG_ENDIAN,
	 typeUnexpandedString       = REG_EXPAND_SZ,
	 typeSymbolicLink           = REG_LINK,
	 typeMultipleString         = REG_MULTI_SZ,
	 typeNone                   = REG_NONE,
	 typeResourceList           = REG_RESOURCE_LIST,
	 typeString                 = REG_SZ
	};

	enum CreateOptions
	{
	 optionsNonVolatile = REG_OPTION_NON_VOLATILE,
	 optionsVolatile    = REG_OPTION_VOLATILE
	};

	enum CreatePermissions
	{
	 permissionAllAccess        = KEY_ALL_ACCESS,
	 permissionCreateLink       = KEY_CREATE_LINK,
	 permissionCreateSubKey     = KEY_CREATE_SUB_KEY,
	 permissionEnumerateSubKeys = KEY_ENUMERATE_SUB_KEYS,
	 permissionExecute          = KEY_EXECUTE,
	 permissionNotify           = KEY_NOTIFY,
	 permissionQueryValue       = KEY_QUERY_VALUE,
	 permissionRead             = KEY_READ,
	 permissionSetValue         = KEY_SET_VALUE,
	 permissionWrite            = KEY_WRITE
	};

	enum CreationDisposition
	{
	 dispositionCreatedNewKey     = REG_CREATED_NEW_KEY,
	 dispositionOpenedExistingKey = REG_OPENED_EXISTING_KEY
	};

	enum NotifyChangeFlag
	{
	 changeKeyAndSubkeys    = TRUE,
	 changeSpecifiedKeyOnly = FALSE
	};

	enum NotifyChangeFilter
	{
	 notifyName       = REG_NOTIFY_CHANGE_NAME,
	 notifyAttributes = REG_NOTIFY_CHANGE_ATTRIBUTES,
	 notifyLastSet    = REG_NOTIFY_CHANGE_LAST_SET,
	 notifySecurity   = REG_NOTIFY_CHANGE_SECURITY
	};

protected:
	HKEY m_KeyHandle;
	HKEY m_RegistryHandle;

	LONG m_ErrorCode;

	CString m_sClassName;
	CString m_sComputerName;
	CString m_sKeyName;
	CString m_sRegistryName;
	DWORD   m_NumberOfSubkeys;
	DWORD   m_NumberOfValues;

	/*
	** Data items filled in by QueryInfo
	*/

	DWORD    m_LongestSubkeyNameLength;
	DWORD    m_LongestClassNameLength;
	DWORD    m_LongestValueNameLength;
	DWORD    m_LongestValueDataLength;
	DWORD    m_SecurityDescriptorLength;
	FILETIME m_LastWriteTime;

	BOOL	 m_bWin95;

private:

// Member functions ---------------------------------------------------------
public:
	COXRegistry();
	// --- In  :
	// --- Out : 
	// --- Returns :
	// --- Effect : Contructor of object
	//				It will initialize the internal state

	static BOOL IsWin95();
	// --- In  :
	// --- Out : 
	// --- Returns : whether this WIN32 environment is Windows95 or Win NT
	// --- Effect : 

	virtual BOOL Disconnect();
	// --- In  :
	// --- Out : 
	// --- Returns : succeeded or not
	// --- Effect : disconnects and releases the handle of the connected registry. 

	virtual BOOL Connect( HKEY    hKeyToOpen   = COXRegistry::keyCurrentUser,
	                    LPCTSTR pszNameOfComputer = NULL );
	// --- In  : pszNameOfComputer : Points to a null-terminated string containing the
	// 				name of the remote computer. The string has the following form:
	// 				\\computername 
	//				if lpszComputerName is NULL, the local computer name is used. 
	//			 hKeyToOpen : Specifies the predefined handle of the registry on
	// 				the remote computer. Currently, the following values can be
	// 				used:
	// 						HKEY_LOCAL_MACHINE
	// 						HKEY_USERS
	//				An application cannot specify the HKEY_CLASSES_ROOT or
	// 				HKEY_CURRENT_USER value for this parameter. 
	// --- Out : 
	// --- Returns : succeeded or not
	// --- Effect : establishes a connection to a predefined registry handle on
	// 				a REMOTE computer.  

	virtual BOOL CreateKey( LPCTSTR          pszNameOfSubkey,
	                   LPCTSTR               pszNameOfClass        = NULL,
	                   CreateOptions         options               = optionsNonVolatile,
	                   CreatePermissions     permissions           = permissionAllAccess,
	                   LPSECURITY_ATTRIBUTES pSecurityAttributes   = NULL,
	                   CreationDisposition*  pDisposition          = NULL );
	// --- In  : pszNameOfSubkey	: Points to a null-terminated string specifying the name
	// 				of a subkey that this function opens or creates. The subkey
	// 				specified must be a subkey of the parent key parameter. This
	// 				subkey must not begin with the backslash character ('\').
	// 				This parameter cannot be NULL. 
	//			 pszNameOfClass : Points to a null-terminated string that specifies the
	// 				class (object type) of this key. This parameter is ignored if
	// 				the key already exists. 
	// 			 options : see ::RegCreateKeyEx parameter fdwOptions and enum CreateOptions
	// 			 permissions : see ::RegCreateKeyEx parameter samDesired and enum
	// 				CreatePermissions in this header file
	// 			 pSecurityAttributes : see ::RegCreateKeyEx parameter lpSecurityAttributes
	// 			 pDisposition : see ::RegCreateKeyEx parameter lpdwDisposition  and enum
	// 				CreationDisposition in this header file
	// --- Out : pDisposition : will receive one of the CreationDisposition values 
	// --- Returns : succeeded or not
	// --- Effect : creates the specified key. If the key already exists in the
	// 				registry, the function opens it. 

	virtual BOOL DeleteKey( LPCTSTR pszNameOfKeyToDelete );
	// --- In  : pszNameOfKeyToDelete :	Points to a null-terminated string specifying
	// 				the name of the key to delete. This parameter cannot be NULL,
	// 				and the specified key must not have subkeys. 
	// --- Out : 
	// --- Returns : succeeded or not
	// --- Effect : 
	// 				Windows 95: The DeleteKey function deletes a key and all its
	// 					descendents.
	//				Windows NT: The DeleteKey function deletes the specified key.
	// 					This function cannot delete a key that has subkeys. 

	virtual BOOL DeleteValue( LPCTSTR pszNameOfValueToDelete );
	// --- In  : pszNameOfValueToDelete : Points to a null-terminated string that
	// 				names the value to remove. If this parameter is NULL or points
	// 				to an empty string, the value set by the RegSetValue function
	// 				is removed. 
	// --- Out : 
	// --- Returns : succeeded or not
	// --- Effect : removes a named value from the current registry key. 

	virtual BOOL EnumerateKeys( const DWORD dwSubkeyIndex,
	                          CString&    sSubkeyName,
	                          CString&    sClassName );
	// --- In  : dwSubkeyIndex : Specifies the index of the subkey to retrieve. This
	// 				parameter should be zero for the first call to the EnumerateKeys
	// 				function and then incremented for subsequent calls. Because subkeys
	// 				are not ordered, any new subkey will have an arbitrary index.
	// 				This means that the function may return subkeys in any order. 
	// --- Out : sSubkeyName : receives the name of the subkey. Only the name of
	// 				the subkey, not the full key hierarchy is returned. 
	// 			 sClassName : receives the class of the enumerated subkey This
	// 				parameter can be empty if the class is not required.  
	// --- Returns : succeeded or not
	// --- Effect : The RegEnumKeyEx function enumerates subkeys of the specified
	// 				open registry key. The function retrieves information about one
	// 				subkey each time it is called. 

	virtual BOOL EnumerateValues( const DWORD    dwValueIndex,
	                            CString&       sNameOfValue,
	                            KeyValueTypes& eTypeCode,
	                            LPBYTE         pbDataBuffer,
	                            DWORD&         dwSizeOfDataBuffer );
	// --- In  : dwValueIndex	: Specifies the index of the value to retrieve. This parameter
	// 				should be zero for the first call to the EnumerateValues function
	// 				and then be incremented for subsequent calls. Because values are
	// 				not ordered, any new value will have an arbitrary index.
	// 				This means that the function may return values in any order. 
	// --- Out : sNameOfValue : receives the name of the value. 
	//			 eTypeCode : see ::RegEnumValue parameter lpdwType  and enum
	// 				KeyValueTypes in this header file
	//			 pbDataBuffer : receives the data for the value entry. This parameter
	// 				can be NULL if the data is not required.
	//			 dwSizeOfDataBuffer : Points to a variable that specifies the size,
	// 				in bytes, of the variable pointed to by the data_buffer parameter.
	// 				When the function returns, it contains the number of bytes stored
	// 				in data_buffer. This parameter can be NULL, only if data_buffer is NULL.
	// --- Returns : succeeded or not
	// --- Effect : enumerates the values for the current open registry key.
	// 				The function copies one indexed value name and data block for
	// 				the key each time it is called. 

	virtual BOOL FlushRegistry();
	// --- In  :
	// --- Out : 
	// --- Returns : succeeded or not
	// --- Effect : writes all the attributes of the current open key into the registry. 

	virtual BOOL GetBinaryValue( LPCTSTR pszNameOfValue, CByteArray& abReturnArray );
	// --- In  : pszNameOfValue : Points to a null-terminated string containing the name
	// 				of the value to be queried. 
	// --- Out : abReturnArray : the binary data
	// --- Returns : succeeded or not
	// --- Effect : retrieves the data with binary type for a specified value name associated
	// 				with current open registry key.

	virtual void GetClassName( CString& sClassName ) const;
	// --- In  :
	// --- Out : 
	// --- Returns : sClassName : current class name
	// --- Effect : gets the class name

	virtual void GetComputerName( CString& sComputerName ) const;
	// --- In  :
	// --- Out : 
	// --- Returns : sComputerName : current computer name
	// --- Effect : gets the current computer name

	virtual BOOL GetDoubleWordValue( LPCTSTR pszNameOfValue, DWORD& dwReturnValue );
	// --- In  : pszNameOfValue : Points to a null-terminated string containing the name
	// 				of the value to be queried. 
	// --- Out : dwReturnValue : the Double word if found
	// --- Returns : succeeded or not
	// --- Effect : retrieves the data with double word type for a specified value name
	// 				associated with current open registry key.

	virtual LONG GetErrorCode() const;
	// --- In  :
	// --- Out : 
	// --- Returns : the last error
	// --- Effect : gets the errorcode associated with the last executed command

	virtual void GetKeyName( CString& sKeyName ) const;
	// --- In  :
	// --- Out : 
	// --- Returns : sKeyName : the current key
	// --- Effect : 

	virtual DWORD GetNumberOfSubkeys() const;
	// --- In  :
	// --- Out : 
	// --- Returns : the number of subkeys the current key contains
	// --- Effect : 

	virtual DWORD GetNumberOfValues() const;
	// --- In  :
	// --- Out : 
	// --- Returns : the number of values the current key contains
	// --- Effect : 

	virtual void GetRegistryName( CString& sRegistryName ) const;
	// --- In  :
	// --- Out : sRegistryName : the name of the current registry
	// --- Returns :
	// --- Effect : 

	virtual BOOL GetSecurity( const SECURITY_INFORMATION whatYouWantToKnow,
	                        PSECURITY_DESCRIPTOR       pDataBuffer,
	                        DWORD&                     dwSizeOfDataBuffer );
	// --- In  : whatYouWantToKnow : Specifies a SECURITY_INFORMATION structure
	// 				that indicates the requested security information. 
	// --- Out : pDataBuffer : receives the data for the entry.
	//			 dwSizeOfDataBuffer : Points to a variable that specifies the size,
	// 				in bytes, of the variable pointed to by the data_buffer parameter.
	// 				When the function returns, it contains the number of bytes stored
	// 				in data_buffer.
 
	// --- Returns : succeeded or not
	// --- Effect : retrieves a copy of the security descriptor protecting the current
	// 				open registry key. 

	virtual BOOL GetStringValue( LPCTSTR pszNameOfValue, CString& sReturnString );
	// --- In  : pszNameOfValue : Points to a null-terminated string containing the name
	// 				of the value to be queried. 
	// --- Out : sReturnString : the string if found
	// --- Returns : succeeded or not
	// --- Effect : retrieves the data with string type for a specified value name
	// 				associated with current open registry key.

	virtual BOOL GetStringArrayValue( LPCTSTR pszNameOfValue, CStringArray& asReturnArray );
	// --- In  : pszNameOfValue : Points to a null-terminated string containing the name
	// 				of the value to be queried. 
	// --- Out : asReturnArray : the string array if found
	// --- Returns : succeeded or not
	// --- Effect : retrieves the data with string array type for a specified value name
	// 				associated with current open registry key.

	virtual BOOL GetValue( LPCTSTR pszNameOfValue, CByteArray& abReturnArray );
	// --- In  : pszNameOfValue : Points to a null-terminated string containing the name
	// 				of the value to be queried. 
	// --- Out : abReturnArray : the binary data
	// --- Returns : succeeded or not
	// --- Effect : retrieves the data with binary type for a specified value name associated
	// 				with current open registry key.

	virtual BOOL GetValue( LPCTSTR pszNameOfValue, DWORD& dwReturnValue );
	// --- In  : pszNameOfValue : Points to a null-terminated string containing the name
	// 				of the value to be queried. 
	// --- Out : dwReturnValue : the Double word if found
	// --- Returns : succeeded or not
	// --- Effect : retrieves the data with double word type for a specified value name
	// 				associated with current open registry key.

	virtual BOOL GetValue( LPCTSTR pszNameOfValue, CString& sReturnString );
	// --- In  : pszNameOfValue : Points to a null-terminated string containing the name
	// 				of the value to be queried. 
	// --- Out : sReturnString : the string if found
	// --- Returns : succeeded or not
	// --- Effect : retrieves the data with string type for a specified value name
	// 				associated with current open registry key.

	virtual BOOL GetValue( LPCTSTR pszNameOfValue, CStringArray& asReturnArray );
	// --- In  : pszNameOfValue : Points to a null-terminated string containing the name
	// 				of the value to be queried. 
	// --- Out : asReturnArray : the string array if found
	// --- Returns : succeeded or not
	// --- Effect : retrieves the data with string array type for a specified value name
	// 				associated with current open registry key.

	virtual BOOL LoadFromFile( LPCTSTR pszNameOfSubkey,
	                 LPCTSTR pszNameOfFileContainingInformation );
	// --- In  : pszNameOfSubkey : Points to a null-terminated string that specifies the
	// 				name of the key to be created under hKey. This subkey is where the
	// 				registration information from the file will be loaded.
	//			 pszNameOfFileContainingInformation : Points to a null-terminated string
	// 				containing the name of a file that has registration information.
	// 				This file must have been created with the RegSaveKey function. Under
	// 				the file allocation table (FAT) file system, the filename may not
	// 				have an extension. 
	// --- Out : 
	// --- Returns : succeeded or not
	// --- Effect : The RegLoadKey function creates a subkey under HKEY_USER or HKEY_LOCAL_MACHINE
	// 				and stores registration information from a specified file into that
	// 				subkey. This registration information is in the form of a hive. A hive
	// 				is a discrete body of keys, subkeys, and values that is rooted at the top
	// 				of the registry hierarchy. A hive is backed by a single file and .LOG file.  

	virtual BOOL NotifyChange( const HANDLE hEventHandle                     = NULL,
	                         const NotifyChangeFilter eChangesToBeReported   = notifyLastSet,
	                         const BOOL bThisSubkeyOrAllSubkeys              = changeSpecifiedKeyOnly,
	                         const BOOL bWaitForChangeOrSignalEvent          = WAIT_FOR_CHANGE );
	// --- In  : hEventHandle : see ::RegNotifyChangeKeyValue parameter hEvent
	//			 eChangesToBeReported : see ::RegNotifyChangeKeyValue parameter fdwNotifyFilter
	// 				and	see this header for enum NotifyChangeFilter
	//			 bThisSubkeyOrAllSubkeys	: see ::RegNotifyChangeKeyValue parameter fWatchSubTree
	//			 bWaitForChangeOrSignalEvent : see ::RegNotifyChangeKeyValue parameter fAsync 
	// --- Out : 
	// --- Returns : succeeded or not
	// --- Effect : indicates when a registry key or any of its subkeys has changed. 

	virtual BOOL OpenKey( LPCTSTR pszNameOfSubkeyToOpen,
	                 const CreatePermissions eSecurityAccessMask = permissionAllAccess );
	// --- In  : pszNameOfSubkeyToOpen : name of sub key to open
	//			 eSecurityAccessMask : see ::RegOpenKeyEx and enum CreatePermissions 
	// --- Out : 
	// --- Returns : succeeded or not
	// --- Effect : opens the a subkey of the main registry key.

	BOOL RenameRegistryKey(LPCTSTR pszToKey);
	// --- In  : pszToKey : the new name for the open subkey
	// --- Out : 
	// --- Returns : succeeded or not
	// --- Effect : Renames the open subkey of the main registry key
	//				Requires connection to a main registry and an open subkey.

	BOOL CopyRegistryKey(LPCTSTR pszToKey, BOOL bReplace = FALSE);
	// --- In  : pszToKey : the new name for the new subkey
	//			 bReplace : whether or not the the open subkey to be copied will be deleted
	// --- Out : 
	// --- Returns : succeeded or not
	// --- Effect : Copies the open subkey to another subkey

	virtual BOOL QueryInfo();
	// --- In  :
	// --- Out : 
	// --- Returns : succeeded or not
	// --- Effect : retrieves information about a current open registry key. 

	virtual BOOL QueryValue( LPCTSTR        pszNameOfValue,
	                       KeyValueTypes&   eValueType,
	                       LPBYTE           pbAddressOfBuffer,
	                       DWORD&           dwSizeOfBuffer );
	// --- In  : pszNameOfValue : Points to a null-terminated string containing the name
	// 				of the value to be queried. 
	// --- Out : eValueType : see ::RegEnumValue parameter lpdwType  and enum
	// 				KeyValueTypes in this header file
	//			 pbAddressOfBuffer : Points to a buffer that receives the value's data.
	// 				This parameter can be NULL if the data is not required.
	//			 dwSizeOfBuffer	: Points to a variable that specifies the size, in bytes
	//				of the buffer pointed to by the address_of_buffer parameter. When the
	// 				function returns, it contains the size of the data copied to address_of_buffer. 
	// --- Returns : succeeded or not
	// --- Effect : retrieves the type and data for a specified value name associated
	// 				with current open registry key. 

	virtual BOOL ReplaceFile( LPCTSTR pszNameOfSubkey,
	                    LPCTSTR pszNameOfFileWithNewData,
	                    LPCTSTR pszNameOfBackupFile );
	// --- In  : pszNameOfSubkey : Points to a null-terminated string containing the name
	// 				of a key whose subkeys and values are replaced by this function. This
	// 				key must be a subkey of the current key. This parameter can be NULL. 
	// 				The selected key must be the root of a hive; that is, it must be an
	// 				immediate descendent of HKEY_LOCAL_MACHINE or HKEY_USERS. 
	//			 pszNameOfFileWithNewData : Points to a null-terminated string containing
	// 				the name of the file with registration information. This file is typically
	//				created by using the RegSaveKey function. Under the file allocation
	// 				table (FAT) file system, the filename may not have an extension. 
	//			 pszNameOfBackupFile : Points to a null-terminated string containing the
	// 				name of a file that receives a backup copy of the registry information
	// 				being replaced. If this file is created under the FAT file system, it
	// 				should not have an extension. 
	// --- Out : 
	// --- Returns : succeeded or not
	// --- Effect : replaces the file backing a key and all its subkeys with another file,
	// 				so that when the system is next started, the key and subkeys will
	// 				have the values stored in the new file. 

	virtual BOOL RestoreFromFile( LPCTSTR pszNameOfFileHoldingSavedTree, const DWORD dwVolatilityFlags = NULL );
	// --- In  : pszNameOfFileHoldingSavedTree : Points to a null-terminated string containing
	// 				the name of the file with registry information. This file is typically 
	//				created by using the RegSaveKey function. Under the file allocation
	// 				table (FAT) file system, the filename may not have an extension. 
	//			 dwVolatilityFlags :	see RegRestoreKey parameter fdw
	// --- Out : 
	// --- Returns : succeeded or not
	// --- Effect : reads the registry information in a specified file and copies it over
	// 				the current open key. This registry information may be in the form of
	// 				a key and multiple levels of subkeys. 

	virtual BOOL SaveToFile( LPCTSTR pszNameOfFileToHoldTree, LPSECURITY_ATTRIBUTES pSecurityAttributes = NULL );
	// --- In  : pszNameOfFileToHoldTree : see ::RegSaveKey parameter lpszFile
	//			 pSecurityAttributes : see ::RegSaveKey parameter lpsa
	// --- Out : 
	// --- Returns : succeeded or not
	// --- Effect : saves the current open key and all of its subkeys and values to a new file. 

	virtual BOOL SetBinaryValue( LPCTSTR pszNameOfValue, const CByteArray& abBytesToWrite );
	// --- In  : pszNameOfValue : Points to a null-terminated string containing the name
	// 				of the value to be set. 
	//			 abBytesToWrite : data to be written
	// --- Out : 
	// --- Returns : succeeded or not
	// --- Effect : writes the data with binary type for a specified value name associated
	// 				with current open registry key.

	virtual BOOL SetDoubleWordValue( LPCTSTR pszNameOfValue, DWORD dwValueToWrite );
	// --- In  : pszNameOfValue : Points to a null-terminated string containing the name
	// 				of the value to be set. 
	//			 dwValueToWrite : data to be written
	// --- Out : 
	// --- Returns : succeeded or not
	// --- Effect : writes the data with double word type for a specified value name associated
	// 				with current open registry key.

	virtual BOOL SetSecurity( const SECURITY_INFORMATION& securityInformation,
	                        const PSECURITY_DESCRIPTOR pSecurityDescriptor );
	// --- In  : securityInformation : Specifies a SECURITY_INFORMATION structure that
	// 				indicates the contents of the supplied security descriptor. 
	//				Because subkeys are not ordered, any new subkey will have an arbitrary
	// 				index. This means the function may return subkeys in any order. 
	//			 pSecurityDescriptor : Points to a SECURITY_DESCRIPTOR structure that
	// 				specifies the security attributes to set for the specified key. 
	// --- Out : 
	// --- Returns : succeeded or not
	// --- Effect : sets the security of the current open registry key 

	virtual BOOL SetStringValue( LPCTSTR pszNameOfValue, const CString& sStringValue,
		BOOL bTypeUnexpandedString = FALSE);
	// --- In  : pszNameOfValue : Points to a null-terminated string containing the name
	// 				of the value to be set. 
	//			 sStringValue : string to be written
	//			 bTypeUnexpandedString : Whether the type is typeString (FALSE) or typeUnexpandedString (TRUE)
	// --- Out : 
	// --- Returns : succeeded or not
	// --- Effect : writes the data with string type for a specified value name associated
	// 				with current open registry key.

	virtual BOOL SetStringArrayValue( LPCTSTR pszNameOfValue, const CStringArray& asStringArray );
	// --- In  : pszNameOfValue : Points to a null-terminated string containing the name
	// 				of the value to be set. 
	//			 asStringArray : string array to be written
	// --- Out : 
	// --- Returns : succeeded or not
	// --- Effect : writes the data with string array type for a specified value name associated
	// 				with current open registry key.

	virtual BOOL SetValue( LPCTSTR pszNameOfValue, const CByteArray& abBytesToWrite );
	// --- In  : pszNameOfValue : Points to a null-terminated string containing the name
	// 				of the value to be set. 
	//			 abBytesToWrite : data to be written
	// --- Out : 
	// --- Returns : succeeded or not
	// --- Effect : writes the data with binary type for a specified value name associated
	// 				with current open registry key.

	virtual BOOL SetValue( LPCTSTR pszNameOfValue, DWORD dwValue );
	// --- In  : pszNameOfValue : Points to a null-terminated string containing the name
	// 				of the value to be set. 
	//			 dwValue : data to be written
	// --- Out : 
	// --- Returns : succeeded or not
	// --- Effect : writes the data with double word type for a specified value name associated
	// 				with current open registry key.

	virtual BOOL SetValue( LPCTSTR pszNameOfValue, const CString& sStringToWrite,
			BOOL bTypeUnexpandedString = FALSE);
	// --- In  : pszNameOfValue : Points to a null-terminated string containing the name
	// 				of the value to be set. 
	//			 sStringToWrite : string to be written
	//			 bTypeUnexpandedString : Whether the type is typeString (FALSE) or typeUnexpandedString (TRUE)
	// --- Out : 
	// --- Returns : succeeded or not
	// --- Effect : writes the data with string type for a specified value name associated
	// 				with current open registry key.

	virtual BOOL SetValue( LPCTSTR pszNameOfValue, const CStringArray& asStringsToWrite );
	// --- In  : name_of_value : Points to a null-terminated string containing the name
	// 				of the value to be set. 
	//			 asStringsToWrite : string array to be written
	// --- Out : 
	// --- Returns : succeeded or not
	// --- Effect : writes the data with string array type for a specified value name associated
	// 				with current open registry key.


	virtual BOOL SetValue( LPCTSTR             pszNameOfSubkey,
	                     const KeyValueTypes eTypeOfValueToSet,
	                     CONST PBYTE         pbAddressOfValueData,
	                     const DWORD         dwSizeOfData );
	// --- In  : pszNameOfSubkey: Points to a string containing the name of the value to
	// 				set. If a value with this name is not already present in the key,
	// 				the function adds it to the key. 
	// 			 eTypeOfValueToSet : see ::RegSetValueEx parameter dwType 
	// 			 pbAddressOfValueData : see ::RegSetValueEx parameter lpData 
	// 			 dwSizeOfData : see ::RegSetValueEx parameter cbData 
	// --- Out : 
	// --- Returns : succeeded or not
	// --- Effect : stores data in the value field of the open registry key. It can also
	// 				set additional value and type information for the specified key. 

	virtual BOOL UnLoadKey( LPCTSTR pszNameOfSubkeyToUnload );
	// --- In  : pszNameOfSubkeyToUnload : address of name of subkey to unload
	// --- Out : 
	// --- Returns : succeeded or not
	// --- Effect : unloads the specified key and subkeys from the registry. 

#if defined( _DEBUG )
    virtual void Dump( CDumpContext& dump_context ) const;
	virtual void AssertValid() const;
#endif // _DEBUG

	virtual ~COXRegistry();
	// --- In  :
	// --- Out : 
	// --- Returns :
	// --- Effect : Destructor of object

	// undocumented feature
	BOOL CheckAndOpenKeyForWrite( LPCTSTR pszNameOfSubkeyToOpen )
	{
		CreatePermissions permiss = (CreatePermissions) (
						(int) permissionCreateSubKey |
						(int) permissionEnumerateSubKeys |
						(int) permissionQueryValue |
						(int) permissionRead |
						(int) permissionSetValue |
						(int) permissionWrite);

		if (!OpenKey(pszNameOfSubkeyToOpen, permiss))
		{
			if (!OpenKey(pszNameOfSubkeyToOpen, permiss))
				return CreateKey(pszNameOfSubkeyToOpen);
			else
				return FALSE;
		}
		return TRUE;
	}

protected:

private:
	void m_Initialize();
	BOOL CopyRegistryKey(HKEY hFromKey, HKEY hToKey, BOOL bReplace);
	LONG DeleteRegistryKey(HKEY hParentKey, LPCTSTR pszNameOfKeyToDelete);
};

#endif // __REGISTRY_H__
