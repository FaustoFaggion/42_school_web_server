#include "WebServ.hpp"

WebServ::WebServ()
{
	_domain = 0;
	_type = 0;
	_flag = 0;
	_port = "";
	_fd_listener = 0;
}

WebServ::WebServ(int type)
{
	_listener.addrinfo(_domain, type, _flag, _port);
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

void	WebServ::fill_struct_conf_file(std::string buff)
{
	
	if (strncmp("listen [::]", buff.c_str(), 11) == 0)
	{
		if (_domain == AF_INET)
			_domain = AF_UNSPEC;
		else
			_domain = AF_INET6;
		if (_port == "")
		{
			int i = 12;
			while (isdigit(buff.at(i)) == true)
			{
				_port += buff.at(i);
				i++;
			}
		}
	}
	else if (strncmp("listen", buff.c_str(), 6) == 0)
	{
		_domain = AF_INET;
		int i = 7;
		while (isdigit(buff.at(i)) == true)
		{
			_port += buff.at(i);
			i++;
		}
	}
	else if (strncmp("server_name", buff.c_str(), 11) == 0)
	{
		std::string	server_name;
		int i = 12;

		while (buff.at(i) != '\0')
		{
			server_name += buff.at(i);
			i++;
		}
		if (strcmp("localhost", server_name.c_str()) == 0)
			_flag = AI_PASSIVE;

	}
	else if (strncmp("worker_connections", buff.c_str(), 18) == 0)
	{
		int i = 18;
		std::string	tmp;
		while (isspace(buff.at(i)))
			i++;
		while (isdigit(buff.at(i)))
		{
			tmp += buff.at(i);
			i++;
		}
		_max_connections = atoi(tmp.c_str());
	}
}

void	cleanSpaces(std::string& str) {
    // Remove leading spaces
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));

    // Remove trailing spaces
    str.erase(std::find_if(str.rbegin(), str.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), str.end());
	str += '\0';
}

void	WebServ::parse_file(char *file)
{
	std::fstream	conf_file;
	std::string		buff;

	conf_file.open(file,  std::fstream::in);
	if (conf_file.fail())
		std::cout << "Configuration file fail to read" << std::endl;
	while (1)
	{
		std::getline(conf_file, buff,'\n');
		cleanSpaces(buff);
		fill_struct_conf_file(buff);
		std::cout << buff << std::endl;
		if (conf_file.eof())
			break ;

	}

}

void	WebServ::setup_server(int type)
{
	_listener.addrinfo(_domain, type, _flag, _port);
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
