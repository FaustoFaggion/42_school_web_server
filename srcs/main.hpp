#ifndef MAIN_HPP
#define MAIN_HPP

#include <iostream>		// For cout
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

#define MAX_CONNECTIONS		10
#define SERVER_PORT			"9999"

#endif