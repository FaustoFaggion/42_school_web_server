#include "main.hpp"
#include "WebServ.hpp"

int main (void)
{
	// struct addrinfo hints;
	// struct addrinfo *result;
	// int				s;

	// memset(&hints, 0, sizeof (struct addrinfo));
	// hints.ai_family = AF_UNSPEC;    /* allow IPv4 or IPv6 */
	// hints.ai_socktype = SOCK_STREAM; /* Stream socket */
	// hints.ai_flags = AI_PASSIVE;    /* for wildcard IP address */

	// if ((s = getaddrinfo (NULL, SERVER_PORT, &hints, &result)) != 0) {
	// 	std::cout << stderr << "getaddrinfo: " << gai_strerror (s) << std::endl;
	// 	exit (EXIT_FAILURE);
	// }
	
	//  /*	Scan through the list of address structures returned by getaddrinfo.
	// 	Stop when the socket and bind calls are successful. */

	// int listener, optval = 1;
	// // socklen_t length;
	// struct addrinfo *rptr; // Don't loose reference of result linked list
    
	// for (rptr = result; rptr != NULL; rptr = rptr -> ai_next)
	// {
	// 	listener = socket (rptr -> ai_family, rptr -> ai_socktype,
	// 						rptr -> ai_protocol);
	// 	if (listener == -1)
	// 		continue;

	// 	if (setsockopt (listener, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (int)) == -1)
	// 		std::cout << "ERROR: setsockopt" << std::endl;

	// 	if (bind (listener, rptr -> ai_addr, rptr -> ai_addrlen) == 0)  // Success
	// 		break;

	// 	if (close (listener) == -1)
	// 		std::cout << "ERROR: close"<< std::endl;
	// }

	// if (rptr == NULL) // Not successful with any address
	// {
	// 	std::cout << stderr << "Not able to bind " << std::endl;
	// 	exit (EXIT_FAILURE);
	// }

	// freeaddrinfo (result);

	// // Mark socket for accepting incoming connections using accept
	// if (listen (listener, BACKLOG) == -1)
	// 	strerror(errno);
	

	WebServ			server(AF_UNSPEC, SOCK_STREAM, AI_PASSIVE, "9999");

	server.create_listener_socket();

	int listener = server.getFdListener();
	std::cout << "main: " << listener << "\n";	
	/*EPOLL FUNCTION*/
	int efd;
	if ((efd = epoll_create1 (0)) == -1)
		std::cout << "ERROR: epoll_create" << std::endl;
	struct epoll_event ev;
	struct epoll_event ep_event [MAX_CONNECTIONS];

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
		if ((nfds = epoll_wait (efd, ep_event, MAX_CONNECTIONS,  -1)) == -1) // '-1' to block indefinitely
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