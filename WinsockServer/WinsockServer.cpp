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

#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT "8000"
#define DEFAULT_ADDR "127.0.0.1"

#include <sstream>


int main()
{

	WSADATA wsaData; // служебная структура для хранение информации
    // о реализации Windows Sockets
    // старт использования библиотеки сокетов процессом
    // (подгружается Ws2_32.dll)

	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	// Если произошла ошибка подгрузки библиотеки
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	struct addrinfo *result = NULL; // структура, хранящая информацию
    // об IP-адресе  слущающего сокета

	// Шаблон для инициализации структуры адреса
	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));

	hints.ai_family = AF_INET; // AF_INET определяет, что будет
    // использоваться сеть для работы с сокетом
	hints.ai_socktype = SOCK_STREAM; // Задаем потоковый тип сокета
	hints.ai_protocol = IPPROTO_TCP; // Используем протокол TCP
	hints.ai_flags = AI_PASSIVE; // Сокет будет биндиться на адрес,
    // чтобы принимать входящие соединения

	// Инициализируем структуру, хранящую адрес сокета - addr
	// Наш HTTP-сервер будет висеть на 8000-м порту локалхоста
	iResult = getaddrinfo(DEFAULT_ADDR, DEFAULT_PORT, &hints, &result);
	// Если инициализация структуры адреса завершилась с ошибкой,
	// выведем сообщением об этом и завершим выполнение программы
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Создание сокета
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	// Если создание сокета завершилось с ошибкой, выводим сообщение,
	// освобождаем память, выделенную под структуру addr,
	// выгружаем dll-библиотеку и закрываем программу
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Привязываем сокет к IP-адресу
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	// Если привязать адрес к сокету не удалось, то выводим сообщение
	// об ошибке, освобождаем память, выделенную под структуру addr.
	// и закрываем открытый сокет.
	// Выгружаем DLL-библиотеку из памяти и закрываем программу.
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	
	//freeaddrinfo(result); //Я не знаю почему в примере от МС нужна эта строка

	// Инициализируем слушающий сокет
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	printf("server listen on: %s:%s\n", DEFAULT_ADDR, DEFAULT_PORT);

	for (;;) {

		// Принимаем входящие соединения
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);

		std::stringstream response; // сюда будет записываться ответ клиенту
		std::stringstream response_body; // тело ответа

		if (iResult > 0) {

			printf("Bytes received: %d\n", iResult);

			// Мы знаем фактический размер полученных данных, поэтому ставим метку конца строки
			// В буфере запроса.
			// Мы знаем фактический размер полученных данных, поэтому ставим метку конца строки
			// В буфере запроса.
			recvbuf[iResult] = '\0';
			
			// Данные успешно получены
			// формируем тело ответа (HTML)
			response_body << "<title>Test C++ HTTP Server</title>\n"
				<< "<h1>Test page</h1>\n"
				<< "<p>This is body of the test page...</p>\n"
				<< "<h2>Request headers</h2>\n"
				<< "<pre>" << recvbuf << "</pre>\n"
				<< "<em><small>Test C++ Http Server</small></em>\n";

			// Формируем весь ответ вместе с заголовками
			response << "HTTP/1.1 200 OK\r\n"
				<< "Version: HTTP/1.1\r\n"
				<< "Content-Type: text/html; charset=utf-8\r\n"
				<< "Content-Length: " << response_body.str().length()
				<< "\r\n\r\n"
				<< response_body.str();


			// Echo the buffer back to the sender
			iSendResult = send(ClientSocket, response.str().c_str(), response.str().length(), 0);
			if (iResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				
			}
			printf("Bytes sent: %d\n", iSendResult);
			closesocket(ClientSocket);
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

