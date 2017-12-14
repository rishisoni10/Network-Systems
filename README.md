
Programming Assignment  - 4
-----------------------------------------------
This is the Readme file for the Network Systems' **Programming Assignment - 4**. The following is the functionality description of the the assignment:

Create a Proxy Server. 

- **Summary**: 

In computer networks, a proxy server is a server (a computer system or an application) that acts as an intermediary for requests from clients seeking resources from other servers.A client connects to the proxy server, requesting some service, such as a file, connection, web page, or other resource available from a different server and the proxy server evaluates the request as a way to simplify and control its complexity. Proxies were invented to add structure and encapsulation to distributed systems.Today, most proxies are web proxies, facilitating access to content on the World Wide Web, providing anonymity and may be used to bypass IP address blocking.

(Source: https://en.wikipedia.org/wiki/Proxy_server)

Compile the code with the following command: __***make***__

Run the code with the following command: __./webproxy__ __***10001***__	__***60***__


where 10001 = proxy_port_number & 60 = cache_timeout in seconds

general command: ./webproxy <proxy_port_number> <cache_timout in seconds>

__***Configure Mozila Firefox to connect to a proxy server with IP address: 127.0.0.1 and port number = 10001***__

__**Functionality:**__

- The code extracts the port number to be used for the server, along with the cache timeout from the command line arguements.
- The proxy runs forever (infinite loop), so that it can always accepts requests. 
- To make the proxy server accept and handle multiple clients at a time, each request from a client creates and child process with respect to that client. 
- Thus, multiple clients can access data from the proxy server, with each unique client request being independent for each other.
- The proxy server parses the client request to the remote server. Before this, it checks if the request is a supported method (GET), a supported HTTP version and whether the host is to be blocked by checking a text file which has a list of the blocked hostnames.
- Before sending the request to the remote host , the program checks its cache (cache/ folder) whether the requested remote host files already exist in memory within the timeout memory. If the timeout period has not expired, the cached files are sent to the client. Else, the old cached files are deleted and the supported request is sent to the remote host.
- For the cacheing the webpages, their names have been encrypted using md5hash and stored locally in the cache/ folder, till the page is requested again AND the timeout period has reached.
- The default port number used to connect the proxy server and the remote host has been set to port 80. 
- The code was tested on the following websites are recommended by the course assistants:
	- www.caida.org
	- www.morse.colorado.edu
	- www.umich.edu
	- www.berkeley.edu
- For the demostration purposes, www.caida.org has been put as a blocked hostname in the "list_of_blocked_host.txt" file
