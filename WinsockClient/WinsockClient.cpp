// WinsockClient.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "8000"
#define DEFAULT_ADDR "127.0.0.1"

#include <iostream>
#include <string>



int main()
{

	WSADATA wsaData; // служебная структура для хранение информации
	// о реализации Windows Sockets
	

	SOCKET ConnectSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL,	*ptr = NULL, hints;

	char sendbuf[DEFAULT_BUFLEN];
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;

	// старт использования библиотеки сокетов процессом
	// (подгружается Ws2_32.dll)
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	// Если произошла ошибка подгрузки библиотеки
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(DEFAULT_ADDR, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	for (;;) {

		// Attempt to connect to an address until one succeeds
		for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

			// Create a SOCKET for connecting to server
			ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
				ptr->ai_protocol);
			if (ConnectSocket == INVALID_SOCKET) {
				printf("socket failed with error: %ld\n", WSAGetLastError());
				WSACleanup();
				return 1;
			}

			// Connect to server.
			iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (iResult == SOCKET_ERROR) {
				closesocket(ConnectSocket);
				ConnectSocket = INVALID_SOCKET;
				continue;
			}
			break;
		}

		//freeaddrinfo(result);

		if (ConnectSocket == INVALID_SOCKET) {
			printf("Unable to connect to server!\n");
			WSACleanup();
			return 1;
		}
		
		std::cout << "Input dec: ";
		std::cin >> sendbuf;

		// Send an initial buffer
		iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
		if (iResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return 1;
		}

		

		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {

			std::string str;
			str.append(recvbuf, (int)strlen(recvbuf));
			int num = atoi(str.c_str());

			printf("bin: %d\n", num);
		}
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed with error: %d\n", WSAGetLastError());

	}

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}

