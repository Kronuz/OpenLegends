#pragma once

#include <winsock2.h>

typedef int CALLBACK NETRD(SOCKET s);

SOCKET InitServer(int port, NETRD *pFSD, NETRD *pFSR);
int EndServer();

SOCKET Connect(LPCSTR szHostname, int port, NETRD *pFCD, NETRD *pFCR);
int Disconnect();

// returns true if a connection is acctually active:
bool Connected();

int Send(SOCKET socket, LPCSTR buff, int size);
int Recv(SOCKET socket, LPSTR buff, int size);
