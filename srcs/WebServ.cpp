#include "WebServ.hpp"

WebServ::WebServ()
{

}

WebServ::WebServ(FileParser fileParser)
{
	_server_config_file = fileParser;
	_domain = 0;
	_type = 0;
	_flag = 0;
	_port = "";
	_server_name = fileParser.getServerName();
	_fd_listener = -1;
	_result = NULL;
	_location = fileParser.getLocations();
	_buff_size = fileParser.getBufferSize();
	_body_max_size = fileParser.getBodySize();
}

WebServ::~WebServ()
{
	if (_result != NULL)
		freeaddrinfo(_result);
}

int		WebServ::getFdListener() const
{
	return (_fd_listener);
}

FileParser							WebServ::getServerConfigFile() const
{
	return (_server_config_file);
}

int		WebServ::setupServers(int max_worker_connections)
{
	
	setup_domain_and_port(_server_config_file.getListen(), _domain, _port);
	setup_server_name_flag(_server_config_file.getServerName(), _flag);
	setup_error_pages();
	
	if (setup_addrinfo() == 1)
		return (1);

	if (ListenerSocket::setupSocket(_result, _fd_listener, max_worker_connections) == 1)
		return (1);
	
	return (0);
}

void	WebServ::parseListenLine(size_t i, std::string &line, std::string &response)
{
	while (i < line.size() && !isdigit(line.at(i)))
		i++;
	while (i < line.size() && isdigit(line.at(i)))
	{
		response += line.at(i);
		i++;
	}
}

void	WebServ::setup_domain_and_port(std::vector<std::string> listens, int &domain, std::string &port)
{
	for (std::vector<std::string>::iterator listen = listens.begin(); listen != listens.end(); listen++)
	{
		if ((*listen).compare(0, 11, "listen [::]") == 0)
		{
			if (domain == AF_INET)
				domain = AF_UNSPEC;
			else
				domain = AF_INET6;
			if (port == "")
			{
				parseListenLine(0, (*listen), port);
			}
		}
		else if ((*listen).compare(0, 6, "listen") == 0)
		{
			domain = AF_INET;
			parseListenLine(0, (*listen), port);
		}
	}
}

void	WebServ::setup_server_name_flag(std::string buff, int &flag)
{
	if (buff.compare(0, 11, "server_name") == 0)
	{
		size_t i = 12;
		while (i < buff.size() && isspace(buff.at(i)))
			i++;
		while (i < buff.size())
		{
			_server_name += buff.at(i);
			i++;
		}
		// if (strcmp("localhost", _server_name.c_str()) == 0)
		flag = AI_PASSIVE;
	}
}

void WebServ::setup_error_pages()
{
	std::string	error_path = "";
	std::string	error_code = "";
	std::string	str = "";
	std::vector<std::string> errorPathVector = _server_config_file.getErrorPath();

	for (std::vector<std::string>::iterator it = errorPathVector.begin(); it != errorPathVector.end(); it++)
	{
		std::istringstream iss(*it);

		iss >> str >> error_code >> error_path;
		/*IF ERROR PATH HAS '.' OR DO NOT HAVE '/' AT THE BEGIN*/
		if ((*(error_path.begin())) == '.')
			error_path = error_path.substr(1, (error_path.size() - 1));
		if (*(error_path.begin()) != '/')
		{
			std::string	tmp = "/" + error_path;
			error_path = tmp;
		}
		/*ADD ERROR_PAGE TO THE MAP*/
		if (_location.find("/") != _location.end())
			_error_page_map[error_code] = _location.at("/")._server_path + error_path;
		else
			_error_page_map[error_code] = error_path;
	}
}

int	WebServ::setup_addrinfo()
{
	memset(&_hints, 0, sizeof (struct addrinfo));
	memset(&_result, 0, sizeof (struct addrinfo *));
	
	_type = SOCK_STREAM;

	_hints.ai_family = _domain;		/* allow IPv4 or IPv6 */
	_hints.ai_socktype = _type;	/* Stream socket */
	_hints.ai_flags = _flag;		/* for wildcard IP address*/
	
	if (_server_name.compare("") == 0)
	{
		if ((_s = getaddrinfo (NULL, _port.c_str(), &_hints, &_result)) != 0)
		{
			std::cout << "getaddrinfo: " << gai_strerror (_s) << std::endl;
			return (1);
		}
		return (0);
	}
	else
	{
		if ((_s = getaddrinfo (_server_name.c_str(), _port.c_str(), &_hints, &_result)) != 0)
		{
			std::cout << "getaddrinfo: " << gai_strerror (_s) << std::endl;
			return (1);
		}
		return (0);
	}
}

//GETTERS
std::string		WebServ::get_port() const
{
	return (_port);
}

std::map<std::string, directive>	WebServ::getLocation() const
{
	return (_location);
}

std::vector<std::string> 			WebServ::getWebservIndex() const
{
	return (_webserv_index);
}

size_t								WebServ::getBuffSize() const
{
	return (_buff_size);
}

size_t								WebServ::getBodyMaxSize() const
{
	return (_body_max_size);
}

std::vector<std::string>			WebServ::getAllowedMethods() const
{
	return (_allowed_methods);
}

std::map<std::string, std::string>	WebServ::getErrorPageMap() const
{
	return (_error_page_map);
}

std::string							WebServ::getServerName() const
{
	return (_server_name);
}