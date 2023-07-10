#include "FileParser.hpp"

FileParser::FileParser()
{

}

FileParser::FileParser(char *file, std::string server_name)
{
	(void)file;
	(void)server_name;
}

FileParser::~FileParser()
{

}

std::map<std::string, directive>	FileParser::getPath() const
{
	return (_locations);
}

std::vector<std::string>	FileParser::getIndex() const
{
	return (_index);
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
		_locations["/"]._server_path = root_path;
		_locations["/"]._autoindex = false;

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

void	FileParser::parse_buffer_size()
{
	size_t		pos, size;
	std::string	buff, digits;

	pos = _server_conf_file.find("proxy_buffer_size", 0);

	if (pos != _server_conf_file.npos)
	{
		parse_path(_server_conf_file, "proxy_buffer_size", buff, SIMPLE_DIRECTIVE);
	}

	if ((pos = buff.find("K", 0)) != buff.npos)
	{
		for (std::string::iterator it = buff.begin(); (*it) != 'K'; it++)
			digits+= (*it);
		size = atoi(digits.c_str());
		_buffer_size = size * 1024;
	}
	else if ((pos = buff.find("M", 0)) != buff.npos)
	{
		for (std::string::iterator it = buff.begin(); (*it) != 'M'; it++)
			digits+= (*it);
		if (digits != "1")
		{
			std::cout << "ERROR: proxy_file_buffer directive too large. Max 1M";
			exit(1);
		}
		size = atoi(digits.c_str());
		_buffer_size = size * 1024 * 1024;
	}
	else
	{
		std::cout << "ERROR: proxy_file_buffer directive too large. Max 1M";
		exit(1);
	}
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

	// for (std::vector<std::string>::iterator it = idx.begin(); it != idx.end(); it++)
	// {
	// 	std::cout << (*it).c_str() << std::endl;
	// }
}

void	FileParser::parse_locations(bool simple_root_directive)
{	
	std::cout << "\nPARSE_LOCATIONS FUNCTION\n";
	
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
		_locations[request_path]._server_path = server_path;

		/*CHECK AUTOINDEX TO LIST DIRECTORY*/
		_locations[request_path]._autoindex = false;
		if (location.find("autoindex", 0) != location.npos)
		{
			std::string auto_idx = str_substring(location, "autoindex", 0, '\n');
			if (auto_idx.find("on", 0) != auto_idx.npos)
				_locations[request_path]._autoindex = true;
		}

		/*CHECK INDEX DIRECTIVE*/
		if (location.find("index", 0) != location.npos)
		{
			std::string	idx;
			idx = str_substring(location, "index", 0, '\n');
			// std::cout << "location: " << location << "  idx: " << idx << "\n";
			parse_index(_locations[request_path]._index_block, idx);
		}
		else
			_locations[request_path]._index_block = _index;

		/*CHECK DUPLICATED DIRECTIVE*/
		/*IF THERE ARE IDENCAL DIRECTIVES, JUST THE FIRST WILL BE CONSIDERED*/
		std::string	duplicate = "location " + request_path + " ";
		while (_server_conf_file.find(duplicate, 0) != _server_conf_file.npos)
			location = str_substring(_server_conf_file, duplicate, 0, '}');
	}
		/*PRINT*/
		// std::map<std::string, directive>::iterator	it;
		// it = _locations.begin();
		// std::cout << "Print Locations:\n";
		// for (; it != _locations.end(); it++)
		// {
		// 	std::cout << "location: " << (*it).first << "\n";
		// 	std::cout << "	server_path: " << (*it).second._server_path << std::endl;
		// 	std::cout << "	autoindex: " << (*it).second._autoindex << std::endl;
		// 	for (std::vector<std::string>::iterator it2 = (*it).second._index_block.begin(); it2 != (*it).second._index_block.end(); it2++)
		// 		std::cout << "	indexblock: " << *it2 << std::endl;
		// }
}

void	FileParser::select_server(std::string file, std::string serv_name)
{
	size_t		start, end, i;
	std::string	tmp_server, tmp1;

	i = 0;
	while (1)
	{
		start = file.find("server", i);
		end = file.find("{", start);
		end++;
		int flag = 1;
		while (flag != 0)
		{
			if (file.at(end) == '{')
			{
				flag++;
				// std::cout << flag << "\n";
			}
			else if (file.at(end) == '}')
			{
				flag--;
				// std::cout << flag << "\n";
			}
			end++;
			// std::cout << file.at(end);
		}
		tmp_server = file.substr(start, (end - start));
		start = tmp_server.find("server_name", 0);
		i = tmp_server.find("\r\n", start);
		tmp1 = tmp_server.substr(start, (i - start));
		if (tmp1.find(serv_name) != tmp1.npos)
			break ;
		else
			i = end;
	}
	_server_conf_file = tmp_server;
}

void	FileParser::parse_server(char *file, std::string server_name)
{
	std::string	configuration_file;

	file_to_string(file, configuration_file);
	
	/*ERASE COMMENTS*/
	while (_server_conf_file.find("#", 0) != _server_conf_file.npos)
		str_substring(_server_conf_file, "#", 0, '\n');
	
	/*PARSE EACH SERVER FROM CONFIGURATION FILE TO A STRING*/
	select_server(configuration_file, server_name);

}
