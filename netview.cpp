// netview.cpp : Defines the entry point for the console application.
//
/*
             _         _               
  _ __   ___| |___   _(_) _____      __
 | '_ \ / _ \ __\ \ / / |/ _ \ \ /\ / /
 | | | |  __/ |_ \ V /| |  __/\ V  V / 
 |_| |_|\___|\__| \_/ |_|\___| \_/\_/  
                        by mubix [at] hak5.org               
                        v1.0

						*/
/*
BackTrack5 R3 
/usr/bin/i586-mingw32msvc-g++ netview.cpp -D__MINGW32__ -DWINVER=0x0501 -DUNICODE -D_UNICODE -s -Wl,--subsystem,windows -Wall -g -I"/usr/i586-mingw32msvc/include" -I"/usr/amd64-mingw32msvc/include/sec_api" -L"/usr/i586-mingw32msvc/lib" -lws2_32 -lnetapi32 -ladvapi32 -lmingw32 -o netview.exe
*/

#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef UNICODE
#define UNICODE
#endif
// C includes
#include <vector>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <assert.h>
					
// Windows includes 
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h> 
#include <lm.h>
#include "./banned.h"



using namespace std;

#ifdef __MINGW32__
// This is if __MINGW32__ is not placed as a -D flag for the complier.
// The compiler will default to _WIN32 options. 
#else

#pragma comment(lib, "netapi32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "advapi32.lib")
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_DEPRECATE 1
#endif

void netview_enum(vector<wstring> &hosts, wchar_t *domain);
void net_enum(wchar_t *host, wchar_t *domain);
void ip_enum(wchar_t *host);
void share_enum(wchar_t *host);
void session_enum(wchar_t *host);
void loggedon_enum(wchar_t *host);

#ifdef __MINGW32__
//This option is to handle the Unicode mangling with the wmain error.
//By default wmain's entry point is not found this is a temporary fix.
//https://github.com/coderforlife/mingw-unicode-main
#include "mingw-unicode.c"
#endif
int wmain(int argc, wchar_t * argv[])
{
	FILE *file_of_hosts;
	FILE *outputfile;
	BOOL bReadFromFile = FALSE;
	BOOL bDomainSpecifed = FALSE;
	BOOL bReadFromFileArg = FALSE;
	BOOL bDomainArg = FALSE;
	BOOL bOutputToFile = FALSE;
	wchar_t *domain = NULL;
	wchar_t *host = NULL;
	char *filename;
	char *outputfilename;
	char line[255];
	char tmphost[255];

	vector<wstring> hosts;

	if (argc == 1)
	{
		printf("\nNetviewer Help\n"
			"--------------------------------------------------------------------\n\n"
			"-d domain \t\t: Specifies a domain to pull a list of hosts from\n"
			"\t\t\t  uses current domain if none specifed\n\n"
			"-f filename.txt \t: Speficies a file to pull a list of hosts from\n"
			"-o filename.txt \t: Out to file instead of STDOUT\n"
			);
		printf("\n");
		return 0;
	}

	// Parse cmdline arguments
	for (int nArg=0; nArg < argc; nArg++)
	{
		if (!_wcsicmp(argv[nArg], L"-o"))
		{
			if ((nArg + 1) > (argc - 1) || !_wcsicmp(argv[(nArg + 1)], L"-d") || !_wcsicmp(argv[(nArg + 1)], L"-f"))
			{
				printf("[-] -o used without a file name specified\n");
				return 0;
			}
			else
			{
				const size_t newsize = 255;
				char nstring[newsize];

				#ifdef __MINGW32__
					wcstombs(nstring, argv[(nArg + 1)],newsize);
				#else	
					size_t origsize = wcslen(argv[(nArg + 1)]) + 1;				
					size_t convertedChars = 0;				
					wcstombs_s(&convertedChars, nstring, origsize, argv[(nArg + 1)], _TRUNCATE);				
				#endif

				outputfilename = nstring;

				if((outputfile=freopen(outputfilename, "w" ,stdout))==NULL) {
					printf("Cannot open %s for writing\n",outputfilename);
					exit(1);
				}
			}
		}
		if (!_wcsicmp(argv[nArg], L"-f"))
		{
			bReadFromFileArg = TRUE;
			// file flag initiated, need to check if the file is there
			if ((nArg + 1) > (argc - 1) || !_wcsicmp(argv[(nArg + 1)], L"-d") || !_wcsicmp(argv[(nArg + 1)], L"-o"))
			{
				printf("[-] -f used without a file name specified\n");
				return 0;
			}
			else
			{				
				const size_t newsize = 255;
				char nstring[newsize];
				
				#ifdef __MINGW32__
					wcstombs(nstring, argv[(nArg + 1)],newsize);
				#else	
					size_t origsize = wcslen(argv[(nArg + 1)]) + 1;				
					size_t convertedChars = 0;				
					wcstombs_s(&convertedChars, nstring, origsize, argv[(nArg + 1)], _TRUNCATE);				
				#endif

				filename = nstring;
				bReadFromFile = TRUE;
			}
		}
		// check for domain argument
		if (!_wcsicmp(argv[nArg], L"-d"))
		{
			// domain flag was used
			bDomainArg = TRUE;
			// domain flag specified
			if (((nArg + 1) > (argc - 1)) || !_wcsicmp(argv[(nArg + 1)], L"-f") || !_wcsicmp(argv[(nArg + 1)], L"-o"))
			{
				printf("[*] -d used without domain specifed - using current domain\n");
			}
			else
			{
				bDomainSpecifed = TRUE;
				domain = argv[(nArg + 1)];
				printf("[+] Domain Specified: %ls\n", domain);
			}

		}
	}

	if (bDomainArg && bReadFromFileArg)
	{
		printf("[-] Domain and File specifed, can't do both - exiting...\n\n");
		return 0;
	}

	// pull file into array if read from file done
	// pull domain via NetServerEnum into array if read from domain done
	if(bReadFromFileArg)
	{
		printf("Reading from the file\n");
		
		file_of_hosts = fopen(filename,"r");
		
		if (file_of_hosts == NULL)
		{
			printf("[-] File not found as specifed by -f\n");
		}
		else
		{
			while (fgets(line, sizeof(line)-1,file_of_hosts))
			{
				sscanf(line, "%s\n", tmphost);
				const size_t newsize = 255;
				wchar_t wcstring[newsize];
				
				#ifdef __MINGW32__
					mbstowcs(wcstring,tmphost,newsize);
				#else				
					size_t origsize = strlen(tmphost) + 1;				
					size_t convertedChars = 0;				
					mbstowcs_s(&convertedChars, wcstring, origsize, tmphost, _TRUNCATE);
				#endif							
				
				hosts.push_back(wstring(wcstring));
			}
			fclose(file_of_hosts);
		}
	}

	if(bDomainArg)
	{
		netview_enum(hosts,domain);
	}
	
	printf("[+] Number of hosts: %d\n",hosts.size());

	for (vector<wstring>::iterator it = hosts.begin(); it != hosts.end(); ++it)
	{
		host = const_cast<wchar_t *>(it->c_str());
		wcout << "\n\n[+] Host: " << host << endl;
		net_enum(host,domain);
		ip_enum(host);
		share_enum(host);
		session_enum(host);
		loggedon_enum(host);
	}

	if (bOutputToFile)
	{
		 fclose(outputfile);
	}
	return 0;
}

void netview_enum(vector<wstring> &hosts, wchar_t *domain)
{
	NET_API_STATUS nStatus;
	LPWSTR pszServerName = NULL;
	DWORD dwLevel = 101;
	LPSERVER_INFO_101 pBuf = NULL;
	LPSERVER_INFO_101 pTmpBuf;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	DWORD dwServerType = SV_TYPE_SERVER;
	LPWSTR pszDomainName = domain;
	DWORD dwResumeHandle = 0;


	nStatus = NetServerEnum(pszServerName,
				dwLevel,
				(LPBYTE *) & pBuf,
				dwPrefMaxLen,
				&dwEntriesRead,
				&dwTotalEntries,
				dwServerType,
				pszDomainName,
				&dwResumeHandle);

	if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
	{
		if ((pTmpBuf = pBuf) != NULL)
		{
			for (unsigned int i = 0; i < dwEntriesRead; i++)
			{
				assert(pTmpBuf != NULL);
				if (pTmpBuf == NULL)
				{
					fprintf(stderr, "An access violation has occurred\n");
					break;
				}
				else
				{
					hosts.push_back(wstring(pTmpBuf->sv101_name));
	                pTmpBuf++;
				}
			}
		}
	}

	if (pBuf != NULL)
	{
		NetApiBufferFree(pBuf);
	}
}

void net_enum(wchar_t *host, wchar_t *domain)
{
	NET_API_STATUS nStatus;
	LPWSTR pszServerName = host;
	DWORD dwLevel = 101;
	LPSERVER_INFO_101 pBuf = NULL;
	LPSERVER_INFO_101 pTmpBuf;

	wcout << "\nEnumerating AD Info" << endl;

	nStatus = NetServerGetInfo(pszServerName,
								dwLevel,
								(LPBYTE *) & pBuf
								);

	if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
	{
		if ((pTmpBuf = pBuf) != NULL)
		{
			assert(pTmpBuf != NULL);
			if (pTmpBuf == NULL)
			{
				fprintf(stderr, "An access violation has occurred\n");
				return;
			}
			else
			{
				wprintf(L"[+] %ws - Comment - %s\n", host, pTmpBuf->sv101_comment);
				printf("[+] %s - OS Version - %d.%d\n", (char*)host, (int)pTmpBuf->sv101_version_major, (int)pTmpBuf->sv101_version_minor);
				if (pTmpBuf->sv101_type & SV_TYPE_DOMAIN_CTRL)
				{
					wprintf(L"[+] %ws - Domain Controller\n", host);
				}
				if (pTmpBuf->sv101_type & SV_TYPE_DOMAIN_BAKCTRL)
				{
					wprintf(L"[+] %ws - Backup Domain Controller\n", host);
				}

				if (pTmpBuf->sv101_type & SV_TYPE_SQLSERVER)
				{
					wprintf(L"[+] %ws - MSSQL Server\n", host);
				}
			}
		}
	}
	if (pBuf != NULL)
	{
		NetApiBufferFree(pBuf);
	}
}

void ip_enum(wchar_t *host)
{

	wprintf(L"The hostcoming into the function is: %ls\n",host);

	WSADATA wsaData;
	int iResult;
	int iRetval;
	DWORD dwRetval;
	
	#ifdef __MINGW32__	
	struct addrinfo *result = NULL;
   	struct addrinfo *ptr = NULL;
    	struct addrinfo hints;
	#else
	// This struct call is used for the Visual Studio Compiler and
	// does not work with the MinGW Compiler's libs as of yet.
	ADDRINFOW *result = NULL;
	ADDRINFOW *ptr = NULL;
	ADDRINFOW hints;
	#endif

	LPSOCKADDR sockaddr_ip;
	wchar_t ipstringbuffer[46];
	DWORD ipbufferlength = 46;

	printf("\nEnumerating IP Info\n");
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		wprintf(L"WSAStartup failed: %d\n", iResult);
		return;
	}

	ZeroMemory( &hints, sizeof(hints) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;


	#ifdef __MINGW32__

	char tmphost[255];
	int len = 0;
	len = wcstombs(tmphost, host, sizeof(tmphost));
	dwRetval = getaddrinfo(tmphost, 0, &hints, &result);

	#else
	// The older veriosn of getaddrinfo is used that does not support Unicode.
	dwRetval = GetAddrInfoW(host, 0, &hints, &result);
	#endif
	
	if ( dwRetval != 0 )
	{
		wprintf(L"[-] %ls - IP(s) could not be enumerated\n", host);
		WSACleanup();
		return;
	}
	else
	{
		// parse each address
		for(ptr=result; ptr != NULL ;ptr=ptr->ai_next)
		{
			switch (ptr->ai_family) {
			case AF_INET:
				wprintf(L"[+] %ls - IPv4 Address - ");
				sockaddr_ip = (LPSOCKADDR) ptr->ai_addr;
				ipbufferlength = 46;
				iRetval = WSAAddressToString(sockaddr_ip, (DWORD) ptr->ai_addrlen, NULL, ipstringbuffer, &ipbufferlength);
				if (iRetval)
					wprintf(L"WSAAddressToString failed with %u\n", WSAGetLastError() );
				else
					wprintf(L"%ls\n", ipstringbuffer);
					break;
			case AF_INET6:
				wprintf(L"[+] %ws - IPv6 Address - ");
				sockaddr_ip = (LPSOCKADDR) ptr->ai_addr;
				ipbufferlength = 46;
				iRetval = WSAAddressToString(sockaddr_ip, (DWORD) ptr->ai_addrlen, NULL, ipstringbuffer, &ipbufferlength );
				if (iRetval)
					wprintf(L"WSAAddressToString failed with %u\n", WSAGetLastError() );
				else
					wprintf(L"%ws\n", ipstringbuffer);
					break;
			default:
				wprintf(L"Other %ld\n", ptr->ai_family);
				break;
			}
		}
		#ifdef __MINGW32__
		freeaddrinfo(result);
		#else
		// The older veriosn of freeaddrinfo is used that does not support Unicode.
		FreeAddrInfoW(result);
		#endif		

		WSACleanup();
	}
}

void share_enum(wchar_t *host)
{
	PSHARE_INFO_1 BufPtr,p;
	NET_API_STATUS res;
	DWORD er=0,tr=0,resume=0, t;

	printf("\nEnumerating Share Info\n");
	do
	{
		res = NetShareEnum (host, 1, (LPBYTE *) &BufPtr, MAX_PREFERRED_LENGTH, &er, &tr, &resume);
					
		if(res == ERROR_SUCCESS || res == ERROR_MORE_DATA)
		{
			p=BufPtr;
			for(t=1;t<=er;t++)
			{
				wprintf(L"[+] %ls - Share - %-20s%-30s\n", host, p->shi1_netname, p->shi1_remark);
				// Suggested mod by "Ben" on R362 comments. Need to figure the best way to print type.
				//wprintf(L"[+] %ls - Share - %s - %s - %s\n", host, p->shi1_netname,p->shi1_type, p->shi1_remark);
				p++;
			}
			NetApiBufferFree(BufPtr);
		} 
		else
		{ 
			wprintf(L"[-] %ls - Share - Error: %ld\n", host, res);
		}  
	} while (res==ERROR_MORE_DATA);
}

void session_enum(wchar_t *host)
{
	LPSESSION_INFO_10 pBuf = NULL;
	LPSESSION_INFO_10 pTmpBuf = NULL;
	DWORD dwLevel = 10;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	DWORD dwResumeHandle = 0;
	DWORD i;
	DWORD dwTotalCount = 0;
	LPTSTR pszClientName = NULL;
	LPTSTR pszUserName = NULL;
	NET_API_STATUS nStatus;

	printf("\nEnumerating Session Info\n");

	do
	{
		nStatus = NetSessionEnum(host,
					pszClientName,
					pszUserName,
					dwLevel,
					(LPBYTE*)&pBuf,
					dwPrefMaxLen,
					&dwEntriesRead,
					&dwTotalEntries,
					&dwResumeHandle);

		if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
		{
			if ((pTmpBuf = pBuf) != NULL)
			{
				for (i = 0; (i < dwEntriesRead); i++)
				{
					assert(pTmpBuf != NULL);
					if (pTmpBuf == NULL)
					{
						fprintf(stderr, "An access violation has occurred\n");
						break;
					}

					wprintf(L"[+] %ws - Session - %s from %s - Active: %d - Idle: %d\n",
						host,
						pTmpBuf->sesi10_username,
						pTmpBuf->sesi10_cname,
						pTmpBuf->sesi10_time,
						pTmpBuf->sesi10_idle_time
						);

					pTmpBuf++;
					dwTotalCount++;
				}
			}
		}
		else
		{
			wprintf(L"[-] %ls - Session - Error: %ld\n", host, nStatus);
		}

		if (pBuf != NULL)
		{
			NetApiBufferFree(pBuf);
			pBuf = NULL;
		}
	} while (nStatus == ERROR_MORE_DATA);
   
	if (pBuf != NULL)
	{
		NetApiBufferFree(pBuf);
	}
}

void loggedon_enum(wchar_t *host)
{
	LPWKSTA_USER_INFO_1 pBuf = NULL;
	LPWKSTA_USER_INFO_1 pTmpBuf;
	DWORD dwLevel = 1;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	DWORD dwResumeHandle = 0;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD z;
	DWORD dwTotalCount = 0;
	NET_API_STATUS nStatus;
	
	printf("\nEnumerating Logged-on Users\n");
	do
	{
		nStatus = NetWkstaUserEnum(host,
					dwLevel,
					(LPBYTE*)&pBuf,
					dwPrefMaxLen,
					&dwEntriesRead,
					&dwTotalEntries,
					&dwResumeHandle);

		if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
		{
			if ((pTmpBuf = pBuf) != NULL)
			{
				for (z = 0; (z < dwEntriesRead); z++)
				{
					assert(pTmpBuf != NULL);
					if (pTmpBuf == NULL)
					{
						fprintf(stderr, "An access violation has occurred\n");
						break;
					}
						   
					if (!wcschr((wchar_t*)(pTmpBuf)->wkui1_username, L'$'))
					{
						wprintf(L"[+] %ws - Logged-on - %s\\%s\n", host, pTmpBuf->wkui1_logon_domain, pTmpBuf->wkui1_username);
					}

					pTmpBuf++;
					dwTotalCount++;
				}
			}
			else
			{
				wprintf(L"[-] %ls - Logged-on - Error: %ld\n", host, nStatus);
			}
		}

	} while (nStatus == ERROR_MORE_DATA);

	if (pBuf != NULL)
	{
		NetApiBufferFree(pBuf);
		pBuf = NULL;
	}
}
