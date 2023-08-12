#pragma once
#include <WinSock2.h>
#include <iostream>
#include <string>
#include<map>
#include <deque>
#include <thread>
#include <iostream>



#pragma comment(lib, "ws2_32.lib")

struct Connection {

		SOCKADDR_IN soc_data;
		SOCKET soc;
		std::thread* reciver;

};

namespace SS{

	class SimpleSocket
	{
	private:
		WSAData wsaData;
		
		SOCKADDR_IN socket_data;
		int sizeofaddr;

		std::string node_name;
		bool type;
		int max_connections;
		std::map <std::string, Connection> connected_nodes;

		std::thread* listner;
		std::thread* processor;


		void listener_func();
		void process();
	public:
		
		
		SimpleSocket(std::string name, std::string ip_adres, int port,bool type_of_node, int max, bool need_processor=0);
		int Connect(std::string adres, int port);
	};

}

