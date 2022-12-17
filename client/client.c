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

#define NO_ERROR 0
void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}

char* convert_dns(char dns[]) {
	char *IPbuffer;
	struct hostent *host_entry;
	host_entry = gethostbyname(dns);
	IPbuffer = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));

	return IPbuffer;

}

msgStruct user_data_entry(msgStruct message) {
	printf(
			"Please enter the numbers and math operators in this format: 'operators first_number seconds_ numbers'. \n");
	char c;
	scanf("%c", &c);
	message.operation = c;
	if (message.operation == '=') {
		return message;
	}
	scanf(" %ld %ld", &message.first_number, &message.second_number);
	fflush(stdin);
	return message;
}

void send_message(SOCKET socket_client, msgStruct message, SOCKADDR_IN sad) {
	int iResult = sendto(socket_client, (char*) &message, sizeof(message), 0,
			(SOCKADDR*) &sad, sizeof(sad));
	if (iResult == SOCKET_ERROR) {
		wprintf(L"sendto failed with error: %d\n", WSAGetLastError());
		closesocket((int) socket_client);
		WSACleanup();
		exit(1);
	} else {
		if (message.operation == '=') {
			fflush(stdin);
			int iResult = closesocket(socket_client);
			if (iResult == SOCKET_ERROR)
				wprintf(L"close function falied, with error: %ld\n",
						WSAGetLastError());
			clearwinsock();
			exit(1);
		}
	}

}

void receive_message(SOCKET socket_client, msgStruct message, SOCKADDR_IN sad) {
	int len = sizeof(sad);
	int iResult = recvfrom(socket_client, (char*) &message, sizeof(message), 0,
			(struct sockaddr*) &sad, &len);

	if (iResult > 0) {
		switch (message.operation) {
		case '+':
		case '-':
		case '*':
			printf("Received result from server %s, ip %s: %ld %c %ld = %d\n",
					host->h_name, inet_ntoa(sad.sin_addr), message.first_number,
					message.operation, message.second_number, (int)message.result);
			break;
		case '/':
			printf(
					"Received result from server %s, ip %s: %ld %c %ld = %.2f\n",
					host->h_name, inet_ntoa(sad.sin_addr), message.first_number,
					message.operation, message.second_number, message.result);
			break;
		}
	} else if (iResult == 0)
		closesocket(socket_client);
	else
		printf("recv failed: %d\n", WSAGetLastError());
}

int main(int argc, char *argv[]) {

#if defined WIN32
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif

	int socket_client;
	msgStruct message;
	char *host_input[2];

	//C:\Windows\System32\drivers\etc
	/// SOCKET CREATION
	socket_client = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socket_client == INVALID_SOCKET) { /// CHECK IF THE OUTCOME OF THE CONNECTION WAS SUCCESSFUL
		wprintf(L"the function socket is failed with error: %u\n",
				WSAGetLastError());
		closesocket(socket_client);
		clearwinsock();

		return 1;
	}

	char *token = strtok(argv[1], ":");
	host_input[0] = token;
	host_input[1] = strtok(NULL, "\0");

	host = gethostbyname((char*) host_input[0]);
	if (host == NULL) {
		fprintf(stderr, "gethostbyname() failed.\n");
		exit(EXIT_FAILURE);
	} else {
		struct in_addr *ina = (struct in_addr*) host->h_addr_list[0];
		sad.sin_family = AF_INET;
		sad.sin_addr.s_addr = inet_addr(inet_ntoa(*ina));
		sad.sin_port = htons(atoi(host_input[1]));
	}

	for (;;) {
		///*************************
		/// USER DATA ENTRY
		///*************************
		message = user_data_entry(message);
		///*************************
		// SEND MESSAGES TO THE SERVER
		///*************************
		send_message(socket_client, message, sad);
		///*************************
		///  RECEIVE MESSAGES TO THE SERVER
		///*************************
		receive_message(socket_client, message, sad);
	}
	///***********************
	/// CLOSE CONNECTION
	///***********************

	closesocket(socket_client);
	clearwinsock();
	return 0;
}
