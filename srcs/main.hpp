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

#define MAX_CONNECTIONS		10

struct directive {
	bool						_autoindex;
	std::vector<std::string>	_index_block;
	std::string					_server_path;
	bool						_path_ok;
};

#endif