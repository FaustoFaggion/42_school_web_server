#include "ListenerSocket.hpp"

ListenerSocket::ListenerSocket()
{
	memset(&_hints, 0, sizeof(struct addrinfo));
	memset(&_result, 0, sizeof(struct addrinfo));
	_s = 0;
	_fd_listener = 0;
}

ListenerSocket::~ListenerSocket()
{
	freeaddrinfo(_result);
}

int		ListenerSocket::get_fd_listener()
{
	return (_fd_listener);
}

void	ListenerSocket::addrinfo(int domain, int type, int flag, std::string port)
{
	memset(&_hints, 0, sizeof (struct addrinfo));
	_hints.ai_family = domain;		/* allow IPv4 or IPv6 */
	_hints.ai_socktype = type;	/* Stream socket */
	_hints.ai_flags = flag;		/* for wildcard IP address*/
	if ((_s = getaddrinfo (NULL, port.c_str(), &_hints, &_result)) != 0)
	{
		std::cout << stderr << "getaddrinfo: " << gai_strerror (_s) << std::endl;
		exit (EXIT_FAILURE);
	}
}

void	ListenerSocket::create_fd()
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