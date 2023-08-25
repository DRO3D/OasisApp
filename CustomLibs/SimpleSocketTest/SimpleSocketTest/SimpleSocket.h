#pragma once
#include <WinSock2.h>
#include <iostream>
#include <string>
#include<map>
#include <deque>
#include <thread>
#include <iostream>
#include <vector>
#include "SimpleLogger.h"

#include <stdio.h>
#include <Windows.h>
#include <Iphlpapi.h>
#include <Assert.h>
#pragma comment(lib, "iphlpapi.lib")

#define BACKLOG_THREADS 5

#pragma comment(lib, "ws2_32.lib")



namespace SS{
	enum NodeType {
		Server = 0,
		Client = 1
	};
	enum ProcessingType {
		Main = 0,
		Background = 1
	};
	struct ConnectionInfo {

			SOCKADDR_IN soc_data;
			SOCKET soc;
			std::thread* reciver;
			std::thread::native_handle_type n_handl;
			std::string name;
			UINT msg_id;
			bool pinger_flag;
			std::string IP_adr;
			std::string MAC;
			bool is_connected;
	};

	struct Adapter {
		std::string MAC_adr;
		std::string IP_adr;
		std::string name;
	};

	struct ProcessData {

		std::string name;
		std::string params;
		std::string sender;

	};
	struct Callback {

		int (*exec)(std::string, std::string);

	};
	class SimpleSocket
	{
		private:
			WSAData wsaData;
		
			SOCKADDR_IN socket_data;
			SOCKET listenSoc;
			int sizeofaddr;

			bool is_started;
			bool is_connected;
			bool is_initiated;
			std::string node_name;
			bool type;
			int max_connections;
			//std::map <std::string, ConnectionInfo>* connected_nodes;
			std::vector <ConnectionInfo>* node_list;
			std::deque <ProcessData>* processing_queue;
			std::string node_ip;
			std::string node_mac;

			

			std::thread* listner;
			std::thread* processor;

			std::map <std::string, Callback>* CallBacks;

			void listener_func();
			void process();
			void reciver(std::string handler_name);

			
		public:
		
			std::vector<SS::Adapter> GetAdapterList();
			//TODO: New Constructor(Adapter based)
			SimpleSocket();
			SimpleSocket(std::string name, std::string ip_adres, int port,NodeType type_of_node);
			int Start(ProcessingType need_processor = Main);
			int Connect(std::string adres, int port, int retrys=1);
			int Disconnect(std::string NodeName="", bool is_external=true);
			int ConnectCommand(std::string name, int (*ptr)(std::string, std::string));
			int Send(std::string data, std::vector<std::string> clients= std::vector<std::string>());
			float Ping(std::string node);
			int EnableNode(std::string IP_adr, std::string MAC_adr,int port =9);
			int WOLbyName(std::vector<std::string> Names=std::vector<std::string>());
			ConnectionInfo* GetUsrByName(std::string Name);
			~SimpleSocket();

			std::vector<std::string>GetUserList();
		};

}

