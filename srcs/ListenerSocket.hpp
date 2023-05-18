#ifndef LISTENERSOCKET_HPP
#define LISTENERSOCKET_HPP

#include "WebServ.hpp"

#include <netdb.h>		// For struct addinfo
#include <arpa/inet.h>	// For inet_ntop function
#include <sys/socket.h>	// For socket functions
#include <netinet/in.h>	// For sockaddr_in

#include <sys/epoll.h>

class ListenerSocket 
{
	private:
		struct addrinfo		_hints;
		struct addrinfo		*_result;
		int					_s;
	public:
		ListenerSocket();
		~ListenerSocket();

		void	addrinfo(void);
		void	get_fd(void);
		void	bind_fd_to_port(void);
		void	listen_fd(void);
		int					_fd_listener;
};


#endif