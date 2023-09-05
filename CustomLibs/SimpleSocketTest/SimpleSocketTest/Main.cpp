
#include <iostream>
#include "SimpleSocket.h"
using namespace std;

SS::SimpleSocket miniMe;

int print(string str, string snd) {
	cout <<snd<< ": " << str << endl;
	return 0;
}

int shutdown(string nan, string non) {

	miniMe.Disconnect();

	return 0;
}

int server_react(string str, string snd) {

	cout << snd << ": " << str << endl;
	vector <string> usrs = miniMe.GetNodeList();
	vector <string> list;

	for (int i = 0; i < usrs.size(); i++)
	{
		if (usrs[i] != snd) {
			list.push_back(usrs[i]);
		}
	}

	miniMe.Send("print " + snd + " " + str, list);
	
	return 0;
}

int ini_ping(string str, string snd) {


	return miniMe.Ping("(1)Test client(1)");
	

}


int main() {
	


	cout << "0-start as server, 1- start as client" << endl;


	int chose;
	cin >> chose;

	switch (chose)
	{
	case 0:
	{
		miniMe=SS::SimpleSocket("Test server", SS::GetAdapterList()[0], 25565, SS::Server);
		miniMe.ConnectCommand("print", server_react);
		miniMe.ConnectCommand("stop", shutdown);
		miniMe.ConnectCommand("png", ini_ping);
		break; 
	}

	case 1:
	{
		miniMe = SS::SimpleSocket("(1)Test client(1)", SS::GetAdapterList()[0], 25566, SS::Client);
		miniMe.Connect("127.0.0.1", 25565);
		miniMe.ConnectCommand("print", print);
			
		break;
	}
	case 2:
	{
		miniMe = SS::SimpleSocket("(2)Test client(2)", SS::GetAdapterList()[0], 25567, SS::Client);
		miniMe.Connect("127.0.0.1", 25565);
		miniMe.ConnectCommand("print", print);

		break;
	}
	default:
		break;
	}

	
	miniMe.Start(SS::Background);

	//miniMe.EnableNode("192.168.0.242", "30:9c:23:ae:33:66");

	while (true) {

		string str;

		cin >> str;

		miniMe.Send("print " + str);

	}

}