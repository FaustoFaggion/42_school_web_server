#ifndef LISTENERSOCKET_HPP
#define LISTENERSOCKET_HPP


#include "main.hpp"

class ListenerSocket
{
	private:
		struct addrinfo		_hints;
		struct addrinfo		*_result;
		int					_s;
		int					_fd_listener;
	public:
		ListenerSocket();
		~ListenerSocket();

		int		get_fd_listener();
		
		void	addrinfo(int domain, int type, int flag, std::string port);
		void	create_fd();
		void	bind_fd_to_port();
		void	listen_fd();
};


#endif