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
	/*FD IS AVAIABLE FOR READ*/
	_ev.events = EPOLLIN;
	_ev.data.fd = _fd_listener;
	if (epoll_ctl (_efd, EPOLL_CTL_ADD, _fd_listener, &_ev) == -1)
		std::cout << "ERROR: epoll_ctl" << std::endl;
}

void	WebServ::run()
{
	_nfds = 0;
	while (1)
	{
		/*MONITOR FDS. STILL WAITING UNTIL A EVENT HEAPPENS IN A FD*/
		if ((_nfds = epoll_wait (_efd, _ep_event, _listener.get_worker_connections(),  -1)) == -1) // '-1' to block indefinitely
			std::cout << "ERROR: epoll_wait" << std::endl;

		/*LOOP INTO EPOLL READY LIST*/
		for (int i = 0; i < _nfds; i++)
		{
			/*CHECK IF EVENT TO READ*/
			if 	((_ep_event[i].events & EPOLLIN) == EPOLLIN)
			{
				/*CHECK IF IT IS THE LISTENER SOCKET*/
				if (_ep_event[i].data.fd == _fd_listener) // request for new connection
				{
					_addrlen = sizeof (struct sockaddr_storage);
					
					/*CREATE NEW FILE DESCRIPTOR TO CONNECTO TO CLIENT*/
					int fd_new;
					if ((fd_new = accept (_fd_listener, (struct sockaddr *) &_client_saddr, &_addrlen)) == -1)
						std::cout << "ERROR: accept" << std::endl;
					
					/*ADD NEW FD EPOOL TO MONNITORING THE EVENTS*/
					_ev.events = EPOLLIN | EPOLLOUT;
					_ev.data.fd = fd_new;
					if (epoll_ctl (_efd, EPOLL_CTL_ADD, fd_new, &_ev) == -1)
						std::cout << "ERROR: epoll_ctl" << std::endl;
					else if (_client_saddr.ss_family != AF_INET && _client_saddr.ss_family == AF_INET6)
					{
						std::cout << stderr << " Address family is neither AF_INET nor AF_INET6" << std::endl;
					}
					/*ADD fd_new TO MAP_CONNECTIONS AND SET TO EMPTY*/
					map_connections[fd_new] = "";
				}
				else /*FD FROM A EXISTING CONNECTION*/
				{
					char	buff[10];

					memset (&buff, '\0', sizeof (buff));
					/*RECEIVING CLIENT DATA CHUNCKS REQUEST */
					ssize_t numbytes = recv (_ep_event[i].data.fd, &buff, sizeof(buff), 0);
					if (numbytes == -1)
						std::cout << "ERROR: recv" << std::endl;
					/*CONNECTION CLOSED BY THE CLIENT*/
					else if (numbytes == 0)
					{
						std::cout << stderr << "Socket " <<
						_ep_event [i].data.fd << " closed by client" << std::endl;
						/*DELETE FD FROM EPOLL*/
						if (epoll_ctl (_efd, EPOLL_CTL_DEL, _ep_event[i].data.fd, &_ev) == -1)
							std::cout << "ERROR: epoll_ctl" << std::endl;
						/*CLOSE FD*/
						if (close (_ep_event [i].data.fd) == -1)
							std::cout << "ERROR: close by client" << std::endl;
					}
					else 
					{
						/*CONCAT DATA UNTIL FIND \r \n THAT MEANS THE END OF REQUEST DATA*/
						map_connections[_ep_event[i].data.fd] += buff;
						/*CHECK IF REQUEST DATA FINISHED*/
						std::map<int, std::string>::iterator	it;
						it = map_connections.find(_ep_event[i].data.fd);
						if (*((*it).second.end() - 1) == '\n' && *((*it).second.end() - 2) == '\r')
						{
							std::cout << "connect fd: " << _ep_event[i].data.fd << "\n";
							request_parser((*it).second);
						}
					}
				}
			}
			else if ((_ep_event[i].events & EPOLLOUT) == EPOLLOUT)
			{
				std::map<int, std::string>::iterator	it;
				it = map_connections.find(_ep_event[i].data.fd);
				/*PROTECTION FROM CONNECTION HAND-SHAKE*/
				if (!(*it).second.empty())
				{
					send(_ep_event[i].data.fd, (*it).second.c_str(), (*it).second.size(), 0);
					/*DELETE FROM EPOLL AND CLOSE FD*/
					epoll_ctl(_efd, EPOLL_CTL_DEL, _ep_event[i].data.fd, &_ev);
					close(_ep_event[i].data.fd);
					map_connections.erase(_ep_event[i].data.fd);
				}
			}
		}
	}
}

void	WebServ::request_parser(std::string &request)
{

	std::cout << "request:\n " << request << "\n";
	// request = "Raoniiiiiiii\n";
	request  = "https://www.google.com";
	if (strncmp("GET / HTTP/1.1", request.c_str(), 14) == 0)
	{
	}
}