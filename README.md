# FTP client server
first mission check point- build ftp client server

# events:
1. client send request to connect to server on control port. 
2. client send to the server int that represent how many files he will like to upload.
3. server return free ports - (new free port for each file the client want to upload)
4. client upload the files thru the free_ports.

#General:
- Wirting the code with C.
- Useing sockets to transfer data.
- Useing select to manage the sockets.

#files:
- there are two file one for the server and one for the client.
- there are two server method (in the server file):
  server2 - without select.
  server1 - with select.

#sockets:
- First set the address (can be done with getaddress).
- Second bind the socket to a port number.
- Third start listen on that socket.
- Fourth accept new connection from that socket (new sd).
- Fifth transfer data thru the new_sd (that return from calling accept system call).