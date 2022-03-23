
/*
Name: Chinmay Gabel
Socket Programming Project
*/


// Socket programming code learned and referred from 'Beej’s Guide to Network ProgrammingUsing Internet Sockets'

// Header files
#include <iostream>
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
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <string>

using namespace std;


#define SERVER_S_UDP 22475 // serverS's UDP port for receiving info from the central server
#define CENTRAL_SERVER_UDP 24475 // Central server's UDP port for sending data to central server 
#define localhost "127.0.0.1" // localhost IP address
#define CHUNK_SIZE 500 // chunk size for sending UDP data in chunks


void sendUDPData(string data, int socketDescp ,struct sockaddr_in socketAddr);
string receiveUDPData(int socketDescp ,struct sockaddr_in socketAddr);


int main()
{
	cout<<"The ServerS is up and running using UDP on port "<<SERVER_S_UDP<<"."<<endl;
	int serverSSocket;
	struct sockaddr_in serverSAddr; // serverS's address information
	struct sockaddr_in clientAddr; // connector’s address information
	struct sockaddr_in centralServerAddress; // central server's address information

	socklen_t addr_len;
	int numbytes = 512;
	//char centralServerMsg[MAXBUFLEN];

	// creating UDP socket for serverS and getting socket descriptor
	if((serverSSocket = socket(PF_INET, SOCK_DGRAM, 0)) == -1) 
	{
		perror("socket");
		exit(1);
	}
	// Setting up serverS's socket address for receiving data from the 
	memset(&serverSAddr.sin_zero, 0, sizeof(serverSAddr));
	serverSAddr.sin_family = AF_INET;// host byte order
	serverSAddr.sin_port = htons(SERVER_S_UDP);// short, network byte order
	serverSAddr.sin_addr.s_addr = inet_addr(localhost); // localhost IP address

	// Setting up central server's socket address for sending data to central server
	memset(&centralServerAddress.sin_zero, 0, sizeof(centralServerAddress));
	centralServerAddress.sin_family = AF_INET;// host byte order
	centralServerAddress.sin_port = htons(CENTRAL_SERVER_UDP);// short, network byte order
	centralServerAddress.sin_addr.s_addr = inet_addr(localhost); // localhost IP address


	// binding socket descriptor and socket address
	if(bind(serverSSocket, (struct sockaddr *)&serverSAddr,sizeof(struct sockaddr)) == -1) 
	{
		perror("bind");
		exit(1);
	}

	while(1)
	{
		string centralServerMessage = "";
		centralServerMessage = receiveUDPData(serverSSocket,clientAddr); // receiving message from the central server which contains the edge ifo
		cout<<"The ServerS received a request from Central to get the scores."<<endl;

		if(centralServerMessage.compare("###") == 0) // checking for special message i.e, if the graph is NULL
		{
			string exit = "###";
			sendUDPData(exit,serverSSocket,centralServerAddress); // sending back the special message if the graph is NULL
			cout<<"The ServerS finished sending the scores to Central."<<endl;
			continue;

		}

		vector <string> nodes; // vector which would contain all the usernames (no duplicates)
		string tempNode = "";
		int i =0;

		while(centralServerMessage[i] != '\0')
		{
			if(centralServerMessage[i] != ' ' && centralServerMessage[i] != '|' )
			{
				tempNode += centralServerMessage[i]; // parsing the usernames character by character
			}
			else
			{
				if(find(nodes.begin(),nodes.end(),tempNode) == nodes.end()) // checking for duplicate entries 
				{
					nodes.push_back(tempNode); // pushing usernames into the vector
					tempNode = "";
				}
				else
				{
					tempNode = "";
				}
			}
			
			i++;
		}
		if(find(nodes.begin(),nodes.end(),tempNode) == nodes.end())
		{
			nodes.push_back(tempNode);
		}	

		string nodeScores = ""; // String that would contain the username along with the scores seperated by space. Two entries would be seperated by '|'.
		ifstream scoreFile;
		scoreFile.open("scores.txt"); // reading the file containing the scores
		if(scoreFile.is_open())
		{
			string fileLine;
			while(getline(scoreFile, fileLine))
			{
				string node = "";
				string score = "";
				int i =0;

				while(fileLine[i] != ' ')
				{
					node += fileLine[i]; // parsing username character by character
					i++;
				}
				if(find(nodes.begin(),nodes.end(),node) != nodes.end()) // checking if the score of this username is required or not
				{
					while(fileLine[i] != '\0') 
					{
						if(fileLine[i] == ' ')
						{
							i++;
							continue;
						}
						score += fileLine[i]; // parsing score character by character
						i++;
					}
					nodeScores += node + " " + score+"|"; // adding to the nodeScores (which would be sent to central server)
				} 

			}
		}
		scoreFile.close(); // closing the file

		sendUDPData(nodeScores,serverSSocket,centralServerAddress); // sending the username along with their scores to the central server
		cout<<"The ServerS finished sending the scores to Central."<<endl;

		nodes.empty();

	}
	

	close(serverSSocket);
	return 0;
}


// Function Definitions 

// This function sends a string data in chunks via UDP protocol
void sendUDPData(string data, int socketDescp ,struct sockaddr_in socketAddr)
{
	int bytesSent = 0;
	int chunksSent = 0;
	int totalChunks = 0;
	int totalSize = data.length();
	int numbytes;
	string temp = "";
	
		// Calculating total chunks to send
	if(totalSize%CHUNK_SIZE == 0)
	{
		totalChunks = totalSize/CHUNK_SIZE;

	}
	else
	{
		totalChunks = (totalSize/CHUNK_SIZE) + 1;
	}
	// Sending string data in chunks
	while(chunksSent < totalChunks)
	{
		if(totalSize - bytesSent < CHUNK_SIZE)
		{
			temp = data.substr(bytesSent);
		}

		else
		{
			temp = data.substr(bytesSent,CHUNK_SIZE);  // string data of size CHUNK_SIZE
		}
		
		char* response = NULL;
    	string tempStr(temp);
    	response = &tempStr[0];
    	int numbytes;
		if ((numbytes = sendto(socketDescp, response, strlen(response), 0,(struct sockaddr *)&socketAddr, sizeof(struct sockaddr))) == -1)  // sending data of size CHUNK_SIZE
		{
			perror("sendto");
			exit(1);
		}

		chunksSent += 1;
	    bytesSent+=CHUNK_SIZE;
	    temp = "";

	}

	string escape = "$#$"; // An escase string is sent at the last to let the receiver know that the sender is done with sending data
	char* escapeChr;
	string tempChr(escape);
	escapeChr = &tempChr[0];

	if ((numbytes = sendto(socketDescp, escapeChr, strlen(escapeChr), 0,(struct sockaddr *)&socketAddr, sizeof(struct sockaddr))) == -1) 
	{
		perror("sendto");
		exit(1);
	}

}

string receiveUDPData( int socketDescp ,struct sockaddr_in socketAddr )
{
	int numbytes;

	string data ("");

	socklen_t addr_len;
	addr_len = sizeof(struct sockaddr);

	while(1){
		char rsp[512]  = "";
		if((numbytes=recvfrom(socketDescp, rsp, sizeof(rsp) , 0,(struct sockaddr *)&socketAddr, &addr_len)) == -1)  // receiving data in chunk
		{
			perror("recvfrom");
			exit(1);
		}

		string temp1(rsp);
		if(temp1.compare("$#$") == 0)
		{
			break;
		}
		else
		{
			data.append(temp1);
		}

	}
	return data;

}
