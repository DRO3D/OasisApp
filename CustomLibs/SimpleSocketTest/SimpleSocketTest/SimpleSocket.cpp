#include "SimpleSocket.h"

#pragma warning(disable: 4996)

/*name: node name
ip_adres: Local IP adress
port : program reserved port
type_of_node : 0 - server, 1 - client
max : max connections to node*/
SS::SimpleSocket::SimpleSocket(std::string name, std::string ip_adres, int port, bool type_of_node, int max, bool need_processor = 0)
{
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error! Winsoc not initiated. (" << WSAGetLastError() << ")" << std::endl;
		exit(1);
	}

	
	socket_data.sin_addr.s_addr = inet_addr(ip_adres.c_str());
	socket_data.sin_port = htons(port);
	socket_data.sin_family = AF_INET;
	sizeofaddr = sizeof(socket_data);
	node_name = name;
	max_connections = max;
	type = type_of_node;

	if (type) {

	}
	else
	{
		
		listner = new std::thread(&SS::SimpleSocket::listener_func,this);
		listner->detach();
		if (need_processor) {
			processor = new std::thread(&SS::SimpleSocket::process, this);
			processor->detach();
		}
		else {
			process();
		}
		
	}
	
}


void SS::SimpleSocket::listener_func()
{
	std::cout << "Listener started."<<std::endl;
	SOCKADDR_IN connection_soc=socket_data;
	int sizeof_new_con=sizeofaddr;
	SOCKET slistener = socket(AF_INET, SOCK_STREAM, NULL);
	bind(slistener, (SOCKADDR*)&socket_data, sizeofaddr);
	listen(slistener, max_connections);
	SOCKET newCon = accept(slistener, (SOCKADDR*)&connection_soc, &sizeof_new_con);
	if (newCon == 0) {
		std::cout << "Error! Client connection failature. (" << WSAGetLastError() << ")" << std::endl;
	}
	else {
		std::cout << "Client Connected!" << std::endl;
		
	}

}

void SS::SimpleSocket::process()
{
	while (true) {

	}
}
/*Connects client to a server 
adres: IP adres
port: external port*/
int SS::SimpleSocket::Connect(std::string adres, int port)
{
	if (type) {
		
		socket_data.sin_addr.s_addr = inet_addr(adres.c_str());
		socket_data.sin_port = htons(port);
		socket_data.sin_family = AF_INET;
		sizeofaddr = sizeof(socket_data);

		SOCKET Connection = socket(AF_INET, SOCK_STREAM, NULL);
		
		if (connect(Connection, (SOCKADDR*)&socket_data, sizeofaddr) != 0) {
			std::cout << "Error! Client connection failature. ("<< WSAGetLastError()<<")" << std::endl;
			return -2;
		}
		std::cout << "Connected!"<<std::endl;
	}
	else {
		std::cout << "Error! Not a client." << std::endl;
		return -1;
	}
	return 0;
}
