#include "SimpleSocket.h"

#pragma warning(disable: 4996)

SS::SimpleSocket::SimpleSocket() {
	initiated = false;

}

/*name: node name
ip_adres: Local IP adress
port : program reserved port
type_of_node : 0 - server, 1 - client
max : max connections to node*/
SS::SimpleSocket::SimpleSocket(std::string name, std::string ip_adres, int port, bool type_of_node, int max, bool need_processor)
{
	initiated = true;
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

		char client_name[256];
		send(newCon, node_name.c_str(), sizeof(node_name.c_str()), NULL);
		recv(newCon, client_name, sizeof(client_name), NULL);
		
		std::string clinm = "trump";

		ConnectionInfo NewCli;
		NewCli.soc = newCon;
		NewCli.soc_data = socket_data;
		NewCli.name = client_name;
		//connected_nodes.insert(std::pair<std::string, ConnectionInfo>(client_name, NewCli));
		node_listl.push_back(NewCli);
		NewCli.reciver = new std::thread(&SS::SimpleSocket::reciver, this, NewCli);
		NewCli.reciver->detach();

		Send("print Yes");

	}

}

void SS::SimpleSocket::process()
{
	while (true) {
		if (processing_queue.size() > 0) {
			ProcessData next_proc= processing_queue[0];
			processing_queue.pop_front();
			if (!(CallBacks.find(next_proc.name) == CallBacks.end())) {
				CallBacks[next_proc.name].exec(next_proc.params, next_proc.sender);
			}
		}
	}
}

/*Connects client to a server 
adres: IP adres
port: external port*/
int SS::SimpleSocket::Connect(std::string adres, int port, bool need_processor)
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
		char host_name[256];

		recv(Connection, host_name, sizeof(host_name), NULL);
		send(Connection, node_name.c_str(), sizeof(node_name.c_str()), NULL);

		ConnectionInfo Host;
		Host.soc = Connection;
		Host.soc_data = socket_data;
		Host.name = host_name;
		//connected_nodes.insert(std::pair<std::string, ConnectionInfo>(host_name, Host));
		node_listl.push_back(Host);

		Host.reciver = new std::thread(&SS::SimpleSocket::reciver, this, Host);
		Host.reciver->detach();


		if (need_processor) {
			processor = new std::thread(&SS::SimpleSocket::process, this);
			processor->detach();
		}
		else {
			process();
		}
	}
	else {
		std::cout << "Error! Not a client." << std::endl;
		return -1;
	}
	return 0;
}

int SS::SimpleSocket::ConnectCommand(std::string name, int(*ptr)(std::string, std::string))
{
	if (initiated) 
		{
		Callback newCB;

		newCB.exec = ptr;

		CallBacks.insert(std::pair<std::string, Callback>(name, newCB));

		return 0;
	}
	else
	{
		std::cout << "Not initiated!" << std::endl;
		return -1;
	}
}

int SS::SimpleSocket::Send(std::string data, std::vector<std::string> clients)
{
	if (initiated){
		if (type) {

			for (int i = 0; i < node_listl.size(); i++) {

				send(node_listl[i].soc, data.c_str(), sizeof(data.c_str()), NULL);

			}

		}
		else
		{
			if (clients.size() == 0) {
				for (int i = 0; i < node_listl.size(); i++) {

					send(node_listl[i].soc, data.c_str(), sizeof(data.c_str()), NULL);

				}
			}
			else
			{
				for (int i = 0; i < node_listl.size(); i++) {

					for (int i = 0; i < clients.size(); i++)
					{
						if (node_listl[i].name == clients[i]) {
							send(node_listl[i].soc, data.c_str(), sizeof(data.c_str()), NULL);
							break;
						}
					}

				}

			}

		}

		return 0;
	}
	else 
	{
		std::cout << "Not initiated!" << std::endl;
		return -1;
	}
}


void SS::SimpleSocket::reciver(ConnectionInfo node_handler)
{
	char data[1024];
	while (true) {
		
		recv(node_handler.soc, data, sizeof(data), NULL);

		bool flag = false;
		int cou = 0;
		std::string func_name="";
		std::string args = "";

		while (data[cou]!='\0')
		{
			if (data[cou] == ' ' and !flag) {
				flag = true;
			}
			else 
			{
				if (flag) {
					args.push_back(data[cou]);
				}
				else
				{
					func_name.push_back(data[cou]);
				}
			}
		}
		ProcessData NewRequest;
		NewRequest.name = func_name;
		NewRequest.params = args;
		NewRequest.sender = node_handler.name;

		processing_queue.push_back(NewRequest);
	}
}