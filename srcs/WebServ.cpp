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
	
	// std::cout << "_domain: " << get_domain() << "\n";
	// std::cout << "_port: " << get_port() << "\n";
	// std::cout << "_type: " << get_type() << "\n";
	// std::cout << "_flag: " << get_flag() << "\n";
	// std::cout << "_work_connections: " << get_worker_connections() << "\n";

	/*PARSE ROOT SIMPLE DIRECTIVE*/
	bool	simple_root_directive;

	simple_root_directive = parse_simple_root_directive();
	
	/*PARSE INDEX*/
	parse_index(_index, _server_conf_file);

	/*PARSE BUFFER_SIZE*/
	parse_buffer_size();
	// std::cout << _buffer_size << "\n";
	
	/*PARSE BODY_SIZE*/
	parse_body_size();

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

	/*SET FD SOCKET TO NONBLOCKING*/
	int fd_flag = fcntl(_fd_listener, F_GETFL, 0);
	fcntl(_fd_listener, F_SETFL, fd_flag | O_NONBLOCK);

	if (epoll_ctl (_efd, EPOLL_CTL_ADD, _fd_listener, &_ev) == -1)
		std::cout << "ERROR: epoll_ctl" << std::endl;
}

void	WebServ::run()
{
	_nfds = 0;
	while (1)
	{
		/*MONITOR FDS. STILL WAITING UNTIL A EVENT HEAPPENS IN A FD*/
		if ((_nfds = epoll_wait (_efd, _ep_event, get_worker_connections(), 0)) == -1) // '-1' to block indefinitely
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
					int fd1 = accept_new_connection();
					(void)fd1;
					// std::cout << "accept buff run func: " << map_connections.at(fd1)._upload_buff_size << "\n";
				}
				/*CLIENT SOCKET*/
				else 
				{
					// std::cout << "accept buff run func 2: " << map_connections.at(_ep_event[i].data.fd)._upload_buff_size << "fd: " << _ep_event[i].data.fd <<  "\n";
					std::cout << "\nRECEIVE_DATA FUNCTION fd: " << _ep_event[i].data.fd << "\n\n";
					receive_data(i);
				}
			}
			/*CHECK IF EVENT TO WRITE*/
			else if ((_ep_event[i].events & EPOLLOUT) == EPOLLOUT)
			{
				std::cout << "\nRESPONSE FUNCTION fd: " << _ep_event[i].data.fd << "\n\n";
				send_response(i);
			}
		}
	}
}

void	WebServ::delete_timeout_socket()
{

	// std::cout << "\nDELETE_TIMEOUT_SOCKET FUNCTION\n";

	for (std::map<int, t_client>::iterator it = map_connections.begin(); it != map_connections.end(); it++)
	{
		
		double	timeout = difftime(time(NULL), (*it).second.connection_time);

			if (timeout > 1.0)
			{
				std::cout << "map.size() A: " << map_connections.size() << " - fd: " << (*it).second.fd << "\n";
				int	fd = (*it).second.fd;

				/*DELETE FROM EPOLL AND CLOSE FD*/
				// epoll_ctl(_efd, EPOLL_CTL_DEL, (*it).second.fd, &_ev);
				if (epoll_ctl(_efd, EPOLL_CTL_DEL, (*it).second.fd, &_ev) == -1)
					std::cout << "EPOLL_CTL_DEL FAIL fd: " << (*it).second.fd << "\n";
				close(fd);
				map_connections.erase((*it).second.fd);
				break ;
			}

	}
};

int		WebServ::accept_new_connection()
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
	_ev.events = EPOLLIN; // | EPOLLONESHOT;
	_ev.data.fd = fd_new;
	
	std::cout << "new_fd: " << fd_new << "\n";

	if (epoll_ctl (_efd, EPOLL_CTL_ADD, fd_new, &_ev) == -1)
		std::cout << "ERROR: epoll_ctl" << std::endl;
	else if (_client_saddr.ss_family != AF_INET && _client_saddr.ss_family == AF_INET6)
	{
		std::cout << stderr << " Address family is neither AF_INET nor AF_INET6" << std::endl;
	}

	/*ADD fd_new TO MAP_CONNECTIONS AND SET TO EMPTY*/
	// map_connections.insert(std::make_pair(fd_new, t_client()));
	map_connections[fd_new];
	initialize_client_struct(map_connections, fd_new);

	std::cout << "accep buff: " << map_connections.at(fd_new)._upload_buff_size << "\n";
	return (fd_new);
}

void	WebServ::initialize_client_struct(std::map<int, t_client> &map, int fd_new)
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
    oss << _max_body_size;
	map.at(fd_new)._max_body_length = oss.str();
	map.at(fd_new)._header = "";
	map.at(fd_new)._method = "";
	map.at(fd_new)._url = "";
	map.at(fd_new)._protocol = "";
	map.at(fd_new)._content_type = "";
	map.at(fd_new)._content_length = "";
	map.at(fd_new)._server_name = "";
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
	map.at(fd_new)._server_path = "";
	map.at(fd_new)._keep_alive = false;
	
	/*REQUEST BODY*/
	map.at(fd_new)._body = "";
	
	/*SERVER CONFIGURATION FILE*/
	map.at(fd_new)._upload_content_size = 0;
	map.at(fd_new)._upload_buff_size = _buffer_size;

	/*PROGRAM LOGIC*/
	map.at(fd_new)._response_step_flag = 0;
	map.at(fd_new)._status_code = "200";
	map.at(fd_new)._status_msg = "OK";
}

void	WebServ::receive_data(int i)
{
	std::map<int, t_client>::iterator	it;
	it = map_connections.find(_ep_event[i].data.fd);
	// std::cout << "buff: " << (*it).second._upload_buff_size << " : " << _buffer_size;

	char	buff[(*it).second._upload_buff_size];
	memset (&buff, '\0', sizeof (buff));
	
	/*RECEIVING CLIENT DATA CHUNCKS REQUEST */
	ssize_t numbytes = recv (_ep_event[i].data.fd, &buff, sizeof(buff), 0);
	if (numbytes == -1)
	{
		std::cout << "ERROR: recv" << std::endl;
		map_connections.erase(_ep_event[i].data.fd);
	}
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
		std::string	tmp(buff);
		/*CHECK IF REQUEST DATA FINISHED*/
		if ((*it).second._response_step_flag == 0 && tmp.find("\r\n\r\n") == std::string::npos)
			map_connections.at(_ep_event[i].data.fd)._header += buff;
		else
		{
			if ((*it).second._response_step_flag == 0)
				verify_received_data((*it).second, _locations, _index, tmp);

			if ((*it).second._response_step_flag == 1)
				decide_how_to_respond((*it).second, _locations, i);
			
			if((*it).second._response_step_flag > 1)
			{
				
				if ((*it).second._response_step_flag == 3)
					write_remaining_body_chunks_to_cgi((*it).second, buff, numbytes);

				if ((*it).second._response_step_flag == 2)
					write_first_body_chunk_to_cgi((*it).second);
					
				if((*it).second._response_step_flag == 4)
				{
					if (it->second._status_code.compare("200") == 0)
						write_response_to_string((*it).second);
					else
					{
						response_parser(it->second, _locations);
						_ev.events = EPOLLOUT | EPOLLONESHOT;
						epoll_ctl(_efd, EPOLL_CTL_MOD, _ep_event[i].data.fd, &_ev);
					}
				}

			}
		}
	}
	(*it).second.connection_time = time(NULL);
}

void	WebServ::verify_received_data(t_client &client, std::map<std::string, directive> &locations, std::vector<std::string> indexes, std::string buff)
{
	split_header_and_body(client, buff);
	request_parser(client);
	looking_for_path(client, locations, indexes);
	std::cout << "_status_code: " << client._status_code << "\n";
	if (client._status_code.compare("200") == 0)
		client._response_step_flag = 1;
	else
		client._response_step_flag = 4;
}

void	WebServ::decide_how_to_respond(t_client &client, std::map<std::string, directive> &locations, int i)
{
	
	if(client._url_file_extension == ".php")
	{

		exec_cgi(client._server_path, client);
		client._response_step_flag = 2;
	}
	else
	{
		response_parser(client, locations);
		_ev.events = EPOLLOUT | EPOLLONESHOT;
		epoll_ctl(_efd, EPOLL_CTL_MOD, _ep_event[i].data.fd, &_ev);
	}
}
void	WebServ::write_first_body_chunk_to_cgi(t_client &client)
{
	if (client._method == "POST")
	{
			std::cout << "Flag 2 CONTENT_SIZE: " << client._body.size() << "\n\n";
			std::cout << "UPLOAD_CONTENT_SIZE: " << client._upload_content_size << "\n\n";
			close(client.pipe0[0]);
			write(client.pipe0[1], client._body.c_str(), client._body.size());
			client._upload_content_size = client._body.size();
			std::cout << client._body.size() << " : " << client._upload_content_size << "\n\n";

			if (client._upload_content_size >= (size_t)atoi(client._content_length.c_str()))
			{
				close(client.pipe0[1]);
				client._response_step_flag = 4;
			}
			else
				client._response_step_flag = 3;
	}
	else if (client._method == "GET")
		client._response_step_flag = 4;
	else if (client._method == "DELETE")
		client._response_step_flag = 4;

}

void	WebServ::write_remaining_body_chunks_to_cgi(t_client &client, char *buff, size_t numbytes)
{
	if (client._method == "POST")
	{
		
		std::cout << "Flag 3 CONTENT_SIZE: " << client._body.size() << "  ";
		std::cout << "UPLOAD_CONTENT_SIZE: " << client._upload_content_size << "\n\n";
			/*RECEIVING CLIENT DATA CHUNCKS REQUEST */
			std::cout << "BUFF: " << buff[0] << " : " << buff[numbytes - 1] << " <>\n\n";
			write(client.pipe0[1], buff, numbytes);
			client._upload_content_size += numbytes;
			std::cout << client._upload_content_size << " : " << numbytes << " > ";
		if (client._upload_content_size >= (size_t)atoi(client._content_length.c_str()))
		{
			close(client.pipe0[1]);
			client._response_step_flag = 4;
		}
	}
}

void	WebServ::write_response_to_string(t_client &client)
{
	client._response = "HTTP/1.1 200 OK\r\n";
	std::cout << "\n\nSTART READ CGI OUTPUT FROM PIPE\n\n";
	close(client.pipe1[1]);
	std::stringstream phpOutput;
	ssize_t bytesRead;

	char	buff[_buffer_size];
	memset (&buff, '\0', sizeof (buff));
	while ((bytesRead = read(client.pipe1[0], buff, sizeof(buff))) != 0)
	{
		phpOutput.write(buff, bytesRead);
	}
	client._response += phpOutput.str();
	std::cout << "\n\nFINISH READ CGI OUTPUT FROM PIPE\n";
	// std::cout << "request: " << request << "\n";
	close(client.pipe1[0]);
	_ev.events = EPOLLOUT | EPOLLONESHOT;
	epoll_ctl(_efd, EPOLL_CTL_MOD, client.fd, &_ev);
}

void	WebServ::exec_cgi(std::string &html, t_client &client)
{
	std::cout << "\nEXEC_CGI FUNCTION" << "\n\n";

	char			*arg2[3];

	arg2[0] = (char *)"/usr/bin/php-cgi7.4";
	arg2[1] = (char *)html.c_str();
	arg2[2] = NULL;

	if (pipe(client.pipe0) == -1)
		exit(write(1, "pipe error\n", 11));
	if (pipe(client.pipe1) == -1)
		exit(write(1, "pipe error\n", 11));
	
	client._pid = fork();
	if (client._pid < 0)
		exit(write(1, "fork error\n", 11));
	if (client._pid == 0)
	{

		cgi_envs_parser(client, html);

		/*CREATE ENVP_CGI ARRAY TO EXECVE*/
		char *envp_cgi[_cgi_envs.size() + 1];
		size_t i = 0;
		while (i < _cgi_envs.size()) {
			envp_cgi[i] = (char *)_cgi_envs.at(i).c_str();
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
		close(_fd_listener);
		close(client.fd);

		if (execve(arg2[0], arg2, envp_cgi) == -1)
		{
			write(2, strerror(errno), strlen(strerror(errno)));
			exit(1);
		}
	}
	int wstatus;
	waitpid(client._pid, &wstatus, WNOHANG);
	std::cout << "wstatus: " << wstatus << "\n";
}

void	WebServ::send_response(int i)
{
	std::map<int, t_client>::iterator	it;
	it = map_connections.find(_ep_event[i].data.fd);
	/*PROTECTION FROM CONNECTION HAND-SHAKE*/
	// if (!(*it).second._header.empty())
	// {
		// std::cout << "inside response fd: " << _ep_event[i].data.fd << "\n" << (*it).second.response.c_str() << "\n";
		send(_ep_event[i].data.fd, (*it).second._response.c_str(), (*it).second._response.size(), 0);
		
		
	if (it->second._keep_alive == true)
	{
		_ev.events = EPOLLIN | EPOLLHUP | EPOLLONESHOT;
		epoll_ctl(_efd, EPOLL_CTL_MOD, _ep_event[i].data.fd, &_ev);
	}
	else
	{
		int	fd = (*it).second.fd;
		if (epoll_ctl(_efd, EPOLL_CTL_DEL, (*it).second.fd, &_ev) == -1)
			std::cout << "EPOLL_CTL_DEL FAIL fd: " << (*it).second.fd << "\n";
		close(fd);
		map_connections.erase((*it).second.fd);
	}
	
	
	
	// }
}


