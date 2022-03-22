# Social-matching-service-model-to-find-compatibility-between-two-clients-on-a-social-network
Implemented a social service matching model wherein two clients on a social network issue a request over TCP sockets to a Central server for finding their compatibility. The central server distributes the task amongst three other servers namely Topology, Score and Processing servers. Depth-First Search is performed on a undirected unweighted graph (representing the social network) to get topological info (or subgraph containing the two users). A new weighted graph is created using the scores and subgraph found previously, and Dijkstra's algorithm is used to find the social path between the two clients having the minimum compatibility score.
