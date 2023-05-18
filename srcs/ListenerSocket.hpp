#ifndef LISTENERSOCKET_HPP
#define LISTENERSOCKET_HPP


#include "main.hpp"

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