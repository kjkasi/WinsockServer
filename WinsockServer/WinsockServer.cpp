// WinsockServer.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "8000"
#define DEFAULT_ADDR "127.0.0.1"

//#include <sstream>
#include <iostream>
#include <string>
#include <bitset>


int main()
{

	WSADATA wsaData; // служебная структура для хранение информации
    // о реализации Windows Sockets
	
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	// старт использования библиотеки сокетов процессом
	// (подгружается Ws2_32.dll)
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); //The WSAStartup function initiates use of the Winsock DLL by a process
	// Если произошла ошибка подгрузки библиотеки
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	struct addrinfo *result = NULL; // структура, хранящая информацию
    // об IP-адресе  слущающего сокета

	// Шаблон для инициализации структуры адреса
	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints)); //Fills a block of memory with zeros.

	hints.ai_family = AF_INET; // AF_INET определяет, что будет
    // использоваться сеть для работы с сокетом
	hints.ai_socktype = SOCK_STREAM; // Задаем потоковый тип сокета
	hints.ai_protocol = IPPROTO_TCP; // Используем протокол TCP
	hints.ai_flags = AI_PASSIVE; // Сокет будет биндиться на адрес,
    // чтобы принимать входящие соединения

	// Инициализируем структуру, хранящую адрес сокета - addr
	// Наш HTTP-сервер будет висеть на 8000-м порту локалхоста
	iResult = getaddrinfo(DEFAULT_ADDR, DEFAULT_PORT, &hints, &result); //The getaddrinfo function provides protocol-independent translation from an ANSI host name to an address.
	// Если инициализация структуры адреса завершилась с ошибкой,
	// выведем сообщением об этом и завершим выполнение программы
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup(); //The WSACleanup function terminates use of the Winsock 2 DLL (Ws2_32.dll).
		return 1;
	}

	// Создание сокета
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol); //The socket function creates a socket that is bound to a specific transport service provider.
	// Если создание сокета завершилось с ошибкой, выводим сообщение,
	// освобождаем память, выделенную под структуру addr,
	// выгружаем dll-библиотеку и закрываем программу
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result); //The freeaddrinfo function frees address information that the getaddrinfo function dynamically allocates in addrinfo structures.
		WSACleanup();
		return 1;
	}

	// Привязываем сокет к IP-адресу
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen); //The bind function associates a local address with a socket.
	// Если привязать адрес к сокету не удалось, то выводим сообщение
	// об ошибке, освобождаем память, выделенную под структуру addr.
	// и закрываем открытый сокет.
	// Выгружаем DLL-библиотеку из памяти и закрываем программу.
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket); //The closesocket function closes an existing socket.
		WSACleanup();
		return 1;
	}

	
	//freeaddrinfo(result); //Я не знаю почему в примере от МС нужна эта строка

	// Инициализируем слушающий сокет
	iResult = listen(ListenSocket, SOMAXCONN); //The listen function places a socket in a state in which it is listening for an incoming connection.
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	printf("server listen on: %s:%s\n", DEFAULT_ADDR, DEFAULT_PORT);

	//Запускаем бесконечный цикл
	for (;;) {

		// Принимаем входящие соединения
		ClientSocket = accept(ListenSocket, NULL, NULL); //The accept function permits an incoming connection attempt on a socket.
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0); //The recv function receives data from a connected socket or a bound connectionless socket.

		if (iResult > 0) {
			std::string str = ""; //Объявляем пустую строку
			str.append(recvbuf, (int)strlen(recvbuf)); //Добавляем в строку информацию из буфера сокета, размером с буфер
			int num = atoi(str.c_str()); //Преобразуем строку в целое число
			std::string result = std::bitset<8>(num).to_string(); //переводим число в двоичную систему счисления
			printf("dec: %d\n", num);
			printf("bin: %s\n", result.c_str());

			// Echo the buffer back to the sender
			iSendResult = send(ClientSocket, result.c_str(), (int)strlen(result.c_str()), 0); //отправляем клиенту
			if (iResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
			}
		}
		else if (iResult == 0)
			// соединение закрыто клиентом
			printf("Connection closing...\n");
		else {
			// ошибка получения данных
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}

	}
	
	closesocket(ClientSocket);
	WSACleanup();

    return 0;
}

