//example mac: b1:27:eb:b4:a0:9e
//example hex mac: 0xb1, 0x27, 0xeb, 0xb4, 0xa0, 0x9e
//put example max in place of #define MAC, make sure to end with a , like shown below
//also update IP, and PORT if you need to

//#include<WinSock2.h>
//#include <stdio.h>
//#include <stdlib.h>
#include <string>
#include <iostream>


#define IP	("192.168.0.242")				// IP to wake
#define PORT	(9)					// wake listen port
#define MAC	0x30, 0x9c, 0x23, 0xae, 0x33, 0x66,	// max address, have a ',' on the end

#pragma warning(disable: 4996)

int main(int argc, char** argv)
{
	/*struct sockaddr_in si;
	SOCKET s;
	WSADATA wsa;
	int status;*/

	//if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
	//	//fprintf(stderr, "Winsock boot failed : %d\n", WSAGetLastError());
	//	return 1;
	//}

	//puts("Winsock startup");

	//if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR) {
	//	//fprintf(stderr, "Failed to create socket : %d\n", WSAGetLastError());
	//	WSACleanup();
	//	return 1;
	//}

	////puts("Winsock socket creation");

	//memset(&si, 0, sizeof(struct sockaddr_in));
	//si.sin_family = AF_INET;
	//si.sin_port = htons(PORT);
	//si.sin_addr.s_addr = inet_addr(IP);

	//puts("Winsock setup");

	char data[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, MAC MAC MAC MAC MAC MAC MAC MAC MAC MAC MAC MAC MAC MAC MAC MAC };

	std::cout << data << std::endl;

	//printf("Size of data: %Iu\n", strlen(data));
	//puts("Data gen");

	//status = sendto(s, data, strlen(data) - 1, 0, (struct sockaddr*)&si, sizeof(si));
	//if (status == SOCKET_ERROR) {
	//	//fprintf(stderr, "Failed to send message : %d\n", WSAGetLastError());
	//	closesocket(s);
	//	WSACleanup();
	//	return 1;
	//}

	////fprintf(stdout, "Sent WOL!\n");

	//closesocket(s);
	//WSACleanup();

	return 0;
}