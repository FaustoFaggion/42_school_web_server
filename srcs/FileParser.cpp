#include "FileParser.hpp"

FileParser::FileParser()
{
	_listener._domain = 0;
	_listener._type = 0;
	_listener._flag = 0;
	_listener._port = "";
	_listener._worker_processes = 0;
}

FileParser::FileParser(char * file)
{
	_listener._domain = 0;
	_listener._type = 0;
	_listener._flag = 0;
	_listener._port = "";
	_listener._worker_processes = 0;
	parse_file(file);
}

FileParser::~FileParser()
{

}

int			FileParser::get_domain() const
{
	return (_listener._domain);
}

std::string	FileParser::get_port() const
{
	return (_listener._port);
}

int			FileParser::get_flag() const
{
	return (_listener._flag);
}

int			FileParser::get_worker_processes() const
{
	return (_listener._worker_processes);
}

int			FileParser::get_type() const
{
	return (_listener._type);
}

void	FileParser::setup_listener(std::string buff)
{
	
	if (strncmp("listen [::]", buff.c_str(), 11) == 0)
	{
		if (_listener._domain == AF_INET)
			_listener._domain = AF_UNSPEC;
		else
			_listener._domain = AF_INET6;
		if (_listener._port == "")
		{
			int i = 12;
			while (isspace(buff.at(i)))
				i++;
			while (isdigit(buff.at(i)))
			{
				_listener._port += buff.at(i);
				i++;
			}
		}
	}
	else if (strncmp("listen", buff.c_str(), 6) == 0)
	{
		
		_listener._domain = AF_INET;
		
		int i = 7;
		while (isspace(buff.at(i)))
			i++;
		while (isdigit(buff.at(i)))
		{
			_listener._port += buff.at(i);
			i++;
		}
	}
	else if (strncmp("server_name", buff.c_str(), 11) == 0)
	{
		std::string	server_name;

		int i = 12;
		while (isspace(buff.at(i)))
			i++;
		while (buff.at(i) != '\0')
		{
			server_name += buff.at(i);
			i++;
		}
		if (strcmp("localhost", server_name.c_str()) == 0)
			_listener._flag = AI_PASSIVE;

	}
	else if (strncmp("worker_processes", buff.c_str(), 18) == 0)
	{
		std::string	tmp;
		
		int i = 19;
		while (isspace(buff.at(i)))
			i++;
		while (isdigit(buff.at(i)))
		{
			tmp += buff.at(i);
			i++;
		}
		_listener._worker_processes = atoi(tmp.c_str());
	}
}

void	cleanSpaces(std::string& str) {
    // Remove leading spaces
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));

    // Remove trailing spaces
    str.erase(std::find_if(str.rbegin(), str.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), str.end());
	str += '\0';
}

void	FileParser::parse_file(char *file)
{
	std::fstream	conf_file;
	std::string		buff;

	conf_file.open(file,  std::fstream::in);
	if (conf_file.fail())
		std::cout << "Configuration file fail to read" << std::endl;
	
	/*PARSE EACH SERVER FROM CONFIGURATION FILE TO A STRING*/
	int i = 0;

	while (!conf_file.eof())
	{
		
		std::getline(conf_file, buff,'\n');
		if (buff.find("server", 0) != buff.npos)
		{
			i += 1;
			while (!conf_file.eof() && i != 0)
			{	
				std::getline(conf_file, buff,'\n');
				if (buff.find("{", 0) != buff.npos)
					i+= 1;
				if (buff.find("}", 0) != buff.npos)
					i-= 1;
				if (i == 0)
					break ;
				_server_conf_file += "\n";
				_server_conf_file += buff;
			}
		}
	}
	
	/*ERASE COMMENTS*/
	while (_server_conf_file.find("#", 0) != _server_conf_file.npos)
	{
		size_t start = _server_conf_file.find("#", 0);
		size_t end = _server_conf_file.find("\n", start);
		_server_conf_file.erase(start, (end- start));
	}
	std::cout << _server_conf_file << "\n\n";
	
	/*PARSE SERVEER FAMILY AND PORT*/
	std::string	str;

	while (_server_conf_file.find("listen", 0) != _server_conf_file.npos)
	{
		size_t start = _server_conf_file.find("listen", 0);
		size_t end = _server_conf_file.find("\n", start);
		std::string str = _server_conf_file.substr(start, (end - start));
		
		end = _server_conf_file.find(";", start);
		if (end == _server_conf_file.npos)
		{
			std::cout << "ERROR: missing ';'" << std::endl;
			exit(2);
		}
		
		str = _server_conf_file.substr(start, (end - start));
		str += '\0';
		std::cout << str << "\n";
		setup_listener(str);
		_server_conf_file.erase(start, (end- start) + 1);
		std::cout << _server_conf_file << "\n\n";
	}
	

	/*PARSE SERVER_NAME*/
	while (_server_conf_file.find("server_name", 0) != _server_conf_file.npos)
	{
		size_t start = _server_conf_file.find("server_name", 0);
		size_t end = _server_conf_file.find("\n", start);
		std::string str = _server_conf_file.substr(start, (end - start));
		
		end = _server_conf_file.find(";", start);
		if (end == _server_conf_file.npos)
		{
			std::cout << "ERROR: " << std::endl;
			exit(2);
		}
		
		str = _server_conf_file.substr(start, (end - start));
		str += '\0';
		setup_listener(str);
		_server_conf_file.erase(start, (end- start) + 1);
		std::cout << _server_conf_file << "\n\n";
	}

	/*PARSE WORKER_PROCESSES*/
	while (_server_conf_file.find("worker_processes", 0) != _server_conf_file.npos)
	{
		size_t start = _server_conf_file.find("worker_processes", 0);
		size_t end = _server_conf_file.find("\n", start);
		std::string str = _server_conf_file.substr(start, (end - start));

		end = _server_conf_file.find(";", start);
		if (end == _server_conf_file.npos)
		{
			std::cout << "ERROR: " << std::endl;
			exit(2);
		}
		
		str = _server_conf_file.substr(start, (end - start));
		str += '\0';
		setup_listener(str);
		_server_conf_file.erase(start, (end- start) + 1);
		std::cout << _server_conf_file << "\n\n";
	}

	// while (_server_conf_file.find("location", 0) != _server_conf_file.npos)
	// {
	// 	size_t start = _server_conf_file.find("location", 0);
	// 	size_t end = _server_conf_file.find("}", start);
	// 	std::string str = _server_conf_file.substr(start, (end - start));
	// }


	// while (!conf_file.eof())
	// {
	// 	std::getline(conf_file, buff,'\n');
	// 	cleanSpaces(buff);
	// 	setup_listener(buff);
	// }

}