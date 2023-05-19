#include "WebServ.hpp"

WebServ::WebServ()
{
	_domain = 0;
	_type = 0;
	_flag = 0;
	_port = "0";
	_fd_listener = 0;
}

WebServ::WebServ(int domain, int type, int flag, std::string port)
{
	_listener.addrinfo(domain, type, flag, port);
}

WebServ::~WebServ()
{

}

ListenerSocket	WebServ::getListener() const
{
	return(_listener);
}

int				WebServ::getFdListener() const
{
	return (_fd_listener);
}

void	WebServ::setup_server(int domain, int type, int flag, std::string port)
{
	_listener.addrinfo(domain, type, flag, port);
}

void	WebServ::create_listener_socket()
{
	_listener.create_fd();
	_listener.bind_fd_to_port();
	_listener.listen_fd();
	_fd_listener = _listener.get_fd_listener();
}

void	WebServ::create_connections()
{
	/*EPOLL FUNCTION*/
	if ((_efd = epoll_create(MAX_CONNECTIONS)) == -1)
		std::cout << "ERROR: epoll_create" << std::endl;
	_ev.events = EPOLLIN; // File descriptor is available for read.
	_ev.data.fd = _fd_listener;
	if (epoll_ctl (_efd, EPOLL_CTL_ADD, _fd_listener, &_ev) == -1)
		std::cout << "ERROR: epoll_ctl" << std::endl;
}

void	WebServ::run()
{
	_nfds = 0;

	while (1)
	{
		// monitor readfds for readiness for reading
		if ((_nfds = epoll_wait (_efd, _ep_event, MAX_CONNECTIONS,  -1)) == -1) // '-1' to block indefinitely
			std::cout << "ERROR: epoll_wait" << std::endl;
		
		// Some sockets are ready. Examine readfds
		for (int i = 0; i < _nfds; i++)
		{
			if 	((_ep_event[i].events & EPOLLIN) == EPOLLIN)
			{
				if (_ep_event[i].data.fd == _fd_listener) // request for new connection
				{
					_addrlen = sizeof (struct sockaddr_storage);
					
					int fd_new;
					if ((fd_new = accept (_fd_listener, (struct sockaddr *) &_client_saddr, &_addrlen)) == -1)
						std::cout << "ERROR: accept" << std::endl;
					
					// add fd_new to epoll
					_ev.events = EPOLLIN;
					_ev.data.fd = fd_new;
					if (epoll_ctl (_efd, EPOLL_CTL_ADD, fd_new, &_ev) == -1)
						std::cout << "ERROR: epoll_ctl" << std::endl;
					else if (_client_saddr.ss_family != AF_INET && _client_saddr.ss_family == AF_INET6)
					{
						std::cout << stderr << " Address family is neither AF_INET nor AF_INET6" << std::endl;
					}
				}
				else // data from an existing connection, receive it
				{
					char	recv_message[100];

					memset (&recv_message, '\0', sizeof (recv_message));
					ssize_t numbytes = recv (_ep_event[i].data.fd, &recv_message, sizeof(recv_message), 0);
					if (numbytes == -1)
						std::cout << "ERROR: recv" << std::endl;
					else if (numbytes == 0) // connection closed by client
					{
						std::cout << stderr << "Socket " <<
							_ep_event [i].data.fd << " closed by client" << std::endl;
						// delete fd from epoll
						if (epoll_ctl (_efd, EPOLL_CTL_DEL, _ep_event[i].data.fd, &_ev) == -1)
							std::cout << "ERROR: epoll_ctl" << std::endl;
						if (close (_ep_event [i].data.fd) == -1)
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
}
