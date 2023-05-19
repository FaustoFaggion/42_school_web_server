// #ifndef CONNECTIONS_HPP
// #define CONNECTIONS_HPP

// #include "main.hpp"
// #include "WebServ.hpp"

// class Connections: public WebServ
// {
// 	private:
// 		struct epoll_event	_ev;
// 		struct epoll_event	_ep_event [MAX_CONNECTIONS];
// 		int					_efd;

// 	public:
// 		Connections();
// 		~Connections();

// 		void	create_poll();
// 		void	add_connection();
// 		void	connections_monitoring();

// };

// #endif