#include "WebServ.hpp"

WebServ::WebServ()
{

}

WebServ::WebServ(char *file, std::string server_name)
{
	parse_file(file, server_name);
}

WebServ::~WebServ()
{

}

int		WebServ::getFdListener() const
{
	return (_fd_listener);
}

void	WebServ::parse_file(char *file, std::string server_name)
{
	parse_server(file, server_name);

	parse_listener(_server_conf_file);
	
	std::cout << "_domain: " << get_domain() << "\n";
	std::cout << "_port: " << get_port() << "\n";
	std::cout << "_type: " << get_type() << "\n";
	std::cout << "_flag: " << get_flag() << "\n";
	std::cout << "_work_connections: " << get_worker_connections() << "\n";

	/*PARSE ROOT SIMPLE DIRECTIVE*/
	bool	simple_root_directive;

	simple_root_directive = parse_simple_root_directive();
	
	/*PARSE INDEX*/
	parse_index(_index, _server_conf_file);

	/*PARSE BUFFER_SIZE*/
	parse_buffer_size();
	std::cout << _buffer_size << "\n";
	/*PARSE LOCATIONS*/
	parse_locations(simple_root_directive);

}

void	WebServ::setup_server(int type)
{
	addrinfo(get_domain(), type, get_flag(), get_port());
}

void	WebServ::create_listener_socket()
{
	create_fd();
	bind_fd_to_port();
	listen_fd();
	_fd_listener = get_fd_listener();
	_ev.data.fd = 0;
	_ev.data.ptr = NULL;
	_ev.data.u32 = 0;
	_ev.data.u64 = 0;
	for (int i = MAX_CONNECTIONS; i != -1; i--)
		_ep_event[i].data.fd = 0;

}

void	WebServ::create_connections()
{
	/*EPOLL FUNCTION*/
	if ((_efd = epoll_create(get_worker_connections())) == -1)
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
		if ((_nfds = epoll_wait (_efd, _ep_event, get_worker_connections(),  1000)) == -1) // '-1' to block indefinitely
			std::cout << "ERROR: epoll_wait" << std::endl;
		
		/*A REQUEST TO SERVER SHOULD NEVER HANG FOREVER.*/
		delete_timeout_socket();

		/*LOOP INTO EPOLL READY LIST*/
		for (int i = 0; i < _nfds; i++)
		{
			/*CHECK IF EVENT TO READ*/
			if 	((_ep_event[i].events & EPOLLIN) == EPOLLIN)
			{

				/*LISTENER SOCKET*/
				if (_ep_event[i].data.fd == _fd_listener)
				{
					std::cout << "accept_new_connection" << "\n";
					accept_new_connection();
				}
				/*CLIENT SOCKET*/
				else 
				{
					std::cout << "\nRECEIVE_DATA FUNCTION fd: " << _ep_event[i].data.fd << "\n\n";
					receive_data(i);
				}
			}
			/*CHECK IF EVENT TO WRITE*/
			else if ((_ep_event[i].events & EPOLLOUT) == EPOLLOUT)
			{
				std::cout << "\nRESPONSE FUNCTION fd: " << _ep_event[i].data.fd << "\n\n";
				response(i);
			}
		}
	}
}

void	WebServ::delete_timeout_socket()
{

	// std::cout << "\nDELETE_TIMEOUT_SOCKET FUNCTION\n";

	int j = 0;

	while (_ep_event[j].data.fd)
	{
		double	timeout = difftime(time(NULL), map_connections[_ep_event[j].data.fd].start_connection);
		if (_ep_event[j].data.fd != _fd_listener)
		{
			if (timeout > 3.0)
			{
				int	fd = _ep_event[j].data.fd;

				/*DELETE FROM EPOLL AND CLOSE FD*/
				epoll_ctl(_efd, EPOLL_CTL_DEL, _ep_event[j].data.fd, &_ev);
				// if (epoll_ctl(_efd, EPOLL_CTL_DEL, _ep_event[j].data.fd, &_ev) == -1)
				// 	std::cout << "EPOLL_CTL_DEL FAIL fd: " << _ep_event[j].data.fd << "\n";
				// _ep_event[j].data.fd = 0;
				close(fd);
				map_connections.erase(fd);
			}
		}
		j++;
	}
};

void	WebServ::accept_new_connection()
{
	_addrlen = sizeof (struct sockaddr_storage);

	/*CREATE NEW FILE DESCRIPTOR TO CONNECTO TO CLIENT*/
	int fd_new;
	if ((fd_new = accept (_fd_listener, (struct sockaddr *) &_client_saddr, &_addrlen)) == -1)
		std::cout << "ERROR: accept" << std::endl;

	/*SET FD SOCKET TO NONBLOCKING*/
	int fd_flag = fcntl(fd_new, F_GETFL, 0);
	fcntl(fd_new, F_SETFL, fd_flag | O_NONBLOCK);
	
	/*ADD NEW FD EPOOL TO MONNITORING THE EVENTS*/
	_ev.events = EPOLLIN | EPOLLHUP | EPOLLONESHOT;
	_ev.data.fd = fd_new;
	
	std::cout << "new_fd: " << fd_new << "\n";

	if (epoll_ctl (_efd, EPOLL_CTL_ADD, fd_new, &_ev) == -1)
		std::cout << "ERROR: epoll_ctl" << std::endl;
	else if (_client_saddr.ss_family != AF_INET && _client_saddr.ss_family == AF_INET6)
	{
		std::cout << stderr << " Address family is neither AF_INET nor AF_INET6" << std::endl;
	}

	/*ADD fd_new TO MAP_CONNECTIONS AND SET TO EMPTY*/
	t_client	c;
	initialize_client_struct(c, fd_new);
	map_connections[fd_new] = c;
}

void			WebServ::initialize_client_struct(t_client &c, int fd_new)
{
	c.fd = fd_new;
	c.start_connection = time(NULL);
	c._request = "";
	c._method = "";
	c._url = "";
	c._protocol = "";
	c._content_type = "";
	c._content_length = "";
	c._server_name = "";
	c._server_port = "";
	c._user_agent = "";
	c._http_host = "";
	c._http_accept = "";
	c._http_accept_encoding = "";
	c._http_accept_language = "";
	c._query_string = "";
	c._path_info = "";
	c._request_uri = "";
	c._remote_host = "";
	c._boundary = "";
	c._content = "";
	c._url_file = "";
	c._response = "";
}

void	WebServ::receive_data(int i)
{
	char	buff[_buffer_size];
                                         

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
		map_connections[_ep_event[i].data.fd]._request += buff;
	
		/*CHECK IF REQUEST DATA FINISHED*/
		std::map<int, t_client>::iterator	it;
		it = map_connections.find(_ep_event[i].data.fd);
		if ((*it).second._request.find("\r\n\r\n") != std::string::npos)
		{
			/*PRINT RECEIVED DATA*/
			std::cout <<  (*it).second._request << "\n";
			
			/*INSTANCIATE A HTTPRESPONSE CLASS TO RESPONSE THE REQUEST*/
			HttpResponse	r(_locations, _index);
			r.response_parser((*it).second);

			/*SET FD SOCKET TO WRITE (EPOLLIN)*/
			_ev.events = EPOLLOUT | EPOLLONESHOT;
			epoll_ctl(_efd, EPOLL_CTL_MOD, _ep_event[i].data.fd, &_ev);
		}
	}
}

void	WebServ::response(int i)
{
	std::map<int, t_client>::iterator	it;
	it = map_connections.find(_ep_event[i].data.fd);
	/*PROTECTION FROM CONNECTION HAND-SHAKE*/
	if (!(*it).second._request.empty())
	{
		// std::cout << "inside response fd: " << _ep_event[i].data.fd << "\n" << (*it).second.response.c_str() << "\n";
		send(_ep_event[i].data.fd, (*it).second._request.c_str(), (*it).second._request.size(), 0);
		
		
		/*If keep-alive SET FD SOCKET TO READ AGAIN*/
		_ev.events = EPOLLIN | EPOLLHUP | EPOLLONESHOT;
		epoll_ctl(_efd, EPOLL_CTL_MOD, _ep_event[i].data.fd, &_ev);
	}
}
