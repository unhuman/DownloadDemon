#include "stdafx.h"

//Function declarations used here
void DeleteInetSession(CInternetSession** pInetSession)
{
	if (pInetSession && *pInetSession)
	{
		(*pInetSession)->Close();
		delete *pInetSession;
		*pInetSession = NULL;
	}
}
void DeleteFtpConnection(CFtpConnection** pFtpConnection)
{
	if (pFtpConnection && *pFtpConnection)
	{
		(*pFtpConnection)->Close();
		delete *pFtpConnection;
		*pFtpConnection = NULL;
	}
}
void DeleteHttpConnection(CHttpConnection** pHttpConnection)
{
	if (pHttpConnection && *pHttpConnection)
	{
		(*pHttpConnection)->Close();
		delete *pHttpConnection;
		*pHttpConnection = NULL;
	}
}
void DeleteFtpFile(CInternetFile** pFtpFile)
{
	if (pFtpFile && *pFtpFile)
	{
		(*pFtpFile)->Close();
		delete *pFtpFile;
		*pFtpFile = NULL;
	}
}
void DeleteHttpFile(CHttpFile** pHttpFile)
{
	if (pHttpFile && *pHttpFile)
	{
		(*pHttpFile)->Close();
		delete *pHttpFile;
		*pHttpFile = NULL;
	}
}

BOOL TimeoutAssociatedError(DWORD dwError)
{
	if ((dwError == ERROR_INTERNET_TIMEOUT) ||
		(dwError == ERROR_INTERNET_CANNOT_CONNECT) ||
		(dwError == ERROR_INTERNET_CONNECTION_ABORTED) ||
		(dwError == ERROR_INTERNET_CONNECTION_RESET))
		return TRUE;
	else
		return FALSE;
}