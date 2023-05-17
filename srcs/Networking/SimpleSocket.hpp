#ifndef SIMPLESOCKET_HPP
#define SIMPLESOCKET_HPP

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h> /* Lib that creates sockaddr_in struct 
						link https://pubs.opengroup.org/onlinepubs/7908799/xns/
						netinetin.h.html*/

namespace webserv
{
	class SimpleSocket {

		private:
			struct sockaddr_in	_address;
			int					_fd_socket;

		public:
			/*CONSTRUCTORS*/
			SimpleSocket(int domain, int type, int protocol, int port, u_long interface);
			~SimpleSocket();

			/*GETTERS AND SETTERS*/
			int					getFdSocket() const;
			struct sockaddr_in	getAddress() const;
			
			/*METHODS*/
			void				test_connection(int item_to_test);
	
	};
}

#endif