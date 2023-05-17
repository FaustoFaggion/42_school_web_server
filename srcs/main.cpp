#include <iostream>		// For cout
#include <string.h>		// For memset
#include <unistd.h>		// For read
#include <cstdlib>		// For exit() and EXIT_FAILURE
#include <errno.h>
#include <cstdio>		// For stderr

#include <netdb.h>		// For struct addinfo
#include <arpa/inet.h>	// For inet_ntop function
#include <sys/socket.h>	// For socket functions
#include <netinet/in.h>	// For sockaddr_in
#include "WebServ.hpp"

// int	main(void)
// {
// 	struct addrinfo	hints;
// 	struct addrinfo	*res;
// 	int	socket_fd;
// 	int status;

// 	memset(&hints, 0, sizeof(hints));
// 	hints.ai_family = AF_UNSPEC; 		/*AF_INET or AF_INET6 to force version*/
// 	hints.ai_socktype = SOCK_STREAM;
// 	hints.ai_flags = AI_PASSIVE;		/* fill in my IP for me */

// 	if ((status = getaddrinfo(NULL, "9999", &hints, &res)) != 0)
// 	{
// 		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
// 		return (2);
// 	}

// 	/*	CREATES AN ENDPOINT FOR COMMUNICATION AND RETURNS A FILE DESCRIPTOR
// 		THAT REFERS TO THAT ENDPOINT*/
// 	socket_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
// 	if (socket_fd == -1)
// 	{
// 		std::cout << "Socket creation failed. errno: " << errno << std::endl;
// 		exit(EXIT_FAILURE);
// 	}

// 	// /*	CONNECT
// 	// 	Notice that we didn’t call bind() . Basically, we don’t care about our
// 	// 	local port number; we only care where we’re going (the remote port).
// 	// 	The kernel will choose a local port for us, and the site we connect to
// 	// 	will automatically get this information from us.*/

// 	connect(socket_fd, res->ai_addr, res->ai_addrlen);

// 	return (0);
// }


// int	main(void)
// {
// 	int	socket_fd;

// 	/*	CREATES AN ENDPOINT FOR COMMUNICATION AND RETURNS A FILE DESCRIPTOR
// 		THAT REFERS TO THAT ENDPOINT*/
// 	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
// 	if (socket_fd == -1)
// 	{
// 		std::cout << "Socket creation failed. errno: " << errno << std::endl;
// 		exit(EXIT_FAILURE);
// 	}

// 	/*	STRUCTURE FOR HANDLING INTERNET ADDRESS
// 		IT IS COMPATIBLE IN SIZE AND DATA ORDER WITH A "sockaddr structure"
// 		USED BY THE connect() FUNCTION.*/
// 	sockaddr_in sockaddr;
// 	sockaddr.sin_family = AF_INET;
// 	sockaddr.sin_addr.s_addr = INADDR_ANY;
// 	sockaddr.sin_port = htons(9999);
// 	memset(&sockaddr.sin_zero, '0', sizeof(sockaddr.sin_zero));

// 	/*	BIND SOCKET - assigning a name to a socket.
// 		When a socket is created with socket(2), it exists in a name space
// 		(address family) but has no address assigned to it.  bind() assigns the
// 		address specified by addr to the socket referred to by the file
// 		descriptor sockfd.*/
// 	if (bind(socket_fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0)
// 	{
// 		std::cout << "Failed to bind to port 9999. errno: " << errno << std::endl;
// 		exit(EXIT_FAILURE);
// 	}

// 	/*	MARKS THE SOCKET REFERRED TO BY SOCKFD AS A PASSIVE SOCKET, THAT IS, AS
// 		A SOCKET THAT WILL BE USED TO ACCEPT INCOMING CONNECTION REQUESTS USING
// 		accept(2).*/
// 	if (listen(socket_fd, MAX_CONNECTIONS) < 0)
// 	{
// 		std::cout << "Failed to listen on socket. errno: " << errno << std::endl;
// 		exit(EXIT_FAILURE);
// 	}

// 	/*	IT EXTRACTS THE FIRST CONNECTION REQUEST ON THE QUEUE OF PENDING
// 		CONNECTIONS FOR THE LISTENING SOCKET, SOCKFD, CREATES A NEW CONNECTED
// 		SOCKET, AND RETURNS A NEW FILE DESCRIPTOR REFERRING TO THAT SOCKET.
// 		THE NEWLY CREATED SOCKET IS NOT IN THE LISTENING STATE.*/
// 	size_t	addrlen = sizeof(sockaddr);
// 	int	connection = accept(socket_fd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
// 	if (connection < 0)
// 	{
// 		std::cout << "Failed to grab connection. errno: " << errno << std::endl;
// 		exit(EXIT_FAILURE);
// 	}

// 	/*	In order to be notified of incoming connections on a socket,  you  can  use  se‐
// 		lect(2),  poll(2),  or  epoll(7).  A readable event will be delivered when a new
// 		connection is attempted and you may then call accept() to get a socket for  that
// 		connection.   Alternatively, you can set the socket to deliver SIGIO when activ‐
// 		ity occurs on a socket; see socket(7) for details.*/

// 	// Read from the connection
// 	char	buffer[10];
// 	// while(1)
// 	// {
// 		size_t	bytesRead = read(connection, buffer, 10);
// 		// if (bytesRead == 0)
// 		// 	break;
// 		std::cout << "The message was: " << buffer << '\n';
// 		std::cout << "The message was: " << bytesRead << '\n';
// 	// }
	
	
// 	// Send a message to the connection
// 	std::string response = "Good talking to you\n";
// 	send(connection, response.c_str(), response.size(), 0);

// 	// Close the connections
// 	close(connection);
// 	close(socket_fd);

// 	return (0);
// }


int main (void)
{
	struct addrinfo hints;
	struct addrinfo *result;
	int				s;

	memset(&hints, 0, sizeof (struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM; /* Stream socket */
	hints.ai_flags = AI_PASSIVE;    /* for wildcard IP address */

	if ((s = getaddrinfo (NULL, SERVER_PORT, &hints, &result)) != 0) {
		std::cout << stderr << "getaddrinfo: " << gai_strerror (s) << std::endl;
		exit (EXIT_FAILURE);
	}
	
	 /*	Scan through the list of address structures returned by getaddrinfo.
		Stop when the socket and bind calls are successful. */

    int listener, optval = 1;
    socklen_t length;
    struct addrinfo *rptr; // Don't loose reference of result linked list
    
	for (rptr = result; rptr != NULL; rptr = rptr -> ai_next)
	{
		listener = socket (rptr -> ai_family, rptr -> ai_socktype,
							rptr -> ai_protocol);
		if (listener == -1)
			continue;

		if (setsockopt (listener, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (int)) == -1)
			std::cout << "ERROR: setsockopt" << std::endl;

		if (bind (listener, rptr -> ai_addr, rptr -> ai_addrlen) == 0)  // Success
			break;

		if (close (listener) == -1)
			std::cout << "ERROR: close"<< std::endl;
	}

	if (rptr == NULL) // Not successful with any address
	{
		std::cout << stderr << "Not able to bind " << std::endl;
		exit (EXIT_FAILURE);
	}

	freeaddrinfo (result);


}