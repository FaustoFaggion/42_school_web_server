#include "ListenerSocket.hpp"

ListenerSocket::ListenerSocket()
{
	addrinfo();
	get_fd();
	bind_fd_to_port();
	listen_fd();
}

ListenerSocket::~ListenerSocket()
{
	freeaddrinfo(_result);
}

void	ListenerSocket::addrinfo()
{
	memset(&_hints, 0, sizeof (struct addrinfo));
	_hints.ai_family = AF_UNSPEC;		/* allow IPv4 or IPv6 */
	_hints.ai_socktype = SOCK_STREAM;	/* Stream socket */
	_hints.ai_flags = AI_PASSIVE;		/* for wildcard IP address*/
	if ((_s = getaddrinfo (NULL, SERVER_PORT, &_hints, &_result)) != 0)
	{
		std::cout << stderr << "getaddrinfo: " << gai_strerror (_s) << std::endl;
		exit (EXIT_FAILURE);
	}
}

void	ListenerSocket::get_fd()
{
	int optval = 1;
	_fd_listener = socket (_result->ai_family, _result->ai_socktype,
							_result->ai_protocol);
		if (_fd_listener == -1)
			std::cout << "ERROR: fd_listener" << std::endl;

		if (setsockopt (_fd_listener, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (int)) == -1)
			std::cout << "ERROR: setsockopt" << std::endl;

}

void	ListenerSocket::bind_fd_to_port()
{
	if (bind (_fd_listener, _result->ai_addr, _result->ai_addrlen) == 0)  // Success
	{
		return ;
	}
	if (close (_fd_listener) == -1)
		std::cout << "ERROR: close"<< std::endl;

}

void	ListenerSocket::listen_fd()
{
	if (listen (_fd_listener, MAX_CONNECTIONS) == -1)
		strerror(errno);
}