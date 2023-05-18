#ifndef CONNECTIONS_HPP
#define CONNECTIONS_HPP

#include "main.hpp"
#include "ListenerSocket.hpp"

class : public ListenerSocket
{
	private:
		struct epoll_event	_ev;
		struct epoll_event	_ep_event [MAX_CONNECTIONS];
		int					_efd;

	public:
		Connections();
		~Connections();

};

#endif