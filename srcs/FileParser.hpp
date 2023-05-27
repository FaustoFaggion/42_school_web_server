#ifndef FileParser_HPP
#define FileParser_HPP

#include "main.hpp"
#include <stdio.h>
#include <fstream>
#include <algorithm>
#include <map>

struct listener_socket {
	int				_domain;
	std::string		_port;
	int				_flag;
	int				_type;
	int				_worker_connections;
};

class FileParser {

	private:
		listener_socket						_listener;
		std::map<std::string, std::string>	_path;

	public:
		FileParser();
		FileParser(char *file);
		~FileParser();

		int			get_domain() const;
		std::string	get_port() const;
		int			get_flag() const;
		int			get_worker_connections() const;
		int			get_type() const;


		void	parse_file(char *file);
		void	setup_listener(std::string buff);


};

#endif