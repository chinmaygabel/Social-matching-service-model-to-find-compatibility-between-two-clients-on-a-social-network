
/*
Name: Chinmay Gabel
Socket Programming Project
*/

// Socket programming code learned and referred from 'Beej’s Guide to Network ProgrammingUsing Internet Sockets'

// Header files
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

#define CENTRAL_SERVER_PORT 25475 // TCP port of the central server for sending data
#define localhost "127.0.0.1" // localhost IP address


int main(int argc, char* argv[]){
	cout<<"The client is up and running."<<endl;
	int clientSocket;
	int connectionStatus;
	char serverResponse[1000000] =""; // response that would be received from the central server
	char username1[512]; // username which would be entered as the command line argument 
	char username2[512]; // second commandline argument which will be ignored
	struct sockaddr_in centralServerAddress; // address of central server for sending username.

	if(argc == 2)
	{
		char username1[sizeof(argv[1])/sizeof(char)];
		strcpy(username1,argv[1]); // getting username from the command line
	}

	if(argc == 3)
	{
		char username1[sizeof(argv[1])/sizeof(char)];
		strcpy(username1,argv[1]);
		char username2[sizeof(argv[2])/sizeof(char)];
		strcpy(username2,argv[2]);
	}


	clientSocket = socket(AF_INET, SOCK_STREAM,0); // creating a TCP socket
	// Setting up central server's socket address for sending username info
	memset(&centralServerAddress.sin_zero, 0, sizeof(centralServerAddress));
	centralServerAddress.sin_family = AF_INET;
	centralServerAddress.sin_port = htons(CENTRAL_SERVER_PORT);
	centralServerAddress.sin_addr.s_addr = inet_addr(localhost); // localhost IP address

	//connecting to the central server's TCP port
	connectionStatus = connect(clientSocket, (struct sockaddr*) &centralServerAddress, sizeof(centralServerAddress));
	send(clientSocket, username1, sizeof(username1), 0); // sending username info to central server
	cout<<"The client sent "<<username1<< " to the Central server."<<endl;
	recv(clientSocket,&serverResponse,sizeof(serverResponse),0); // receiving info from central server
	string s1(serverResponse);
	cout<<serverResponse<<endl;
	close(clientSocket); // closing the client socket

	return 0;
}

