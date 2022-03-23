Socket Programming Project

Full Name: Chinmay Gabel


Optional part (Extra credits):
I've completed the optional part and I would like to be considered for the extra credits.

Brief description of what I have done in the project:

The entire project has been coded in C++. There are 6 entities in this project namely clientA, clientB, central server, serverT, serverS and serverP.
Both the clients (clientA and clientB) only communicate with the central server over TCP connection and the serverT, serverS and serverP communicate 
with the central server over UDP. 


On booting up, the central servers listens to two TCP ports for the incomming message from the clients. Both clients send a username (clientB can send 
two usernames) to central server over TCP. 

For extra credits: The cleintB sends two usernames. If the second username doesnot exist, it indicates this to central server by sending a special message
("#$#$") inplace of the second username.  

The central server sends these two usernames to the serverT. The serverT needs to be booted up before the central server sends him the two username 
(at a time). Upon booting up, the serverT reads the edgelist.txt and builds un undirected non-weighted graph (more about the graph representation described later). In serverT, Depth First Search is used to find the different connected components in the graph.
Once different subgraphs are figured out, next task which is performed is to figure out in which subgraph, both the usernames exists. If both the 
usernames exist in any of the subgraph, the respective subgraph is sent to the central server. If the two usernames do not exist in a common subgraph,
a special message ("###") is sent to the central server indicating that the two users (or either one of tem) either do not exist in the entire graph or
they exist in different subgraphs and there is no way to make a connection between them. 

Once the central server receives the subgraph info from the serverT, it sends the subgraph data to the serverS. ServerS has to be already booted up by
the time central server sends it the subgraph data. Now, serverS reads scores.txt file and gets all the scores of the users present in the subgraph  
and sends the score info back to central server. If the graph is NULL, the central server sends a special message ("###") to serverS, indicating that
the graph is empty. The serverS in response also sends the same special message to central server indicating that no score info is available for that 
NULL graph.

The central server now has the usernames (received from the two clients), subgraph info (received from the serverT), and the score info (received from 
serverS). The Central server first communicates to serverP whether the subgraph exists or not (by sending a special message ("###") if the subgraph does
not exist and the serverP also sends the same special message to central server indicating that no info has been sent related to minimal cost path). 
If the subgraph exists, the central server sends all these info to the serverP. The serverP first creates an undirected weighted graph out of the 
info sent by the central server. The weight of this graph is the matching-gap between the two directly connected users in the graph. The matching gap is calculated  using the scores of the two directly connected users (matching gap = abs(s1-s2)/(s1+s2)), where s1 is the score of first user and s2 is the 
score of second user. Once the weighted graph is created, serverP executes Dijkstra's algorithm  to get the paths to all the other users from the source 
having the minimum total matching gap. The serverP then sends the path info and the total matching gap to the central server. Upon receiving the info from serverP, the central server creates a response for both the clients and send them their respective responses.




Breif description of each code file:


1.) clientA.cpp
This file represents the client side application wherein the script is run with the username as a command line argument. The username is then sent to the
central server over TCP and then waits for the response from the central server. Once the response is received, it prints the response on the console and
then exits the program.

2.) clientB.cpp
The functionality of this script is same as clientA.cpp, the only difference is the extra credit part where two users can be sent to the central server at
a time and both the clients (clientA and clientB) receive mimimal cost path to clientA from both the usernames sent by the cleintB. If the second username 
doesnot exist, clientB communicates this to central server by sending a special message("#$#$") inplace of the second username.  

3.) central.cpp
The central server is the main server which is responsible for coordinating and communicating information between the other entities in this ecosystem.
When it boots up, it binds up two TCP ports and one UDP port. The TCP ports are used for communicating with the clients and the UDP port is used for 
communicating with the other servers. Initially, it expects to receive messages from the two clients. Once it receives the usernames from the clients,
it sends it to the serverT. If the clientB sends two usernames say, usernameB and usernameB1 (extra credit) and let the username sent by the clientA is
usernameA, the central server sends the info to serverT for each pair i.e, (usernameA,usernameB) and (usernameA,usernameB1) gets the subgraph (containing
the two users) for each pair from serverT. The central server will send serverS the subgrah(s) and get back the score information(s). The central server 
will send subgraph for each pair to serverS if and only if usernameB1 exists (i.e, the special message "#$#$" is not sent by the clientB for usernameB1),
Otherwise the central server will send subgraph info for the pair (usernameA and usernameB) to the serverS and get back the score information. Now,
for each pair, the central server will first indicate serverP whether the subgraph exist or not and if it exists, it will send the usernames, subgraph and 
the score info to the serverP and get back the path info and respective minimum total matching gap from serverP. It will then finally relay back path and 
the minimum total cost info to the clients.

4.) serverT.cpp
The serverT is responsible for finding the connected component of a graph (subgraph) containg the two users. As mentioned above, upon booting up, the serverT reads the edgelist.txt and builds un undirected non-weighted graph.

The graph is represented in the form of an adjacency list in which the distinct nodes of the graph (or vertex) are stored as linked list of type struct Connectivity (user-defined data structure defined in the souce code file or the serverT.cpp file). Each element in this node of type struct Connectivity contains two pointers: nextC and firstV. One points to the next node of type struct Connectivity nextC) and the other one points to a linked list where the nodes are of type struct Vertex (defined below).  The first element (firstV) of this linked list (of type struct Vertex) contains data associated with the corresponding struct Connectivity node (represents a node in the graph) The rest of the elements of this linked list are the nodes in the graph which have an edge with firstV (first element).

serverT uses Depth First Search to find the different connected components in the entire graph. These subgraphs a stored in a string array containing 
the usernames of the graph. The delimeter used to seperate different subgraphs is '|' (pipe symbol). After figuring out different subgraphs, it is determined  in which subgraph both the usernames exists. If both the usernames exist in any of the subgraph, the respective subgraph is sent to the central server. If the two usernames do not exist in a common subgraph, a special message ("###") is sent to the central server indicating that the two users (or either one of tem) either do not exist in the entire graph or they exist in different subgraphs and there is no way to make a connection between them.


5.) serverS.cpp
serverS is responsible for providing the score information of all the users present in a graph (subgraph) to the central server. Once the subgraph is received by ServerS from the central server, serverS reads scores.txt file and gets all the scores of the users present in the subgraph and sends the score info back to central server. 

6.) serverP.cpp
serverP is responsible for finding out the path between two users in an undirected weighted graph having the minimum total matching gap. The graph representation is very similar to the one used in serverT, the only significant change is that the edges here would be the weighted edges having matching gap b/w two directly connected users as edge weights. 
serverP first receives a confirmation message from central server that the subgraph exists and if it does not exist, the serverP gets a special message ("###") indicating that subgraph doesnot exist for the pair and serverP responds back to central with the same special message signifying that no path info has been sent.

Otherwise, the serverP receives info regarding the usernames, subgraph and scores from the central server. The central server then creates weighted undirected graph out of the info sent by the central server. Once the weighted graph is created, serverP executes Dijkstra's algorithm to get the paths to all the other users from the source having the minimum total matching gap. The serverP then sends the path info (A to B & B to A) and the total matching gap (b/w A & B) to the central server


Format of all the messages exchanged:
1.) Messages exchanged b/w central server and clients

Printed on client's terminal when client sends username to central server:
"The client sent <INPUT1> to the Central server."

Printed on client's terminal when client receives info from central server:
"Found compatibility for <INPUT1> and <INPUT2>:
<INPUT1> --- <USERY> ---<USERX> --- <INPUT2>
Matching Gap : <VALUE>"

(or)

"Found no compatibility for <INPUT1> and <INPUT2>"

Printed on central server's terminal when central server receives info from clients:
"The Central server received input=<INPUT1> from the client using TCP over port <port number>."
"The Central server received input=<INPUT2> from the client using TCP over port <port number>."

Printed on central server's terminal when central server sends info back to clients:
"The Central server sent the results to client <A or B>."

Note (for extra credits): clientB sends the username info seperated by '|' symbol.
Eg - if only one username is entered in command line:  username1|#$#$ 
Eg - if two usernames are entered in the command line:  username1|username2

2.) Messages exchanged b/w central server and serverT

Printed on central server's terminal when central server sends a request to serverT:
"The Central server sent a request to Backend-Server T".

Printed on central server's terminal when it receives info from serverT
"The Central server received information from Backend-Server T using UDP over port <port number>."

Printed on serverT's terminal when it gets info to central server:
"The ServerT received a request from Central to get the topology."

Printed on serverT's terminal when it sends info to central server: 
"The ServerT finished sending the topology to Central."

The central server sends the two usernames to serverT in string (or character Array) format in chunks.

The message sent to central server by serverT is a string containg different edges (two usernames seperated 
by space " ") of the subgraph seperated by '|' symbol. This is sent in chunks
Eg: "Rachael Victor|Rachael King|Rachael Oliver|Victor King"

3.) Messages exchanged b/w central server and serverS


Printed on central server's terminal when central server sends a request to serverS:
"The Central server sent a request to Backend-Server S".

Printed on central server's terminal when it receives info from serverS
"The Central server received information from Backend-Server S using UDP over port <port number>."

Printed on serverS's terminal when it gets info to central server:
"The ServerS received a request from Central to get the scores."

Printed on serverS's terminal when it sends info to central server: 
"The ServerS finished sending the scores to Central."

The message sent to serverS by central server is a string containg different edges (two usernames seperated 
by space " ") of the subgraph seperated by '|' symbol. (same message which it received from serverT). This is 
sent in chunks.
Eg: "Rachael Victor|Rachael King|Rachael Oliver|Victor King"

The message sent to central server by serverS is a string containg users and their scores seperated 
by space " ". Different entries are seperated by '|' symbol. This is sent in chunks.
Eg: "Rachael 43|King 3|Oliver 94|Victor 8|"

4.) Messages exchanged b/w central server and serverP

Printed on central server's terminal when central server sends a request to serverP:
"The Central server sent a processing request to Backend-Server P."

Printed on central server's terminal when it receives info from serverP:
"The Central server received information from Backend-Server P using UDP over port <port number>."

Printed on serverP's terminal when it gets info to central server:
"The ServerP received the topology and score information."

Printed on serverP's terminal when it sends info to central server:
"The ServerP finished sending the results to the Central."

The central server sends usernames, info received from serverT (as it is), info received fron serverS (as it is)
to serverP in chunks.


Idiosyncrasy of the project:
None to best of my knowledge

Reused Code:
Socket programming concepts (and code) learned and referred from 'Beej’s Guide to Network Programming Using Internet Sockets'.
This document has been considered as a base reference doucment while coding the socket programming parts of the project.

Function directly used from the text:
void sigchld_handler(int s)  ---> in central.cpp

The rest of the socket programming concepts and code have been referred from Beej's text but not directly reused. They have been 
modified catering to the requirements of this project
