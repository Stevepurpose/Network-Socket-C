# Repository Showing Socket Networking in C. 
A socket stream sending hello world from server to client.

## Algorithm for Client-Server Communication
1. Initialize socket for server and Client.
2. Bind server socket to an IP address. This will be the server's IP address. 
3. Server listens for connections.
4. Connect to server by client.
5. Server accepts connections.
6. Server and client can now send and receive messages
7. Close both Connections.


**Note:** Make sure server is running before trying to connect client.
Also run client executable with `./client localhost`