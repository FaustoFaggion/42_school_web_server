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
	parse_configuration_file(file);
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

std::map<std::string, std::string>	FileParser::getPath() const
{
	return (_path);
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

void		setup_location(std::string str)
{
	(void)str;
}

void		FileParser::file_to_string(char *file, std::string &response)
{
	std::fstream	open_file;
	std::string		buff;

	open_file.open(file,  std::fstream::in);
	if (open_file.fail())
		std::cout << "Configuration file fail to read" << std::endl;

	while (!open_file.eof())
	{
		std::getline(open_file, buff,'\n');
		response += buff;
		response += "\n";
	}
	response += '\0';
}

std::string	FileParser::str_substring(std::string &str, std::string find, int init, char finish)
{
	std::string	response;
	if (str.find(find, init) != str.npos)
	{
		size_t start = str.find(find, 0);
		size_t end = str.find(finish, start);
		
		std::string tmp = str.substr(start, (end - start));
		response = tmp;
		response += '\0';
		str.erase(start, (end- start));
	}
	return (response);
}

void	FileParser::chk_simple_directive(std::string &str)
{

	size_t end = str.find(";", 0);
	if (end == str.npos)
	{
		std::cout << "ERROR:root missing ';'" << std::endl;
		exit(2);
	}
	str = str.substr(0, end);
	str += '\0';
}

std::string	FileParser::get_simple_directive_value(std::string &str, char finish)
{
	std::string	response;
	int			i;
	
	i = 0;
	while (!isspace(str.at(i)))
		i++;
	while (isspace(str.at(i)))
		i++;
	while (str.at(i) != finish)
	{
		response += str.at(i);
		i++;
	}
	response += '\0';
	return (response);
}

void	FileParser::parse_listener()
{
	// /*PARSE SERVEER FAMILY AND PORT*/
	std::string	ltn;
	while (_server_conf_file.find("listen", 0) != _server_conf_file.npos)
	{
		ltn = str_substring(_server_conf_file, "listen", 0, '\n');
		chk_simple_directive(ltn);
		setup_listener(ltn);
	}
	std::cout << "PARSE FAMILY AND PORT \n" << _server_conf_file << "\n\n";

	// // /*PARSE SERVER_NAME*/

	std::string	sn;;
	while (_server_conf_file.find("server_name", 0) != _server_conf_file.npos)
	{
		sn = str_substring(_server_conf_file, "server_name", 0, '\n');
		chk_simple_directive(sn);
		setup_listener(sn);
	}

	std::cout << "PARSE SERVER_NAME \n" << _server_conf_file << "\n\n";

	// // /*PARSE WORKER_PROCESSES*/
	std::string	wp;;
	while (_server_conf_file.find("worker_processes", 0) != _server_conf_file.npos)
	{
		wp = str_substring(_server_conf_file, "worker_processes", 0, '\n');
		chk_simple_directive(wp);
		setup_listener(wp);
	}

	std::cout << "PARSE WORKER_PROCESSES \n" << _server_conf_file << "\n\n";

}

void	FileParser::parse_locations()
{
	// /*PARSE ROOT*/
	std::string	root_directive;
	std::string	root_request_path;
	while (_server_conf_file.find("root", 0) != _server_conf_file.npos)
	{
		root_directive = str_substring(_server_conf_file, "root", 0, '\n');
		chk_simple_directive(root_directive);
		root_request_path = get_simple_directive_value(root_directive, '\0');
		
		std::cout << _server_conf_file << "\n\n";

		if (access(root_request_path.c_str(), F_OK) != 0)
		{
			std::cout << "Path " << root_request_path << " do not exist!" << std::endl;
		}
		_path["/"] = root_request_path;

		std::cout << "root: " << _path["/"] << std::endl;
	}

	/*PARSE LOCATIONS*/
	std::string	location;
	std::string	request_path;
	std::string	server_path;

	while (_server_conf_file.find("location", 0) != _server_conf_file.npos)
	{
		location = str_substring(_server_conf_file, "location", 0, '}');
		request_path = str_substring(location, "location", 0, '\n');
		request_path = get_simple_directive_value(request_path, ' ');
		
		std::cout << "request_path: " << request_path << std::endl;
	
		server_path = str_substring(location, "root", 0, '\n');

		size_t start = location.find("root", 0);
		size_t end = location.find("\n", start);
		/*NO ROOT DIRECTIVE INSIDE LOCATION BLOCK*/
		if (start == location.npos)
		{
			if (_path["/"] == "")
			{
				std::cout << "ERROR: root not defined" << std::endl;
				exit(2);
			}
			server_path = _path["/"] + request_path;
		}
	// 	/*THERE IS A ROOT DIRECTIVE INSIDE LOCATION BLOCK*/
		else
		{
			std::string	root_directive = location.substr(start, (end - start));
			root_directive += '\0';
			server_path = get_simple_directive_value(root_directive, ' ');
		
			server_path += request_path;
		}
		/*REWRITE THE PATH CORRECTILY IF '//' IS FIND*/
		size_t pos = server_path.find("//");
		if (pos != server_path.npos)
		{
			server_path.replace(pos, 1, "");
		}

		/*INSERT LOCATION PATH INTO THE _PATH MAP*/
		_path[request_path] = server_path;

		/*PRINT*/
		std::map<std::string, std::string>::iterator	it;
		it = _path.begin();
		for (; it != _path.end(); it++)
			std::cout << "location: " << (*it).first << " : " << (*it).second << std::endl;
	}
}

void	FileParser::parse_configuration_file(char *file)
{
	std::string	configuration_file;

	file_to_string(file, configuration_file);

	std::cout << configuration_file << std::endl;
	
	// /*PARSE EACH SERVER FROM CONFIGURATION FILE TO A STRING*/
	// while()
	_server_conf_file = str_substring(configuration_file, "server", 0, '\0');
	
	std::cout << _server_conf_file << std::endl;
	
	/*ERASE COMMENTS*/
	while (_server_conf_file.find("#", 0) != _server_conf_file.npos)
		str_substring(_server_conf_file, "#", 0, '\n');
	
	std::cout << "ERASE COMMENTS \n" << _server_conf_file << "\n\n";
	
	parse_listener();
	
	parse_locations();
	
}
