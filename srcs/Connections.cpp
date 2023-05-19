// #include "Connections.hpp"

// Connections::Connections()
// {
// 	memset(&_ev, 0, sizeof(struct epoll_event));
// 	memset(_ep_event, 0, sizeof(struct epoll_event));
// 	_efd = 0;
// }

// Connections::~Connections()
// {
// }

// void	Connections::create_poll()
// {
// 	if ((_efd = epoll_create(MAX_CONNECTIONS)) == -1)
// 		std::cout << "ERROR: epoll_create" << std::endl;
// }

// void	Connections::add_connection()
// {
// 	std::cout<< "------  " << getListener().get_fd_listener() << "\n";
// 	_ev.events = EPOLLIN; // File descriptor is available for read.
// 	_ev.data.fd = getListener().get_fd_listener();
// 	if (epoll_ctl (_efd, EPOLL_CTL_ADD, getListener().get_fd_listener(), &_ev) == -1)
// 		std::cout << "ERROR: epoll_ctl" << std::endl;
// }

// void	Connections::connections_monitoring()
// {
// 	struct sockaddr_storage	client_saddr; // Can store a IPv4 and IPv6 struct
// 	socklen_t				addrlen;
// 	int						nfds = 0;
// 	while (1)
// 	{
// 		// monitor readfds for readiness for reading
// 		if ((nfds = epoll_wait (_efd, _ep_event, MAX_CONNECTIONS,  -1)) == -1) // '-1' to block indefinitely
// 			std::cout << "ERROR: epoll_wait" << std::endl;
		
// 		// Some sockets are ready. Examine readfds
// 		for (int i = 0; i < nfds; i++)
// 		{
// 			if 	((_ep_event[i].events & EPOLLIN) == EPOLLIN)
// 			{
// 				if (_ep_event[i].data.fd == getListener().get_fd_listener()) // request for new connection
// 				{
// 					addrlen = sizeof(struct sockaddr_storage);
// 					int fd_new;
// 					if ((fd_new = accept (getListener().get_fd_listener(), (struct sockaddr *) &client_saddr, &addrlen)) == -1)
// 						std::cout << "ERROR: accept" << std::endl;
					
// 					// add fd_new to epoll
// 					_ev.events = EPOLLIN;
// 					_ev.data.fd = fd_new;
// 					if (epoll_ctl (_efd, EPOLL_CTL_ADD, fd_new, &_ev) == -1)
// 						std::cout << "ERROR: epoll_ctl" << std::endl;
// 				}
// 				else // data from an existing connection, receive it
// 				{
// 					char	recv_message[100];

// 					memset (&recv_message, '\0', sizeof (recv_message));
// 					ssize_t numbytes = recv (_ep_event[i].data.fd, &recv_message, sizeof(recv_message), 0);
// 					if (numbytes == -1)
// 						std::cout << "ERROR: recv" << std::endl;
// 					else if (numbytes == 0) // connection closed by client
// 					{
// 						std::cout << stderr << "Socket " <<
// 							_ep_event [i].data.fd << " closed by client" << std::endl;
// 						// delete fd from epoll
// 						if (epoll_ctl (_efd, EPOLL_CTL_DEL, _ep_event [i].data.fd, &_ev) == -1)
// 							std::cout << "ERROR: epoll_ctl" << std::endl;
// 						if (close (_ep_event [i].data.fd) == -1)
// 							std::cout << "ERROR: close by client" << std::endl;
// 					}
// 					else 
// 					{
// 						// data from client
// 						std::cout << recv_message << '\n';

// 						// std::string response = "Good talking to you\n";
// 						// send(ep_event[i].data.fd, response.c_str(), response.size(), 0);
// 					}
// 				}
// 			}
// 		}
// 	}
// }