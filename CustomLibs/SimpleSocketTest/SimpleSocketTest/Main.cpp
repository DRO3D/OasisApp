
#include <iostream>
#include "SimpleSocket.h"
using namespace std;

int print(string str, string snd) {
	cout << str << endl;
	return 0;
}

int main() {
	cout << "0-start as server, 1- start as client" << endl;
	int chose;
	cin >> chose;
	SS::SimpleSocket miniMe;
	switch (chose)
	{
	case 0:
	{
		miniMe=SS::SimpleSocket("Test server", "127.0.0.1", 25565, 0, 1, 1);
		
		break; 
	}

	case 1:
	{
		miniMe = SS::SimpleSocket("Test client", "127.0.0.1", 25565, 1, 1, 1);
		miniMe.Connect("127.0.0.1", 25565);
		break;
	}
	default:
		break;
	}

	miniMe.ConnectCommand("print", print);


	while (true) {

		string str;

		cin >> str;

		miniMe.Send("print Yes");

	}

}