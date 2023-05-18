#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iostream>		// For cout
#include <string.h>		// For memset
#include <unistd.h>		// For read
#include <cstdlib>		// For exit() and EXIT_FAILURE
#include <errno.h>
#include <cstdio>		// For stderr

#include "ListenerSocket.hpp"


#define MAX_CONNECTIONS		10
#define SERVER_PORT			"9999"


class WebServ
{
	private:
		ListenerSocket	listener;
	public:
		WebServ();
		~WebServ();
	
	ListenerSocket	getListener() const;
		
};

#endif