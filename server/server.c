#if defined WIN32
#include <winsock.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif

#include <stdio.h>
#include "header.h"
//...

#define NO_ERROR 0
void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}

float add(int a, int b) {
	return a + b;
}

float mult(int a, int b) {
	return a * b;
}

float sub(int a, int b) {
	return a - b;
}

float division(int a, int b) {
	return (float) a / (float) b;
}

SOCKADDR_IN receive_message(SOCKET socket_server, msgStruct *message,
		SOCKADDR_IN cad) {
	int len = sizeof(cad);
	int iResult = recvfrom(socket_server, (char*) message, sizeof(*message), 0,
			(struct sockaddr*) &cad, &len);
	if (iResult > 0) {
		switch (message->operation) {
		case '+':
			message->result = add(message->first_number,
					message->second_number);
			break;
		case '-':
			message->result = sub(message->first_number,
					message->second_number);

			break;
		case '*':
			message->result = mult(message->first_number,
					message->second_number);

			break;
		case '/':
			message->result = division(message->first_number,
					message->second_number);
			break;
		}
	} else if (iResult == 0)
		closesocket(socket_server);
	else
		printf("recv failed: %d\n", WSAGetLastError());
	return cad;
}

void send_message(SOCKET socket_server, msgStruct message, SOCKADDR_IN cad) {
	int iResult = sendto(socket_server, (char*) &message, sizeof(message), 0,
			(SOCKADDR*) &cad, sizeof(cad));
	if (iResult == SOCKET_ERROR) {
		wprintf(L"sendto failed with error: %d\n", WSAGetLastError());
		closesocket((int) socket_server);
		WSACleanup();
		exit(1);
	}
}

int main(int argc, char *argv[]) {
	//...
#if defined WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif
	///***********************
	///***********************

	int iResult = 0; /// VARIABLE USED TO CHECK THE RETURN VALUES OF THE FUNCTIONS
	int socket_server;
	struct hostent *remoteHost;
	msgStruct message;
	memset(&sad, 0, sizeof(sad));
	memset(&cad, 0, sizeof(cad));
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr(ADDRESS_SERVER);
	sad.sin_port = htons((int) PORT_WELCOME_SERVER);

	/// SOCKET CREATION
	socket_server = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socket_server == INVALID_SOCKET) { /// CHECK IF THE OUTCOME OF THE CONNECTION WAS SUCCESSFUL
		wprintf(L"the function socket is failed with error: %u\n",
				WSAGetLastError());
		closesocket(socket_server);
		clearwinsock();
		return 1;
	}

	iResult = bind(socket_server, (struct sockaddr*) &sad, sizeof(sad));
	if (iResult == SOCKET_ERROR) {
		wprintf(L"the function bind is failed with error: %u\n",
				WSAGetLastError());
		closesocket(socket_server);
		clearwinsock();
		return 1;
	}

	for (;;) {

		cad = receive_message(socket_server, &message, cad);
		remoteHost = gethostbyaddr((char*) &cad.sin_addr, 4, AF_INET);
		printf("Operation request '%c %ld %ld' from client %s, ip %s\n",
				message.operation, message.first_number, message.second_number,
				remoteHost->h_name, inet_ntoa(cad.sin_addr));
		if (message.operation != '=') {
			send_message(socket_server, message, cad);
		}

	}
	///***********************
	/// CLOSE CONNECTION
	///***********************

	iResult = closesocket(socket_server);
	if (iResult == SOCKET_ERROR)
		wprintf(L"close function falied, with error: %ld\n", WSAGetLastError());
	clearwinsock();
	return 0;
} // main end
