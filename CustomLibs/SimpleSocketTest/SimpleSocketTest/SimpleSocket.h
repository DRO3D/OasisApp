#pragma once
#include <WinSock2.h>
#include <iostream>
#include <string>
#include<map>
#include <thread>
#include <iostream>
#include <vector>
#include "SimpleLogger.h"

#include <Iphlpapi.h>
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
			std::string name;
			UINT msg_id;
			bool pinger_flag;
			std::string IP_adr;
			std::string MAC_adr;
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
			WSAData wsa_data;
		
			SOCKADDR_IN socket_data;
			SOCKET listen_soc;
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

			std::map <std::string, Callback>* callbacks;

			void ListenerFunc();
			void Process();
			void Reciver(std::string handler_name);

			
		public:
		
			
			/// <summary>
			/// Empty initiator, cant be used in network connections
			/// </summary>
			SimpleSocket();
			/// <summary>
			/// Socket initiator
			/// </summary>
			/// <param name="name">: Name of node(MUST be unique)</param>
			/// <param name="adapter_info">: Network adapter, used for connection</param>
			/// <param name="port">: Connection port</param>
			/// <param name="type_of_node">: Declare type: Server or client</param>
			SimpleSocket(std::string name, SS::Adapter adapter_info, int port,NodeType type_of_node);
			/// <summary>
			/// Start network node
			/// </summary>
			/// <param name="need_processor"> Declare thread behaviour</param>
			/// <returns>-1 if already started</returns>
			int Start(ProcessingType need_processor = Main);
			/// <summary>
			/// Connect client to the server
			/// </summary>
			/// <param name="adres">: server IP adress</param>
			/// <param name="port">: server port</param>
			/// <param name="retrys">: amount of retrys if failed</param>
			/// <returns> -1 if not a client, -2 if not initiated, error code if network error </returns>
			int Connect(std::string adres, int port, int retrys=1);
			/// <summary>
			/// Stop processing and close connection
			/// </summary>
			/// <param name="NodeName">: If server, name of kicking node</param>
			/// <param name="is_external">: Is need to send disconnection signal</param>
			/// <returns></returns>	
			int Disconnect(std::string NodeName="", bool is_external=true);
			/// <summary>
			/// Connects external command to text command
			/// </summary>
			/// <param name="name">: name of executioner (called, when recived name of command)</param>
			/// <param name="ptr">: pointer to a function</param>
			/// <returns>-1 if not initiated</returns>
			int ConnectCommand(std::string name, int (*ptr)(std::string, std::string));
			/// <summary>
			/// Send text to another node
			/// </summary>
			/// <param name="data">: String, which need to be send</param>
			/// <param name="clients">: vector of client names, who recieve this string</param>
			/// <returns>-1 if not initiated, -2 if not connected, -3 if not started</returns>
			int Send(std::string data, std::vector<std::string> clients= std::vector<std::string>());
			/// <summary>
			/// Send ping packet to a node
			/// </summary>
			/// <param name="node">:Name of the pingable node</param>
			/// <returns>time in msec</returns>
			int Ping(std::string node);
			/// <summary>
			/// Send WOL packet to the node
			/// </summary>
			/// <param name="IP_adr">IP adress of node</param>
			/// <param name="MAC_adr">MAC adress of node</param>
			/// <param name="port">Port to recieve a packet (9-default)</param>
			/// <returns>-1 Socket error, -2 not initiated</returns>
			int EnableNode(std::string IP_adr, std::string MAC_adr,int port =9);
			/// <summary>
			/// Send WOL to several nodes by name
			/// </summary>
			/// <param name="Names">Vector of names to wake up nodes</param>
			/// <returns></returns>
			int WOLbyName(std::vector<std::string> Names=std::vector<std::string>());
			/// <summary>
			/// Returns node info by name
			/// </summary>
			/// <param name="Name">Name of node</param>
			/// <returns></returns>
			ConnectionInfo* GetNodeByName(std::string Name);
			/// <summary>
			/// Returns know nodes
			/// </summary>
			/// <returns>vector of nodes</returns>
			std::vector<std::string>GetNodeList();

			~SimpleSocket();

			
		};
	/// <summary>
	/// Returns all network adapters of the device
	/// </summary>
	/// <returns>vector of adaters</returns>
	std::vector<SS::Adapter> GetAdapterList();
}

