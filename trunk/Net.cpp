/* QuestDesigner - Open Zelda's Project
   Copyright (C) 2003. Kronuz (Germán Méndez Bravo)
   Copyright (C) 2001-2003. Open Zelda's Project
 
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"
#include "Net.h"
#pragma comment ( lib, "Ws2_32.lib" )

#define TIMEOUT_VALUE 500
static HANDLE g_hServerThread = NULL;
static HANDLE g_hClientThread = NULL;
static CRITICAL_SECTION g_SocketCritical;
static bool g_bConnected = false;
static int g_nServer = -1;
static int g_nClient = -1;

NETRD *ServerDispatch = NULL;
NETRD *ServerRequest = NULL;
NETRD *ClientDispatch = NULL;
NETRD *ClientRequest = NULL;

bool Connected()
{
	return g_bConnected;
}
int SocketEnd()
{
	WSACleanup();
	WSACleanup();
	DeleteCriticalSection(&g_SocketCritical);
	return 1;
}

int SocketInit()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( 2, 2 );

	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		// Tell the user that we could not find a usable
		// WinSock DLL. 
		return 0;
	}

	// Confirm that the WinSock DLL supports 2.2.
	// Note that if the DLL supports versions greater
	// than 2.2 in addition to 2.2, it will still return
	// 2.2 in wVersion since that is the version we
	// requested.

	if( LOBYTE( wsaData.wVersion ) != 2 ||
		HIBYTE( wsaData.wVersion ) != 2 ) {
			// Tell the user that we could not find a usable
			// WinSock DLL.
			WSACleanup( );
			return 0; 
		}

	// The WinSock DLL is acceptable. Proceed.
	InitializeCriticalSection(&g_SocketCritical);
	return 1;
}

int Recv(SOCKET socket, LPSTR buff, int size)
{
	int len = SOCKET_ERROR;
	while(g_bConnected) {
		if((len = recv(socket, buff, size, 0)) == SOCKET_ERROR) {
			int err = WSAGetLastError();
			if(err!=WSAETIMEDOUT && err!=TRY_AGAIN) break;
		} else break;
	}
	if(len == 0 || len == SOCKET_ERROR) g_bConnected = false;
	return len;
}

int Send(SOCKET socket, LPCSTR buff, int size)
{
	int len = SOCKET_ERROR;
	EnterCriticalSection(&g_SocketCritical);
	while(g_bConnected) {
		if((len=send(socket, buff, size, 0)) == SOCKET_ERROR) {
			int err = WSAGetLastError();
			if(err!=WSAETIMEDOUT && err!=TRY_AGAIN) break;
		} else break;
	} 
	LeaveCriticalSection(&g_SocketCritical);
	return len;
}

int Listen(SOCKET s)
{
	int ret = SOCKET_ERROR;
	while(g_nServer>0) { 
		if((ret=listen(s, 20)) == SOCKET_ERROR) {
			int err = WSAGetLastError();
			if(err!=WSAETIMEDOUT && err!=TRY_AGAIN) break;
		} else break;
	}
	return ret;
}

bool GetIpAddress(LPCSTR szHostname, sockaddr_in *pDest)
{
	HOSTENT *lpHost = gethostbyname(szHostname);
	if(lpHost == NULL) return false;
	if(lpHost->h_addr_list[0] == NULL) return false;

	memcpy(&(pDest->sin_addr), lpHost->h_addr_list[0], lpHost->h_length);
	return true;
}

DWORD WINAPI ThreadClnt2( LPVOID lpParameter )
{
	SOCKET ns = (SOCKET)lpParameter;
	int ret = ClientRequest(ns);
	g_bConnected = false;
	shutdown(ns, SD_SEND);
	return ret;
}

DWORD WINAPI ThreadClnt( LPVOID lpParameter )
{
	SOCKET ns = (SOCKET)lpParameter;
	HANDLE hThreadClnt2 = NULL;

	DWORD dwThreadID = 0;
	// create a thread to handle the requests to the client:
	hThreadClnt2 = CreateThread(NULL, 0, ThreadClnt2, (void*)ns, 0, &dwThreadID);
	if(hThreadClnt2 == NULL) {
		g_bConnected = false;
		closesocket(ns);
		return 1;
	}

	int ret = ClientDispatch(ns);
	g_bConnected = false;
	shutdown(ns, SD_RECEIVE);

	// wait for the request thread to finish:
	if(WaitForSingleObject(hThreadClnt2, 1000) == WAIT_TIMEOUT) {
		MessageBeep((UINT)-1);
		TerminateThread(hThreadClnt2, 1);
		CONSOLE_DEBUG("Thread killed at: %s (%d)", __FILE__, __LINE__);
	}
	ASSERT(WaitForSingleObject(hThreadClnt2, 2000) == WAIT_OBJECT_0);
	CloseHandle(hThreadClnt2);
	hThreadClnt2 = NULL;

	closesocket(ns);
	g_nClient = -1;
	return ret;
}

SOCKET Connect(LPCSTR szHostname, int port, NETRD *pFCD, NETRD *pFCR)
{
	if(g_nServer != -1 || g_nClient != -1) return INVALID_SOCKET;

	ASSERT(pFCD && pFCR);
	ClientDispatch = pFCD;
	ClientRequest = pFCR;

	ASSERT(port != 0);
	if(port==0) return INVALID_SOCKET;

	// Initialize the socket library:
	if(!SocketInit()) return INVALID_SOCKET;

	SOCKET s;
	sockaddr_in address;

	// Fill out the server socket's address information.
	address.sin_family = AF_INET;
    address.sin_port = htons((short)port);
	if(!GetIpAddress(szHostname, &address)) return INVALID_SOCKET;
	int addrlen = sizeof(sockaddr_in);

	// Create a socket:
	if((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) return INVALID_SOCKET;

	if(connect(s, (sockaddr*)&address, addrlen) == SOCKET_ERROR) {
		closesocket(s);
		return INVALID_SOCKET;
	}
	g_nClient = 0;
	g_bConnected = true;
	DWORD dwThreadID = 0;
	g_hClientThread = CreateThread(NULL, 0, ThreadClnt, (void*)s, 0, &dwThreadID);
	if(g_hClientThread == NULL) {
		g_bConnected = false;
		closesocket(s);
		g_nClient = -1;
		return INVALID_SOCKET;
	}

	return s;
}

SOCKET Accept(SOCKET s)
{
	SOCKET ns;
	sockaddr_in address;
	int addrlen = sizeof(sockaddr_in);

	while(g_nServer>0) { 
		if((ns = accept(s, (sockaddr*)&address, &addrlen)) == INVALID_SOCKET) {
			int err = WSAGetLastError();
			if(err!=WSAETIMEDOUT && err!=TRY_AGAIN) break;
		} else break;
	}
	return ns;
}

DWORD WINAPI ThreadServ2( LPVOID lpParameter )
{
	SOCKET ns = (SOCKET)lpParameter;
	int ret = ServerRequest(ns);
	g_bConnected = false;
	shutdown(ns, SD_SEND);
	return ret;
}
DWORD WINAPI ThreadServ( LPVOID lpParameter )
{
	SOCKET ns;
	SOCKET s = (SOCKET)lpParameter;
	HANDLE hThreadServ2 = NULL;

	int ret = 1;
	g_nServer = 1;
	while(g_nServer>0) {
		if(Listen(s) != SOCKET_ERROR) {
			if((ns = Accept(s)) != INVALID_SOCKET) {
				g_bConnected = true;
				DWORD dwThreadID = 0;
				// create a thread to handle the requests to the client:
				hThreadServ2 = CreateThread(NULL, 0, ThreadServ2, (void*)ns, 0, &dwThreadID);
				if(hThreadServ2 == NULL) {
					g_bConnected = false;
					closesocket(ns);
					continue;
				}
				// dispatch the client's requests:
				ret = ServerDispatch(ns);
				g_bConnected = false;
				shutdown(ns, SD_RECEIVE);

				// wait for the request thread to finish:
				if(WaitForSingleObject(hThreadServ2, 1000) == WAIT_TIMEOUT) {
					MessageBeep((UINT)-1);
					TerminateThread(hThreadServ2, 1);
					CONSOLE_DEBUG("Thread killed at: %s (%d)", __FILE__, __LINE__);
				}
				ASSERT(WaitForSingleObject(hThreadServ2, 2000) == WAIT_OBJECT_0);
				CloseHandle(hThreadServ2);
				hThreadServ2 = NULL;
				// close the socket wuth the client:
				closesocket(ns);
			}
		}
	}
	closesocket(s);
	g_nServer = -1;
	return ret;
}

int Disconnect()
{
	g_bConnected = false;

	if(g_nClient != -1) {
		g_nClient = 0;

		SocketEnd();
		if(WaitForSingleObject(g_hClientThread, 5000) == WAIT_TIMEOUT) {
			MessageBeep((UINT)-1);
			TerminateThread(g_hClientThread, 1);
			CONSOLE_DEBUG("Thread killed at: %s (%d)", __FILE__, __LINE__);
		}
		ASSERT(WaitForSingleObject(g_hClientThread, 2000) == WAIT_OBJECT_0);
		CloseHandle(g_hClientThread);
		g_hClientThread = NULL;
		g_nClient = -1;
	}
	return 1;
}

int EndServer()
{
	if(g_nServer == -1) return 0;

	g_nServer = 0;
	g_bConnected = false;

	SocketEnd();
	if(WaitForSingleObject(g_hServerThread, 5000) == WAIT_TIMEOUT) {
		MessageBeep((UINT)-1);
		TerminateThread(g_hServerThread, 1);
		CONSOLE_DEBUG("Thread killed at: %s (%d)", __FILE__, __LINE__);
	}
	ASSERT(WaitForSingleObject(g_hServerThread, 2000) == WAIT_OBJECT_0);
	CloseHandle(g_hServerThread);
	g_hServerThread = NULL;
	g_nServer = -1;
	return 1;
}

SOCKET InitServer(int port, NETRD *pFSD, NETRD *pFSR)
{
	if(g_nServer != -1 || g_nClient != -1) return INVALID_SOCKET;

	ASSERT(pFSD && pFSR);
	ServerDispatch = pFSD;
	ServerRequest = pFSR;

	ASSERT(port != 0);
	if(port==0) return INVALID_SOCKET;

	SOCKET s;
	sockaddr_in address;

	address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons((short)port);

	// Initialize the socket library:
	if(!SocketInit()) return INVALID_SOCKET;
	
	// Create a socket:
	if((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) return INVALID_SOCKET;

	int err;
	// allow the socket to bind a used port:
	int bflag = 1;
    err = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (LPCSTR)&bflag, sizeof(bflag));
	if(err != NO_ERROR) {
		closesocket(s);
		return INVALID_SOCKET;
	}

	// Set the send/receive timeout for the socket:
	int timeout = TIMEOUT_VALUE;
	err = setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (LPCSTR)&timeout, sizeof(timeout));
	if(err != NO_ERROR) {
		closesocket(s);
		return INVALID_SOCKET;
	}
	err = setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (LPCSTR)&timeout, sizeof(timeout));
	if(err != NO_ERROR) {
		closesocket(s);
		return INVALID_SOCKET;
	}

	// Bind the socket to the specified port:
	if(bind(s,(struct sockaddr *)&address, sizeof(sockaddr_in)) == SOCKET_ERROR) {
		closesocket(s);
		return INVALID_SOCKET;
	}

	ASSERT(g_hServerThread == NULL);

	g_nServer = 0;
	DWORD dwThreadID = 0;
	g_hServerThread = CreateThread(NULL, 0, ThreadServ, (void*)s, 0, &dwThreadID);
	if(g_hServerThread == NULL) {
		closesocket(s);
		g_nServer = -1;
		return INVALID_SOCKET;
	}

	return s;
}
// must receive a buffer of at least one byte:
bool RecvLine(SOCKET s, LPSTR buff, int size)
{
	static char buffer[2001];
	static char *buffaux = buffer;
	static int nReceived = 0;
	static char *aux = NULL;

	while(true) {
		if(!aux){
			if(sizeof(buffer) - nReceived - 1 <= 0) {
				buffaux = buffer;
				nReceived = 0;
			}
			int len = ::Recv(s, buffaux, sizeof(buffer) - nReceived - 1);
			if(len == SOCKET_ERROR || len == 0) return false;

			nReceived += len;
			*(buffaux += len) = '\0';
			aux = buffer;
		} else {
			char *aux2;
			if( (aux2 = strchr(aux, '\n')) ) {
				*aux2 = '\0';
				while(*aux && *aux < ' ') aux++;
				char *aux3 = aux2;
				while(*aux3 < ' ' && aux3 != aux) *aux3-- = '\0';
				if(*aux) strncpy(buff, aux, size);
				else *buff = '\0';
				aux = aux2 + 1;
				return true;
			} else {
				if(aux > buffer) {
					buffaux = buffer;
					while(*aux) *buffaux++ = *aux++;
					*buffaux = '\0';
					nReceived = strlen(buffaux);
				}
				aux = NULL;
			}
		}
	}
	return true;
}
