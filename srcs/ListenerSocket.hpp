#ifndef LISTENERSOCKET_HPP
#define LISTENERSOCKET_HPP


#include "main.hpp"

class ListenerSocket
{
	public:
		ListenerSocket();
		~ListenerSocket();

		static int setupSocket(addrinfo *_result, int &_fd_listener, int max_worker_connections);
		static int	create_listener_socket(addrinfo *_result, int &_fd_listener);
		static int	bind_listener_socket(addrinfo *_result, int &_fd_listener);
		static int	start_listen(int &_fd_listener, int max_worker_connections);

};


#endif