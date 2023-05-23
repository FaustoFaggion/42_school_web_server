#include "WebServ.hpp"

WebServ::WebServ()
{

}

WebServ::WebServ(FileParser file)
{
	_listener.set_domain(file.get_domain());
	_listener.set_type(file.get_type());
	_listener.set_port(file.get_port());
	_listener.set_flag(file.get_flag());
	_listener.set_worker_connections(file.get_worker_connections());
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

void	WebServ::setup_server(int type)
{
	_listener.addrinfo(_listener.get_domain(), type, _listener.get_flag(),
						_listener.get_port());
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
	if ((_efd = epoll_create(_listener.get_worker_connections())) == -1)
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
		if ((_nfds = epoll_wait (_efd, _ep_event, _listener.get_worker_connections(),  -1)) == -1) // '-1' to block indefinitely
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
					_ev.events = EPOLLIN | EPOLLOUT;
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
					char	buff[10];

					memset (&buff, '\0', sizeof (buff));
					ssize_t numbytes = recv (_ep_event[i].data.fd, &buff, sizeof(buff), 0);
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
						std::map<int, std::string>::iterator	it;
						
						connections[_ep_event[i].data.fd] += buff;
						// data from client
						it = connections.find(_ep_event[i].data.fd);
						
						if (*((*it).second.end() - 1) == '\n' && *((*it).second.end() - 2) == '\r')
							request_parser((*it).second);
					}
				}
			}
			else if ((_ep_event[i].events & EPOLLOUT) == EPOLLOUT)
			{
				std::map<int, std::string>::iterator	it;
				it = connections.find(_ep_event[i].data.fd);
				if (!(*it).second.empty())
				{
					std::cout << _ep_event[i].data.fd << "\n";
					send(_ep_event[i].data.fd, (*it).second.c_str(), (*it).second.size(), 0);
					(*it).second.clear();
					epoll_ctl(_efd, EPOLL_CTL_DEL, _ep_event[i].data.fd, &_ev);
					close(_ep_event[i].data.fd);
					connections.erase(_ep_event[i].data.fd);
				}
			}
		}
	}
}

void	WebServ::request_parser(std::string &request)
{

	request = "Raoniiiiiiii\n";
	if (strncmp("GET / HTTP/1.1", request.c_str(), 14) == 0)
		request = "Raoniiiiiiii\n";
}