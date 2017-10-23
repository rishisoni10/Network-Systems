
Programming Assignment  - 2
-----------------------------------------------
This is the Readme file for the Network Systems' **Programming Assignment - 2**. The following is the functionality description of the the assignment:

- **Summary**: 

HTTP is the abbreviation of "HyperText Transfer Protocol". This HTTP Server is the implementation of that protocol. The protocol specifies how the information must be requested and how the responses are formed, so we have two important actors here: the HTTP Client (a web Browser) and the HTTP Server. The HTTP Server "serves" content located in the server, which includes HTML, images, flash and any file related. The server is not restricted to server static content, it also serves dynamic content generated on fly from a database or similar. The HTTP server in this assignment leverages the use of the TCP/IP network protocol. This assignment uses the C programming language.

Compile the code with the following command: __***make***__

Run the code with the following command: __***make***__ __***run***__

__**Functionality:**__

- The code extracts the port number to be used for the server, along with the root file directory from the ws.conf file. 
- The server runs forever (infinite loop), so that it can always accepts requests. 
- To make the HTTP server accept and handle multiple clients at a time, each request from a client creates and child process   with repect to that client. 
- Thus, multiple clients can access data from the server, with each unique client request being independent for each other.
- Two HTTP functionalities have been implemented: GET and POST. If any other method is request by a client, an HTTP 501 error   (not implemented) is returned to the client, and it is given another chance to send a request. 
- If the requested file is not found on the server, a HTTP 404 error is returned to the client. Similarly, if the HTTP        version of the client is different from what is supported by this server (HTTP/1.1 or HTTP/1.0), a HTTP 400 error is returned to the client. 
- Finally, if the server runs of out of memory, a HTTP 500 error is sent to the client. 
- The server checks its ws.conf file everytime a file requested from a client, limiting its support to only the file extensions found in the config file. The config file contains HTTP Content-Type corresponding to the file extensions, which need to the parsed in the server program to populate the header for the server response. 
- Additionally, pipelining support has been added to the server. After a client request is completed, the socket left opne till the timeout value (in seconds) parsed from ws.conf configuration file. This makes sure that the server code enters the corresponding signal handler every T seconds. 
