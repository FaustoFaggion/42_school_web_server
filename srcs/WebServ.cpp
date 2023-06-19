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
	
	locations = file.getPath();
	_indexes = file.getIndex();
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

	while (_ep_event[j].data.fd)
	{
		double	timeout = difftime(time(NULL), map_connections[_ep_event[j].data.fd].start_connection);
		if (_ep_event[j].data.fd != _fd_listener)
		{
			if (timeout > 0.0)
			{
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

void	clean(std::string& str) {
    // Remove leading spaces
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));

    // Remove trailing spaces
    str.erase(std::find_if(str.rbegin(), str.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), str.end());
	str += '\0';
}

std::string	WebServ::looking_for_path(std::string path)
{
	std::string	html;

	/*IF REQUEST IS A LOCATION, APPEND INDEX.HTML FILES DEFINED INTO CONFIGURATION FILE*/
	if(locations.find(path) != locations.end())
	{

		html = locations[path] + "/" + _indexes.at(0);
		size_t i = 1;
		while (i < _indexes.size() && access(html.c_str(), F_OK) != 0)
		{
			html = locations[path] + "/" + _indexes.at(i);
			i++;
		}
	
		std::cout << "find path\n" << html << "\n";
		return (html);
	}

	/*IF REQUEST PATH NOT MATCH, IT TAKES THE LONGEST PATH THAT INICIATES WITH THE REQUEST*/
	size_t		s = 0;
	std::string	comp;
	if (*(path.end() - 1) != '/')
		comp = path + "/";
	else
		comp = path;

	for(std::map<std::string, std::string>::iterator it = locations.begin(); it != locations.end(); it++)
	{
		if ((*it).first.compare(0, comp.size(), comp) == 0)
		{
			if ((*it).first.size() > s)
			{
				html = (*it).second;
				html += "/index.html";
				s = (*it).first.size();
			}
		}
	}
	/*IF PATH MATCH, RETURN HTML STRING*/
	if (s > 0)
	{
		std::cout << "find alternative path\n" << html << "\n";
		return (html);
	}
	
	/*CHECK FOR FILE IN THE END OF THE PATH REQUEST*/
	if (path.find(".", 0) == path.npos)
	{
		html == path;
		std::cout << "path not found" << html << "\n";
		return (html);
	}
	
	/*REMOVE FILE FROM PATH TO LOOKING FOR INTO THE LOCATIONS MAP*/
	size_t start = path.find_last_of("/", path.size());
	std::cout << "start: " << start << std::endl;
	size_t end = path.size();
	std::cout << "end: " << end << std::endl;
	std::string	request_path = path.substr(0, start + 1);
	std::cout << "request_path: " << request_path << std::endl;
	std::string file = path.substr(start + 1, (end - (start + 1)));
	std::cout << "file: " << file << std::endl;
	if(locations.find(request_path) != locations.end())
	{
		html = locations[request_path] + "/" + file;
		std::cout << "find path with file: " << html << "\n";
	}
	else
	{
		html = path;
		std::cout << "path not found with file: " << html << "\n";
	}
	return (html);
}

void	WebServ::request_parser(std::string &request)
{
	std::fstream			conf_file;
	std::stringstream		buff;
	std::string 			html;


	// Find the end of the request line
   	size_t requestLineEnd = request.find("\r\n");

    // Extract the request line
    std::string requestLine = request.substr(0, requestLineEnd);

    // Parse the request line
    std::istringstream iss(requestLine);
    std::string method, path, protocol;
    iss >> method >> path >> protocol;

	//Raoni passou por aqui PARSE UNTIL GET PATH BEFORE ? SIGN
	size_t pos = path.find("?");
	if (pos != path.npos)
	{
		std::string tmp = path.substr(0, pos);
		path = tmp;

	}
	html = looking_for_path(path);

	std::cout << "Method: " << method << std::endl;
	std::cout << "Path: " << path << std::endl;
	std::cout << "Protocol: " << protocol << std::endl;
	std::cout << "html: " << html << " &html: " << &html << std::endl;
	std::cout << "html.c_str(): " << html.c_str() << "\n";

	if (method.compare("GET") == 0)
	{
		for (std::map<std::string, std::string>::iterator it = locations.begin(); it != locations.end(); it++)
		{
			std::cout << (*it).first << " : " << (*it).second << "\n";
		}
		std::cout << "OK" << "\n\n";
		conf_file.open(html.c_str() , std::fstream::in);
		if (conf_file.fail())
		{
			std::cout << "Configuration file fail to read" << std::endl;
			conf_file.open("./locations/test/error.html",  std::fstream::in);
			if (conf_file.fail())
				std::cout << "Configuration file fail to read" << std::endl;
			buff << conf_file.rdbuf();
		}
		else
			buff << conf_file.rdbuf();
		
		std::cout <<"BUFF\n" << buff.str() << "\n";
		
		request = "HTTP/1.1 200 OK\r\n";
    	request += "Content-Type: text/html\r\n";
		request += "Connection: close\r\n";
    	request += "\r\n";
    	request += buff.str();
		request += "\r\n";
		conf_file.close();
	}
	else if (method.compare("POST") == 0)
	{
		conf_file.open(html.c_str() , std::fstream::in);
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
	else if (method.compare("DELETE") == 0)
	{
		/*WRITE THE HTML FILE INTO A BUFFER STREAM TO CONCAT INTO THE HTTP RESPONSE*/

		conf_file.open(html.c_str() ,  std::fstream::in);
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
		conf_file.open("./locations/test/error.html",  std::fstream::in);
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