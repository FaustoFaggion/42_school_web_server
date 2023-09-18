#ifndef MAIN_HPP
#define MAIN_HPP

#include <stdio.h>
#include <iostream>		// For cout
#include <fstream>
#include <sstream>
#include <string>		//std::to_string
#include <string.h>		// For memset
#include <unistd.h>		// For read
#include <cstdlib>		// For exit() and EXIT_FAILURE
#include <cstdio>		// For stderr

#include <netdb.h>		// For struct addinfo
#include <arpa/inet.h>	// For inet_ntop function
#include <sys/socket.h>	// For socket functions
#include <netinet/in.h>	// For sockaddr_in

#include <sys/epoll.h>
#include <algorithm>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>

#include <vector>
#include <map>

# include <sys/wait.h>

#define CONNECTION_MAX_TIME		4

struct directive {
	bool						_autoindex;
	std::vector<std::string>	_index_block;
	std::string					_server_path;
	bool						_path_ok;
	std::vector<std::string>	_allowed_methods;
	std::string					_redirect_url;
	std::string					_redirect_code;
	std::string					_cgi_pass;
	size_t						_max_body_size;
};

typedef struct client {
	
	int				fd;
	
	/*TIMEOUT CONNECTION*/
	time_t			connection_time;
	
	/*PHP_CGI*/
	int				_pid;
	int				pipe0[2];
	int				pipe1[2];

	std::string		_response;
	
	/*REQUEST HEADER*/
	std::string		_header;
	std::string		_method;
	std::string		_url;
	std::string		_protocol;
	std::string		_content_type;
	std::string		_content_length;
	std::string		_max_body_length;
	std::string		_server_name;
	std::string		_server_port;
	std::string		_user_agent;
	std::string		_http_host;
	std::string		_http_accept;
	std::string		_http_accept_encoding;
	std::string		_http_accept_language;
	std::string		_query_string;
	std::string		_path_info;
	std::string		_request_uri;
	std::string		_remote_host;
	std::string		_boundary;
	std::string		_url_file; /*FILE IN THE END OF URL REQUEST*/
	std::string		_url_file_extension; /*IF .PHP RUN CGI*/
	std::string		_url_location;
	std::string		_server_path;
	bool			_keep_alive;
	
	/*REQUEST BODY*/
	std::string		_body;

	/*SERVER CONFIGURATION FILE*/
	size_t			_upload_content_size;
	size_t			_upload_buff_size;
	
	/*PROGRAM LOGIC*/
	std::string		_status_code;
	std::string		_status_msg;
	int				_response_step_flag;

	std::map<std::string, directive>	_location_;
	std::vector<std::string>			_index_;
	std::vector<std::string>			_cgi_envs;
	std::vector<std::string>			_allowed_methods;
	std::map<std::string, std::string>	_error_page_map;
} t_client;

#endif