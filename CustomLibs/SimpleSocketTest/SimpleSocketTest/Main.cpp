
#include <iostream>
#include "SimpleSocket.h"
using namespace std;

int main() {
	cout << "0-start as server, 1- start as client" << endl;
	int chose;
	cin >> chose;

	switch (chose)
	{
	case 0:
	{
		SS::SimpleSocket miniMe1("Test server", "127.0.0.1", 25565, 0, 1);
		break; 
	}

	case 1:
	{
		SS::SimpleSocket miniMe2("Test client", "127.0.0.1", 25565, 1, 1);
		miniMe2.Connect("127.0.0.1", 25565);
		break;
	}
	default:
		break;
	}


	cout << "Hello world" << endl;

}