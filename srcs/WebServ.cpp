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
			if (timeout > 2.0)
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
	_ev.events = EPOLLIN | EPOLLHUP; // | EPOLLONESHOT;
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
	c._server_path = "";
	c._response = "";
	c._upload_content_size = 0;
	c._upload_buff_size = _buffer_size;
	c.pipe0[0] = 0;
	c.pipe0[1] = 0;
	c.pipe1[0] = 0;
	c.pipe1[1] = 0;

}

void	WebServ::receive_data(int i)
{
	std::map<int, t_client>::iterator	it;
	it = map_connections.find(_ep_event[i].data.fd);
	t_client client = (*it).second;
	
	char	buff[client._upload_buff_size];
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
		std::string	tmp(buff);
		/*CHECK IF REQUEST DATA FINISHED*/
		if (tmp.find("\r\n\r\n") == std::string::npos)
			map_connections[_ep_event[i].data.fd]._request += buff;
		else
		{
			size_t	pos;
			std::string	str;

			pos = tmp.find("\r\n\r\n");
			pos += 4;
			str = tmp.substr(0, pos);
			
			map_connections[_ep_event[i].data.fd]._request += str;
			request_parser((*it).second);
			
			(*it).second._content = tmp.substr(pos, atoi((*it).second._content_length.c_str()));

			std::cout << "tmp:\n" << tmp << "\n";
			std::cout << "str:\n" << str << "\n";
			std::cout << "_content:\n" << (*it).second._content << "\n";

			(*it).second._server_path = looking_for_path((*it).second, _locations, _index);
			
			if((*it).second._url_file_extension == ".php")
			{
				exec_cgi((*it).second._server_path, (*it).second, i);
			}
			else
				response_parser((*it).second, _locations);

			_ev.events = EPOLLOUT | EPOLLONESHOT;
			epoll_ctl(_efd, EPOLL_CTL_MOD, _ep_event[i].data.fd, &_ev);
		}
	}
}


void	WebServ::exec_cgi(std::string &html, t_client &client, int i)
{
	std::cout << "\nEXEC_CGI FUNCTION" << html << "\n\n";

	int				pid;
	char			*arg2[3];
	
	arg2[0] = (char *)"/usr/bin/php-cgi7.4";
	arg2[1] = (char *)html.c_str();
	arg2[2] = NULL;

	if (pipe(client.pipe0) == -1)
		exit(write(1, "pipe error\n", 11));
	if (pipe(client.pipe1) == -1)
		exit(write(1, "pipe error\n", 11));
	
	pid = fork();
	if (pid < 0)
		exit(write(1, "fork error\n", 11));
	if (pid == 0)
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
		if (execve(arg2[0], arg2, envp_cgi) == -1)
		{
			write(2, strerror(errno), strlen(strerror(errno)));
			exit(1);
		}
	}

	if (client._method == "POST")
	{
		char	buff[client._upload_buff_size];

		std::cout << "CONTENT: " << client._content << "\n\n";
		std::cout << "CONTENT_SIZE: " << client._content.size() << "\n\n";
		std::cout << "UPLOAD_BUFF_SIZE: " << client._upload_buff_size << "\n\n";
		close(client.pipe0[0]);
		write(client.pipe0[1], client._content.c_str(), client._content.size());
		client._upload_content_size = client._content.size();
		std::cout << client._content.size() << " : " << client._upload_buff_size << "\n\n";
		while (client._upload_content_size < (size_t)atoi(client._content_length.c_str()))
		{
			memset (&buff, '\0', sizeof (buff));
			/*RECEIVING CLIENT DATA CHUNCKS REQUEST */
			ssize_t numbytes = recv (_ep_event[i].data.fd, &buff, sizeof(buff), 0);
			// std::cout << "BUFF: " << buff << "\n\n";
			write(client.pipe0[1], buff, numbytes);
			client._upload_content_size += numbytes;
			std::cout << client._upload_content_size << " : " << numbytes << " > ";
		}
		close(client.pipe0[1]);
	}
	waitpid(pid, NULL, 0);
	client._response = "HTTP/1.1 200 OK\r\n";
	std::cout << "enter while\n";
	close(client.pipe1[1]);
	std::stringstream phpOutput;
	char		buffer[client._upload_buff_size];
	ssize_t bytesRead;
	while ((bytesRead = read(client.pipe1[0], buffer, sizeof(buffer))) != 0)
	{
		phpOutput.write(buffer, bytesRead);
	}
	client._response += phpOutput.str();
	std::cout << "out while\n";
	// std::cout << "request: " << request << "\n";
	close(client.pipe1[0]);

}

void	WebServ::response(int i)
{
	std::map<int, t_client>::iterator	it;
	it = map_connections.find(_ep_event[i].data.fd);
	/*PROTECTION FROM CONNECTION HAND-SHAKE*/
	if (!(*it).second._request.empty())
	{
		// std::cout << "inside response fd: " << _ep_event[i].data.fd << "\n" << (*it).second.response.c_str() << "\n";
		send(_ep_event[i].data.fd, (*it).second._response.c_str(), (*it).second._response.size(), 0);
		
		
		/*If keep-alive SET FD SOCKET TO READ AGAIN*/
		_ev.events = EPOLLIN | EPOLLHUP | EPOLLONESHOT;
		epoll_ctl(_efd, EPOLL_CTL_MOD, _ep_event[i].data.fd, &_ev);
	}
}
