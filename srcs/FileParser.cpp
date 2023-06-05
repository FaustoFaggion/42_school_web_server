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
	if (buff.compare(0, 11, "listen [::]") == 0)
	{
		if (_listener._domain == AF_INET)
			_listener._domain = AF_UNSPEC;
		else
			_listener._domain = AF_INET6;
		if (_listener._port == "")
		{
			size_t i = 0;
			while (i < buff.size() && !isdigit(buff.at(i)))
				i++;
			while (i < buff.size() && isdigit(buff.at(i)))
			{
				_listener._port += buff.at(i);
				i++;
			}
		}
	}
	else if (buff.compare(0, 6, "listen") == 0)
	{
		
		_listener._domain = AF_INET;
		
		size_t i = 0;
		while (i < buff.size() && !isdigit(buff.at(i)))
			i++;
		while (i < buff.size() && isdigit(buff.at(i)))
		{
			_listener._port += buff.at(i);
			i++;
		}
	}
	else if (buff.compare(0, 11, "server_name") == 0)
	{
		std::string	server_name;

		size_t i = 12;
		while (i < buff.size() && isspace(buff.at(i)))
			i++;
		while (i < buff.size())
		{
			server_name += buff.at(i);
			i++;
		}
		if (strcmp("localhost", server_name.c_str()) == 0)
			_listener._flag = AI_PASSIVE;

	}
	else if (buff.compare(0, 11, "worker_processes") == 0)
	{
		std::string	tmp;
		
		size_t i = 19;
		while (i < buff.size() && isspace(buff.at(i)))
			i++;
		while (i < buff.size() && isdigit(buff.at(i)))
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
}

std::string	FileParser::str_substring(std::string &str, std::string find, int init, char finish)
{
	std::string	response = "";
	if (str.find(find, init) != str.npos)
	{
		size_t start = str.find(find, 0);
		size_t end = str.find(finish, start);
		
		response = str.substr(start, (end - start));
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
}

std::string	FileParser::get_simple_directive_value(std::string &str)
{
	std::string	response;
	// int			i;
	
		std::istringstream iss(str);
    	std::string tmp1, tmp2;
    	iss >> tmp1 >> response >> tmp2;

		std::cout << "tmp1: " << tmp1 << "\n";
		std::cout << "tmp2: " << tmp2 << "\n";
		std::cout << "response: " << response << "\n";
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

void	FileParser::parse_root(std::string &str, std::string &root_path)
{
	std::string	root_directive;
	
	root_directive = str_substring(str, "root", 0, '\n');
	chk_simple_directive(root_directive);
	root_path = get_simple_directive_value(root_directive);
	
	std::cout << _server_conf_file << "\n\n";

	if (access(root_path.c_str(), F_OK) != 0)
	{
		std::cout << "Path " << root_path << " do not exist!" << std::endl;
	}
	_path["/"] = root_path;
	std::cout << "root: " << _path["/"] << std::endl;

}

void	FileParser::parse_locations()
{
	std::string	root_path = "";
	int			not_root_path = 0;
	
	if (_server_conf_file.find("location ", 0) == _server_conf_file.npos)
	{
		std::cout << "No location defined in server configuration file!!\n";
		exit(1);
	}
	/*IS THERE A SIMPLE DIRECTIVE DEFINIG ROOT?*/
	size_t end = _server_conf_file.find("location ", 0);
	size_t start = _server_conf_file.find("root", 0);
	/*NO*/
	if (start > end)
		not_root_path = 1;
	/*YES*/
	else
		parse_root(_server_conf_file, root_path);

	/*PARSE LOCATIONS*/
	std::string	location;
	std::string	request_path;
	std::string	server_path;

	while (_server_conf_file.find("location ", 0) != _server_conf_file.npos)
	{
		location = str_substring(_server_conf_file, "location ", 0, '}');
		request_path = str_substring(location, "location ", 0, '\n');
		request_path = get_simple_directive_value(request_path);
		
		std::cout << "request_path: " << request_path << std::endl;

		/*DEFINE ROOT DIRECTIVE IN NOT DEFINED*/
		if (not_root_path == 1)
		{
			if (location.find("root", 0) == location.npos)
			{
				std::cout << "ERROR: root not defined" << std::endl;
				exit(2);
			}
			parse_root(location, root_path);
		}

		server_path = root_path + request_path;
		
		/*REWRITE THE PATH CORRECTILY IF '//' IS FIND*/
		size_t pos = server_path.find("//");
		if (pos != server_path.npos)
		{
			server_path.replace(pos, 1, "");
		}

		/*INSERT LOCATION PATH INTO THE _PATH MAP*/
		_path[request_path] = server_path;

	}
		/*PRINT*/
		std::map<std::string, std::string>::iterator	it;
		it = _path.begin();
		for (; it != _path.end(); it++)
			std::cout << "location: " << (*it).first << " : " << (*it).second << std::endl;
}

void	FileParser::parse_configuration_file(char *file)
{
	std::string	configuration_file;

	file_to_string(file, configuration_file);

	std::cout << configuration_file << std::endl;
	
	// /*PARSE EACH SERVER FROM CONFIGURATION FILE TO A STRING*/
	
	// Falta parsear arquivos com mais de um servidor.
	size_t	start;
	size_t	end;
	start = configuration_file.find("server", 0);
	end = configuration_file.find_last_of("}", start);
	_server_conf_file = configuration_file.substr(start, (end - start));
	
	/*ERASE COMMENTS*/
	while (_server_conf_file.find("#", 0) != _server_conf_file.npos)
		str_substring(_server_conf_file, "#", 0, '\n');
	
	std::cout << "ERASE COMMENTS \n" << _server_conf_file << "\n\n";
	
	parse_listener();
	
	parse_locations();
	
}
