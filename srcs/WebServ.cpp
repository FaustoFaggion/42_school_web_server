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
	_listener.set_worker_connections(MAX_CONNECTIONS);

	std::cout << "domain: " << _listener.get_domain() << "\n";
	std::cout << "flag: " << _listener.get_flag() << "\n";
	std::cout << "type: " << _listener.get_type() << "\n";
	std::cout << "port: " << _listener.get_port() << "\n";
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
		if ((_nfds = epoll_wait (_efd, _ep_event, _listener.get_worker_connections(),  2000)) == -1) // '-1' to block indefinitely
			std::cout << "ERROR: epoll_wait" << std::endl;
		
		std::cout << "nfds: " << _nfds << "\n";
		
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
					std::cout << "receive data" << "\n";
					receive_data(i);
				}
			}
			/*CHECK IF EVENT TO WRITE*/
			else if ((_ep_event[i].events & EPOLLOUT) == EPOLLOUT)
			{
				std::cout << "response" << "\n";
				response(i);
			}
		}
	}
}

void	WebServ::delete_timeout_socket()
{
	int j = 0;
	std::cout << "while\n";
	while (_ep_event[j].data.fd)
	{
		double	timeout = difftime(time(NULL), map_connections[_ep_event[j].data.fd].start_connection);
		std::cout << "start time: " << map_connections[_ep_event[j].data.fd].start_connection << " time" << time(NULL) << " timeout: "<< timeout << " fd: " << _ep_event[j].data.fd << "\n";
		if (_ep_event[j].data.fd != _fd_listener)
		{
			if (timeout > 2.0)
			{
				std::cout << "now > 2.0\n";
				std::cout << "fd: " <<_ep_event[j].data.fd << "\n";
				int	fd = _ep_event[j].data.fd;

				/*DELETE FROM EPOLL AND CLOSE FD*/
				epoll_ctl(_efd, EPOLL_CTL_DEL, _ep_event[j].data.fd, &_ev);
				_ep_event[j].data.fd = 0;
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
	_ev.events = EPOLLIN | EPOLLOUT;
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
	c.fd = fd_new;
	c.start_connection = time(NULL);
	c.response = "";
	map_connections[fd_new] = c;
}

void	WebServ::receive_data(int i)
{
	char	buff[2048];


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
		map_connections[_ep_event[i].data.fd].response += buff;
	
		/*CHECK IF REQUEST DATA FINISHED*/
		std::map<int, t_client>::iterator	it;
		it = map_connections.find(_ep_event[i].data.fd);
		if ((*it).second.response.find("\r\n\r\n") != std::string::npos)
		{
			std::cout << "received data fd: " << _ep_event[i].data.fd << "\n";
			std::cout <<  (*it).second.response << "\n";
			request_parser((*it).second.response);

			/*SET FD SOCKET TO WRITE (EPOLLIN)*/
			_ev.events = EPOLLOUT;
			epoll_ctl(_efd, EPOLL_CTL_MOD, _ep_event[i].data.fd, &_ev);
		}
	}
}

void	WebServ::response(int i)
{
	std::map<int, t_client>::iterator	it;
	it = map_connections.find(_ep_event[i].data.fd);
	/*PROTECTION FROM CONNECTION HAND-SHAKE*/
	if (!(*it).second.response.empty())
	{
		std::cout << "inside response fd: " << _ep_event[i].data.fd << "\n" << (*it).second.response.c_str() << "\n";
		send(_ep_event[i].data.fd, (*it).second.response.c_str(), (*it).second.response.size(), 0);
		
		/*SET FD SOCKET TO READ AGAIN*/
		_ev.events = EPOLLIN;
		epoll_ctl(_efd, EPOLL_CTL_MOD, _ep_event[i].data.fd, &_ev);
	}
}

void	WebServ::request_parser(std::string &request)
{
	std::fstream			conf_file;
	std::stringstream		buff;

	std::cout << "request parser\n";
	if (strncmp("GET / HTTP/1.1", request.c_str(), 14) == 0)
	{
		conf_file.open("./srcs/locations/html_get.html",  std::fstream::in);
		if (conf_file.fail())
			std::cout << "Configuration file fail to read" << std::endl;
		buff << conf_file.rdbuf();

		request = "HTTP/1.1 200 OK\r\n";
    	request += "Content-Type: text/html\r\n";
		request += "Connection: close\r\n";
    	request += "\r\n";
    	request += buff.str();
		request += "\r\n";
		conf_file.close();
	}
	else if (strncmp("POST / HTTP/1.1", request.c_str(), 15) == 0)
	{
		conf_file.open("./srcs/locations/html_post.html",  std::fstream::in);
		if (conf_file.fail())
			std::cout << "Configuration file fail to read" << std::endl;
		buff << conf_file.rdbuf();

		request = "HTTP/1.1 200 OK\r\n";
    	request += "Content-Type: text/html\r\n";
    	request += "Connection: close\r\n";
		request += "\r\n";
    	request += buff.str();
		conf_file.close();
	}
	else if (strncmp("DELETE / HTTP/1.1", request.c_str(), 17) == 0)
	{
		/*WRITE THE HTML FILE INTO A BUFFER STREAM TO CONCAT INTO THE HTTP RESPONSE*/

		conf_file.open("./srcs/locations/html_del.html",  std::fstream::in);
		if (conf_file.fail())
			std::cout << "Configuration file fail to read" << std::endl;
		buff << conf_file.rdbuf();
		
		/*HTTP RESPONSE SYNTAX*/
		request = "HTTP/1.1 200 OK\r\n";
    	request += "Content-Type: text/html\r\n";
    	request += "Connection: close\r\n";
		request += "\r\n";
		request += buff.str();
		conf_file.close();
	}
	else
	{
		/*WRITE THE HTML FILE INTO A BUFFER STREAM TO CONCAT INTO THE HTTP RESPONSE*/
		conf_file.open("./srcs/locations/error.html",  std::fstream::in);
		if (conf_file.fail())
			std::cout << "Configuration file fail to read" << std::endl;
		buff << conf_file.rdbuf();

		/*HTTP RESPONSE SYNTAX*/
		request = "HTTP/1.1 404 Not Found\r\n";
		request += "Content-Type: text/html\r\n";
		request += "Connection: close\r\n";
		request += "\r\n";
		request += buff.str();
		conf_file.close();
	}
}