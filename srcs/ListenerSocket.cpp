#include "ListenerSocket.hpp"

ListenerSocket::ListenerSocket()
{

}

ListenerSocket::~ListenerSocket()
{
	
}

int	ListenerSocket::create_listener_socket(addrinfo *_result, int &_fd_listener)
{
	int optval = 1;

	_fd_listener = socket (_result->ai_family, _result->ai_socktype, _result->ai_protocol);
	
	int fd_flag = fcntl(_fd_listener, F_GETFL, 0);
	fcntl(_fd_listener, F_SETFL, fd_flag | O_NONBLOCK);

	if (_fd_listener == -1)
	{
		std::cout << "ERROR: fd_listener" << std::endl;
		return (1);
	}
	if (setsockopt (_fd_listener, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (int)) == -1)
	{
		std::cout << "ERROR: setsockopt" << std::endl;
		return (1);
	}
	return (0);
}

int	ListenerSocket::bind_listener_socket(addrinfo *_result, int &_fd_listener)
{
	if (bind (_fd_listener, _result->ai_addr, _result->ai_addrlen) != 0)  // Success
	{
		if (close (_fd_listener) == -1)
		{
			std::cout << "ERROR: binding port fd close " << std::endl;
			return (1);
		}
		std::cout << "ERROR: binding port: " << _result->ai_addr << std::endl;
		return (1);
	}
	return (0);
}

int	ListenerSocket::start_listen(int &_fd_listener, int max_worker_connections)
{
	if (listen (_fd_listener, max_worker_connections) == -1)
	{
		std::cout << "ERROR: start listen port: " << std::endl;
		return (1);
	}
	return(0);
}

int ListenerSocket::setupSocket(addrinfo *_result, int &_fd_listener, int max_worker_connections)
{
	if (create_listener_socket(_result, _fd_listener) == 1)
		return (1);
	if (bind_listener_socket(_result, _fd_listener) == 1)
		return (1);
	if (start_listen(_fd_listener, max_worker_connections) == 1)
		return (1);
	return (0);
}
