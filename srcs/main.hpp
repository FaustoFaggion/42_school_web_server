#ifndef MAIN_HPP
#define MAIN_HPP

#include <stdio.h>
#include <iostream>		// For cout
#include <fstream>
#include <sstream>
#include <string.h>		// For memset
#include <unistd.h>		// For read
#include <cstdlib>		// For exit() and EXIT_FAILURE
#include <errno.h>
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

#define MAX_CONNECTIONS		10

struct directive {
	bool						_autoindex;
	std::vector<std::string>	_index_block;
	std::string					_server_path;
	bool						_path_ok;
};

typedef struct client {
	int				fd;
	time_t			start_connection;
	
	std::string		_request;
	std::string		_method;
	std::string		_url;
	std::string		_protocol;
	std::string		_content_type;
	std::string		_content_length;
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
	std::string		_content;
	std::string		_url_file; /*FILE IN THE END OF URL REQUEST*/
	std::string		_url_file_extension; /*IF .PHP RUN CGI*/
	std::string		_url_location;
	std::string		_server_path;

	std::string		_response;
} t_client;

#endif