#include "ListenerSocket.hpp"

ListenerSocket::ListenerSocket()
{
	memset(&_hints, 0, sizeof(struct addrinfo));
	memset(&_result, 0, sizeof(struct addrinfo));
	_s = 0;
	_fd_listener = 0;

}

ListenerSocket::~ListenerSocket()
{
	freeaddrinfo(_result);
}

int		ListenerSocket::get_fd_listener()
{
	return (_fd_listener);
}

int			ListenerSocket::get_domain() const
{
	return (_domain);
}

std::string	ListenerSocket::get_port() const
{
	return (_port);
}

int			ListenerSocket::get_flag() const
{
	return (_flag);
}

int			ListenerSocket::get_worker_connections() const
{
	return (_worker_connections);
}

int			ListenerSocket::get_type() const
{
	return (_type);
}

void		ListenerSocket::set_domain(int domain)
{
	_domain = domain;
}

void		ListenerSocket::set_port(std::string port)
{
	_port = port;
}

void		ListenerSocket::set_flag(int flag)
{
	_flag = flag;
}

void		ListenerSocket::set_worker_connections(int connections)
{
	_worker_connections = connections;
}

void		ListenerSocket::set_type(int type)
{
	_type = type;
}



std::string	ListenerSocket::str_substring(std::string &str, std::string find, int init, char finish)
{
	std::string	response = "";
	if (str.find(find, init) != str.npos)
	{
		size_t start = str.find(find, 0);
		size_t end = str.find(finish, start);
		
		response = str.substr(start, (end - start));
		str.erase(start, (end- start));
	}
	return (response);
}

void	ListenerSocket::chk_simple_directive(std::string &str)
{

	size_t end = str.find(";", 0);
	if (end == str.npos)
	{
		std::cout << "ERROR:root missing ';'" << std::endl;
		exit(2);
	}
	str = str.substr(0, end);
}

void	ListenerSocket::setup_listener(std::string buff)
{
	if (buff.compare(0, 11, "listen [::]") == 0)
	{
		if (_domain == AF_INET)
			_domain = AF_UNSPEC;
		else
			_domain = AF_INET6;
		if (_port == "")
		{
			size_t i = 0;
			while (i < buff.size() && !isdigit(buff.at(i)))
				i++;
			while (i < buff.size() && isdigit(buff.at(i)))
			{
				_port += buff.at(i);
				i++;
			}
		}
	}
	else if (buff.compare(0, 6, "listen") == 0)
	{
		
		_domain = AF_INET;
		
		size_t i = 0;
		while (i < buff.size() && !isdigit(buff.at(i)))
			i++;
		while (i < buff.size() && isdigit(buff.at(i)))
		{
			_port += buff.at(i);
			i++;
		}
	}
	else if (buff.compare(0, 11, "server_name") == 0)
	{
		std::string	server_name;

		size_t i = 12;
		while (i < buff.size() && isspace(buff.at(i)))
			i++;
		while (i < buff.size())
		{
			server_name += buff.at(i);
			i++;
		}
		if (strcmp("localhost", server_name.c_str()) == 0)
			_flag = AI_PASSIVE;

	}
	else if (buff.compare(0, 18, "worker_connections") == 0)
	{
		std::string	tmp;
		
		size_t i = 19;
		while (i < buff.size() && isspace(buff.at(i)))
			i++;
		while (i < buff.size() && isdigit(buff.at(i)))
		{
			tmp += buff.at(i);
			i++;
		}
		_worker_connections = atoi(tmp.c_str());
	}
}

void	ListenerSocket::parse_listener(std::string &_server_conf_file)
{
	std::cout << "PARSER_LISTENER FUNCTION\n\n";
	std::cout << _server_conf_file << "\n\n";
	// /*PARSE SERVEER FAMILY AND PORT*/
	std::string	ltn;
	while (_server_conf_file.find("listen", 0) != _server_conf_file.npos)
	{
		ltn = str_substring(_server_conf_file, "listen", 0, '\n');
		chk_simple_directive(ltn);
		setup_listener(ltn);
	}

	// // /*PARSE SERVER_NAME*/
	std::string	sn;;
	while (_server_conf_file.find("server_name", 0) != _server_conf_file.npos)
	{
		sn = str_substring(_server_conf_file, "server_name", 0, '\n');
		chk_simple_directive(sn);
		setup_listener(sn);
	}

	// // /*PARSE WORKER_PROCESSES*/
	std::string	wp;;
	while (_server_conf_file.find("worker_connections", 0) != _server_conf_file.npos)
	{
		wp = str_substring(_server_conf_file, "worker_connections", 0, '\n');
		chk_simple_directive(wp);
		setup_listener(wp);
	}

}

void		ListenerSocket::addrinfo(int domain, int type, int flag, std::string port)
{
	memset(&_hints, 0, sizeof (struct addrinfo));
	_hints.ai_family = domain;		/* allow IPv4 or IPv6 */
	_hints.ai_socktype = type;	/* Stream socket */
	_hints.ai_flags = flag;		/* for wildcard IP address*/
	if ((_s = getaddrinfo (NULL, port.c_str(), &_hints, &_result)) != 0)
	{
		std::cout << stderr << "getaddrinfo: " << gai_strerror (_s) << std::endl;
		exit (EXIT_FAILURE);
	}
}

void	ListenerSocket::create_fd()
{
	int optval = 1;
	_fd_listener = socket (_result->ai_family, _result->ai_socktype,
							_result->ai_protocol);
		if (_fd_listener == -1)
			std::cout << "ERROR: fd_listener" << std::endl;

		if (setsockopt (_fd_listener, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (int)) == -1)
			std::cout << "ERROR: setsockopt" << std::endl;
}

void	ListenerSocket::bind_fd_to_port()
{
	if (bind (_fd_listener, _result->ai_addr, _result->ai_addrlen) == 0)  // Success
	{
		return ;
	}
	if (close (_fd_listener) == -1)
		std::cout << "ERROR: close"<< std::endl;

}

void	ListenerSocket::listen_fd()
{
	if (listen (_fd_listener, MAX_CONNECTIONS) == -1)
		strerror(errno);
} 	