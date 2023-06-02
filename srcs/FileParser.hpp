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
	int				_worker_processes;
};

class FileParser {

	private:
		std::string							_server_conf_file;
		listener_socket						_listener;
		std::map<std::string, std::string>	_path;

	public:
		FileParser();
		FileParser(char *file);
		~FileParser();

		int			get_domain() const;
		std::string	get_port() const;
		int			get_flag() const;
		int			get_worker_processes() const;
		int			get_type() const;


		void		parse_file(char *file);
		void		setup_listener(std::string buff);
		void		setup_location(std::string str);


};

#endif