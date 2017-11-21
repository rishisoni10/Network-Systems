
Programming Assignment  - 1
-----------------------------------------------
This is the Readme file for the Network Systems' **Programming Assignment - 3**. The following is the functionality description of the the assignment:

- **Objective**: 

To create a Distributed File System for reliable and secure file storage, using the TCP/IP protocol. 


- **Summary**: 

A Distributed File System is a client/server-based application that allows client to store
and retrieve files on multiple servers. One of the features of Distributed file system is
that each file can be divided in to pieces and stored on different servers and can be
retrieved even if one server is not active.

To complile and run the client, use the following command: __***gcc -o dfc dfc.c -lssl -lcrypto && ./dfc dfc.conf***__

To complile the server, use the follwoing command: __***gcc -g dfs.c***__

To run the server, use the following command on 4 different terminals: __***./a.out /DFS<server number> <port number>***__


In this application, the client side takes the input in the following format: ***functionality*** <file_name> <subfolder_name>.
This command is sent to the server, which decodes it and replies back to the server. The client acknowledges this command and the transmission from server to client or vice-versa, starts. 


__**Functionality 1:**__ ***put*** <file_name> <subfolder_name>

- **Detailed Description**: 

	- The client sends the command to the servers. The server then decodes that this is the command for the client to send the requested file to the server, and sends back the message ***"Send file <file_name> <subfolder_name>"*** back to the client. 
	-  The server then prepares itself to receive the file from the client. The client first splits the file into four parts, decides the scheme of sending the files to the server so as to introduce reliability and redundancy, sends the user credentials to the servers, and then sends the file parts to all the (active) servers. 
	-  The client encrypts each file with ***XOR encryption***, and sends it to the server.
	-  If the subfolder has been specified, then the file parts are stored in the specified subfolder inside the username folder in each server. The username and subfolder (if specified) are created if they are not present in the server before sending the files to the server.
	-  After finishing to receive the entire file, the server exits the ****put_file**** function, and return back to the **Server Menu**, waiting to receive more commands from the client. 

__**Functionality 2:**__ ***get*** <file_name>

- **Detailed Description**: 

	- The client sends the command to the servers. The server then decodes that this is the command for it to send the requested file to the client, and sends back the message ***"Sending file <file_name> <subfolder_name>*** back to the client. 
	-  The client then prepares itself to receive the file from the servers. The servers first sends all the part file names of the specified file. The client then checks each (local) server file name buffer, and requests the file part from the first server it is found in. In this way, ***traffic optimization*** takes place as only the 4 file parts are transferred from the (acive)servers to the client, instead of the 8 file parts when the traffic optimization algorithm has not been implemented. 
	-  The client then decrypts each packet with ***XOR encryption***, and writes it to a file in the correct order, so as to re-construct the original file.
	-  After finishing to receive the entire file, the client exits the ****get_file**** function, and return back to the **Main Menu** for the user to enter more commands. 


__**Functionality 3:**__ ***list*** <subfolder_name>

- **Detailed Description**: 

	- The client sends the command to the server. The server then decodes that this is the command for the server to send the list of the files present in it (in the username folder, if the subfolder is not specified), and sends back the message ***"Sending list <subfolder_name>"*** back to the client. 
	-  The server then sends all the files present in it. If one part file does not exist, the client lists the particular file with an ***Incomplete*** tag.
	-  If the list operation was successful, the client displays that the deletion was successful and exits the delete function back to the **Server Menu**, waiting to receive more commands from the client. 


__**Functionality 4:**__ ***mkdir <subfolder_name>***

- **Detailed Description**: 

	- The client sends the command to the server. The server then decodes that this is the command for the server to create a subfolder inside the current username folder, and sends back the message ***"Send subfolder"*** back to the client. 
	-  The server then calls the system function, and creates a subfolder inside the username folder, if the folder does not exist.
	-  The server then exits the ***mkdir_folder*** function back to the **Server Menu**, waiting to receive more commands from the client.


__**XOR Encryption**__:

    - This encryption technique is almost unbreakable by brute force, because of the inherent nature of the XOR operation, without knowing the key (initial) value. 
	- If two unknown values are XORed, it is impossible to repredict what their values might be. 
	- However if one value is known, it is wasy to predict the other value. 
	- In this code, the client uses the password as the key which is used for encryption. To decrypt the packet, it is double XORed with the key to recover the original packet. 