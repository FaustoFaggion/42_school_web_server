#include "Connections.hpp"

Connections::Connections()
{
	if ((_efd = epoll_create(MAX_CONNECTIONS)) == -1)
		std::cout << "ERROR: epoll_create1" << std::endl;
	_ev.events = EPOLLIN; // File descriptor is available for read.
	_ev.data.fd = listener;
	if (epoll_ctl (_efd, EPOLL_CTL_ADD, listener, &_ev) == -1)
		std::cout << "ERROR: epoll_ctl" << std::endl;
}