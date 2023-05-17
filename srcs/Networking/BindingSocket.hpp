#ifndef BINDINGSOCKET_HPP
#define BINDINGSOCKET_HPP

#include <iostream>
#include "SimpleSocket.hpp"

namespace webserv 
{
	class BindingSocket: public SimpleSocket
	{
		private:
			int		_connection;

		public:
			BindingSocket(int domain, int type, int protocol, int port, u_long interface);
			~BindingSocket();
	
			int	connect_to_network(int fd_socket, struct sockaddr_in address);

	};
}

#endif