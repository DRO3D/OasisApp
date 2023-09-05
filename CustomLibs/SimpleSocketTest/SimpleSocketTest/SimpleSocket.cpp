#include "SimpleSocket.h"

#pragma warning(disable: 4996)


/// <summary>
/// Empty initiator, cant be used in network connections
/// </summary>
SS::SimpleSocket::SimpleSocket() {
	is_initiated = false;
	is_started = false;
	is_connected = false;

	callbacks = nullptr;
	node_list = nullptr;
	processing_queue = nullptr;

	processor = nullptr;
	listner = nullptr;

	listen_soc = SOCKET();


	max_connections = -1;
	sizeofaddr = -1;
	type = -1;
}

/// <summary>
/// Socket initiator
/// </summary>
/// <param name="name">: Name of node(MUST be unique)</param>
/// <param name="adapter_info">: Network adapter, used for connection</param>
/// <param name="port">: Connection port</param>
/// <param name="type_of_node">: Declare type: Server or client</param>
SS::SimpleSocket::SimpleSocket(std::string name, SS::Adapter adapter_info, int port, NodeType type_of_node)
{
	
	is_initiated = false;
	is_started = false;
	is_connected = false;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsa_data) != 0) {
		SS::Log("Winsoc not initiated. Code(" + std::to_string(WSAGetLastError()) + ")", SS::Crit);
		exit(1);
	}
	is_initiated = true;
	callbacks = new std::map <std::string, Callback>;
	node_list = new std::vector <ConnectionInfo>;
	processing_queue= new std::deque <ProcessData>;
	processor = nullptr;
	listner = nullptr;
	
	node_ip = adapter_info.IP_adr;
	node_mac = adapter_info.MAC_adr;

	socket_data.sin_addr.s_addr = inet_addr(node_ip.c_str());
	socket_data.sin_port = htons(port);
	socket_data.sin_family = AF_INET;
	sizeofaddr = sizeof(socket_data);
	node_name = name;
	max_connections = BACKLOG_THREADS;
	type = type_of_node;

}


/// <summary>
/// Start processing of node
/// </summary>
/// <param name="need_processor"> Declare thread behaviour</param>
/// <returns>-1 if already started</returns>
int SS::SimpleSocket::Start(ProcessingType need_processor) 
{
	
	if (!is_started){
		is_started = true;
		if (type) {
			SS::Log("Client started", SS::Inf);
			if (need_processor) {
				processor = new std::thread(&SS::SimpleSocket::Process, this);
				processor->detach();
			}
			else {
				Process();
			}
		}
		else
		{
			SS::Log("Server started", SS::Inf);
			listner = new std::thread(&SS::SimpleSocket::ListenerFunc, this);
			listner->detach();
			if (need_processor) {
				processor = new std::thread(&SS::SimpleSocket::Process, this);
				processor->detach();
			}
			else {
				Process();
			}
		}
	}
	else
	{
		return -1;
	}
	return 0;
}


void SS::SimpleSocket::ListenerFunc()
{
	SS::Log("Listening started", SS::Inf);
	SOCKADDR_IN connection_soc = socket_data;
	int sizeof_new_con = sizeofaddr;
	SOCKET slistener = socket(AF_INET, SOCK_STREAM, NULL);
	listen_soc = slistener;
	bind(slistener, (SOCKADDR*)&socket_data, sizeofaddr);
	listen(slistener, max_connections);
	while(is_started)
	{
		SOCKET newCon;
		newCon = 0;
		newCon = accept(slistener, (SOCKADDR*)&connection_soc, &sizeof_new_con);
		if (newCon == 0 or WSAGetLastError() == 10004) {
			SS::Log("Error! Client connection failature. (" + std::to_string(WSAGetLastError()) + ")", SS::Err);
		}
		else {
			
			is_connected = true;
			char client_name[256];
			char client_ip[32];
			char client_mac[32];
			send(newCon, node_name.c_str(), node_name.size() + 1, NULL);
			recv(newCon, client_name, sizeof(client_name), NULL);

			send(newCon, node_ip.c_str(), node_ip.size() + 1, NULL);
			recv(newCon, client_ip, sizeof(client_ip), NULL);

			send(newCon, node_mac.c_str(), node_mac.size() + 1, NULL);
			recv(newCon, client_mac, sizeof(client_mac), NULL);

			SS::Log("Client " + std::string(client_name)+" connected", SS::Inf);

			std::string clinm = "trump";

			ConnectionInfo NewCli;
			NewCli.soc = newCon;
			NewCli.soc_data = socket_data;
			NewCli.name = client_name;
			NewCli.IP_adr = client_ip;
			NewCli.MAC_adr = client_mac;
			//connected_nodes.insert(std::pair<std::string, ConnectionInfo>(client_name, NewCli));	
			NewCli.reciver = new std::thread(&SS::SimpleSocket::Reciver, this, client_name);
			NewCli.reciver->detach();
			NewCli.msg_id = 0;
			node_list->push_back(NewCli);

		

		}
	}

}

void SS::SimpleSocket::Process()
{
	while (is_started) {
		if (processing_queue->size() > 0) {
			ProcessData next_proc= processing_queue->front();
			processing_queue->pop_front();
			SS::Log("Routin processing (Name: " + next_proc.name + ", Args: " + next_proc.params + ", From: " + next_proc.sender + ")", SS::Inf);
			if (!(callbacks->find(next_proc.name) == callbacks->end())) {
				int result;
				result=(*callbacks)[next_proc.name].exec(next_proc.params, next_proc.sender);
				SS::Log("Routin ended (Name: " + next_proc.name + ", Args: " + next_proc.params + ", From: " + next_proc.sender + ")"+ " Result: "+std::to_string(result), SS::Inf);
			}
			else {
				SS::Log("Routin not found (Name: " + next_proc.name + ", Args: " + next_proc.params + ", From: " + next_proc.sender + ")", SS::Inf);
			}
		}
	}
}




/// <summary>
/// Connect client to the server
/// </summary>
/// <param name="adres">: server IP adress</param>
/// <param name="port">: server port</param>
/// <param name="retrys">: amount of retrys if failed</param>
/// <returns> -1 if not a client, -2 if not initiated, error code if network error </returns>
int SS::SimpleSocket::Connect(std::string adres, int port, int retrys)
{
	if(is_initiated)
	{
		if (type)
		{

			bool flag = true;
			int times = retrys;
			int outp = 0;
			while (flag and times > 0) {
				times--;
				socket_data.sin_addr.s_addr = inet_addr(adres.c_str());
				socket_data.sin_port = htons(port);
				socket_data.sin_family = AF_INET;
				sizeofaddr = sizeof(socket_data);

				SOCKET Connection = socket(AF_INET, SOCK_STREAM, NULL);

				if (connect(Connection, (SOCKADDR*)&socket_data, sizeofaddr) != 0) {
					SS::Log("Error! Connection to server failature. (" + std::to_string(WSAGetLastError()) + ")", SS::Err);
					outp = WSAGetLastError();
				}
				else
				{
					SS::Log("Connected to server", SS::Inf);
					char host_name[256];
					char host_ip[36];
					char host_mac[32];
					is_connected = true;
					recv(Connection, host_name, sizeof(host_name), NULL);
					send(Connection, node_name.c_str(), node_name.size() + 1, NULL);

					recv(Connection, host_ip, sizeof(host_ip), NULL);
					send(Connection, node_ip.c_str(), node_ip.size() + 1, NULL);

					recv(Connection, host_mac, sizeof(host_mac), NULL);
					send(Connection, node_mac.c_str(), node_mac.size() + 1, NULL);

					ConnectionInfo Host;
					Host.soc = Connection;
					Host.soc_data = socket_data;
					Host.name = host_name;
					Host.IP_adr = host_ip;
					Host.MAC_adr = host_mac;
					//connected_nodes.insert(std::pair<std::string, ConnectionInfo>(host_name, Host));
					Host.reciver = new std::thread(&SS::SimpleSocket::Reciver, this, Host.name);
					Host.reciver->detach();
					Host.msg_id = 0;
					node_list->push_back(Host);
				}


			}
			return outp;
		}
		else {
			SS::Log("Trying connection when not a client", SS::Warn);
			return -1;
		}
		
	}
	else
	{
		SS::Log("Trying connection when not initiated", SS::Err);
		return -2;
	}
}

//TODO: Fix disconnectors

/// <summary>
/// Stop processing and close connection
/// </summary>
/// <param name="NodeName">: If server, name of kicking node</param>
/// <param name="is_external">: Is need to send disconnection signal</param>
/// <returns></returns>
int SS::SimpleSocket::Disconnect(std::string NodeName, bool is_external)
{

	if (type) {

		if (is_external) {
			Send("DisconnectMe");
		}
		
		SS::Log("Disconnected from server", SS::Inf);
		
		shutdown((*node_list)[0].soc, CF_BOTH);
		for (int i = 0; i < node_list->size(); i++)
		{
			closesocket((*node_list)[i].soc);
			(*node_list)[i].reciver->~thread();
			delete (*node_list)[i].reciver;
			shutdown((*node_list)[i].soc, CF_BOTH);
			(*node_list)[i].name.clear();


		}

		if (processor != nullptr) {
			processor->~thread();
			delete processor;
			processor = nullptr;
		}

		if (listner != nullptr) {
			closesocket(listen_soc);
			listner->~thread();
			delete listner;
			listner = nullptr;
		}

		is_started = false;
		is_connected = false;

		delete node_list;
		delete processing_queue;

		shutdown(listen_soc, CF_BOTH);

	}
	else
	{

		if (NodeName != "") {
			for (int  i = 0; i < node_list->size(); i++)
			{
				if ((*node_list)[i].name == NodeName) 
				{
					if (is_external) {
						Send("DisconnectMe");
					}

					closesocket((*node_list)[i].soc);
					(*node_list)[i].reciver->~thread();
					delete (*node_list)[i].reciver;
					shutdown((*node_list)[i].soc, CF_BOTH);
					(*node_list)[i].name.clear();

					SS::Log("Client"+ (*node_list)[i].name+" disconnected", SS::Inf);

				}
			}
		}
		else {
			for (int i = 0; i < node_list->size(); i++)
			{
				if (is_external) {
					Send("DisconnectMe");
				}
				closesocket((*node_list)[i].soc);
				(*node_list)[i].reciver->~thread();
				delete (*node_list)[i].reciver;
				shutdown((*node_list)[i].soc, CF_BOTH);
				(*node_list)[i].name.clear();
				
				SS::Log("All clients disconnected", SS::Inf);
				
			}
			
			if (processor != nullptr) {
				processor->~thread();
				delete processor;
				processor = nullptr;
			}

			if (listner != nullptr) {
				closesocket(listen_soc);
				listner->~thread();
				delete listner;
				listner = nullptr;
			}

			is_started = false;
			is_connected = false;

			delete node_list;
			delete processing_queue;

			shutdown(listen_soc, CF_BOTH);
		}
	}
	return 0;
}


/// <summary>
/// Connects external command to text command
/// </summary>
/// <param name="name">: name of executioner (called, when recived name of command)</param>
/// <param name="ptr">: pointer to a function</param>
/// <returns>-1 if not initiated</returns>
int SS::SimpleSocket::ConnectCommand(std::string name, int(*ptr)(std::string, std::string))
{
	if (is_initiated) 
		{
		Callback newCB;

		newCB.exec = ptr;

		callbacks->insert(std::pair<std::string, Callback>(name, newCB));

		return 0;
	}
	else
	{
		SS::Log("Not initiated (Connect command)", SS::Err);
		return -1;
	}
}


/// <summary>
/// Send text to another node
/// </summary>
/// <param name="data">: String, which need to be send</param>
/// <param name="clients">: vector of client names, who recieve this string</param>
/// <returns>-1 if not initiated, -2 if not connected, -3 if not started</returns>
int SS::SimpleSocket::Send(std::string data, std::vector<std::string> clients)
{
	if(is_started){
		if(is_connected){
			if (is_initiated) {
				std::string pref;
				

				if (type) {

					for (int i = 0; i < node_list->size(); i++) {
						pref = std::to_string(++(*node_list)[i].msg_id);
						pref.append(" ");
						pref.append(data);
						SS::Log("SENDING: "+ pref + " (to:" + (*node_list)[i].name + ")", SS::Inf);
						send((*node_list)[i].soc, pref.c_str(), pref.size() + 1, NULL);

					}

				}
				else
				{
					if (clients.size() == 0) {
						for (int i = 0; i < node_list->size(); i++) {
							pref = std::to_string(++(*node_list)[i].msg_id);
							pref.append(" ");
							pref.append(data);
							SS::Log("SENDING: " + pref + " (to:" + (*node_list)[i].name + ")", SS::Inf);
							send((*node_list)[i].soc, pref.c_str(), pref.size() + 1, NULL);

						}
					}
					else
					{
						for (int i = 0; i < node_list->size(); i++) {

							for (int g = 0; g < clients.size(); g++)
							{
								if ((*node_list)[i].name == clients[g]) {
									pref = std::to_string(++(*node_list)[i].msg_id);
									pref.append(" ");
									pref.append(data);
									SS::Log("SENDING: " + pref+" (to:"+ (*node_list)[i].name+")", SS::Inf);
									send((*node_list)[i].soc, pref.c_str(), pref.size() + 1, NULL);
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
				SS::Log("Not initiated", SS::Err);
				return -1;
			}
		}
		else 
		{
			SS::Log("Not connected", SS::Err);
			return -2;
		}

	}
	else 
	{
		SS::Log("Not started", SS::Err);
		return -3;
	}
}


//TODO: Fix Ping Timer(or smth)
//TODO: Add autopinger

/// <summary>
/// Send ping packet to a node
/// </summary>
/// <param name="node">:Name of the pingable node</param>
/// <returns>time in msec</returns>
int SS::SimpleSocket::Ping(std::string node)
{
	if (GetNodeByName(node)==nullptr) {
		return -1;
	}
	long long int start;
	start = CUR_TIME;
	Send("PingINI", {node});

	GetNodeByName(node)->pinger_flag=true;

	while (GetNodeByName(node)->pinger_flag) {

	}
	
	start = CUR_TIME - start;
	start = start / 100000;

	return start;
}

/// <summary>
/// Send WOL packet to the node
/// </summary>
/// <param name="IP_adr">IP adress of node</param>
/// <param name="MAC_adr">MAC adress of node</param>
/// <param name="port">Port to recieve a packet (9-default)</param>
/// <returns>-1 Socket error, -2 not initiated</returns>
int SS::SimpleSocket::EnableNode(std::string IP_adr, std::string MAC_adr, int port)
{
	if(is_initiated){
		SOCKET temp_soc;

		if ((temp_soc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR) {
			SS::Log("Failed to create WOL socket code("+std::to_string(WSAGetLastError())+")", SS::Crit);
			return -1;
		}

		SOCKADDR_IN sock_data;
		sock_data.sin_addr.s_addr = inet_addr(IP_adr.c_str());
		sock_data.sin_port = htons(port);
		sock_data.sin_family = AF_INET;

		char WOLpac[1024];

		for (int i = 0; i < 8; i++)
		{
			WOLpac[i] = 0xFF;
		}

		int cou = 8;

		for (int g = 0; g < 16; g++)
		{
			for (int i = 0; i < MAC_adr.size(); i += 3)
			{
				std::string temp;
				temp.push_back(MAC_adr[i]);
				temp.push_back(MAC_adr[i + 1]);
				WOLpac[cou] = std::stoul(temp, nullptr, 16);
				cou++;
			}
		}

		sendto(temp_soc, WOLpac, strlen(WOLpac) - 1, 0, (struct sockaddr*)&sock_data, sizeof(sock_data));

		closesocket(temp_soc);
		return 0;
	}
	else {
		return -2;
	}
}

/// <summary>
/// Send WOL to several nodes by name
/// </summary>
/// <param name="Names">Vector of names to wake up nodes</param>
/// <returns></returns>
int SS::SimpleSocket::WOLbyName(std::vector<std::string> Names)
{
	if (Names.size() > 0) {

		for (int i = 0; i < Names.size(); i++)
		{
			for (int g = 0; g < node_list->size(); g++) {
				if (Names[i] == (*node_list)[g].name) {
					EnableNode((*node_list)[g].IP_adr, (*node_list)[g].IP_adr);
				}
			}
		}

	}
	else
	{
		for (int i = 0; i < node_list->size(); i++)
		{
			EnableNode((*node_list)[i].IP_adr, (*node_list)[i].IP_adr);
		}
	}
	
	return 0;
}

/// <summary>
/// Returns node info by name
/// </summary>
/// <param name="Name">Name of node</param>
/// <returns></returns>
SS::ConnectionInfo* SS::SimpleSocket::GetNodeByName(std::string Name)
{

	for (int i = 0; i < (*node_list).size(); i++)
	{

		if ((*node_list)[i].name==Name) {
			return (&(*node_list)[i]);
		}

	}

	return nullptr;
}

SS::SimpleSocket::~SimpleSocket()
{

}

/// <summary>
/// Returns know nodes
/// </summary>
/// <returns>vector of nodes</returns>
std::vector<std::string> SS::SimpleSocket::GetNodeList()
{
	std::vector <std::string> ret;
	
	for (int i = 0; i < node_list->size(); i++)
	{
		ret.push_back((*node_list)[i].name);
	}

	return ret;
}


void SS::SimpleSocket::Reciver(std::string handler_name)
{
	char data[1024];
	UINT last_id = 0;
	ConnectionInfo* node_handler = nullptr;
	while (node_handler == nullptr) {
		node_handler = GetNodeByName(handler_name);
	}
	while (is_started) {
		
		recv(node_handler->soc, data, sizeof(data), NULL);
		SS::Log("RECCIEVED: " + std::string(data) + " (from:" + handler_name+")");
		bool flag = false;
		int cou = 0;
		std::string func_name="";
		std::string args = "";
		int verify = 0;
		while (data[cou] != ' ') {
			verify = verify * 10;
			verify = data[cou]-48;
			cou++;
		}

		if (last_id == verify) {
			SS::Log("Message dual detected ("+ handler_name+")");
			Disconnect(node_handler->name);
			return;
		}
		else
		{
			cou++;
			last_id = verify;
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
				cou++;
			}

			if (func_name == "DisconnectMe") {
				Disconnect(node_handler->name, false);
			}
			else if (func_name == "PingINI") {

				Send("PingANS", {node_handler->name});

			}
			else if (func_name == "PingANS") {

				node_handler->pinger_flag = false;

			}
			else {
				ProcessData NewRequest;
				NewRequest.name = func_name;
				NewRequest.params = args;
				NewRequest.sender = node_handler->name;

				SS::Log("Routin stored (Name: " + NewRequest.name + ", Args: " + NewRequest.params + ", From: "+NewRequest.sender+")", SS::Inf);
				processing_queue->push_back(NewRequest);

			}
		}

		
	}
}

/// <summary>
/// Returns all network adapters of the device
/// </summary>
/// <returns>vector of adaters</returns>
std::vector<SS::Adapter> SS::GetAdapterList()
{

	PIP_ADAPTER_INFO AdapterInfo;
	DWORD dwBufLen = sizeof(IP_ADAPTER_INFO);
	
	std::vector<SS::Adapter> ret;

	AdapterInfo = new IP_ADAPTER_INFO;
	if (AdapterInfo == NULL) {
		delete AdapterInfo;
		return std::vector<SS::Adapter>();
	}

	int cou = 1;
	while (GetAdaptersInfo(AdapterInfo, &dwBufLen) != NO_ERROR) 
	{
		delete AdapterInfo;
		AdapterInfo = new IP_ADAPTER_INFO[cou];
		dwBufLen = sizeof(IP_ADAPTER_INFO) * cou;
		cou++;
	}


	if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR) {
		
		PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
		do {
			SS::Adapter tempInf;
			tempInf.IP_adr = pAdapterInfo->IpAddressList.IpAddress.String;
			
			tempInf.name = pAdapterInfo->AdapterName;

			for (int i = 0; i < pAdapterInfo->AddressLength; i++)
			{
				
				char* hex=new char[4];
				_itoa(pAdapterInfo->Address[i], hex, 16);
				tempInf.MAC_adr.append(hex);
				tempInf.MAC_adr.push_back(':');
				delete[] hex;
			}
			ret.push_back(tempInf);
			pAdapterInfo = pAdapterInfo->Next;
		} while (pAdapterInfo);
	}

	delete AdapterInfo;
	return ret;

}
