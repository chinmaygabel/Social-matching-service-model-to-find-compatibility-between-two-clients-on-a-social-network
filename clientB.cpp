
/*
Name: Chinmay Gabel
Socket Programming Project
*/

// Socket programming code learned and referred from 'Beej’s Guide to Network ProgrammingUsing Internet Sockets'

//Header files
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include<limits.h>

using namespace std;

#define CENTRAL_SERVER_PORT 26475 // TCP port of the central server for sending data
#define localhost "127.0.0.1" // localhost IP address


int main(int argc, char** argv)
{
	cout<<"The client is up and running."<<endl;
	int clientSocket;
	int connectionStatus;
	char serverResponse[1000000] = ""; // response that would be received from the central server
	char username1[512] = "";  // username which would be entered as the command line argument
	char username2[512] = ""; // second commandline argument (optional)
	struct sockaddr_in centralServerAddress;  // address of central server for sending username.
	bool areTwoUsers = false; // flag for second command line argument

	if(argc == 2)
	{
		strcpy(username1,argv[1]);
	}

	if(argc == 3){// second command line argument entered (Extra credits)
		strcpy(username1,argv[1]);
		strcpy(username2,argv[2]);
		areTwoUsers = true;
	}

	clientSocket = socket(AF_INET, SOCK_STREAM,0);  // creating a TCP socket

	// Setting up central server's socket address for sending username info
	memset(&centralServerAddress.sin_zero, 0, sizeof(centralServerAddress));
	centralServerAddress.sin_family = AF_INET;
	centralServerAddress.sin_port = htons(CENTRAL_SERVER_PORT);
	centralServerAddress.sin_addr.s_addr = inet_addr(localhost);
	connectionStatus = connect(clientSocket, (struct sockaddr*) &centralServerAddress, sizeof(centralServerAddress));


	if(areTwoUsers == false) // only one user
	{
		strcpy(username2,"#$#$"); // special message to indicate that only one username is sent
		string temp1(username1);
		string temp2(username2);
		string username1Username2 = temp1 +"|" +temp2+"\0"; // <username1>|#$#$ would be sent to the central server
		char* username1Username2Char = NULL;
		username1Username2Char = &username1Username2[0];
		send(clientSocket, username1Username2Char, strlen(username1Username2Char), 0); // sending data to central server
		cout<<"The client sent "<<username1<< " to the Central server."<<endl;
	}
	else // two users (extra credits)
	{
		string temp1(username1);
		string temp2(username2);
		string username1Username2 = temp1 + "|"+temp2 + "\0"; // two usernames seperated by '|'  would be sent to central server
		char* username1Username2Char = NULL;
		username1Username2Char = &username1Username2[0];
		send(clientSocket, username1Username2Char, strlen(username1Username2Char), 0); // sending data to central server
		cout<<"The client sent "<<username1<< " to the Central server."<<endl;
		cout<<"The client sent "<<username2<< " to the Central server."<<endl;
	}

	recv(clientSocket,&serverResponse,sizeof(serverResponse),0); // receiving data back from central server
	string s1(serverResponse);
	cout<<s1<<endl;
	close(clientSocket);
	return 0;
}

