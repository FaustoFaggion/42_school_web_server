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

#include <sys/epoll.h>

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
	// socklen_t length;
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

	// Mark socket for accepting incoming connections using accept
	if (listen (listener, BACKLOG) == -1)
		strerror(errno);
	
	/*EPOLL FUNCTION*/
	int efd;
	if ((efd = epoll_create1 (0)) == -1)
		std::cout << "ERROR: epoll_create1" << std::endl;
	
	struct epoll_event ev;
	struct epoll_event ep_event [MAX_EVENTS];

	ev.events = EPOLLIN; // File descriptor is available for read.
	ev.data.fd = listener;
	if (epoll_ctl (efd, EPOLL_CTL_ADD, listener, &ev) == -1)
		std::cout << "ERROR: epoll_ctl" << std::endl;
	
	int nfds = 0;

	socklen_t addrlen;
	struct sockaddr_storage client_saddr; // Can store a IPv4 and IPv6 struct
	char str [INET6_ADDRSTRLEN]; // size of IPv6 address
	struct sockaddr_in  *ptr;
	struct sockaddr_in6  *ptr1;

	while (1)
	{
		// monitor readfds for readiness for reading
		if ((nfds = epoll_wait (efd, ep_event, MAX_EVENTS,  -1)) == -1) // '-1' to block indefinitely
			std::cout << "ERROR: epoll_wait" << std::endl;
		
		// Some sockets are ready. Examine readfds
		for (int i = 0; i < nfds; i++)
		{
			if 	((ep_event[i].events & EPOLLIN) == EPOLLIN)
			{
				if (ep_event[i].data.fd == listener) // request for new connection
				{
					addrlen = sizeof (struct sockaddr_storage);
					
					int fd_new;
					if ((fd_new = accept (listener, (struct sockaddr *) &client_saddr, &addrlen)) == -1)
						std::cout << "ERROR: accept" << std::endl;
					
					// add fd_new to epoll
					ev.events = EPOLLIN;
					ev.data.fd = fd_new;
					if (epoll_ctl (efd, EPOLL_CTL_ADD, fd_new, &ev) == -1)
						std::cout << "ERROR: epoll_ctl" << std::endl;
					
					// print IP address of the new client
					if (client_saddr.ss_family == AF_INET)
					{
						ptr = (struct sockaddr_in *) &client_saddr;
						inet_ntop (AF_INET, &(ptr -> sin_addr), str, sizeof (str));
					}
					else if (client_saddr.ss_family == AF_INET6)
					{
						ptr1 = (struct sockaddr_in6 *) &client_saddr;
						inet_ntop (AF_INET6, &(ptr1 -> sin6_addr), str, sizeof (str));
					}
					else
					{
						ptr = NULL;
						std::cout << stderr << " Address family is neither AF_INET nor AF_INET6" << std::endl;
					}
                    // if (ptr) 
                    //     syslog (LOG_USER | LOG_INFO, "%s %s", "Connection from client", str);
				}
				else // data from an existing connection, receive it
				{
					char	recv_message[100];

					memset (&recv_message, '\0', sizeof (recv_message));
					ssize_t numbytes = recv (ep_event[i].data.fd, &recv_message, sizeof(recv_message), 0);
					if (numbytes == -1)
						std::cout << "ERROR: recv" << std::endl;
					else if (numbytes == 0) // connection closed by client
					{
						std::cout << stderr << "Socket " <<
							ep_event [i].data.fd << " closed by client" << std::endl;
						// delete fd from epoll
						if (epoll_ctl (efd, EPOLL_CTL_DEL, ep_event [i].data.fd, &ev) == -1)
							std::cout << "ERROR: epoll_ctl" << std::endl;
						if (close (ep_event [i].data.fd) == -1)
							std::cout << "ERROR: close by client" << std::endl;
					}
					else 
					{
						// data from client
						std::cout << recv_message << '\n';

						// std::string response = "Good talking to you\n";
						// send(ep_event[i].data.fd, response.c_str(), response.size(), 0);
					}
				}
			}
		}
	}
	return (0);
}