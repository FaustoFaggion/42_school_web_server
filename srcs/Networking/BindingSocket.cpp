#include "BindingSocket.hpp"

webserv::BindingSocket::BindingSocket(int domain, int type, int protocol,
							int port, u_long interface) : SimpleSocket(domain,
							type, protocol, port, interface)
{
	/*BIND*/
	_connection = connect_to_network(getFdSocket(), getAddress());
	test_connection(_connection);
}

int	webserv::BindingSocket::connect_to_network(int fd_socket,
							struct sockaddr_in address)
{
	return (bind(fd_socket, (struct sockaddr *)&address, sizeof(address)));
}
