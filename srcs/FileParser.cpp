#include "FileParser.hpp"

FileParser::FileParser()
{
	_domain = 0;
	_type = 0;
	_flag = 0;
	_port = "";
	_worker_connections = 0;
}

FileParser::FileParser(char * file)
{
	_domain = 0;
	_type = 0;
	_flag = 0;
	_port = "";
	_worker_connections = 0;
	parse_listener_socket(file);
}

FileParser::~FileParser()
{

}

int			FileParser::get_domain() const
{
	return (_domain);
}

std::string	FileParser::get_port() const
{
	return (_port);
}

int			FileParser::get_flag() const
{
	return (_flag);
}

int			FileParser::get_worker_connections() const
{
	return (_worker_connections);
}

int			FileParser::get_type() const
{
	return (_type);
}

void	FileParser::fill_struct_conf_file(std::string buff)
{
	
	if (strncmp("listen [::]", buff.c_str(), 11) == 0)
	{
		if (_domain == AF_INET)
			_domain = AF_UNSPEC;
		else
			_domain = AF_INET6;
		if (_port == "")
		{
			int i = 12;
			while (isdigit(buff.at(i)) == true)
			{
				_port += buff.at(i);
				i++;
			}
		}
	}
	else if (strncmp("listen", buff.c_str(), 6) == 0)
	{
		_domain = AF_INET;
		int i = 7;
		while (isdigit(buff.at(i)) == true)
		{
			_port += buff.at(i);
			i++;
		}
	}
	else if (strncmp("server_name", buff.c_str(), 11) == 0)
	{
		std::string	server_name;
		int i = 12;

		while (buff.at(i) != '\0')
		{
			server_name += buff.at(i);
			i++;
		}
		if (strcmp("localhost", server_name.c_str()) == 0)
			_flag = AI_PASSIVE;

	}
	else if (strncmp("worker_connections", buff.c_str(), 18) == 0)
	{
		int i = 18;
		std::string	tmp;
		while (isspace(buff.at(i)))
			i++;
		while (isdigit(buff.at(i)))
		{
			tmp += buff.at(i);
			i++;
		}
		_worker_connections = atoi(tmp.c_str());
	}
}

void	cleanSpaces(std::string& str) {
    // Remove leading spaces
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));

    // Remove trailing spaces
    str.erase(std::find_if(str.rbegin(), str.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), str.end());
	str += '\0';
}

void	FileParser::parse_listener_socket(char *file)
{
	std::fstream	conf_file;
	std::string		buff;

	conf_file.open(file,  std::fstream::in);
	if (conf_file.fail())
		std::cout << "Configuration file fail to read" << std::endl;
	while (1)
	{
		std::getline(conf_file, buff,'\n');
		cleanSpaces(buff);
		fill_struct_conf_file(buff);
		// std::cout << buff << std::endl;
		if (conf_file.eof())
			break ;

	}

}