#ifndef FileParser_HPP
#define FileParser_HPP

#include "main.hpp"
#include <stdio.h>
#include <fstream>
#include <algorithm>
#include <map>

class FileParser {

	private:
		int				_domain;
		std::string		_port;
		int				_flag;
		int				_type;
		int				_worker_connections;

	public:
		FileParser();
		FileParser(char *file);
		~FileParser();

		int			get_domain() const;
		std::string	get_port() const;
		int			get_flag() const;
		int			get_worker_connections() const;
		int			get_type() const;


		void	parse_listener_socket(char *file);
		void	fill_struct_conf_file(std::string buff);


};

#endif