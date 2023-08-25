#include "SimpleSocket.h"

#pragma warning(disable: 4996)


/*Creating simple socket WITHOUT initialization */
SS::SimpleSocket::SimpleSocket() {
	is_initiated = false;
	is_started = false;
	is_connected = false;

	CallBacks = nullptr;
	node_list = nullptr;
	processing_queue = nullptr;

	processor = nullptr;
	listner = nullptr;
}

/*name: node name
ip_adres: Local IP adress
port : program reserved port
type_of_node : Server or Client */
SS::SimpleSocket::SimpleSocket(std::string name, std::string ip_adres, int port, NodeType type_of_node)
{
	node_mac = "ff:ff:ff:ff:ff:ff";
	is_initiated = false;
	is_started = false;
	is_connected = false;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error! Winsoc not initiated. (" << WSAGetLastError() << ")" << std::endl;
		exit(1);
	}
	is_initiated = true;
	CallBacks = new std::map <std::string, Callback>;
	node_list = new std::vector <ConnectionInfo>;
	processing_queue= new std::deque <ProcessData>;
	processor = nullptr;
	listner = nullptr;
	
	node_ip = ip_adres;

	socket_data.sin_addr.s_addr = inet_addr(ip_adres.c_str());
	socket_data.sin_port = htons(port);
	socket_data.sin_family = AF_INET;
	sizeofaddr = sizeof(socket_data);
	node_name = name;
	max_connections = BACKLOG_THREADS;
	type = type_of_node;

}


/*Start node*/
int SS::SimpleSocket::Start(ProcessingType need_processor) 
{
	
	if (!is_started){
		is_started = true;
		if (type) {
			if (need_processor) {
				processor = new std::thread(&SS::SimpleSocket::process, this);
				processor->detach();
			}
			else {
				process();
			}
		}
		else
		{

			listner = new std::thread(&SS::SimpleSocket::listener_func, this);
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
	else
	{
		return -1;
	}
	return 0;
}


void SS::SimpleSocket::listener_func()
{

	std::cout << "Listener started." << std::endl;
	SOCKADDR_IN connection_soc = socket_data;
	int sizeof_new_con = sizeofaddr;
	SOCKET slistener = socket(AF_INET, SOCK_STREAM, NULL);
	listenSoc = slistener;
	bind(slistener, (SOCKADDR*)&socket_data, sizeofaddr);
	listen(slistener, max_connections);
	while(is_started)
	{
		SOCKET newCon;
		newCon = 0;
		newCon = accept(slistener, (SOCKADDR*)&connection_soc, &sizeof_new_con);
		if (newCon == 0 or WSAGetLastError() == 10004) {
			std::cout << "Error! Client connection failature. (" << WSAGetLastError() << ")" << std::endl;
		}
		else {
			std::cout << "Client Connected!" << std::endl;
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


			std::string clinm = "trump";

			ConnectionInfo NewCli;
			NewCli.soc = newCon;
			NewCli.soc_data = socket_data;
			NewCli.name = client_name;
			NewCli.IP_adr = client_ip;
			NewCli.MAC = client_mac;
			//connected_nodes.insert(std::pair<std::string, ConnectionInfo>(client_name, NewCli));	
			NewCli.reciver = new std::thread(&SS::SimpleSocket::reciver, this, client_name);
			NewCli.n_handl=NewCli.reciver->native_handle();
			NewCli.reciver->detach();
			NewCli.msg_id = 0;
			node_list->push_back(NewCli);

		

		}
	}

}

void SS::SimpleSocket::process()
{
	while (is_started) {
		if (processing_queue->size() > 0) {
			ProcessData next_proc= processing_queue->front();
			processing_queue->pop_front();
			if (!(CallBacks->find(next_proc.name) == CallBacks->end())) {
				(*CallBacks)[next_proc.name].exec(next_proc.params, next_proc.sender);
			}
		}
	}
}


/*Connects client to a server 
adres: IP adres
port: external port*/
int SS::SimpleSocket::Connect(std::string adres, int port, int retrys)
{
	if (type) {
		
		bool flag = true;
		int times = retrys;
		int outp = 0;
		while (flag and times>0) {
			times--;
			socket_data.sin_addr.s_addr = inet_addr(adres.c_str());
			socket_data.sin_port = htons(port);
			socket_data.sin_family = AF_INET;
			sizeofaddr = sizeof(socket_data);

			SOCKET Connection = socket(AF_INET, SOCK_STREAM, NULL);

			if (connect(Connection, (SOCKADDR*)&socket_data, sizeofaddr) != 0) {
				std::cout << "Error! Client connection failature. (" << WSAGetLastError() << ")" << std::endl;
				outp= WSAGetLastError();
			}
			else
			{
				std::cout << "Connected!" << std::endl;
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
				Host.MAC = host_mac;
				//connected_nodes.insert(std::pair<std::string, ConnectionInfo>(host_name, Host));
				Host.reciver = new std::thread(&SS::SimpleSocket::reciver, this, Host.name);
				Host.n_handl = Host.reciver->native_handle();
				Host.reciver->detach();
				Host.msg_id = 0;
				node_list->push_back(Host);
			}
			

		}
		return outp;
	}
	else {
		std::cout << "Error! Not a client." << std::endl;
		return -1;
	}
	return 0;
}

//TODO: Fix disconnectors
/*Stop node*/
int SS::SimpleSocket::Disconnect(std::string NodeName, bool is_external)
{

	if (type) {

		if (is_external) {
			Send("DisconnectMe");
		}
		
		std::cout << "Client disconnected." << std::endl;
		
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
			closesocket(listenSoc);
			listner->~thread();
			delete listner;
			listner = nullptr;
		}

		is_started = false;
		is_connected = false;

		delete node_list;
		delete processing_queue;

		shutdown(listenSoc, CF_BOTH);

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

					std::cout << "Client "<< (*node_list)[i] .name<<" disconnected." << std::endl;

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
				
				std::cout << "All clients disconnected." << std::endl;
				
			}
			
			if (processor != nullptr) {
				processor->~thread();
				delete processor;
				processor = nullptr;
			}

			if (listner != nullptr) {
				closesocket(listenSoc);
				listner->~thread();
				delete listner;
				listner = nullptr;
			}

			is_started = false;
			is_connected = false;

			delete node_list;
			delete processing_queue;

			shutdown(listenSoc, CF_BOTH);
		}
	}
	return 0;
}


/*Connect external command to socket*/
int SS::SimpleSocket::ConnectCommand(std::string name, int(*ptr)(std::string, std::string))
{
	if (is_initiated) 
		{
		Callback newCB;

		newCB.exec = ptr;

		CallBacks->insert(std::pair<std::string, Callback>(name, newCB));

		return 0;
	}
	else
	{
		std::cout << "Not initiated!" << std::endl;
		return -1;
	}
}


/*Send message to clients*/
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
						std::cout << "SENDING:" << pref << std::endl;
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
							std::cout << "SENDING:" << pref << std::endl;
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
									std::cout << "SENDING:" << pref << std::endl;
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
				std::cout << "Not initiated!" << std::endl;
				return -1;
			}
		}
		else 
		{
			std::cout << "Not connected!" << std::endl;
			return -2;
		}

	}
	else 
	{
		std::cout << "Not started!" << std::endl;
		return -3;
	}
}


//TODO: Fix Ping Timer(or smth)
//TODO: Add autopinger
float SS::SimpleSocket::Ping(std::string node)
{
	if (GetUsrByName(node)==nullptr) {
		return -1;
	}
	std::chrono::microseconds iniTime;
	iniTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	Send("PingINI", {node});

	GetUsrByName(node)->pinger_flag=true;

	while (GetUsrByName(node)->pinger_flag) {

	}
	
	float ans = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - iniTime.count();

	return ans;
}

int SS::SimpleSocket::EnableNode(std::string IP_adr, std::string MAC_adr, int port)
{
	if(is_initiated){
		SOCKET temp_soc;

		if ((temp_soc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR) {
			std::cout << "Failed to create socket : " << WSAGetLastError() << std::endl;
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

SS::ConnectionInfo* SS::SimpleSocket::GetUsrByName(std::string Name)
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

std::vector<std::string> SS::SimpleSocket::GetUserList()
{
	std::vector <std::string> ret;
	
	for (int i = 0; i < node_list->size(); i++)
	{
		ret.push_back((*node_list)[i].name);
	}

	return ret;
}


void SS::SimpleSocket::reciver(std::string handler_name)
{
	char data[1024];
	UINT last_id = 0;
	ConnectionInfo* node_handler = nullptr;
	while (node_handler == nullptr) {
		node_handler = GetUsrByName(handler_name);
	}
	while (is_started) {
		
		recv(node_handler->soc, data, sizeof(data), NULL);
		std::cout << "GETING:" << data << std::endl;
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
			Disconnect(node_handler->name);
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

				processing_queue->push_back(NewRequest);
			}
		}

		
	}
}


std::vector<SS::Adapter> SS::SimpleSocket::GetAdapterList()
{

	PIP_ADAPTER_INFO AdapterInfo;
	DWORD dwBufLen = sizeof(IP_ADAPTER_INFO);
	
	std::vector<SS::Adapter> ret;

	AdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
	if (AdapterInfo == NULL) {
		free(AdapterInfo);
		return std::vector<SS::Adapter>();
	}

	
	if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW) {
		free(AdapterInfo);
		AdapterInfo = (IP_ADAPTER_INFO*)malloc(dwBufLen);
		if (AdapterInfo == NULL) {
			return std::vector<SS::Adapter>();
		}
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

	free(AdapterInfo);
	return ret;

}
