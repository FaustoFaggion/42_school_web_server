#include "Monitor.hpp"

Monitor::Monitor()
{
	_servers_config_file = "";
}

Monitor::~Monitor()
{
	for (std::vector<WebServ*>::iterator it = _web_servers.begin(); it!= _web_servers.end(); it++)
		delete (*it);
}

std::vector<WebServ*>	Monitor::getWebServers() const
{
	return (_web_servers);
}


std::vector<int>		Monitor::getVectorFds() const
{
	return (_vector_fds);
}

struct epoll_event		Monitor::getMev() const
{
	return (_m_ev);
}

int						Monitor::getMefd() const
{
	return (_m_efd);
}

void	Monitor::initialize_monitor(char *file)
{
	if (parse_server_conf_file(file) == 1)
		return;
	struct epoll_event	tmp[_max_worker_connections];
	_m_ep_event = tmp;
	memset(_m_ep_event, 0, sizeof(_m_ep_event[_max_worker_connections]));
	// memset(&_m_ev, 0, sizeof(_m_ev));
	create_connections();

	for(std::vector<WebServ*>::iterator it = _web_servers.begin(); it != _web_servers.end(); it++)
	{
		_map_web_servers[(*it)->getFdListener()] = *it;
	}
	run();
}

int		Monitor::parse_server_conf_file(char *file)
{
	try {
		FileParser::parseHead(file, _servers_config_file, _events, _max_worker_connections, _servers);
		for (std::vector<std::string>::iterator it = _servers.begin(); it != _servers.end(); it++)
		{
			FileParser	fileParser = FileParser::parseServersBlock(*it);
			WebServ		*webServ = new WebServ(fileParser);
			if (webServ->setupServers(_max_worker_connections) == 0)
				_web_servers.push_back(webServ);
			else
				delete (webServ);
		}
	} catch (ServerExceptions &e){
		std::cout << e.what() << std::endl;
		return (1);
	}
	return (0);
}

void	Monitor::create_connections()
{
	_m_ev.data.fd = 0;
	_m_ev.data.ptr = NULL;
	_m_ev.data.u32 = 0;
	_m_ev.data.u64 = 0;
	
	
	/*EPOLL FUNCTION*/
	if ((_m_efd = epoll_create(_max_worker_connections)) == -1)
		std::cout << "ERROR: epoll_create" << std::endl;
	_vector_fds.push_back(_m_efd);

	for (std::vector<WebServ*>::iterator it = _web_servers.begin(); it != _web_servers.end(); it++)
	{
		/*SET FD SOCKET TO NONBLOCKING*/
		int fd_flag = fcntl((*it)->getFdListener(), F_GETFL, 0);
		fcntl((*it)->getFdListener(), F_SETFL, fd_flag | O_NONBLOCK);
		
		/*FD IS AVAIABLE FOR READ*/
		_m_ev.events = EPOLLIN;
		_m_ev.data.fd = (*it)->getFdListener();

		/*SET FD SOCKET TO NONBLOCKING*/
		if (epoll_ctl (_m_efd, EPOLL_CTL_ADD, (*it)->getFdListener(), &_m_ev) == -1)
			std::cout << "ERROR: epoll_ctl" << std::endl;
		
		/*VECTOR OF FDS USED TO HANDLE OPENED FDS WHEN CLOSE THE APPLICATION*/
		_vector_fds.push_back((*it)->getFdListener());
	}
}

int		Monitor::chk_listener(int ep_event_fd)
{
	for(std::map<int, WebServ*>::iterator it = _map_web_servers.begin(); it != _map_web_servers.end(); it++)
	{
		if (it->first == ep_event_fd)
			return (ep_event_fd);
	}
	return (-1);
}

void	setrunning(bool &run)
{
	static bool *st = NULL;

	if (st == NULL)
		st = &run;
	else
		*st = false;
}

void	signalhandler(int signum)
{
	bool sig;

	setrunning(sig);

	/*Only to compile*/
	(void)signum;
}

void	Monitor::run()
{
	_m_nfds = 0;
	bool	running = true;
	setrunning(running);
	signal(SIGINT, signalhandler);
	
	while (running == true)
	{
		/*A REQUEST TO SERVER SHOULD NEVER HANG FOREVER.*/

		/*MONITOR FDS. STILL WAITING UNTIL A EVENT HEAPPENS IN A FD*/
		if ((_m_nfds = epoll_wait (_m_efd, _m_ep_event, _max_worker_connections, 0)) == -1) // '-1' to block indefinitely
			std::cout << "ERROR: epoll_wait" << std::endl;

		/*LOOP INTO EPOLL READY LIST*/
		for (int i = 0; i < _m_nfds; i++)
		{
			/*CHECK IF EVENT TO READ*/
			if 	((_m_ep_event[i].events & EPOLLIN) == EPOLLIN)
			{

				/*LISTENER SOCKET*/
				if (chk_listener(_m_ep_event[i].data.fd) != -1)
				{
					if (_vector_fds.size() < (size_t)_max_worker_connections)
						accept_new_connection(_map_web_servers.at(_m_ep_event[i].data.fd));
				}
				/*CLIENT SOCKET*/
				else
					receive_data(i);
			}
			/*CHECK IF EVENT TO WRITE*/
			else if ((_m_ep_event[i].events & EPOLLOUT) == EPOLLOUT)
				send_response(i);
		}
		delete_timeout_socket();
	}
	
	close(*(_vector_fds.begin()));
	_vector_fds.erase(_vector_fds.begin());
	for (std::vector<int>::iterator it = _vector_fds.begin(); it != _vector_fds.end(); it++)
	{

		_m_ev.events = EPOLLOUT;
		_m_ev.data.fd = *it;

		epoll_ctl(_m_efd, EPOLL_CTL_DEL, *it, &_m_ev);
		close(*it);
	}
}

void	Monitor::delete_timeout_socket()
{
	for (std::map<int, t_client>::iterator it = _map_connect.begin(); it != _map_connect.end(); it++)
	{
		
		double	timeout = difftime(time(NULL), (*it).second.connection_time);

			if (timeout > CONNECTION_MAX_TIME)
			{
				int	fd = (*it).second.fd;
			
				/*DELETE FROM EPOLL AND CLOSE FD*/
				_m_ev.events = EPOLLOUT;
				_m_ev.data.fd = fd;
				if (epoll_ctl(_m_efd, EPOLL_CTL_DEL, fd, &_m_ev) == -1)
					std::cout << "EPOLL_CTL_DEL FAIL fd: " << (*it).second.fd << "\n";
				close(fd);
				_map_connect.erase(fd);
				
				for (std::vector<int>::iterator it = _vector_fds.begin(); it != _vector_fds.end(); it++)
				{
					if (fd == *it)
					{
						_vector_fds.erase(it);
						break ;
					}
				}
				break ;
			}

	}
}

void		Monitor::close_fd(int i)
{
	int fd = _m_ep_event[i].data.fd;

	_map_connect.erase(fd);

	for (std::vector<int>::iterator it = _vector_fds.begin(); it != _vector_fds.end(); it++)
	{
		if (fd == *it)
		{
			_vector_fds.erase(it);
			break ;
		}
	}

	/*DELETE FD FROM EPOLL*/
	_m_ev.events = EPOLLOUT;
	_m_ev.data.fd = fd;
	if (epoll_ctl (_m_efd, EPOLL_CTL_DEL, fd, &_m_ev) == -1)
		std::cout << "ERROR: epoll_ctl_del receive_data" << std::endl;
	
	/*CLOSE FD*/
	if (close (fd) == -1)
		std::cout << "ERROR: close fd receive_data" << std::endl;
}

void	Monitor::accept_new_connection(WebServ *server)
{
	_m_addrlen = sizeof (struct sockaddr_storage);

	/*CREATE NEW FILE DESCRIPTOR TO CONNECTO TO CLIENT*/
	int fd_new;
	if ((fd_new = accept (server->getFdListener(), (struct sockaddr *) &_m_client_saddr, &_m_addrlen)) == -1)
		std::cout << "ERROR: accept" << std::endl;
	
	/*ADD NEW FD EPOOL TO MONNITORING THE EVENTS*/
	_m_ev.events = EPOLLIN;
	_m_ev.data.fd = fd_new;
	
	if (epoll_ctl (_m_efd, EPOLL_CTL_ADD, fd_new, &_m_ev) == -1)
		std::cout << "ERROR: epoll_ctl" << std::endl;
	else if (_m_client_saddr.ss_family != AF_INET &&_m_client_saddr.ss_family != AF_INET6)
	{
		std::cout << " Address family is neither AF_INET nor AF_INET6" << std::endl;
	}

	/*VECTOR OF FDS USED TO HANDLE OPENED FDS*/
	_vector_fds.push_back(fd_new);
	
	/*ADD fd_new TO MAP_CONNECTIONS AND SET TO EMPTY*/
	_map_connect[fd_new];
	initialize_client_struct(server, _map_connect, fd_new);
}

void	Monitor::initialize_client_struct(WebServ *server, std::map<int, t_client> &map, int fd_new)
{
	map.at(fd_new).fd = fd_new;

	/*TIMEOUT CONNECTION*/
	map.at(fd_new).connection_time = time(NULL);
	
	/*PHP_CGI*/
	map.at(fd_new).pipe0[0] = 0;
	map.at(fd_new).pipe0[1] = 0;
	map.at(fd_new).pipe1[0] = 0;
	map.at(fd_new).pipe1[1] = 0;

	map.at(fd_new)._response = "";
	
	/*REQUEST HEADER*/
	std::ostringstream oss;
    oss << server->getBodyMaxSize();
	map.at(fd_new)._max_body_length = oss.str();
	map.at(fd_new)._header = "";
	map.at(fd_new)._method = "";
	map.at(fd_new)._url = "";
	map.at(fd_new)._protocol = "";
	map.at(fd_new)._content_type = "";
	map.at(fd_new)._content_length = "";
	map.at(fd_new)._server_name = server->getServerName();
	map.at(fd_new)._server_port = "";
	map.at(fd_new)._user_agent = "";
	map.at(fd_new)._http_host = "";
	map.at(fd_new)._http_accept = "";
	map.at(fd_new)._http_accept_encoding = "";
	map.at(fd_new)._http_accept_language = "";
	map.at(fd_new)._query_string = "";
	map.at(fd_new)._path_info = "";
	map.at(fd_new)._request_uri = "";
	map.at(fd_new)._remote_host = "";
	map.at(fd_new)._boundary = "";
	map.at(fd_new)._url_file = "";
	map.at(fd_new)._url_file_extension = "";
	map.at(fd_new)._url_location = "";
	map.at(fd_new)._server_path = "";
	map.at(fd_new)._keep_alive = false;
	
	/*REQUEST BODY*/
	map.at(fd_new)._body = "";
	
	/*SERVER CONFIGURATION FILE*/
	map.at(fd_new)._upload_content_size = 0;
	map.at(fd_new)._upload_buff_size = server->getBuffSize();

	/*PROGRAM LOGIC*/
	map.at(fd_new)._response_step_flag = 0;
	map.at(fd_new)._status_code = "200";
	map.at(fd_new)._status_msg = "OK";

	map.at(fd_new)._location_ = server->getLocation();
	map.at(fd_new)._index_ = server->getWebservIndex();
	map.at(fd_new)._allowed_methods = server->getAllowedMethods();
	map.at(fd_new)._error_page_map = server->getErrorPageMap();
}

void	Monitor::receive_data(int i)
{
	std::map<int, t_client>::iterator	it;
	it = _map_connect.find(_m_ep_event[i].data.fd);
	/*PROTECTION AGAINST NOT EXISTED FD IN _MAP_CONNECT*/
	if (it == _map_connect.end())
		return;
	char	buff[(*it).second._upload_buff_size];
	memset (&buff, '\0', sizeof (buff));
	
	/*RECEIVING CLIENT DATA CHUNCKS REQUEST */
	ssize_t numbytes = recv (_m_ep_event[i].data.fd, &buff, sizeof(buff), 0);
	if (numbytes == -1)
		close_fd(i);
	
	/*CONNECTION CLOSED BY THE CLIENT*/
	else if (numbytes == 0 )
	{
		std::cout << "Socket " << _m_ep_event [i].data.fd << " closed by client" << std::endl;
	
		close_fd(i);
	}
	else 
	{
		std::string	tmp(buff, sizeof(buff));
		/*CHECK IF REQUEST DATA FINISHED*/
		if ((*it).second._response_step_flag == 0 && tmp.find("\r\n\r\n") == std::string::npos)
			_map_connect.at(_m_ep_event[i].data.fd)._header += buff;
		else
		{
			if ((*it).second._response_step_flag == 0)
				verify_received_data((*it).second, tmp);

			if ((*it).second._response_step_flag == 20)
				write_body_chunks((*it).second, buff, numbytes);
			
			if ((*it).second._response_step_flag == 30)
				save_file((*it).second);

			if ((*it).second._response_step_flag == 1)
				decide_how_to_respond((*it).second);
			
			if ((*it).second._response_step_flag == 3)
				write_remaining_body_chunks_to_cgi((*it).second, buff, numbytes);

			if ((*it).second._response_step_flag == 2)
				write_first_body_chunk_to_cgi((*it).second);

			if ((*it).second._response_step_flag == 200)
				delete_method((*it).second);
		}
	}
}

void	Monitor::verify_received_data(t_client &client, std::string buff)
{
	split_header_from_body(client, buff);
	request_parser(client);
	split_body_from_header(client, buff);
	looking_for_path(client);
	if (client._status_code.compare("200") == 0)
	{
		client._response_step_flag = 1;
	}
	else
	{
		client._response_step_flag = 5;
		_m_ev.events = EPOLLOUT;
		_m_ev.data.fd = client.fd;
		epoll_ctl(_m_efd, EPOLL_CTL_MOD, client.fd, &_m_ev);
	}
}

void	Monitor::decide_how_to_respond(t_client &client)
{
	std::string	cgi_file_extension = client._location_.at(client._url_location)._cgi_pass;
	if(client._location_.at(client._url_location)._cgi_pass.compare("") != 0 && cgi_file_extension.compare(client._url_file_extension) == 0)
	{
		exec_cgi(client._server_path, client);
		client._response_step_flag = 2;
	}
	else
	{
		if (client._method.compare("POST") == 0)
			client._response_step_flag = 20;
		else if (client._method.compare("DELETE") == 0)
			client._response_step_flag = 200;
		else
		{
			client._response_step_flag = 5;
			_m_ev.events = EPOLLOUT;
			_m_ev.data.fd = client.fd;
			epoll_ctl(_m_efd, EPOLL_CTL_MOD, client.fd, &_m_ev);
		}
	}
}

void	Monitor::delete_method(t_client &client)
{
	std::string filename = client._server_path;

	if (remove(filename.c_str()) == 0)
	{
		http_response_delete_file(client);
		client._response_step_flag = 6;
	}
	else {
		client._status_code = "500";
		client._status_msg = "Internal Server Error";
		client._response_step_flag = 5;

	}

	_m_ev.events = EPOLLOUT;
	_m_ev.data.fd = client.fd;
	epoll_ctl(_m_efd, EPOLL_CTL_MOD, client.fd, &_m_ev);

}

void	Monitor::write_body_chunks(t_client &client, char *buff, size_t numbytes)
{
	std::string	tmp(buff, sizeof(buff));
	client._body += tmp;
	client._upload_content_size += numbytes;

	if (client._upload_content_size >= (size_t)atoi(client._content_length.c_str()))
	{
		client._response_step_flag = 30;
		
		/*CHECK ERROR CONTENT_LENGTH*/
		if (client._status_code.compare("200") == 0 && atoi(client._content_length.c_str()) > atoi(client._max_body_length.c_str()))
		{
			client._status_code = "413";
			client._status_msg = "Request Entity Too Large";
			client._response_step_flag = 5;

			_m_ev.events = EPOLLOUT;
			_m_ev.data.fd = client.fd;
			epoll_ctl(_m_efd, EPOLL_CTL_MOD, client.fd, &_m_ev);
		}
	}

}

void	Monitor::save_file(t_client &client)
{
	size_t start, end;
	
	start = client._body.find("filename=", 0);
	start = start + 10;
	end = client._body.find("\"", start);
	
	std::string filename = "./server_root/simple_upload/files/";
	
	filename += client._body.substr(start, (end - start));
	
	std::string requestBody = client._body;
	start = requestBody.find(client._boundary) + client._boundary.length() + 2; // Add 2 to skip CRLF
	end = requestBody.find(client._boundary, start) - 4; // Subtract 4 to exclude CRLF-- at the end
	
	std::string fileContent = requestBody.substr(start, end - start);
	std::ofstream outfile(filename.c_str());
	
	if (outfile.is_open()) {
		outfile << fileContent;
		outfile.close();
		http_response_save_file(client);
		client._response_step_flag = 6;
	}
	else
	{
		std::cerr << "Unable to open file '" << filename << "' for writing." << std::endl;
		client._status_code = "500";
		client._status_msg = "Internal Server Error";
		client._response_step_flag = 5;
	}

	_m_ev.events = EPOLLOUT;
	_m_ev.data.fd = client.fd;
	epoll_ctl(_m_efd, EPOLL_CTL_MOD, client.fd, &_m_ev);
}

void	Monitor::write_first_body_chunk_to_cgi(t_client &client)
{
	if (client._method == "POST")
	{
			close(client.pipe0[0]);
			if (write(client.pipe0[1], client._body.c_str(), client._body.size()) == -1)
			{
				kill(client._pid, SIGTERM);
				client._response_step_flag = 5;
				client._status_code = "500";
				client._status_msg = "Internal Server Error";
				close(client.pipe1[0]);
				return;
			}
			client._upload_content_size = client._body.length();

			if (client._upload_content_size >= (size_t)atoi(client._content_length.c_str()))
			{
				/*CHECK ERROR CONTENT_LENGTH*/
				if (client._status_code.compare("200") == 0 && atoi(client._content_length.c_str()) > atoi(client._max_body_length.c_str()))
				{
					client._status_code = "413";
					client._status_msg = "Request Entity Too Large";
					client._response_step_flag = 5;
					close(client.pipe1[0]);
					close(client.pipe1[1]);
				}
				else
				{
					close(client.pipe0[1]);
					client._response_step_flag = 4;
				}
					_m_ev.events = EPOLLOUT;
					_m_ev.data.fd = client.fd;
					epoll_ctl(_m_efd, EPOLL_CTL_MOD, client.fd, &_m_ev);
			}
			else
				client._response_step_flag = 3;
	}
	else if (client._method == "GET")
	{
		close(client.pipe0[0]);
		close(client.pipe0[1]);
		client._response_step_flag = 4;
		_m_ev.events = EPOLLOUT;
		_m_ev.data.fd = client.fd;
		epoll_ctl(_m_efd, EPOLL_CTL_MOD, client.fd, &_m_ev);
	}
	else if (client._method == "DELETE")
	{
		close(client.pipe0[0]);
		close(client.pipe0[1]);
		client._response_step_flag = 4;
		_m_ev.events = EPOLLOUT;
		_m_ev.data.fd = client.fd;
		epoll_ctl(_m_efd, EPOLL_CTL_MOD, client.fd, &_m_ev);
	}

}

void	Monitor::write_remaining_body_chunks_to_cgi(t_client &client, char *buff, size_t numbytes)
{
	if (client._method == "POST")
	{
		/*RECEIVING CLIENT DATA CHUNCKS REQUEST */
		if (write(client.pipe0[1], buff, numbytes) == -1)
		{
			kill(client._pid, SIGTERM);
			client._response_step_flag = 5;
			client._status_code = "500";
			client._status_msg = "Internal Server Error";
			close(client.pipe1[0]);
			return;
		}
		client._upload_content_size += numbytes;
		if (client._upload_content_size >= (size_t)atoi(client._content_length.c_str()))
		{
			close(client.pipe0[1]);
			client._response_step_flag = 4;
			_m_ev.events = EPOLLOUT;
			_m_ev.data.fd = client.fd;
			epoll_ctl(_m_efd, EPOLL_CTL_MOD, client.fd, &_m_ev);
		
			/*CHECK ERROR CONTENT_LENGTH*/
			if (client._status_code.compare("200") == 0 && atoi(client._content_length.c_str()) > atoi(client._max_body_length.c_str()))
			{
				client._status_code = "413";
				client._status_msg = "Request Entity Too Large";
				client._response_step_flag = 5;
				close(client.pipe1[0]);
				close(client.pipe1[1]);

			}
		}
	}
}

void	Monitor::split_header_from_body(t_client &client, std::string buff)
{
	size_t	pos;

	pos = buff.find("\r\n\r\n");
	pos += 4;
	client._header += buff.substr(0, pos);
}

void	Monitor::split_body_from_header(t_client &client, std::string buff)
{
	size_t	pos;
	size_t	content_length = (size_t)atoi(client._content_length.c_str());
	pos = buff.find("\r\n\r\n");
	pos += 4;
	if (content_length < (buff.length() - pos))
	{
		client._body = buff.substr(pos, (content_length));
		client._upload_content_size = client._body.length();
	}
	else
	{
		client._body = buff.substr(pos, (buff.size() - pos));
		client._upload_content_size = client._body.length();
	}
}

void	Monitor::request_parser(t_client &client)
{
	size_t			pos, start, end;
	std::string		tmp;
	std::string		requestLine;

	/*REDIRECT*/

	/*METHOD, URL, PROTOCOL*/
	size_t requestLineEnd = client._header.find("\r\n");
	requestLine = client._header.substr(0, requestLineEnd);
	std::istringstream iss0(requestLine);
	iss0 >> client._method >> client._url >> client._protocol;

	/*URL, QUERY_STRING*/
	pos = client._url.find("?");
	if (pos != client._url.npos)
	{
		std::istringstream iss0(client._url);
		getline(iss0, tmp, '?');
		getline(iss0, client._query_string, '?');
		client._url = tmp;
	}

	/*_URL_FILE, _URL_FILE_EXTENSION, _URL_LOCATION*/
	pos = client._url.find(".");
	if (pos != client._url.npos)
	{
		end = client._url.size() - pos;
		client._url_file_extension = client._url.substr(pos, end);
		
		start = client._url.find_last_of("/");
		end = client._url.size() - start;
		client._url_file = client._url.substr(start, end);

		end = start;
		client._url_location = client._url.substr(0, end);
		if (client._url_location == "")
			client._url_location = '/';
	}
	else
	{
		if (client._url.compare("/") != 0)
		{
			// if (client._url.at(client._url.size() - 1) == '/')
			// {
			// 	client._url.erase(client._url.size() -1);
			// }
		}
		client._url_location = client._url;
	}

	/*CHECK METHOD ERROR*/
	if (client._location_.find(client._url_location) != client._location_.end())
	{
		bool	er = 0;
		if (client._location_.at(client._url_location)._allowed_methods.size() > 0)
		{
			er = 1;
			for (std::vector<std::string>::iterator it = client._location_.at(client._url_location)._allowed_methods.begin(); it != client._location_.at(client._url_location)._allowed_methods.end(); it++ )
			{
				if (client._method.compare(*it) == 0)
					er = 0;
			}
		}

		/*CHECK FOR UNKNOW METHOD*/
		std::string	methods_list[] = {"GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH"};
		bool		unknow = true;

		for (size_t i = 0; i < 9; i++)
		{
			if (methods_list[i].compare(client._method) == 0)
			{
				unknow = false;
				break ;
			}
		}

		if (er == 1)
		{
			client._status_code = "405";
			client._status_msg = "Method Not Allowed";
		}
		if (unknow == true)
		{
			client._status_code = "501";
			client._status_msg = "Not Implemented";
		}
	} 
	
	/*CHECK REDIRECTION*/
	if (client._status_code.compare("200") == 0)
	{
		if (client._location_.find(client._url_location) != client._location_.end())
		{
			if (client._location_.at(client._url_location)._redirect_code.compare("") != 0)
			{
				client._status_code = client._location_.at(client._url_location)._redirect_code;
				if (client._status_code.compare("301") == 0)
					client._status_msg = "Move Permanently";
				else if (client._status_code.compare("307") == 0)
					client._status_msg = "Temporary Redirect";
				else if (client._status_code.compare("308") == 0)
					client._status_msg = "Permanent Redirect";
			}
		}
	}

	/*PATH_INFO, REQUEST_URI*/
	{
		start = client._url.find("/");
		end = client._url.size() - start;
		client._path_info = client._url.substr(start, end);
		client._request_uri = client._url.substr(start, end);
	}

	/*CONTENT_TYPE*/
	pos = client._header.find("Content-Type: ");
	if (pos != client._header.npos)
	{
		requestLine = parse_line(client._header, "Content-Type: ", "\r\n");
		std::istringstream iss0(requestLine);
		getline(iss0, tmp, ' ');
		getline(iss0, client._content_type, ' ');
		getline(iss0, client._boundary, ' ');
		client._content_type += " " + client._boundary;
	}

	/*CONTENT_LENGTH*/
	pos = client._header.find("Content-Length: ");
	if (pos != client._header.npos)
	{
		requestLine = parse_line(client._header, "Content-Length: ", "\r\n");
		std::istringstream iss0(requestLine);
		getline(iss0, tmp, ' ');
		getline(iss0, client._content_length, ' ');
	}
	else
	{
		if (client._status_code.compare("200") == 0 && client._method.compare("POST") == 0)
		{
			client._status_code = "411";
			client._status_msg = "Length Required";
		}
	}

	/*CHECK ERROR CONTENT_LENGTH*/
	// if (client._status_code.compare("200") == 0 && atoi(client._content_length.c_str()) > atoi(client._max_body_length.c_str()))
	// {
	// 	client._status_code = "413";
	// 	client._status_msg = "Request Entity Too Large";
	// }

	/*HTTP_HOST*/
	pos = client._header.find("Host: ");
	if (pos != client._header.npos)
	{
		std::string	host;

		requestLine = parse_line(client._header, "Host: ", "\r\n");
		std::istringstream iss0(requestLine);
		getline(iss0, tmp, ' ');
		getline(iss0, client._http_host, ' ');
		std::istringstream iss1(client._http_host);
		getline(iss1, host, ':');
		getline(iss1, client._server_port, ':');
	}

	/*USER_AGENT*/
	pos = client._header.find("User-Agent: ");
	if (pos != client._header.npos)
	{
		requestLine = parse_line(client._header, "User-Agent: ", "\r\n");
		std::istringstream iss0(requestLine);
		getline(iss0, tmp, ' ');
		getline(iss0, client._user_agent, ' ');
	}

	/*ACCEPT*/
	pos = client._header.find("Accept: ");
	if (pos != client._header.npos)
	{
		requestLine = parse_line(client._header, "Accept: ", "\r\n");
		std::istringstream iss0(requestLine);
		getline(iss0, tmp, ' ');
		getline(iss0, client._http_accept, ' ');
	}

	/*ACCEPT_LANGUAGE*/
	pos = client._header.find("Accept-Language: ");
	if (pos != client._header.npos)
	{
		requestLine = parse_line(client._header, "Accept-Language: ", "\r\n");
		std::istringstream iss0(requestLine);
		getline(iss0, tmp, ' ');
		getline(iss0, client._http_accept_language, ' ');
	}

	/*ACCEPT_ENCODING*/
	pos = client._header.find("Accept-Encoding: ");
	if (pos != client._header.npos)
	{
		requestLine = parse_line(client._header, "Accept-Encoding: ", "\r\n");
		std::istringstream iss0(requestLine);
		getline(iss0, tmp, ' ');
		getline(iss0, client._http_accept_encoding, ' ');
	}

	/*REMOTE_HOST*/
	pos = client._header.find("Origin: ");
	if (pos != client._header.npos)
	{
		requestLine = parse_line(client._header, "Origin: ", "\r\n");
		std::istringstream iss0(requestLine);
		getline(iss0, tmp, ' ');
		getline(iss0, client._remote_host, ' ');
	}

	/*CONNECTION KEEP_ALIVE*/
	pos = client._header.find("Connection: ");
	if (pos != client._header.npos)
	{
		requestLine = parse_line(client._header, "Connection: ", "\r\n");
		std::istringstream iss0(requestLine);
		getline(iss0, tmp, ' ');
		getline(iss0, tmp, ' ');
		if (tmp.compare("keep_alive") == 0)
			client._keep_alive = true;
	}

}

std::string	Monitor::parse_line(std::string &request, std::string start, std::string end)
{
	size_t		requestLineEnd;
	size_t		requestLineStart;
	std::string requestLine;
	
	// Find the start and end of the request line
	requestLineStart = request.find(start);
	if (requestLineStart != request.npos)
	{
		requestLineEnd = request.find(end, requestLineStart);
		requestLine = request.substr(requestLineStart,(requestLineEnd - requestLineStart));
	}
	else
		requestLine = "";

	return (requestLine);
}

void	Monitor::looking_for_path(t_client &client)
{
	/*IF REQUEST IS A LOCATION, APPEND INDEX.HTML FILES DEFINED INTO CONFIGURATION FILE*/
	if (client._url.at(client._url.length() - 1) == '/')
	{
		std::string	url = "";
		std::string url_server = "";

		if (client._url.compare("/") != 0)
		{
			url = client._url;
			url.erase(url.end() -1);
		}
		else
			url = client._url;
		

		if(client._location_.find(client._url) != client._location_.end())
		{
			chk_indexies(client, client._server_path, client._url);
			if (client._location_.at(client._url)._path_ok == true)

				return ;
		}

		if(client._location_.find(url) != client._location_.end())
		{
			client._url.erase(client._url.end() - 1);
			client._url_location.erase(client._url_location.end() - 1);
			chk_indexies(client, client._server_path, url);
			if (client._location_.at(url)._path_ok == true)
				return ;
		}

		if (client._status_code.compare("200") == 0)
		{
			client._status_code = "404";
			client._status_msg = "Not Found";
		}
	}
	else
	{
		/*CHECK IF THE FILE IS ACTUALY A DIRETORY*/
		if(client._location_.find(client._url) != client._location_.end())
		{
			client._location_.at(client._url_location)._redirect_url = client._url + "/";
			client._status_code = "301";
			client._status_msg = "Move Permanently";
			return;
		}
		/*CHECK FOR FILE IN THE END OF THE PATH REQUEST*/
		if (client._url_file == "")
		{
			if (client._location_.find(client._url) != client._location_.end())
			{
				client._server_path = client._location_.at(client._url)._server_path;
				if (client._status_code.compare("200") == 0)
				{
					client._status_code = "404";
					client._status_msg = "Not Found";
				}
				client._location_.at(client._url)._path_ok = false;
				return ;
			}
		}

		/**/
		if(client._location_.find(client._url_location) != client._location_.end())
		{
			client._server_path = client._location_.at(client._url_location)._server_path + client._url_file;
			client._url = client._url_location;
			if (access(client._server_path.c_str(), F_OK) == 0)
				client._location_.at(client._url_location)._path_ok = true;
			else
			{
				if (client._status_code.compare("200") == 0)
				{
					client._status_code = "404";
					client._status_msg = "Not Found";
				}
				client._location_.at(client._url_location)._path_ok = false;
			}
		}
		else
		{
			if (client._status_code.compare("200") == 0)
			{
				client._status_code = "404";
				client._status_msg = "Not Found";
			}
			// client._location_.at(client._url_location)._path_ok = false;
		}
	}
	/*REWRITE THE PATH CORRECTILY IF '//' IS FIND*/
		size_t pos = client._server_path.find("//");
		if (pos != client._server_path.npos)
			client._server_path.replace(pos, 1, "");
}

void	Monitor::chk_indexies(t_client &client, std::string &html, std::string url)
{
	bool	flag = false;
	size_t	i = 0;
		
	while (flag == false && i < client._location_.at(url)._index_block.size())
	{
		/*IF CGI_PASS, CHECK FILE EXTENSION = TO CGI_PASS*/
		if(client._location_.at(url)._cgi_pass.compare("") != 0)
		{
			if (client._location_.at(url)._index_block.at(i).find(client._location_.at(url)._cgi_pass, 0) == client._location_.at(url)._index_block.at(i).npos)
			{
				i++;
				continue;
			}
		}

		html = client._location_.at(url)._server_path + "/" + client._location_.at(url)._index_block.at(i);
		if(access(html.c_str(), F_OK) == 0)
		{
			flag = true;
			client._location_.at(url)._path_ok = true;
		}
		i++;
	}
	i = 0;
	if (flag == false)
	{
		while (flag == false && i < client._index_.size())
		{
			/*IF CGI_PASS, CHECK FILE EXTENSION = TO CGI_PASS*/
			if(client._location_.at(url)._cgi_pass.compare("") != 0)
			{
				if (client._index_.at(i).find(client._location_.at(url)._cgi_pass, 0) == client._index_.at(i).npos)
				{
					i++;
					continue;
				}
			}

			html = client._location_.at(url)._server_path + "/" + client._index_.at(i);
			if(access(html.c_str(), F_OK) == 0)
			{
				flag = true;
				client._location_.at(url)._path_ok = true;
			}
			i++;
		}
	}
	if (flag == false)
	{
		/*NGINX HTML PAGE WHEN ROOT SIMPLE DIRECTIVE NOT DEFINED*/
		if (url.compare("/") == 0 && client._location_.at("/")._server_path.compare("./server_root/webserver.html") == 0)
		{
			client._location_.at(url)._path_ok = true;
			html = client._location_.at(url)._server_path;
			return;
		}

		/*IF CGI_PASS, CHECK FILE EXTENSION = TO CGI_PASS*/
		if(client._location_.at(url)._cgi_pass.compare("") != 0)
		{
			std::string	cgi_file = "index" + client._location_.at(url)._cgi_pass;
			html = client._location_.at(url)._server_path + cgi_file;
			if (access(html.c_str(), F_OK) == 0)
			{
				client._location_.at(url)._path_ok = true;
				return;
			}
		}
		else
		{
			/*ADD INDEX FILE TO PATH TO SEE IF DEFINED*/
			html = client._location_.at(url)._server_path + "/index.html";
			if (access(html.c_str(), F_OK) == 0)
			{
				client._location_.at(url)._path_ok = true;
				return;
			}
		}

		/*CHECK ERROR. IF 301 ELSE 403*/
		/*IF PATH NOT FOUND RETURN ERROR*/
		if (client._location_.at(url)._autoindex == false)
		{
			if (client._status_code.compare("200") == 0)
			{
				client._status_code = "403";
				client._status_msg = "Forbidden";
				client._location_.at(url)._path_ok = false;
			}
			html = client._location_.at(url)._server_path;
		}
		else
		{
			if (client._status_code.compare("200") == 0)
			{
				client._status_code = "200";
				client._status_msg = "OK";
				client._location_.at(url)._path_ok = false;
			}
			html = client._location_.at(url)._server_path;
		}
	}
}

void	Monitor::exec_cgi(std::string &html, t_client &client)
{
	char			*arg2[3];

	arg2[0] = (char *)"/usr/bin/php-cgi7.4";
	arg2[1] = (char *)html.c_str();
	arg2[2] = NULL;


	if (pipe(client.pipe0) == -1)
		exit(write(1, "pipe error\n", 11));
	if (pipe(client.pipe1) == -1)
		exit(write(1, "pipe error\n", 11));
	
	/*WHEN INFINITY LOOP WITH NO WRITING. PIPE DO NOT BLOCK WHEN READ IN WRITE_RESPONSE_TO_STRING*/
	int fd_flag = fcntl(*client.pipe1, F_GETFL, 0);
	fcntl(*client.pipe1, F_SETFL, fd_flag | O_NONBLOCK);

	client._pid = fork();
	if (client._pid < 0)
		exit(write(1, "fork error\n", 11));
	
	/*CHILD PROCESS*/
	if (client._pid == 0)
	{

		cgi_envs_parser(client, html);

		/*CREATE ENVP_CGI ARRAY TO EXECVE*/
		char *envp_cgi[client._cgi_envs.size() + 1];
		size_t i = 0;
		while (i < client._cgi_envs.size()) {
			envp_cgi[i] = (char *)client._cgi_envs.at(i).c_str();
			i++;
		}
		envp_cgi[i] = NULL;


		if (client._method == "POST")
			dup2(client.pipe0[0], STDIN_FILENO);

		dup2(client.pipe1[1], STDOUT_FILENO);
		close(client.pipe0[0]);
		close(client.pipe0[1]);
		close(client.pipe1[0]);
		close(client.pipe1[1]);

		if (execve(arg2[0], arg2, envp_cgi) == -1)
		{
			write(2, "ERROR: execve", strlen("ERROR: execve"));
			exit(1);
		}
	}
	int wstatus;
	waitpid(client._pid, &wstatus, WNOHANG);
}

void	Monitor::cgi_envs_parser(t_client &client, std::string html)
{
	std::string		env;
	
	/*SET REDIRECT_STATUS ENV*/
	client._cgi_envs.push_back("REDIRECT_STATUS=200");

	/* SSL HAS TO BE CHECKED INTO THE FILE PARSER*/
	client._cgi_envs.push_back("AUTY_TYPE=NULL");
	
	/*CONTENT_LENGTH*/
	env = "CONTENT_LENGTH=" + client._content_length;
	client._cgi_envs.push_back(env);
	
	/*MAX_BODY_LENGTH*/
	env = "MAX_BODY_LENGTH=" + client._max_body_length;
	client._cgi_envs.push_back(env);

	/*CONTENT_TYPE*/
	env = "CONTENT_TYPE=" + client._content_type;
	client._cgi_envs.push_back(env);

	/*GATWAY_INTERFACE*/
	client._cgi_envs.push_back("GATEWAY_INTERFACE=CGI/7.4");

	/*HTTP_ACCEPT*/
	env = "HTTP_ACCEPT=" + client._http_accept;
	client._cgi_envs.push_back(env);

	/*HTTP_ACCEPT_ENCODING*/
	env = "HTTP_ACCEPT_ENCODING=" + client._http_accept_encoding;
	client._cgi_envs.push_back(env);

	/*HTTP_ACCEPT_LANGUAGE*/
	env = "HTTP_ACCEPT_LANGUAGE=" + client._http_accept_language;
	client._cgi_envs.push_back(env);

	/*HTTP_HOST*/
	env = "HTTP_HOST=" + client._http_host;
	client._cgi_envs.push_back(env);

	/*SERVER_NAME*/
	env = "SERVER_NAME=" + client._server_name;
	client._cgi_envs.push_back(env);

	/*SERVER_PORT*/
	env = "SERVER_PORT=" + client._server_port;
	client._cgi_envs.push_back(env);

	/*HTTP_USER_AGENT*/
	env = "HTTP_USER_AGENT=" + client._user_agent;
	client._cgi_envs.push_back(env);

	/*QUERY_STRING*/
	env = "QUERY_STRING=" + client._query_string;
	client._cgi_envs.push_back(env);

	/*PATH_INFO*/
	client._cgi_envs.push_back(env);

	/*REQUEST_URI*/
	env = "REQUEST_URI=" + client._request_uri;
	client._cgi_envs.push_back(env);

	/*REQUEST_METHOD*/
	env = "REQUEST_METHOD=" + client._method;
	client._cgi_envs.push_back(env);

	/*SERVER_PROTOCOL*/
	env = "SERVER_PROTOCOL=" + client._protocol;
	client._cgi_envs.push_back(env);

	/*REMOTE_HOST*/
	env = "REMOTE_HOST=" + client._remote_host;
	client._cgi_envs.push_back(env);

	/*SCRIPT_NAME*/
	env = "SCRIPT_NAME=/usr/bin/php-cgi";
	client._cgi_envs.push_back(env);

	/*SCRIPT_FILENAME*/
	env = "SCRIPT_FILENAME=" + html;
	client._cgi_envs.push_back(env);

	/*DOCUMENT ROOT*/
	env = "DOCUMENT_ROOT=/home/fausto/42SP/webserv_git";
	client._cgi_envs.push_back(env);

	/*REDIRECT STATUS*/
	client._cgi_envs.push_back("REDIRECT_STATUS=true");

	/*BUFFER SIZE*/
	std::ostringstream oss;
    oss << client._upload_buff_size;
	env = "BUFFER_SIZE=" + oss.str();
	client._cgi_envs.push_back(env);

	/*ADD ENVP TO CGI_ENVS */
	extern char** environ; 
	for (int i = 0; environ[i] != NULL; i++)
		client._cgi_envs.push_back(environ[i]);
}

void	Monitor::send_response(int i)
{
	
	std::map<int, t_client>::iterator	it;
	it = _map_connect.find(_m_ep_event[i].data.fd);

	if((*it).second._response_step_flag == 4)
	{
		write_response_to_string((*it).second);
	}
	if((*it).second._response_step_flag == 5)
	{
		response_parser(it->second);
		(*it).second._response_step_flag = 6;
	}

	if((*it).second._response_step_flag == 6)
	{
		if (send(_m_ep_event[i].data.fd, (*it).second._response.c_str(), (*it).second._response.size(), 0) == -1)
			close_fd(i);
		else
		{
			if (it->second._keep_alive == true)
			{
				_m_ev.events = EPOLLIN;
				_m_ev.data.fd = _m_ep_event[i].data.fd;
				epoll_ctl(_m_efd, EPOLL_CTL_MOD, _m_ep_event[i].data.fd, &_m_ev);
			}
			else
				close_fd(i);
		}
	}
}

void	Monitor::write_response_to_string(t_client &client)
{
	close(client.pipe1[1]);
	std::stringstream phpOutput;
	ssize_t bytesRead;
	char	buff[client._upload_buff_size];
	memset (&buff, '\0', sizeof (buff));
	double	timeout = difftime(time(NULL), client.connection_time);
	if ((bytesRead = read(client.pipe1[0], buff, sizeof(buff))) != 0)
	{
		phpOutput.write(buff, bytesRead);
		client._response += phpOutput.str();
		if ((int)timeout >= CONNECTION_MAX_TIME)
		{
			kill(client._pid, SIGTERM);
			client._response_step_flag = 5;
			client._status_code = "500";
			client._status_msg = "Internal Server Error";
			close(client.pipe1[0]);
			return;
		}
	}
	if (bytesRead == 0)
	{
		client._response += phpOutput.str();
		std::string tmp = client._response;
		client._response = "HTTP/1.1 200 OK\r\n";
		client._response += tmp;
		close(client.pipe1[0]);
		client._response_step_flag = 6;
		if (tmp.find("File fail to delete.") != tmp.npos)
		{
			client._response_step_flag = 5;
			client._status_code = "404";
			client._status_msg = "Not Found";
			close(client.pipe1[0]);
			return;
		}
	}
	if (bytesRead == -1)
	{
		if ((int)timeout >= CONNECTION_MAX_TIME)
		{
			kill(client._pid, SIGTERM);
			client._response_step_flag = 5;
			client._status_code = "500";
			client._status_msg = "Internal Server Error";
			close(client.pipe1[0]);
			return;
		}	
	}
	
}

void	Monitor::response_parser(t_client &client)
{
	std::fstream			conf_file;
	std::stringstream		buff;

	/*CHECK IF SERVER_PATH PARSED FROM REQUEST_PATH EXIST*/
	int	flag = 0;
	if(access(client._server_path.c_str(), F_OK) == 0)
		flag = 1;

	/*RESPONSE STRING PARSE*/


	if (client._status_code.compare("200") == 0 || (flag == 1 && client._location_.find(client._url_location) != client._location_.end() && client._location_.at(client._url_location)._autoindex == true && client._status_code == "404"))
	{
		if (client._method.compare("GET") == 0)
		{
			if (client._location_.at(client._url)._autoindex == true)
			{
				if (client._location_.at(client._url)._path_ok == false)
					diretory_list(buff, client._url, client._server_path);
				else
					buff_file(conf_file, buff, client._server_path);
			}
			else
			{
				buff_file(conf_file, buff, client._server_path);
			}
			http_response_syntax(client, buff);
			conf_file.close();
		}
		else if (client._method.compare("POST") == 0)
		{
			if (client._location_.at(client._url)._autoindex == true)
			{
				if (client._location_.at(client._url)._path_ok == false)
					diretory_list(buff, client._url, client._server_path);
				else
					buff_file(conf_file, buff, client._server_path);
			}
			else
			{
				buff_file(conf_file, buff, client._server_path);
			}
			http_response_syntax(client, buff);
			conf_file.close();
		}
		else if (client._method.compare("DELETE") == 0)
		{
			if (client._location_.at(client._url)._autoindex == true)
			{
				if (client._location_.at(client._url)._path_ok == false)
					diretory_list(buff, client._url, client._server_path);
				else
					buff_file(conf_file, buff, client._server_path);
			}
			else
			{
				buff_file(conf_file, buff, client._server_path);
			}
			http_response_syntax(client, buff);
			conf_file.close();
		}
	}
	else if (client._status_code.find("30") != client._status_code.npos)
		http_response_redirection (client);
	else
		http_response_error (client);
}

void	Monitor::diretory_list(std::stringstream &buff, std::string path, std::string html)
{
	buff << "<html>\n";
	buff << "<body>\n";
	buff << "<h1>Directory Listing</h1>\n";
	buff << "<ul>\n";
	DIR* dir;
	struct dirent* entry;
	dir = opendir(html.c_str());
	if (dir != NULL)
	{
    	while ((entry = readdir(dir)) != NULL)
		{
			std::string link;
    		std::string filename = entry->d_name;
    		if (path.compare("/") != 0) 
				link = path + "/" + filename;
			else
				link = "/" + filename;
    		buff << "<li><a href=\"" << link << "\">" << filename << "</a></li>\n";
    	}
    	closedir(dir);
	}
}

void	Monitor::buff_file(std::fstream &conf_file, std::stringstream &buff, std::string html)
{
	conf_file.open(html.c_str() , std::fstream::in);
	if (conf_file.fail())
	{
		conf_file.open("./server_root/test/error.html",  std::fstream::in);
		if (conf_file.fail())
			std::cout << "Configuration file fail to read" << std::endl;
		buff << conf_file.rdbuf();
	}
	else
		buff << conf_file.rdbuf();
}

void	Monitor::http_response_syntax(t_client &client, std::stringstream &buff)
{
	std::string status = client._protocol + " " + client._status_code + " " + client._status_msg + "\r\n";
	client._response = status;
	client._response += client._content_type;
	client._response += "Connection: close";
	client._response += "\r\n\r\n";
	client._response += buff.str();
	client._response += "\r\n";
}

void		Monitor::http_response_redirection (t_client &client)
{
	std::string status = client._protocol + " " + client._status_code + " " + client._status_msg + "\r\n";
	client._response = status;
	client._response += client._content_type;
	client._response += "Location: " + client._location_.at(client._url_location)._redirect_url + "\r\n";
	client._response += "Connection: close";
	client._response += "\r\n\r\n";
}

void	Monitor::http_response_error(t_client &client)
{
	std::fstream			error_file;
	std::stringstream		buff;

	std::string status = client._protocol + " " + client._status_code  + " " + client._status_msg + "\r\n";
	client._response = status;
	client._response += client._content_type;
	client._response += "Connection: close";
	client._response += "\r\n\r\n";

	if (client._error_page_map.find(client._status_code) != client._error_page_map.end())
	{
		buff_file(error_file, buff, client._error_page_map.at(client._status_code));
		client._response += buff.str();
	}
	else
	{
		client._response += "<!DOCTYPE html>\n \
		<html lang='en'>\n \
			<head>\n \
				<meta charset='UTF-8'> \n \
				<meta http-equiv='X-UA-Compatible' content='IE=edge'> \n \
				<meta name='viewport' content='width=device-width, initial-scale=1.0'> \n \
				<title>Error</title> \n \
				<link rel='icon' href='favicon.ico' type='image/x-icon'> \n \
			<style> \n \
				body { \n \
				font-family: Arial, sans-serif; \n \
				margin: 0; \n \
				padding: 0; \n \
				background-color: #f4f4f4; \n \
			} \n \
			\n \
			.container { \n \
				display: flex; \n \
				align-items: center; \n \
				justify-content: center; \n \
				height: 100vh; \n \
				text-align: center; \n \
			} \n \
			\n \
			.error-code { \n \
				font-size: 100px; \n \
				font-weight: bold; \n \
				margin-bottom: 20px; \n \
			} \n \
			\n \
			.error-message { \n \
				font-size: 30px; \n \
				color: #888; \n \
			} \n \
			</style> \n \
		</head> \n \
		<body> \n \
			<div class='container'> \n \
				<div> \n \
				<h1 class='error-code'>Error " + client._status_code + "</h1> \n \
				<p class='error-message'>" + client._status_msg + "</p> \n \
				</div> \n \
			</div> \n \
		</body> \n \
		</html> \n";
	}
}

void	Monitor::http_response_save_file(t_client &client)
{
	std::string status = client._protocol + " " + client._status_code  + " " + client._status_msg + "\r\n";
	client._response = status;
	client._response += client._content_type;
	client._response += "Connection: close";
	client._response += "\r\n\r\n";

	client._response += "<!DOCTYPE html>\n \
		<html lang='en'>\n \
			<head>\n \
				<meta charset='UTF-8'> \n \
				<meta http-equiv='X-UA-Compatible' content='IE=edge'> \n \
				<meta name='viewport' content='width=device-width, initial-scale=1.0'> \n \
				<title>Error</title> \n \
				<link rel='icon' href='favicon.ico' type='image/x-icon'> \n \
			<style> \n \
				body { \n \
				font-family: Arial, sans-serif; \n \
				margin: 0; \n \
				padding: 0; \n \
				background-color: #f4f4f4; \n \
			} \n \
			\n \
			.container { \n \
				display: flex; \n \
				align-items: center; \n \
				justify-content: center; \n \
				height: 100vh; \n \
				text-align: center; \n \
			} \n \
			\n \
			.error-code { \n \
				font-size: 100px; \n \
				font-weight: bold; \n \
				margin-bottom: 20px; \n \
			} \n \
			\n \
			.error-message { \n \
				font-size: 30px; \n \
				color: #888; \n \
			} \n \
			</style> \n \
		</head> \n \
		<body> \n \
			<div class='container'> \n \
				<div> \n \
				<h1 class='error-code'> FILE UPLOADED </h1> \n \
				</div> \n \
			</div> \n \
		</body> \n \
		</html> \n";

}

void	Monitor::http_response_delete_file(t_client &client)
{
	std::string status = client._protocol + " " + client._status_code  + " " + client._status_msg + "\r\n";
	client._response = status;
	client._response += client._content_type;
	client._response += "Connection: close";
	client._response += "\r\n\r\n";


	client._response += "<!DOCTYPE html>\n \
		<html lang='en'>\n \
			<head>\n \
				<meta charset='UTF-8'> \n \
				<meta http-equiv='X-UA-Compatible' content='IE=edge'> \n \
				<meta name='viewport' content='width=device-width, initial-scale=1.0'> \n \
				<title>Error</title> \n \
				<link rel='icon' href='favicon.ico' type='image/x-icon'> \n \
			<style> \n \
				body { \n \
				font-family: Arial, sans-serif; \n \
				margin: 0; \n \
				padding: 0; \n \
				background-color: #f4f4f4; \n \
			} \n \
			\n \
			.container { \n \
				display: flex; \n \
				align-items: center; \n \
				justify-content: center; \n \
				height: 100vh; \n \
				text-align: center; \n \
			} \n \
			\n \
			.error-code { \n \
				font-size: 100px; \n \
				font-weight: bold; \n \
				margin-bottom: 20px; \n \
			} \n \
			\n \
			.error-message { \n \
				font-size: 30px; \n \
				color: #888; \n \
			} \n \
			</style> \n \
		</head> \n \
		<body> \n \
			<div class='container'> \n \
				<div> \n \
				<h1 class='error-code'> FILE DELETED </h1> \n \
				</div> \n \
			</div> \n \
		</body> \n \
		</html> \n";

}
