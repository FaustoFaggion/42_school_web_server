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

std::map<std::string, directive>	FileParser::getPath() const
{
	return (_path);
}

std::vector<std::string>	FileParser::getIndex() const
{
	return (_index);
}

std::map<std::string, std::string>	FileParser::getDirList() const
{
	return (_dir_list);
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
	
	std::istringstream iss(str);
	std::string tmp1, tmp2;
	iss >> tmp1 >> response >> tmp2;
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

	// // /*PARSE SERVER_NAME*/
	std::string	sn;;
	while (_server_conf_file.find("server_name", 0) != _server_conf_file.npos)
	{
		sn = str_substring(_server_conf_file, "server_name", 0, '\n');
		chk_simple_directive(sn);
		setup_listener(sn);
	}

	// // /*PARSE WORKER_PROCESSES*/
	std::string	wp;;
	while (_server_conf_file.find("worker_processes", 0) != _server_conf_file.npos)
	{
		wp = str_substring(_server_conf_file, "worker_processes", 0, '\n');
		chk_simple_directive(wp);
		setup_listener(wp);
	}
}

void	FileParser::parse_path(std::string &str, std::string find, std::string &path, int flag)
{
	std::string	simple_directive;
	
	simple_directive = str_substring(str, find, 0, '\n');
	/*0 FOR SIMPLE DIRECTIVE; 1 FOR BLOCK DIRECTIVE*/
	if (flag == SIMPLE_DIRECTIVE)
		chk_simple_directive(simple_directive);
	path = get_simple_directive_value(simple_directive);
	// if (access(path.c_str(), F_OK) != 0)
	// {
	// 	std::cout << "Path " << path << " do not exist!" << std::endl;
	// }
}

 bool	FileParser::parse_simple_root_directive()
 {
	std::string	root_path;

	/*CHECK FOR LOCATION BLOCK DIRECTIVES*/
	if (_server_conf_file.find("location ", 0) == _server_conf_file.npos)
	{
		std::cout << "No location defined in server configuration file!!\n";
		exit(1);
	}

	/*IS THERE A ROOT SIMPLE DIRECTIVE?*/
	size_t end = _server_conf_file.find("location ", 0);
	size_t start = _server_conf_file.find("root", 0);
	/*NO*/
	if (start > end)
		return (false);
	/*YES*/
	else
	{
		parse_path(_server_conf_file, "root", root_path, SIMPLE_DIRECTIVE);
		_path["/"]._server_path = root_path;
		_path["/"]._autoindex = false;

		/*CHECK DUPLICATED DIRECTIVE*/
		/*IF THERE ARE IDENCAL DIRECTIVES, JUST THE FIRST WILL BE CONSIDERED*/
		while (1)
		{
			size_t end = _server_conf_file.find("location ", 0);
			size_t start = _server_conf_file.find("root", 0);
			if (start > end)
				break ;
			else
				std::string tmp = str_substring(_server_conf_file, "root", 0, '\n');
		}
	}
	/*IF DIRECTORY DO NOT EXIST EXIT PROGRAM*/
	if (access(root_path.c_str(), F_OK) != 0)
	{
		std::cout << "Path " << root_path << " do not exist!" << std::endl;
		exit(2);
	}
	return (true);
 }

/*GET INDEX FILES TO INCLUDE IN A VECTOR*/
void	FileParser::parse_index(std::vector<std::string> &idx, std::string &str)
{
	std::string index_tmp;
	size_t pos = str.find("index", 0);

	if (pos == index_tmp.npos)
		return;

	int j = 0;
	index_tmp = str_substring(str, "index", 0, '\n');
	for (size_t i = 6; i < index_tmp.size(); i++)
	{
		if (index_tmp.at(i) == ';' || isspace(index_tmp.at(i)) != 0)
		{
			idx.push_back(index_tmp.substr(j, (i - j)));
			j = 0;
		}
		else if (j == 0)
		{
			j = i;
		}
	}
	/*CHECK DUPLICATED DIRECTIVE*/
	/*IF THERE ARE IDENCAL DIRECTIVES, JUST THE FIRST WILL BE CONSIDERED*/
	while (1)
	{
		size_t end = str.find("location ", 0);
		size_t start = str.find("index", 0);
		if (start >= end)
			break ;
		else
			std::string tmp = str_substring(str, "index", 0, '\n');
	}

	for (std::vector<std::string>::iterator it = idx.begin(); it != idx.end(); it++)
	{
		std::cout << (*it).c_str() << std::endl;
	}
}

void	FileParser::parse_locations(bool simple_root_directive)
{	
	std::string	root_path;
	std::string	location;
	std::string	request_path;
	std::string	server_path;

	while (_server_conf_file.find("location ", 0) != _server_conf_file.npos)
	{
		location = str_substring(_server_conf_file, "location ", 0, '}');
		parse_path(location, "location ", request_path, BLOCK_DIRECTIVE);

		/*ROOT INSIDE LOCATION BLOCK TAKES PRECEDENCE OVER SIMPLE ROOT DIRECTIVE*/
		if (location.find("root", 0) != location.npos)
			parse_path(location, "root", root_path, SIMPLE_DIRECTIVE);
		else
		{
			/*IF ROOT NOT DEFINED*/
			if (simple_root_directive == false)
			{
				std::cout << "ERROR: root not defined" << std::endl;
				exit(2);
			}
		}

		server_path = root_path + request_path;
		
		/*REWRITE THE PATH CORRECTILY IF '//' IS FIND*/
		size_t pos = server_path.find("//");
		if (pos != server_path.npos)
			server_path.replace(pos, 1, "");

		/*IF DIRECTORY DO NOT EXIST EXIT PROGRAM*/
		if (access(server_path.c_str(), F_OK) != 0)
		{
			std::cout << "Path " << server_path << " do not exist!" << std::endl;
			exit(2);
		}

		/*INSERT LOCATION PATH INTO THE _PATH MAP*/
		_path[request_path]._server_path = server_path;

		/*CHECK AUTOINDEX TO LIST DIRECTORY*/
		_path[request_path]._autoindex = false;
		if (location.find("autoindex", 0) != location.npos)
		{
			std::string dir = str_substring(location, "autoindex", 0, '\n');
			if (dir.find("on", 0) != dir.npos)
				_path[request_path]._autoindex = true;
		}

		/*CHECK INDEX DIRECTIVE*/
		if (location.find("index", 0) != location.npos)
		{
			std::string	idx;
			idx = str_substring(location, "index", 0, '\n');
			std::cout << "location: " << location << "  idx: " << idx << "\n";
			parse_index(_path[request_path]._index_block, idx);
		}
		else
			_path[request_path]._index_block = _index;

		/*CHECK DUPLICATED DIRECTIVE*/
		/*IF THERE ARE IDENCAL DIRECTIVES, JUST THE FIRST WILL BE CONSIDERED*/
		std::string	duplicate = "location " + request_path + " ";
		while (_server_conf_file.find(duplicate, 0) != _server_conf_file.npos)
			location = str_substring(_server_conf_file, duplicate, 0, '}');
	}
		/*PRINT*/
		std::map<std::string, directive>::iterator	it;
		it = _path.begin();
		std::cout << "Print Locations:\n";
		for (; it != _path.end(); it++)
		{
			std::cout << "location: " << (*it).first << "\n";
			std::cout << "	server_path: " << (*it).second._server_path << std::endl;
			std::cout << "	autoindex: " << (*it).second._autoindex << std::endl;
			for (std::vector<std::string>::iterator it2 = (*it).second._index_block.begin(); it2 != (*it).second._index_block.end(); it2++)
				std::cout << "	indexblock: " << *it2 << std::endl;
		}
}


void	FileParser::parse_configuration_file(char *file)
{
	std::string	configuration_file;

	file_to_string(file, configuration_file);
	
	/*PARSE EACH SERVER FROM CONFIGURATION FILE TO A STRING*/
	// Falta parsear arquivos com mais de um servidor.
	size_t	start;
	size_t	end;
	start = configuration_file.find("server", 0);
	end = configuration_file.find_last_of("}", start);
	_server_conf_file = configuration_file.substr(start, (end - start));
	
	/*ERASE COMMENTS*/
	while (_server_conf_file.find("#", 0) != _server_conf_file.npos)
		str_substring(_server_conf_file, "#", 0, '\n');
	
	parse_listener();
	
	/*PARSE ROOT SIMPLE DIRECTIVE*/
	bool	simple_root_directive;

	simple_root_directive = parse_simple_root_directive();
	
	/*PARSE INDEX*/

	parse_index(_index, _server_conf_file);
	
	/*PARSE LOCATIONS*/
	parse_locations(simple_root_directive);

}
