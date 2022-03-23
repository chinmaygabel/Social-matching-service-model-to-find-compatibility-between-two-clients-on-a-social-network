
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
#include <string>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <algorithm>

using namespace std;


// Named constants
#define PORT_FOR_B 26475 // TCP Port used for communicating with clientA
#define PORT_FOR_A 25475 // TCP Port used for communicating with clientA
#define UDP_PORT 24475 // UDP port for receiving data from ServerT, serverS and serverP
#define SERVER_P_UDP 23475 // UDP port of serverP to which data is sent 
#define SERVER_T_UDP 21475 // UDP port of serverT to which data is sent 
#define SERVER_S_UDP 22475 // UDP port of serverS to which data is sent 
#define CHUNK_SIZE 500 // Chunk size of data while sending over UDP
#define localhost "127.0.0.1" // localhost IP address



// Function Declaration (defined and explained at the bottom)
void sigchld_handler(int s); // resued code from Beej's text
void sendUDPData(string data, int socketDescp ,struct sockaddr_in socketAddr);
string receiveUDPData(int socketDescp ,struct sockaddr_in socketAddr);
void sendExitMsgToSP( int centralServerSocketUDP ,struct sockaddr_in serverSAddress, struct sockaddr_in serverPAddress);


int main(){
	cout<<"The Central server is up and running."<<endl;

	int clientAChildSocket; // child socket descriptor used for communicating with clientA 
	int clientBChildSocket; // child socket descriptor used for communicating with clientB
	int centralServerSocketA; // parent socket descriptor - for clientA
	int centralServerSocketB; // parent socket descriptor - for clientB
	int centralServerSocketUDP; // UDP socket descriptor
	int connectionStatus;
	int bytesSent = 0;
	int chunksSent = 0;
	int totalChunks = 0;
	int totalSize;
	string temp = "";
	socklen_t sin_size;
	socklen_t addr_len;
	sin_size = sizeof(struct sockaddr_in);
	addr_len = sizeof(struct sockaddr);




	// getting socket descriptors

	if((centralServerSocketA = socket(AF_INET, SOCK_STREAM,0)) == -1)
	{
		perror("socket");
		exit(1);
	}
        
	if((centralServerSocketB = socket(AF_INET, SOCK_STREAM,0)) == -1)
	{
		perror("socket");
		exit(1);
	}

	if ((centralServerSocketUDP = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
	{
		perror("socket");
		exit(1);
	}


	int yes=1;
	if (setsockopt(centralServerSocketA,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) 
	{
		perror("setsockopt");
		exit(1);
	}

	//int yes=1;
	if (setsockopt(centralServerSocketB,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) 
	{
		perror("setsockopt");
		exit(1);
	}



	struct sockaddr_in centralServerAddressA; // Address for the central server's socket created for communicating with clientA
	struct sockaddr_in centralServerAddressB; // Address for the central server's socket created for communicating with clientB
	struct sockaddr_in centralServerUDP; // Address for the central server's UDP socket
	struct sockaddr_in clientAAddress; // clientA's socket address
	struct sockaddr_in clientBAddress; // clientB's socket address
	struct sockaddr_in serverPAddress; // serverP's socket address
	struct sockaddr_in serverTAddress; // serverT's socket address
	struct sockaddr_in serverSAddress; // serverS's socket address



	// Setting up central server's socket address created for communicating with clientA
	memset(&centralServerAddressA.sin_zero, 0, sizeof(centralServerAddressA));
	centralServerAddressA.sin_family = AF_INET;
	centralServerAddressA.sin_port = htons(PORT_FOR_A);
	centralServerAddressA.sin_addr.s_addr = inet_addr(localhost); // localhost IP


	// Setting up central server's socket address created for communicating with clientB 
	memset(&centralServerAddressB.sin_zero, 0, sizeof(centralServerAddressB));
	centralServerAddressB.sin_family = AF_INET;
	centralServerAddressB.sin_port = htons(PORT_FOR_B);
	centralServerAddressB.sin_addr.s_addr = inet_addr(localhost);  // localhost IP



	// Setting up serverP's socket address for sending data to serverP
	memset(&serverPAddress.sin_zero, 0, sizeof(serverPAddress)); // zero the rest of the struct
	serverPAddress.sin_family = AF_INET; // host byte order
	serverPAddress.sin_port = htons(SERVER_P_UDP); // short, network byte order
	serverPAddress.sin_addr.s_addr = inet_addr(localhost);  // localhost IP


	// Setting up serverT's socket address for sending data to serverT
	memset(&serverTAddress.sin_zero, 0, sizeof(serverTAddress));
	serverTAddress.sin_family = AF_INET; // host byte order
	serverTAddress.sin_port = htons(SERVER_T_UDP); // short, network byte order
	serverTAddress.sin_addr.s_addr = inet_addr(localhost);  // localhost IP


	// Setting up serverS's socket address for sending data to serverS
	memset(&serverSAddress.sin_zero, 0, sizeof(serverSAddress));
	serverSAddress.sin_family = AF_INET; // host byte order
	serverSAddress.sin_port = htons(SERVER_S_UDP); // short, network byte order
	serverSAddress.sin_addr.s_addr = inet_addr(localhost);  // localhost IP

	// Setting up central server's UDP socket address for receiving data from other servers
	memset(&centralServerUDP.sin_zero, 0, sizeof(centralServerUDP));
	centralServerUDP.sin_family = AF_INET; // host byte order
	centralServerUDP.sin_port = htons(UDP_PORT); // short, network byte order
	centralServerUDP.sin_addr.s_addr = inet_addr(localhost);  // localhost IP


	// binding socket descriptors and socket addresses
	if(bind(centralServerSocketA, (struct sockaddr*) &centralServerAddressA, sizeof(centralServerAddressA)) == -1)
	{
		perror("bind");
		exit(1);
	}

	if(bind(centralServerSocketB, (struct sockaddr*) &centralServerAddressB, sizeof(centralServerAddressB)) == -1)
	{
		perror("bind");
		exit(1);
	}

	if(bind(centralServerSocketUDP, (struct sockaddr *)&centralServerUDP,sizeof(centralServerUDP)) == -1) 
	{
		perror("bind");
		exit(1);
	}

	struct sigaction sa;
	sa.sa_handler = sigchld_handler; 
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) 
	{
		perror("sigaction");
		exit(1);
	}

	// Started listening to any communication from clientA (TCP connection)
	if(listen(centralServerSocketA, 10) == -1)
	{
		perror("listen");
		exit(1);
	}

	// Started listening to any communication from clientB (TCP connection)
	if(listen(centralServerSocketB, 10) == -1)
	{
		perror("listen");
		exit(1);
	}

	while(1)
	{
		// Initializing variables 

		char userNameA[256] = ""; // username sent from the client -A in character array format.

		// username(s) sent from the client-B. If two usernames are sent, then they are seperated by '|' symbol <usernameB|usernameB1>, 
		// otherwise this contains <usernameB>|#$#$. Here #$#$ is a special symbol which means that the cleintB has sent only one username
		char userNamesB[256] = ""; 
		char userNameB[256] = ""; // 1st username send by clientB
		char userNameB1[256] = "";// 2nd username (if any) sent by clientB. (For bonus credits)
		string responseForAStr = ""; // Final response sent to clientA
    	string responseForBStr = ""; // Final response sent to clientB
    	string uA = ""; // username sent from the client -A in C++ string format
    	string uB = ""; // 1st username sent from client-B in C++ string format
    	string uB1 = ""; // 2nd username sent from client-B in C++ string format
    	string responseS = ""; // response received from serverS (for usernameA and usernameB pair)
    	string responseS1 = ""; // response received from serverS (for usernameA and usernameB1 pair)
    	string responseT = ""; // response received from serverT (for usernameA and usernameB pair)
    	string responseT1 = ""; // response received from serverS (for usernameA and usernameB1 pair) -For Bonus credits
    	string totalCostStr = ""; // response received from serverP (for usernameA and usernameB pair) regarding cost
    	string totalCostStr1 = ""; // response received from serverP (for usernameA and usernameB1 pair) regarding cost - For bonus credits
    	string responsePathAtoB = ""; // response received from serverP (for usernameA and usernameB pair) regarding path having minimum matching gap
    	string responsePathAtoB1 = ""; // response received from serverP (for usernameA and usernameB1 pair) regarding path having minimum matching gap- for bonus credits
    	string responsePathBtoA = ""; // response received from serverP (for usernameB and usernameA pair) regarding path having minimum matching gap
    	string responsePathB1toA = ""; // response received from serverP (for usernameB1 and usernameA pair) regarding path having minimum matching gap- For bonus credits
    	int i =0;
    	int numbytes;


    	// Child socket getting created after accepting connection from client-A
		if ((clientAChildSocket = accept(centralServerSocketA,(struct sockaddr *)&clientAAddress,&sin_size)) == -1) 
		{
			perror("accept");
		}       
		
		// receiving username from client-A
		if(recv(clientAChildSocket,&userNameA,sizeof(userNameA),0) == -1)
		{
			perror("recv");
		}
		
		cout<<"The Central server received input=\""<< userNameA<< "\" from the client using TCP over port "<<PORT_FOR_A<<"."<<endl;
		

		// Child socket getting created after accepting connection from client-B
		if ((clientBChildSocket = accept(centralServerSocketB,(struct sockaddr *)&clientBAddress,&sin_size)) == -1) 
		{
			perror("accept");
		}    

		//receiving username(s) from client-B 
		if(recv(clientBChildSocket,&userNamesB,sizeof(userNamesB),0) == -1)
		{
			perror("recv");
		}




		// Splitting userNamesB (<userNameB>|<userNameB1>) into userNameB and userNameB1

		while(userNamesB[i] != '\0')
		{
			if(userNamesB[i] == '|') // checking for the delimeter
			{
				break;
			}
			else
			{
				userNameB[i] = userNamesB[i]; // constructing userNameB
			}
			i+=1;

		}
		i+=1;
		userNameB[i] = '\0';
		int j =0;

		while(userNamesB[i] != '\0')
		{
			userNameB1[j] = userNamesB[i]; // constructing userNameB1
			i+=1;
			j+=1;
		}
		userNameB1[j+1] = '\0';
		i = 0;
		j=0;


		cout<<"The Central server received input=\""<< userNameB<< "\" from the client using TCP over port "<<PORT_FOR_B<<"."<<endl;

		// if there is only one username sent by client-B, then userNameB1 would be equal to a special signature "#$#$" (For bonus credits)

		if(strcmp(userNameB1,"#$#$\0") != 0) // checking for special signature
		{
			cout<<"The Central server received input=\""<< userNameB1<< "\" from the client using TCP over port "<<PORT_FOR_B<<"."<<endl;

		}

		// Sending usernames to serverT for pair userNameA and userNameB
		if ((numbytes = sendto(centralServerSocketUDP, userNameA, strlen(userNameA), 0,(struct sockaddr *)&serverTAddress, sizeof(struct sockaddr))) == -1) 
		{
			perror("sendto");
			exit(1);
		}

		if ((numbytes = sendto(centralServerSocketUDP, userNameB, strlen(userNameB), 0,(struct sockaddr *)&serverTAddress, sizeof(struct sockaddr))) == -1) 
		{
			perror("sendto");
			exit(1);
		}

		cout<<"The Central server sent a request to Backend-Server T."<<endl;


		
		// Getting response from serverT regarding the reduced graph containing userNameA and userNameB
		responseT = receiveUDPData(centralServerSocketUDP,serverTAddress);
		cout<<"The Central server received information from Backend-Server T using UDP over port "<<UDP_PORT<<"."<<endl;


		if(strcmp(userNameB1,"#$#$\0") != 0) // checking if userNameB1 (second username sent by client-B ) exists
		{// If the signature matches, then there's no second username sent by the client-B

			// Sending usernames to serverT for pair userNameA and userNameB1, iff userNameB1 exits (for bonus credits)

			if ((numbytes = sendto(centralServerSocketUDP, userNameA, strlen(userNameA), 0,(struct sockaddr *)&serverTAddress, sizeof(struct sockaddr))) == -1) {
				perror("sendto");
				exit(1);
			}

			if ((numbytes = sendto(centralServerSocketUDP, userNameB1, strlen(userNameB1), 0,(struct sockaddr *)&serverTAddress, sizeof(struct sockaddr))) == -1) {
				perror("sendto");
				exit(1);
			}
			cout<<"The Central server sent a request to Backend-Server T."<<endl;

			// Getting response from serverT regarding the reduced graph containing userNameA and userNameB1
			responseT1 = receiveUDPData(centralServerSocketUDP,serverTAddress);
			cout<<"The Central server received information from Backend-Server T using UDP over port "<<UDP_PORT<<"."<<endl;
		}
		
		


		bool ABPath = false; // flag for path between userNameA and userNameB
		bool AB1Path = false; // flag for path between userNameA and userNameB1
		bool UserB1Exists = false; // flag for userNameB1 -> exists or not


		if(strcmp(userNameB1,"#$#$\0") == 0) // checking if userNameB1 does not exist.
		{
			ABPath = true;
			
			if(responseT.compare("###") == 0) // "###" is a special signature sent by serverT if there is no path between the two users
			{// If there is no path between the userNameA and userNameB, then send the response directly to the clients 

	    		uA = userNameA;
	    		uB = userNameB;

	    		sendExitMsgToSP(centralServerSocketUDP,serverSAddress,serverPAddress);

	    		responseForAStr = "Found no compatibility for "+uA+" and "+uB;
	    		responseForBStr = "Found no compatibility for "+uB+" and "+uA;
	    		char* responseForA = NULL;
	    		string tempStrAB(responseForAStr);
	    		responseForA = &tempStrAB[0];
	    		char* responseForB = NULL;
	    		string tempStrBA(responseForBStr);
	    		responseForB = &tempStrBA[0];
	    		if (send(clientAChildSocket, responseForA, strlen(responseForA), 0) == -1) // Sending response to clientA
	    		{
	    			perror("send");
	    		}
	    		cout<<"The Central server sent the results to client A."<<endl;
	    		if (send(clientBChildSocket, responseForB, strlen(responseForB), 0) == -1) // Sending response to clientB
	    		{
	    			perror("send");
	    		}
	    		cout<<"The Central server sent the results to client B."<<endl;
	    		continue;
	    	}


		}
		else
		{// if userNameB1 exists (for bonus credits)
			UserB1Exists = true;
			if(responseT.compare("###") == 0 && responseT1.compare("###")==0) // check if there is no path b/w userNameA and userNameB as well as userNameA and userNameB1
			{

	    		uA  = userNameA;
	    		uB  = userNameB;
	    		uB1 = userNameB1;
	    		sendExitMsgToSP(centralServerSocketUDP,serverSAddress,serverPAddress);
	    		sendExitMsgToSP(centralServerSocketUDP,serverSAddress,serverPAddress);


	    		responseForAStr = "Found no compatibility for "+uA+" and "+uB+"\n"+ "Found no compatibility for "+uA+" and "+uB1; //response for client-A
	    		responseForBStr = "Found no compatibility for "+uB+" and "+uA+"\n" + "Found no compatibility for "+uB1+" and "+uA;// response for client-B
	    		char* responseForA = NULL;
	    		string tempStrAB(responseForAStr);
	    		responseForA = &tempStrAB[0];
	    		char* responseForB = NULL;
	    		string tempStrBA(responseForBStr);
	    		responseForB = &tempStrBA[0];
	    		if (send(clientAChildSocket, responseForA, strlen(responseForA), 0) == -1) // sending to client-A
	    		{
	    			perror("send");
	    		}
	    		cout<<"The Central server sent the results to client A."<<endl;
	    		if (send(clientBChildSocket, responseForB, strlen(responseForB), 0) == -1) // sending to client-B
	    		{
	    			perror("send");
	    		}
	    		cout<<"The Central server sent the results to client B."<<endl;
	    		continue; // skipping the rest of the while loop
			}

			else if(responseT.compare("###") == 0 && responseT1.compare("###") != 0) 
			{
				ABPath = false; // path between userNameA and userNameB does not exist.
				sendExitMsgToSP(centralServerSocketUDP,serverSAddress,serverPAddress);
				AB1Path = true; // path between usernameA and userNameB1 exists.
			}
			else if(responseT.compare("###") != 0 && responseT1.compare("###") == 0) //
			{
				ABPath = true; // path between userNameA and userNameB exists.
				AB1Path = false; // path between userNameA and userNameB1 does not exist.
				sendExitMsgToSP(centralServerSocketUDP,serverSAddress,serverPAddress);

			}
			else
			{
				ABPath =true; // path between userNameA and userNameB exists.
				AB1Path = true; // path between userNameA and userNameB1 exists.
			}
		}


		if(ABPath == true)
		{ // getting score information for the users in the subgraph containing userNameA and userNameB
			sendUDPData(responseT, centralServerSocketUDP ,serverSAddress); // sending the subgraph info to serverS
			cout<<"The Central server sent a request to Backend-Server S."<<endl;
			responseS = receiveUDPData(centralServerSocketUDP,serverSAddress);
			cout<<"The Central server received information from Backend-Server S using UDP over port "<<UDP_PORT<<"."<<endl;

		}


		if(AB1Path == true)
		{// getting score information for the users in the subgraph containing userNameA and userNameB1
			sendUDPData(responseT1, centralServerSocketUDP ,serverSAddress); // sending the subgraph info to serverS             //<--- lookinto it
			cout<<"The Central server sent a request to Backend-Server S."<<endl;
			responseS1 =  receiveUDPData(centralServerSocketUDP,serverSAddress);
			cout<<"The Central server received information from Backend-Server S using UDP over port "<<UDP_PORT<<"."<<endl;
		}

		if(ABPath == true) // Communicating with serverP for userNameA, userNameB pair
		{//  Sending the name of the users to serverP

			string graphExist = "111"; // sending that graph or path exists 
			sendUDPData(graphExist, centralServerSocketUDP ,serverPAddress);

			if ((numbytes = sendto(centralServerSocketUDP, userNameA, strlen(userNameA), 0,(struct sockaddr *)&serverPAddress, sizeof(struct sockaddr))) == -1) 
			{
				perror("sendto");
				exit(1);
			}

			if ((numbytes = sendto(centralServerSocketUDP, userNameB, strlen(userNameB), 0,(struct sockaddr *)&serverPAddress, sizeof(struct sockaddr))) == -1) 
			{
				perror("sendto");
				exit(1);
			}

			// sending subgraph containing the users (Topology info) and scores to serverP 

			sendUDPData(responseT, centralServerSocketUDP ,serverPAddress);
			sendUDPData(responseS, centralServerSocketUDP ,serverPAddress);
			cout<<"The Central server sent a processing request to Backend-Server P."<<endl;



			responsePathAtoB =  receiveUDPData(centralServerSocketUDP,serverPAddress); // receiving path having minimum matching gap from serverP
			responsePathBtoA = receiveUDPData(centralServerSocketUDP,serverPAddress);

			char totalCost[16] = "";
			// receiving the cost (matching gap) info from serverP
			if((numbytes=recvfrom(centralServerSocketUDP, totalCost, sizeof(totalCost) , 0,(struct sockaddr *)&serverPAddress, &addr_len)) == -1) 
			{
				perror("recvfrom");
				exit(1);
			}

			cout<<"The Central server received the results from backend server P."<<endl;
			totalCostStr  = totalCost;

		}

		if(AB1Path == true)// Communicating with serverP for userNameA, userNameB1 pair
		{//  Sending the name of the users to serverP


			string graphExist = "111"; // sending that graph or path exists 
			sendUDPData(graphExist, centralServerSocketUDP ,serverPAddress);

			//cout<<"DEBUG: "<<userNameA<<" "<<userNameB1<<endl;


			if ((numbytes = sendto(centralServerSocketUDP, userNameA, strlen(userNameA), 0,(struct sockaddr *)&serverPAddress, sizeof(struct sockaddr))) == -1) 
			{
				perror("sendto");
				exit(1);
			}

			if ((numbytes = sendto(centralServerSocketUDP, userNameB1, strlen(userNameB1), 0,(struct sockaddr *)&serverPAddress, sizeof(struct sockaddr))) == -1) 
			{
				perror("sendto");
				exit(1);
			}
			// sending subgraph containing the users (Topology info) and scores to serverP

			sendUDPData(responseT1, centralServerSocketUDP ,serverPAddress);
			sendUDPData(responseS1, centralServerSocketUDP ,serverPAddress);
			cout<<"The Central server sent a processing request to Backend-Server P."<<endl;



			responsePathAtoB1 =  receiveUDPData(centralServerSocketUDP,serverPAddress);// receiving path having minimum matching gap from serverP
			responsePathB1toA = receiveUDPData(centralServerSocketUDP,serverPAddress);

			char totalCost[16] = "";
			// receiving the cost (matching gap) info from serverP
			if((numbytes=recvfrom(centralServerSocketUDP, totalCost, sizeof(totalCost) , 0,(struct sockaddr *)&serverPAddress, &addr_len)) == -1) 
			{
				perror("recvfrom");
				exit(1);
			}

			cout<<"The Central server received the results from backend server P."<<endl;
			totalCostStr1  = totalCost;

		}

    	uA  = userNameA;
    	uB  = userNameB;
    	uB1 = userNameB1;

    	if(UserB1Exists == false)
    	{// responses for client-A and client-B if userNameB1 (additional user sent by client-B) does not exist
    		
	    		responseForAStr = "Found compatibility for "+uA+" and "+uB+":\n"+responsePathAtoB+"\nMatching Gap : "+totalCostStr;
	    		responseForBStr = "Found compatibility for "+uB+" and "+uA+":\n"+responsePathBtoA+"\nMatching Gap : "+totalCostStr;

    	}
    	else
    	{// 4 conditions when userNameB1 exists. One of the 4 has been handled previously
	    	if(ABPath == true && AB1Path == true)
	    	{// Path from userNameA and userNameB exists & Path from userNameA and userNameB1 exists
	    		responseForAStr = "Found compatibility for "+uA+" and "+uB+":\n"+responsePathAtoB+"\nMatching Gap : "+totalCostStr + "\n";
	    		responseForAStr += "Found compatibility for "+uA+" and "+uB1+":\n"+responsePathAtoB1+"\nMatching Gap : "+totalCostStr1;
	    		responseForBStr = "Found compatibility for "+uB+" and "+uA+":\n"+responsePathBtoA+"\nMatching Gap : "+totalCostStr + "\n";
	    		responseForBStr += "Found compatibility for "+uB1+" and "+uA+":\n"+responsePathB1toA+"\nMatching Gap : "+totalCostStr1;
	    	}
	    	else if(ABPath == true && AB1Path == false)
	    	{// Path from userNameA and userNameB exists & Path from userNameA and userNameB1 does not exist
	    		responseForAStr = "Found compatibility for "+uA+" and "+uB+":\n"+responsePathAtoB+"\nMatching Gap : "+totalCostStr + "\n";
	    		responseForAStr += "Found no compatibility for "+uA+" and "+uB1;
	    		responseForBStr = "Found compatibility for "+uB+" and "+uA+":\n"+responsePathBtoA+"\nMatching Gap : "+totalCostStr + "\n";
	    		responseForBStr += "Found no compatibility for "+uB1+" and "+uA;
	    		

	    	}
	    	else if(ABPath == false && AB1Path == true)
	    	{// Path from userNameA and userNameB does not exist & Path from userNameA and userNameB1 exists
	    		responseForAStr = "Found no compatibility for "+uA+" and "+uB+"\n";
	    		responseForAStr += "Found compatibility for "+uA+" and "+uB1+":\n"+responsePathAtoB1+"\nMatching Gap : "+totalCostStr1 + "\n";
	    		responseForBStr = "Found no compatibility for "+uB+" and "+uA+"\n";
	    		responseForBStr += "Found compatibility for "+uB1+" and "+uA+":\n"+responsePathB1toA+"\nMatching Gap : "+totalCostStr1 + "\n";


	    	}
	    }
    	
    	char* responseForA = NULL;
    	string tempStrAB(responseForAStr);
    	responseForA = &tempStrAB[0];

    	char* responseForB = NULL;
    	string tempStrBA(responseForBStr);
    	responseForB = &tempStrBA[0];


		if (send(clientAChildSocket, responseForA, strlen(responseForA), 0) == -1)
		{// Sending the response to clientA
			perror("send");
		}

		cout<<"The Central server sent the results to client A."<<endl;

		if (send(clientBChildSocket, responseForB, strlen(responseForB), 0) == -1)
		{// Sending the response to client-B
			perror("send");
		}

		cout<<"The Central server sent the results to client B."<<endl;

	}
	

	close(centralServerSocketA);
	close(centralServerSocketB);
	return 0;
}

// resued function from Beej's text
void sigchld_handler(int s)
{
        while(waitpid(-1, NULL, WNOHANG) > 0);
}


// This function sends a string data in chunks via UDP protocol
void sendUDPData(string data, int socketDescp ,struct sockaddr_in socketAddr)
{
	int bytesSent = 0;
	int chunksSent = 0;
	int totalChunks = 0;
	int totalSize = data.length();
	int numbytes;
	string temp;
	
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
			temp = data.substr(bytesSent,CHUNK_SIZE); // string data of size CHUNK_SIZE
		}
		
		char* response;
    	string tempStr(temp);
    	response = &tempStr[0];
    	int numbytes;
		if ((numbytes = sendto(socketDescp, response, strlen(response), 0,(struct sockaddr *)&socketAddr, sizeof(struct sockaddr))) == -1) // sending data of size CHUNK_SIZE
		{
			perror("sendto");
			exit(1);
		}
		chunksSent += 1;
	    bytesSent+=CHUNK_SIZE;

	}

	string escape = "$#$"; // An escase string is sent at the last to let the receiver know that the sender is done with sending data
	char* escapeChr;
	string tempChr(escape);
	escapeChr = &tempChr[0];

	if ((numbytes = sendto(socketDescp, escapeChr, strlen(escapeChr), 0,(struct sockaddr *)&socketAddr, sizeof(struct sockaddr))) == -1) // sending the escape char
	{
		perror("sendto");
		exit(1);
	}
}


// Function that receive data in chunks, adds all the chunks to make the final string data  
string receiveUDPData( int socketDescp ,struct sockaddr_in socketAddr )
{
	int numbytes;

	string data ("");

	socklen_t addr_len;
	addr_len = sizeof(struct sockaddr);

	while(1)
	{
		char rsp[512]  = "";
		if((numbytes=recvfrom(socketDescp, rsp, sizeof(rsp) , 0,(struct sockaddr *)&socketAddr, &addr_len)) == -1) // receiving data in chunks
		{
			perror("recvfrom");
			exit(1);
		}


		string temp1(rsp);

		if(temp1.compare("$#$") == 0) // checking for the escape string
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


// A function which sends special messages to serverS and serverP indicating that the graph is NULL 
void sendExitMsgToSP( int centralServerSocketUDP ,struct sockaddr_in serverSAddress, struct sockaddr_in serverPAddress)
{
	string exit = "###";
	string exitRespS = "";
	string exitRespP = "";
	cout<<"The Central server sent a request to Backend-Server S."<<endl;
	sendUDPData(exit, centralServerSocketUDP ,serverSAddress);
	exitRespS = receiveUDPData(centralServerSocketUDP,serverSAddress);
	cout<<"The Central server received information from Backend-Server S using UDP over port "<<UDP_PORT<<"."<<endl;
	sendUDPData(exit, centralServerSocketUDP ,serverPAddress);
	cout<<"The Central server sent a processing request to Backend-Server P."<<endl;
	exitRespP = receiveUDPData(centralServerSocketUDP,serverPAddress);
	cout<<"The Central server received the results from backend server P."<<endl;
}

