
Programming Assignment  - 1
-----------------------------------------------
This is the Readme file for the Network Systems' **Programming Assignment - 1**. The following is the functionality description of the the assignment:

- **Summary**: 

UDP(User Datagram Protocol) is a connectionless transmission model with a minimum of protocol mechanism. It has no handshaking dialogues and no mechanism of
acknowledgements(ACKs). Therefore, every packet delivered by using UDP protocol is not guaranteed to be received. That’s why UDP is an unreliability protocol. Due to the simplicity, it avoids unnecessary delay in some specific application such as streaming media.

To run this code, compile the respective files in their folders (clientFolder and serverFolder) with the following command: __***make***__

At the client side, the run code with the server IP address and port number. The format is: __***./client <ip_address> <port_number>***__

At the server side, run the code with the port number. The format is: __***./server <port_number>***__

In this application, the client side takes the input in the following format: ***functionality*** <file_name>.
This command is sent to the server, which decodes it and replies back to the server. The client acknowledges this command and the transmission from server to client or vice-versa, starts, packet-by-packet. 

__**Functionality 1:**__ ***get*** <file_name>

- **Detailed Description**: 

	- The client sends the command to the server. The server then decodes that this is the command for it to send the requested file to the client, and sends back the message ***"Sending file <file_name>"*** back to the client. 
	-  The client then prepares itself to receive the file from the server, packet-by-packet. The server first sends the size of the file (in bytes) that it is going to be sent. 
	-  The server encrypts each packet with ***XOR encryption***, and sends it to the client. The client sends back an ACK after the packet with the expected index has been received. 
	-  The reliability algorithm has been designed such that the server keeps waiting for the correct ACK from the client. Only when it receives the correct ACK packet, it sends out the next packet. 
	-  At the server side, a timeout of 500ms has been set so that the server sends it data packet again if it does not receive an ACK within 500ms. 
	-  The client side waits for a packet with the index that it is expecting and sends back an ACK of the same. If it receives a packet that it has already received (old packet), it then sends back the ACK of that packet. 
	-  If the received packet is neither an old packet nor the expected packet, it then sends back an NACK in the form of packet_index = 0. The server side would then send back the packet that the client expects to receive. 
	-  Whenever a packet is receives by the client, it is decrypted applying a ***double XOR operation*** on it. If it is the expected packet, then it is written in the file. Else, it is ignored. 
	-  After finishing to receive the entire file, the client exits the ****get_file**** function, and return back to the **Main Menu** for the user to enter more commands. 


__**Functionality 2:**__ ***put*** <file_name>

- **Detailed Description**: 

	- The client sends the command to the server. The server then decodes that this is the command for the client to send the requested file to the server, and sends back the message ***"Send file <file_name>"*** back to the client. 
	-  The server then prepares itself to receive the file from the client, packet-by-packet. The client first sends the size of the file (in bytes) that it is going to be sent. 
	-  The client encrypts each packet with ***XOR encryption***, and sends it to the server. The server sends back an ACK after the packet with the expected index has been received. 
	-  The reliability algorithm has been designed such that the client keeps waiting for the correct ACK from the server. Only when it receives the correct ACK packet, it sends out the next packet. 
	-  At the client side, a timeout of 500ms has been set so that the client sends it data packet again if it does not receive an ACK within 500ms. 
	-  The server side waits for a packet with the index that it is expecting and sends back an ACK of the same. If it receives a packet that it has already received (old packet), it then sends back the ACK of that packet. 
	-  If the received packet is neither an old packet nor the expected packet, it then sends back an NACK in the form of packet_index = 0. The client side would then send back the packet that the server expects to receive. 
	-  Whenever a packet is receives by the server, it is decrypted applying a ***double XOR operation*** on it. If it is the expected packet, then it is written in the file. Else, it is ignored. 
	-  After finishing to receive the entire file, the server exits the ****put_file**** function, and return back to the **Server Menu**, waiting to receive more commands from the client. 


__**Functionality 3:**__ ***delete*** <file_name>

- **Detailed Description**: 

	- The client sends the command to the server. The server then decodes that this is the command for the server to delete the requested file, and sends back the message ***"Deleting <file_name>"*** back to the client. 
	-  The server then deletes the requested file. If the file does not exist or if the delete operation fails, the server shows an error and exits the delete function. 
	-  If the delete operation was successful, the server displays that the deletion was successful and exits the delete function back to the **Server Menu**, waiting to receive more commands from the client. 


__**Functionality 4:**__ ***ls***

- **Detailed Description**: 

	- The client sends the command to the server. The server then decodes that this is the command for the server to list its current directory contents at the client side, and sends back the message ***"Listing server directory contents"*** back to the client. 
	-  The server then calls the system function, and writes the list of files in a text file, sends this to the client. 
	-  The client reads this file byte-by-byte and displays the contents on the terminal. The server then deletes this file (at the server side) as it is no longer needed, and exits the ***list_directory*** function back to the **Server Menu**, waiting to receive more commands from the client.


__**Functionality 5:**__ ***exit***

- **Detailed Description**: 

	- The client sends the command to the server. The server then decodes that this is the command for the server to exit, and sends back the message ***"Server is exiting"*** back to the client. 
	-  The server then breaks out of the infinite ***while*** loop, closes the socket, and stops executing.
	-  The client reads the message sent by the server and exits in a similar fashion. 


__**XOR Encryption**__:

- This encryption technique is almost unbreakable by brute force , the 
- Because of inherent nature of the XOR operation, without knowing the key (initial) value. If two unknown values are XORed, it is impossible to repredict what their values might be. 
- However if one value is known, it is wasy to predict the other value. 
- In this code, both the client and server know the key which is used for encryption. To decrypt the packet, it is double XORed with the key to recover the original packet. 