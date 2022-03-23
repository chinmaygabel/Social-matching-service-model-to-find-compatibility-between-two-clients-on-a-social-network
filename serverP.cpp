
/*
Name: Chinmay Gabel
Socket Programming Project
*/

// Socket programming code learned and referred from 'Beej’s Guide to Network ProgrammingUsing Internet Sockets'

// importing header files
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
#include<fstream>
#include<sstream>
#include<map>
#include<vector>
#include<algorithm>
#include<cmath>
#include<limits.h>
using namespace std;


// named constants
#define SERVER_P_UDP 23475 // ServerP's UDP port for receiving data from central server
#define localhost "127.0.0.1" // localhost IP address
#define MAXBUFLEN 1024 // MAx buffer length for usernames
#define CENTRAL_SERVER_UDP 24475 // central server's UDP port for sending data
#define CHUNK_SIZE 500 // chunk size for sending UDP data

/* 
Adjacency list representation for the graph: The graph is represented in the form of an adj list in which the distinct nodes (or vertex) are stored as linked
list of type struct Connectivity (defined below). Each element in this node of type struct Connectivity contains two pointers: nextC and firstV. One points to 
the next node of type struct Connectivity (nextC) and the other one points to a linked list where the nodes are of type struct Vertex (defined below). 
The first element (firstV) of this linked list (of type struct Vertex) contains data associated with the corresponding struct Connectivity node (represents a 
node in the graph) The rest of the elements of this linked list are the nodes in the graph which have an edge with firstV (first element.

*/


struct Vertex
{
	string data;
	double weight; //  Matching gap b/w this and connected node (first node in adj list or 'firstV')
	struct Vertex* nextV;
		
};

struct Connectivity
{
	struct Connectivity* pi;
	double score;
	struct Connectivity* nextC;
	struct Vertex* firstV;
};


// A Simle linked list which would be used as a priority queue while performing Dijkstra's algorithm
struct DistList
{
	double cost;
	struct Connectivity* vertex;
	struct DistList* next;
};


// Function Declarations (defined and explained at the bottom)
int checkVertex(struct Connectivity *head, string data,double* scoreTmp);
int checkEdge(struct Vertex *head, string data);
struct Connectivity* insertVertex(struct Connectivity *head, string a, double score);
void insertEdgeHelper(struct Connectivity *head, string a, string b, double scoreA, double scoreB);
void insertEdge(struct Connectivity *head, string a, string b);
void printGraph(struct Connectivity* head);
struct Connectivity* getVertex(struct Connectivity* head,struct Vertex* node);
double stringToDouble(string s);
string doubleToString(double d);
struct Connectivity* getVertex(struct Connectivity* head, int data);
void printPath(struct Connectivity* head, string lastNode);
void dijkstra(struct Connectivity* head, string source);
struct DistList* insertAtStart(struct DistList *head, struct Connectivity* node, double cost);
struct DistList* popMinElement(struct DistList* head, struct DistList** deletedNode );
struct DistList* popMinElementHelper(struct DistList* head, struct DistList* popedNode);
string shortestPath(struct Connectivity* head, string lastNode);
void sendUDPData(string data, int socketDescp ,struct sockaddr_in socketAddr);
string receiveUDPData(int socketDescp ,struct sockaddr_in socketAddr);
void deleteGraph(struct Connectivity* head);
double round(double gap);


// Global variable
map<struct Connectivity*,double> distanceMap;




int main()
{
	cout<<"The ServerP is up and running using UDP on port "<<SERVER_P_UDP<<"."<<endl;
	int serverPSocket;
	struct sockaddr_in serverPAddr; // serverP's address information
	struct sockaddr_in clientAddr; // clients’s address information
	struct sockaddr_in centralServerAddress;
	socklen_t addr_len;
	int numbytes = 1024;
	char userName1[MAXBUFLEN];
	char userName2[MAXBUFLEN];
	//char nodeScoresChar[MAXBUFLEN];
	//char nodeEdgesChar[MAXBUFLEN];

	if((serverPSocket = socket(PF_INET, SOCK_DGRAM, 0)) == -1) 
	{
		perror("socket");
		exit(1);
	}

	// Setting serverP's UDP socket address for receiving data from central server
	memset(&serverPAddr.sin_zero, 0, sizeof(serverPAddr));
	serverPAddr.sin_family = AF_INET;// host byte order
	serverPAddr.sin_port = htons(SERVER_P_UDP);// short, network byte order
	serverPAddr.sin_addr.s_addr = inet_addr(localhost); // localhost IP address
	
	// Setting up central server's UDP socket address for sending data to central server
	memset(&centralServerAddress.sin_zero, 0, sizeof(centralServerAddress));
	centralServerAddress.sin_family = AF_INET;// host byte order
	centralServerAddress.sin_port = htons(CENTRAL_SERVER_UDP);// short, network byte order
	centralServerAddress.sin_addr.s_addr = inet_addr(localhost); // automatically fill with my IP
	
	// binding socket descriptors and socket addresses
	if(bind(serverPSocket, (struct sockaddr *)&serverPAddr,sizeof(struct sockaddr)) == -1) 
	{
		perror("bind");
		exit(1);
	}

	int i=0;
	vector <string> scores; // vector that would store the score info of the users 
	//vector <vector<string> > scorePairs; // 
	vector <string> edges;// vector that would store the edges (users seperated by space)
	vector <vector<string> > edgePairs; // vector of vectors containg edge pairs (two users having a direct edge)
	vector <string> temp; // temp string vector
	addr_len = sizeof(struct sockaddr);


	while(1)
	{

		// Initializing variables
		char userName1[MAXBUFLEN] = ""; // 1st user
		char userName2[MAXBUFLEN] = ""; // 2nd user
		struct Connectivity* head; // head of the adj list
		head = NULL;
		string nodeEdges(""); // variable that stores edge info of the graph
		string nodeScores(""); // variable that stores score info
		string responseAPath(""); // variable that stores path info for 1st user
		string responseBPath(""); // variable that stores path info for 2nd user
		string userNameA(""); // string variable that stores 1st user
		string userNameB("");  // string variable that stores 2nd user
		string tempStrCost(""); // temp string variable which stores the total minimum cost
		string graphExists(""); // variable that would contain either "###" (graph doesnot exist) or "111" 

		graphExists = receiveUDPData(serverPSocket,clientAddr); // receiving data from central server regarding the graph existence
		if(graphExists.compare("###") == 0) // checking if the graph exists or not 
		{
			cout<<"The ServerP received the topology and score information."<<endl;
			string exit = "###";
			sendUDPData(exit, serverPSocket, centralServerAddress);
			cout<<"The ServerP finished sending the results to the Central."<<endl;
			scores.clear();
			//scorePairs.clear();
			edges.clear();
			edgePairs.clear();
			distanceMap.clear();
			deleteGraph(head);
			head = NULL;
			tempStrCost= "";
			graphExists = "";
			continue;
		}


		// receiving the usernames from the central server

		if((numbytes=recvfrom(serverPSocket, userName1, MAXBUFLEN-1 , 0,(struct sockaddr *)&clientAddr, &addr_len)) == -1) 
		{
			perror("recvfrom");
			exit(1);
		}

		userNameA.append(userName1);

		if((numbytes=recvfrom(serverPSocket, userName2, MAXBUFLEN-1 , 0,(struct sockaddr *)&clientAddr, &addr_len)) == -1) 
		{
			perror("recvfrom");
			exit(1);
		}

		userNameB.append(userName2);
		nodeEdges.append(receiveUDPData(serverPSocket,clientAddr)); // receiving edge info of the graph from the central server
		nodeScores.append(receiveUDPData(serverPSocket,clientAddr)); // receiving score info from the central server

		cout<<"The ServerP received the topology and score information."<<endl;


		string t ("");
		i =0;
		string d ("|");
		while(nodeScores[i] != '\0') // extracting score info from the received string and pushing it into the scores vector 
		{
			if(nodeScores[i] != d[0])
			{
				t += nodeScores[i];
				i++;
			}
			else
			{
				if((int) t.size()>0)
				{
					t+="\0";
					scores.push_back(t);

				}
				
				t = "";

				i++;

			}
		}


		if((int) t.size() > 0)
		{
			t+="\0";
			scores.push_back(t);
		}

		t = "";
		i =0;
		while(nodeEdges[i] != '\0') // extracting edge info from the received string and pushing it into the edges vector 
		{
			if(nodeEdges[i] != d[0])
			{
				t += nodeEdges[i];
				i++;
			}
			else
			{
				if((int) t.size()>0)
				{
					t+="\0";
					edges.push_back(t);

				}
				
				t = "";

				i++;

			}
		}

		

		if((int) t.size() > 0)
		{
			t+="\0";
			edges.push_back(t);
		}


		
		

		d = " ";
		
		int j;

		for(i=0;i<scores.size();i++)
		{
			t = "";
			string tempNodeScore = scores[i];
			j = 0;
			while(tempNodeScore[j] != '\0') // Extracting usernames and scores & creating a graph with the usernames and their scores
			{
				if(tempNodeScore[j] != d[0])
				{
					t += tempNodeScore[j];
					j++;
				}
				else
				{
					if((int) t.size()>0)
					{
						t+="\0";
						temp.push_back(t);
					}
					t = "";
					j++;
				}
			}
			if((int) t.size() > 0)
			{
				t+="\0";
				temp.push_back(t);
			}

			string nodeName(temp[0]);
			string scoreStr(temp[1]);
			double scr = stringToDouble(scoreStr);
			head = insertVertex(head,nodeName,scr); // inserting nodes in the graph
			temp.clear();

		}



		
		for(i=0;i<edges.size();i++) // extracting usernames and creating a weighted edge in the graph between the users
		{
			t = "";
			string tempEdges = edges[i];
			j = 0;
			while(tempEdges[j] != '\0')
			{
				if(tempEdges[j] != d[0])
				{
					t += tempEdges[j];
					j++;
				}
				else
				{
					if((int) t.size()>0)
					{
						t+="\0";
						temp.push_back(t);
					}
					t = "";
					j++;
				}
			}
			if((int) t.size() > 0)
			{
				t+="\0";
				temp.push_back(t);
			}

			string nodeName1 = temp[0];
			string nodeName2 = temp[1];

			insertEdge(head,nodeName1,nodeName2); // inserting edge in the graph
			temp.clear();
		}


		dijkstra(head,userNameB); // Performing Dijkstra's algorithm on the graph with source as 2nd user
		responseAPath.append(shortestPath(head,userNameA)); // path info with minimum matching gap from 1st user to 2nd user


		struct Connectivity* headCpy;
		headCpy = head;
		while(headCpy->firstV->data != userNameA)
		{
			headCpy = headCpy->nextC;
		}
		double totalCost = distanceMap[headCpy]; // total minimum matching gap

		distanceMap.clear();

		dijkstra(head,userNameA);  // Performing Dijkstra's algorithm on the graph with source as 1st user
		responseBPath.append(shortestPath(head,userNameB)); // path info with minimum matching gap from 2nd user to 1st user


		
	    char* responseTotalCostChr = NULL;
	    tempStrCost = doubleToString(round(totalCost));
	    responseTotalCostChr = &tempStrCost[0];

	    // sending path data and matching gap data back to the central server
		sendUDPData(responseAPath, serverPSocket, centralServerAddress);
		sendUDPData(responseBPath, serverPSocket,centralServerAddress);

		if ((numbytes = sendto(serverPSocket, responseTotalCostChr, strlen(responseTotalCostChr), 0,(struct sockaddr *)&clientAddr, sizeof(struct sockaddr))) == -1)
		{
			perror("sendto");
			exit(1);
		}

		cout<<"The ServerP finished sending the results to the Central."<<endl;

		// resetting all the variables

		scores.clear();
		//scorePairs.clear();
		edges.clear();
		edgePairs.clear();
		distanceMap.clear();
		deleteGraph(head);
		head = NULL;
		totalCost = 0;
		tempStrCost= "";
	}
	

	close(serverPSocket);
	return 0;
}



// Function Definitions 

// A function to parse a double from a string
double stringToDouble(string s)
{
     double d;
     stringstream ss;
     ss.str(s); //convery string into a stream
     ss >> d; 
     ss.str().clear();
     return d;
}

// A function to convert a double to a string
string doubleToString(double d)
{
	ostringstream ss;
	ss<<d;
	string s = ss.str();
	ss.clear();
	return s;
}



// Checks if the Vertex or node is already present in the graph, returns 1 if vervex is already present, otherwise returns 0
int checkVertex(struct Connectivity *head, string data, double* scoreTmp)
{
	if (head == NULL)
	{
		*scoreTmp = -1;
		return(0);
		
	}
	else
	{
		while(head != NULL)
		{
			if(head->firstV->data == data)
			{
				*scoreTmp = head->score; 
				return(1);
				
			}
			head = head->nextC;
		}
		*scoreTmp = -1;
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
struct Connectivity* insertVertex(struct Connectivity *head, string a, double score)
{
	int checkStatus;
	double scoreVert;
	checkStatus = checkVertex(head,a,&scoreVert); // checks if the vertex is already present in the graph - dublicte entries are not allowed
	if(checkStatus == 0)
	{
		if(head == NULL) // Empty graph
		{
			struct Vertex* temp1;
			temp1 = new Vertex();
			if(temp1 == NULL) {
			}
			temp1->data = a;
			temp1->weight = -1;
			temp1->nextV = NULL;
			struct Connectivity* temp2;
			temp2 = new Connectivity();
			if(temp2 == NULL) {
			}
			temp2->nextC = NULL;
			temp2->firstV = temp1;
			temp2->score = score;
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
			temp1->weight = -1;
			temp1->nextV = NULL;
			struct Connectivity* temp2;
			temp2 = new Connectivity();
			temp2->nextC = NULL;
			temp2->firstV = temp1;
			temp2->score = score;
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
void insertEdgeHelper(struct Connectivity *head, string a, string b, double scoreA, double scoreB)
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
		double weightAB = ( (double) abs(scoreA - scoreB))/((double)(scoreA+scoreB)); // calculating the matching gap
		temp1->weight = weightAB;
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
	double aScore,bScore;
	checkStatusA = checkVertex(head,a, &aScore);
	checkStatusB = checkVertex(head,b, &bScore);

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
		insertEdgeHelper(head,a,b,aScore,bScore); // adding edge a to b (it's an undirected graph)
		insertEdgeHelper(head,b,a,bScore,aScore); // adding edge b to a
	}

}

// Function which prints the graph (used for debugging)
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
			cout <<head->firstV->data<<" "<<head->score <<": " ;
			temp1 = head->firstV;
			temp1 = temp1->nextV;
			while(temp1 != NULL)
			{
				cout << temp1->data << "->"<< temp1->weight<<" ";
				temp1 = temp1->nextV;
			}
			printf("\n");
			head = head->nextC;
		}
	}
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

// Utility function which gets a pointer to Node of type Connectivity, given a string data corresponding to a graph node 
struct Connectivity* getVertex(struct Connectivity* head, string data)
{
	while(head!= NULL)
	{
		if(head->firstV->data == data)
		{
			return(head);
		}
		head = head->nextC;
	}

	return NULL;
}



void printPath(struct Connectivity* head, string lastNode)
{
 	while (head->firstV->data != lastNode)
 	{
 	 	head = head->nextC;
	}

	while (head != NULL)
	{
		cout << head->firstV->data<<endl;
		head = head->pi;
	}
 }


// This function is used to get the shortest path from any node to the source node (node on which dijksta is performed) 
string shortestPath(struct Connectivity* head, string lastNode)
{
	string t = "";
 	while (head->firstV->data != lastNode)
 	{
 	 	head = head->nextC;
	}

	while (head != NULL)
	{
		if(head->pi == NULL) // pi is a pointer to the previous node in the shortest route 
		{
			t = t+ head->firstV->data ;
		}
		else
		{
			t = t+ head->firstV->data + "--"; // storing the nodes in the path seperated by '--'
		}
		
		head = head->pi;

	}
	return t;
 }

// This function implements the Dijkstra's algorithm to figure out the path having smallest matching gap 
// between a source node to every other node in the graph. 
void dijkstra(struct Connectivity* head, string source)
{
	struct Connectivity* headCpy,*sourceNode;
	headCpy =head;
	struct DistList* headL; // It's the head of a linked list which would used as a priority queue
	headL = NULL;
	sourceNode = getVertex(headCpy, source); // getting the node in the adj list which correspods to the source node


	while(headCpy!=NULL)
	{
		if(headCpy->firstV->data == source)
		{
			sourceNode->pi = NULL; // making pointer to previous node NULL for source node 
			distanceMap[headCpy]=0;// For source node, make the distance 0 (distance to itself is 0)
		}
		else
		{
			headCpy->pi = NULL; // // making pointer to previous node NULL for source node for every node in the weighted graph
			distanceMap[headCpy] = INT_MAX; // Initially distance of source node to every other node is INF.
		}
		headCpy=headCpy->nextC;
	}
	headCpy = head;

	headL = insertAtStart(headL,sourceNode,distanceMap[sourceNode]); // Inserting source node in the priority queue

	while(headL != NULL)
	{
		struct DistList* deletedNode;
		headL = popMinElement(headL,&deletedNode); // Popping out the node having the minimum cost or the matching gap with the source node
		double currCost = deletedNode->cost;
		struct Connectivity* delNode;
		delNode = deletedNode->vertex;
		delete(deletedNode);

		struct Vertex* adjDelNode;
		adjDelNode = delNode->firstV->nextV;


		while(adjDelNode!=NULL) // Loop runs for all neighbours of the popped element.
		{
			struct Connectivity* u;
			struct Connectivity* v;
			u = getVertex(head,delNode->firstV); 
			v = getVertex(head, adjDelNode);

			if(adjDelNode->weight + distanceMap[u] < distanceMap[v]) // Dijkstra's Algorithm -> checking if the new distance (matching gap) is less than the previous distance
			{
				distanceMap[v] = adjDelNode->weight + distanceMap[u]; // updating the new distance (or the matching gap)
				headL = insertAtStart(headL,v,distanceMap[v]); // if the new distance is less then the previous distance, insert it priority queue
				v->pi = u; // node u is preceded by node v in the shortest path 
			}
			adjDelNode = adjDelNode->nextV;
		} 
	}
}

// function which inserts a node of type struct DistList at the start of the linked list (of type struct DistList)
struct DistList* insertAtStart(struct DistList *head, struct Connectivity* node, double cost)
{
	struct DistList *temp;
	temp = new DistList();
	temp->vertex = node;
	temp->cost = cost;			
	temp->next = head;
	head = temp;
	return head;
}


// Function that fetches the node of type struct DistList from the linked list with the minimum cost.  
struct DistList* popMinElement(struct DistList* head, struct DistList** deletedNode )
{
	double min = INT_MAX;
	struct DistList* headCpy;
	struct DistList* minNode = NULL;
	headCpy = head;

	while(headCpy != NULL)
	{
		if(headCpy->cost < min) // Identifying the minimum cost (matching gap) node in the linked list
		{
			min = headCpy->cost;
			minNode = headCpy;
		}
		headCpy = headCpy->next;
	}


	headCpy = popMinElementHelper(head,minNode); // calling the helper function to remove the minimum cost node from the linked list
	*deletedNode = minNode; // the calling function gets the minNode
	return headCpy;
}

// Helper function for popMinElement function which romoves the node with minimum cost from the linked list 
struct DistList* popMinElementHelper(struct DistList* head, struct DistList* popedNode)
{
	struct DistList* headCpy, *temp;
	headCpy = head;
	if(head == NULL)
	{
		return head;
	}
	else if(head == popedNode)
	{
		head = head->next;
		return head;
	}
	else
	{
		while(head->next != NULL)
		{
			if(head->next == popedNode)
			{
				temp = head->next;
				head->next = temp->next; // removing (skipping the connection) the popedNode from the linked list
				return(headCpy);

			}
			head = head->next;
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
	else{
		totalChunks = (totalSize/CHUNK_SIZE) + 1;
	}


	// Sending string data in chunks
	while(chunksSent < totalChunks)
	{
		if(totalSize - bytesSent < CHUNK_SIZE){
			temp = data.substr(bytesSent);
		}

		else{
			temp = data.substr(bytesSent,CHUNK_SIZE); // string data of size CHUNK_SIZE
		}
		
		char* response = NULL;
    	string tempStr(temp);
    	response = &tempStr[0];
    	int numbytes;
		if ((numbytes = sendto(socketDescp, response, strlen(response), 0,(struct sockaddr *)&socketAddr, sizeof(struct sockaddr))) == -1) { // sending data of size CHUNK_SIZE
			perror("sendto");
			exit(1);
		}
		chunksSent += 1;
	    bytesSent+=CHUNK_SIZE;
	    temp = "";

	}

	string escape = "$#$"; // An escase string is sent at the last to let the receiver know that the sender is done with sending data
	char* escapeChr = NULL;
	string tempChr(escape);
	escapeChr = &tempChr[0];

	if ((numbytes = sendto(socketDescp, escapeChr, strlen(escapeChr), 0,(struct sockaddr *)&socketAddr, sizeof(struct sockaddr))) == -1) {  // sending the escape char
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

	while(1){
		char rsp[512]  = "";
		if((numbytes=recvfrom(socketDescp, rsp, sizeof(rsp) , 0,(struct sockaddr *)&socketAddr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}

		string temp1(rsp);
		if(temp1.compare("$#$") == 0)  // checking for the escape string
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


// function for deleting the entire graph and free up the allocated memory
void deleteGraph(struct Connectivity* head)
{
	if(head == NULL)
	{
		return;
	}
	else
	{
		while(head != NULL)
		{
			struct Connectivity* headCpy = head;
			struct Vertex* temp1;
			temp1 = head->firstV;
			while(temp1 != NULL)
			{
				struct Vertex* temp2;
				temp2 = temp1;
				temp1 = temp1->nextV;
				delete(temp2);
			}
			head = head->nextC;
			delete(headCpy);
		}
	}
}


// Rounds a double to 2 decimal places
double round(double gap)
{

    double gapRounded = (int)(gap * 100 + .5);
    return (double)gapRounded / 100;
}
 
