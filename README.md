# client_server_c
first mission check point- build client server in c.
FTP - the client upload files to the server.

# events:
1. client send request to connect to server on control port.
2. client tell the server how many files he will like to upload.
3. server return free ports - (new free port for each file the client want to upload)
4. client upload each file thruogh new connection.
