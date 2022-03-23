
/*
Name: Chinmay Gabel
Socket Programming Project
*/

// Socket programming code learned and referred from 'Beej’s Guide to Network ProgrammingUsing Internet Sockets'


// header files
#include <iostream>
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
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <algorithm>

using namespace std;

// named constants
#define SERVER_T_UDP 21475 // serverT's UDP port for receiving data from central server
#define localhost "127.0.0.1" // localhost IP address
#define MAXBUFLEN 256 // max lenght of the usernames
#define CHUNK_SIZE 500 // chunk size for sending UDP data


/* 
Adjacency list representation for the graph: The graph is represented in the form of an adj list in which the distinct nodes (or vertex) are stored as linked
list of type struct Connectivity (defined below). Each element in this node of type struct Connectivity contains two pointers: nextC and firstV. One points to 
the next node of type struct Connectivity (nextC) and the other one points to a linked list where the nodes are of type struct Vertex (defined below). 
The first element (firstV) of this linked list (of type struct Vertex) contains data associated with the corresponding struct Connectivity node (represents a 
node in the graph) The rest of the elements of this linked list are the nodes in the graph which have an edge with firstV (first element).

*/

struct Vertex
{
	string data; // username data
	struct Vertex* nextV;
	
};
 
struct Connectivity
{
	struct Connectivity* nextC; // points to the next node of type struct Connectivity
	struct Vertex* firstV; // points to the head of the linked list where the nodes are of the type struct 
	bool visited; // used in DFS to check whether this node has been previously visited or not
};

// Function Declaration (defined and explained at the bottom)
int checkVertex(struct Connectivity *head, string data);
int checkEdge(struct Vertex *head, string data);
struct Connectivity* insertVertex(struct Connectivity *head, string a);
void insertEdgeHelper(struct Connectivity *head, string a, string b);
void insertEdge(struct Connectivity *head, string a, string b);
void printGraph(struct Connectivity* head);
void DFS(struct Connectivity *head);
void DFSHelper(struct Connectivity *head, struct Connectivity *node);
struct Connectivity* getVertex(struct Connectivity* head,struct Vertex* node);
void sendUDPData(string data, int socketDescp ,struct sockaddr_in socketAddr);



// Global Variables
string subGraphs[1000]; // String array that stores nodes in different subgraphs. The dirrefent subgraphs are seperated by '|'. 
int subGraphsCounter = 0; // Stores the number of elements in subGraphs string Array. 



int main()
{
	cout<<"The ServerT is up and running using UDP on port "<<SERVER_T_UDP<<"."<<endl;
	int serverTSocket;
	struct sockaddr_in serverTAddr; // address information for serverT
	struct sockaddr_in clientAddr; // address information for the connector (central server)
	socklen_t addr_len;
	int numbytes = 256;
	char userName1[MAXBUFLEN];
	char userName2[MAXBUFLEN];

	// getting socket descriptor
	if((serverTSocket = socket(PF_INET, SOCK_DGRAM, 0)) == -1) { 
		perror("socket");
		exit(1);
	}


	// Setting up serverT's socket address created for communicating with clientA
	memset(&serverTAddr.sin_zero, 0, sizeof(serverTAddr));
	serverTAddr.sin_family = AF_INET;// host byte order
	serverTAddr.sin_port = htons(SERVER_T_UDP);// short, network byte order
	serverTAddr.sin_addr.s_addr = inet_addr(localhost); // setting the IP address to local host



	// binding socket descriptor and socket address

	if(bind(serverTSocket, (struct sockaddr *)&serverTAddr,sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(1);
	}

	addr_len = sizeof(struct sockaddr);


	struct Connectivity* head; // Head pointer for the adj list

	head = NULL;

	ifstream edgeFile;
	edgeFile.open("edgelist.txt"); // opening the file containg the edges
	if(edgeFile.is_open())
	{
		string fileLine;
		while(getline(edgeFile, fileLine)) // reading from the file line by line
		{
			string firstNode = "";
			string secondNode = "";
			int i =0;

			while(fileLine[i] != ' ')
			{
				firstNode += fileLine[i]; // constructing first node char by char
				i++;
			}
			head = insertVertex(head,firstNode); // Inserting first node in the adj list representation of the graph


			while(fileLine[i] != '\0') 
			{
				if(fileLine[i] == ' ')
				{
					i++;
					continue;
				}

				secondNode += fileLine[i]; // constructing second node char by char
				i++;
			}
			head = insertVertex(head,secondNode);  // Inserting second node in the adj list representation of the graph
			insertEdge(head,firstNode,secondNode); // adding edge between the first node and the second node

		}
	}

	// Till now the graph has been successfully constructed 
	edgeFile.close(); // closing the file



	// Running DFS on the graph to get the different connected components (different subgraphs).
	DFS(head);

	while(1)
	{
		char userName1[MAXBUFLEN] = "";
		char userName2[MAXBUFLEN] = "";


		if((numbytes=recvfrom(serverTSocket, userName1, MAXBUFLEN-1 , 0,(struct sockaddr *)&clientAddr, &addr_len)) == -1) { // receiving 1st username from central server
			perror("recvfrom");
			exit(1);
		}

		if((numbytes=recvfrom(serverTSocket, userName2, MAXBUFLEN-1 , 0,(struct sockaddr *)&clientAddr, &addr_len)) == -1) {//receiving 2nd username from central server
			perror("recvfrom");
			exit(1);
		}
		cout<<"The ServerT received a request from Central to get the topology."<<endl;

		string userNameA(userName1); // Converting char Array into C++ string
		string userNameB(userName2);


		bool user1flag = false; // Flag for the presence of 1st user in a subgraph
		bool user2flag = false; // Flag for the presence of 2nd user in a subgraph
		bool boothUserflag = false; // Flag for the presence of both the users in a subgraph


		int j =0;
		j =0;
		vector<string> mainSubGraph; // the vector which would ultimately contain users in the subgraph having the two users
		while(j<subGraphsCounter)
		{
			if(subGraphs[j] != "|")
			{
				mainSubGraph.push_back(subGraphs[j]);

				if(subGraphs[j] == userNameA)
				{
					user1flag = true;
				}
				if(subGraphs[j] == userNameB)
				{
					user2flag = true;
				}

				if(user1flag == true && user2flag == true)
				{
					boothUserflag = true;

				}
				j+=1;
			}
			

			if(subGraphs[j]== "|") //Checking end of a subgraph
			{
				if(boothUserflag == true)
				{
					break; // if both the users are present in that particular subgraph, then no need to process other subgraphs
				}
				else if ((user1flag == true && user2flag == false) || (user1flag == false && user2flag == true))
				{// If any one of the user exist in a subgraph but not the other, then there is no path b/w those two users 
					break;
				}
				else
				{ // Else keep checking in the other subgraphs for two users
					mainSubGraph.clear();
					bool user1flag = false;
					bool user2flag = false;
					bool boothUserflag = false;
					j+=1;

				}
				
			}
		}


		if(boothUserflag == false)
		{ // Checking the 'No path b/w the two users' scenerio and if this scenerio exits, communicating it back to the central server
			string exit = "###";

			sendUDPData(exit, serverTSocket ,clientAddr); // Sending UDP data to central server
			cout<<"The ServerT finished sending the topology to Central."<<endl;
			continue; // Skipping the rest of the while loop

		}

		string subGraphNodes = "";

		for(int i= 0;i<mainSubGraph.size();i++)
		{
			if(i != mainSubGraph.size()-1)
			{
				subGraphNodes += mainSubGraph[i] + " ";
			}
			else
			{
				subGraphNodes += mainSubGraph[i];

			}
			
		}
		
		vector <string> edgePairs; // temp string vector which would contain a pair of edge at a time. (2 entries)
		vector<vector<string> > mainSubGraphEdges; // Vector which would contain different vectors of edge pairs in the main sub graph i.e, graph containing the two users
		vector <string> reverseEdgePairs; // temp string vector which would contain edgePairs's data in reverse. This would be used for checking entries

		struct Connectivity* headCpy;
		headCpy = head;

		for (int i = 0; i < mainSubGraph.size(); i++)
		{// Searching in graph for each node in the mainSubGraph vector 
			while(headCpy != NULL)
			{ 
				struct Vertex* temp1;
				if(mainSubGraph[i] == headCpy->firstV->data) // checking the adj list for the node
				{ 
					
					temp1 = headCpy->firstV;
					temp1 = temp1->nextV;
					while(temp1 != NULL)
					{ // getting all the other nodes connected to ith element in the mainSubGraph
						edgePairs.push_back(mainSubGraph[i]);
						edgePairs.push_back(temp1->data);
						reverseEdgePairs.push_back(temp1->data);
						reverseEdgePairs.push_back(mainSubGraph[i]);
						if(find(mainSubGraphEdges.begin(),mainSubGraphEdges.end(),reverseEdgePairs) == mainSubGraphEdges.end()) 
						{ // checking for dublicates (same entry but reversed), and adding the edge if no duplicate is present
							mainSubGraphEdges.push_back(edgePairs);
						}
						temp1 = temp1->nextV;
						edgePairs.clear();
						reverseEdgePairs.clear();
					}

					headCpy = head; // added
					break; // added


				}
				headCpy = headCpy->nextC;


			}
			headCpy = head;
		}
		string graphData = ""; // The sting that would contain the edges (nodes seperated by " "), different edge pairs would be seperated by '|'  


		for (int i = 0; i < mainSubGraphEdges.size(); i++)
		{
			for (int j = 0; j < mainSubGraphEdges[i].size();j++)
			{
				if(j == mainSubGraphEdges[i].size()-1)
				{
					graphData += mainSubGraphEdges[i][j]; // building the graphData string character by character

				}
				else
				{
					graphData += mainSubGraphEdges[i][j] + " "; // two nodes in a pair are seperated by " "
				}
				
			}
			if(i !=  mainSubGraphEdges.size()-1)
			{
				graphData += "|"; //different edge pairs would be seperated by '|'
			}
			
		}

		char* response;
	    string tempStr(graphData);
	    response = &tempStr[0];

		sendUDPData(graphData, serverTSocket ,clientAddr); // sending the graph data back to the central server
		cout<<"The ServerT finished sending the topology to Central."<<endl;

		mainSubGraph.empty();
		edgePairs.empty();
		reverseEdgePairs.empty();


	}
	
	close(serverTSocket);
	return 0;
}


// Function Definitions 


// Checks if the Vertex or node is already present in the graph, returns 1 if vervex is already present, otherwise returns 0
int checkVertex(struct Connectivity *head, string data)
{
	if (head == NULL)
	{
		return(0);
	}
	else
	{
		while(head != NULL)
		{
			if(head->firstV->data == data)
			{
				return(1);
			}
			head = head->nextC;
		}
		return(0);
	}
}



// Checks if the edge is already present in the graph, returns 1 if edge is already present, otherwise returns 0
int checkEdge(struct Vertex *head, string data)
{
	while(head != NULL)
	{
		if(head->data == data)
		{
			return(1);
		}
		head = head->nextV;
	}
	return(0);
}


// Function for inserting a new vertex in the graph
struct Connectivity* insertVertex(struct Connectivity *head, string a)
{
	int checkStatus;
	checkStatus = checkVertex(head,a); // checks if the vertex is already present in the graph - dublicte entries are not allowed
	if(checkStatus == 0)
	{
		if(head == NULL) // Empty graph
		{
			struct Vertex* temp1;
			temp1 = new Vertex();
			temp1->data = a;
			temp1->nextV = NULL;
			struct Connectivity* temp2;
			temp2 = new Connectivity();
			temp2->nextC = NULL;
			temp2->firstV = temp1;
			return(temp2);
		}
		else // Adding vertex at the end of the Adj List
		{
			struct Connectivity* headCpy;
			headCpy = head;
			while(head->nextC != NULL) // traversing the adj list
			{
				head = head->nextC;
			}
			struct Vertex* temp1;
			temp1 = new Vertex();
			temp1->data = a;
			temp1->nextV = NULL;
			struct Connectivity* temp2;
			temp2 = new Connectivity();
			temp2->nextC = NULL;
			temp2->firstV = temp1;
			head->nextC = temp2;
			return(headCpy);
		}
	}
	else
	{
		return(head); // return head if vertex is already present in the graph
	}
}


// Helper function for insertEdge 
void insertEdgeHelper(struct Connectivity *head, string a, string b)
{
	int alreadyB;
	while(head->firstV->data != a)
	{
		head = head->nextC;
	}
	alreadyB = checkEdge(head->firstV,b); // checking if the edge already exists
	if(alreadyB == 0)
	{ // adding new edge entry in the adj list 
		struct Vertex* temp1,*temp2;
		temp1 = new Vertex();
		temp1->data = b;
		temp1->nextV = NULL;
		temp2 = head->firstV;
		while(temp2->nextV != NULL)
		{
			temp2 = temp2->nextV;
		}
		temp2->nextV = temp1;

	}
	else
	{ // if edge is already present - don't do anything 
		return;
	}
}



// Function for inserting an edge between the two nodes

void insertEdge(struct Connectivity *head, string a, string b)
{
	int checkStatusA, checkStatusB;
	checkStatusA = checkVertex(head,a);
	checkStatusB = checkVertex(head,b);

	if(head == NULL)
	{
		return;
	}

	else if((checkStatusA==0) || (checkStatusB==0)) // checking if any of the vertex or node is not present
	{
		return;
	}
	else
	{
		insertEdgeHelper(head,a,b); // adding edge a to b (it's an undirected graph)
		insertEdgeHelper(head,b,a); // adding edge b to a
	}

}

void printGraph(struct Connectivity* head)
{
	if(head == NULL)
	{
		return;
	}
	else
	{
		while(head != NULL)
		{
			struct Vertex* temp1;
			cout <<head->firstV->data<<": " ;
			temp1 = head->firstV;
			temp1 = temp1->nextV;
			while(temp1 != NULL)
			{
				cout << temp1->data <<" ";
				temp1 = temp1->nextV;
			}
			cout<<endl<<endl;
			head = head->nextC;
		}
	}
}


//Checks is a given vertex has already been visited before- This function is used in DFS

bool checkVertexVisited(struct Connectivity* head,struct Vertex* node)
{
	struct Connectivity* headCpy;
	headCpy = head;
	bool vist = false;

	while(headCpy!=NULL)
	{
		if(node->data == headCpy->firstV->data && headCpy->visited == true) // checking for 'visited' field in the Node of type Connectivity
		{
			vist = true;
			break;
		}
		headCpy = headCpy->nextC;
	}
	return vist;


}


// Utility function which gets a pointer to Node of type Connectivity, given a pointer to the node of type Vertex
struct Connectivity* getVertex(struct Connectivity* head,struct Vertex* node)
{
	struct Connectivity* headCpy;
	headCpy = head;

	while(headCpy!=NULL)
	{
		if(node->data == headCpy->firstV->data)
		{
			break;
		}
		headCpy = headCpy->nextC;
	}
	return headCpy;


}

// Helper function for DFS function
void DFSHelper(struct Connectivity *head, struct Connectivity *node)
{
	node->visited = true; // marking current node as visited
	if (node->firstV != NULL)
	{
		subGraphs[subGraphsCounter] = node->firstV->data; // adding node data to global subGraphs string array 
		subGraphsCounter+=1;
	}

	struct Vertex* adjIterator;
	adjIterator = node->firstV;
	adjIterator = adjIterator->nextV;

	while(adjIterator != NULL)
	{
		if(checkVertexVisited(head,adjIterator) == false)
		{
			DFSHelper(head,getVertex(head,adjIterator)); // recursively calling DFSHelper on the neighbouring nodes
		}
		adjIterator = adjIterator->nextV;
	}
	return;

}


// A function to perform Depth First Search on the graph to find out the different connected components
void DFS(struct Connectivity *head)
{
	struct Connectivity* headCpy;
	headCpy = head;

	while(headCpy!= NULL)
	{ // making 'visited' as false for every node 

		headCpy->visited = false;
		headCpy = headCpy->nextC;

	}
	headCpy = head;

	while(headCpy!=NULL)
	{
		if(headCpy->visited == false) // checking if the graph node is already visited
		{// calling DFSHelper, when this function is returned, all the nodes in a subgraph is visited and DFSHelper is called on a different subgraph (not yet visited)
			DFSHelper(head,headCpy); 
		}
		headCpy = headCpy->nextC;


		if( subGraphsCounter>0 && subGraphs[subGraphsCounter-1] != "|")
		{
			
			subGraphs[subGraphsCounter] = "|"; // Adding a delimiter to seperate the elements of different subgraphs.
			subGraphsCounter+=1;
			
		}
		

	}
}


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
			temp = data.substr(bytesSent,CHUNK_SIZE); // string data of size CHUNK_SIZE
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

	if ((numbytes = sendto(socketDescp, escapeChr, strlen(escapeChr), 0,(struct sockaddr *)&socketAddr, sizeof(struct sockaddr))) == -1) // sending the escape char
	{
		perror("sendto");
		exit(1);
	}

}
