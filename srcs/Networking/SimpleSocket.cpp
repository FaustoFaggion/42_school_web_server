#include "SimpleSocket.hpp"

/* COMMENTS

1-	The htons() function converts the unsigned short integer hostshort from host
	byte order to network byte order.

2-	The htonl() function converts the unsigned integer hostlong from host byte
	order to network byte order.
*/

webserv::SimpleSocket::SimpleSocket(int domain, int type, int protocol, int port,
									u_long interface)
{
	/*DEFINE ADDRESS STRUCT*/
	_address.sin_family = domain;
	_address.sin_port = htons(port); // 1
	_address.sin_addr.s_addr = ntohl(interface); // 2
	/*CREATE SOCKET*/
	_fd_socket = socket(domain, type, protocol);
	test_connection(_fd_socket);
}

/*GETTERS AND SETTERS*/

int					webserv::SimpleSocket::getFdSocket() const
{
	return(_fd_socket);
}

struct sockaddr_in	webserv::SimpleSocket::getAddress() const
{
	return(_address);
}

/*METHODS*/

/*CONFIRM IF _fd_socket AND _connection WERE CREATED*/
void	webserv::SimpleSocket::test_connection(int item_to_test)
{
	if (item_to_test < 0)
	{
		perror("Socket constructor Failure!");
		exit(EXIT_FAILURE);
	}
}