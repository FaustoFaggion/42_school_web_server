#include "FileParser.hpp"

FileParser::FileParser()
{
}


FileParser::~FileParser()
{
}

int		FileParser::parse_path(std::string &str, std::string find, std::string &path, int flag)
{
	std::string	simple_directive;
	int			value_count;
	
	simple_directive = str_substring(str, find, 0, '\n');
	
	/*0 FOR SIMPLE DIRECTIVE; 1 FOR BLOCK DIRECTIVE*/
	if (flag == SIMPLE_DIRECTIVE)
	{
		chk_simple_directive(simple_directive);
		path = get_simple_directive_value(simple_directive, value_count);
	}
	else
		path = get_block_directive_value(simple_directive, value_count);

	if (value_count > 1)
	{
		throw ServerExceptions("ERROR: more than one value in simple directive - " + path);
	}
	return (0);
}

void FileParser::count_brackets(size_t bk_close, size_t bk_open, size_t it, size_t size, std::string &servers_config_file)
{
	while (it > size)
	{
		if (servers_config_file.at(it) == '}')
			bk_open--;	
		if (servers_config_file.at(it) == '{')
			bk_open++;
		if (bk_open == 1)
			throw ServerExceptions("ERROR: there is an irregular open bracket in server block");
		it--;
	}
	if (bk_close - bk_open != 0)
		throw ServerExceptions("ERROR: there is an irregular close bracket in server block");
}

void FileParser::atoi_bytes(std::string buff, size_t &param_size, std::string error_msg)
{
	size_t size, pos;
	std::string digits;

	if ((pos = buff.find_last_of("B")) != buff.npos)
	{
		for (std::string::iterator it = buff.begin(); it != buff.end() - 1; it++)
		{
			if (!isdigit(*it))
				throw ServerExceptions("ERROR: " + error_msg);
			digits+= (*it);		
		}	
		size = atoi(digits.c_str());
		param_size = size;
	}
	else if ((pos = buff.find_last_of("K")) != buff.npos)
	{
		for (std::string::iterator it = buff.begin(); it != buff.end() - 1; it++)
		{
			if (!isdigit(*it))
				throw ServerExceptions("ERROR: " + error_msg);
			digits+= (*it);		
		}	
		size = atoi(digits.c_str());
		param_size = size * 1024;
	}
	else if ((pos = buff.find_last_of("M")) != buff.npos)
	{
		for (std::string::iterator it = buff.begin(); it != buff.end() - 1; it++){
			if (!isdigit(*it))
				throw ServerExceptions("ERROR: " + error_msg);
			digits+= (*it);
		}
		size = atoi(digits.c_str());
		param_size = size * 1024 * 1024;
	}
	else if ((pos = buff.find_last_of("G")) != buff.npos)
	{
		for (std::string::iterator it = buff.begin(); it != buff.end() - 1; it++){
			if (!isdigit(*it))
				throw ServerExceptions("ERROR: " + error_msg);
			digits+= (*it);
		}
		size = atoi(digits.c_str());
		param_size = size * 1024 * 1024 * 1024;
	}
	else
	{
		for (std::string::iterator it = buff.begin(); it != buff.end(); it++){
			if (!isdigit(*it))
				throw ServerExceptions("ERROR: " + error_msg);
			digits+= (*it);
		}
		if (!digits.empty())
		{
			size = atoi(digits.c_str());
			param_size = size;
		}
		else
			throw ServerExceptions("ERROR: " + error_msg);
	}
}

std::string	FileParser::str_substring(std::string &str, std::string find, int init, char finish)
{
	std::string	response = "";
	if (str.find(find, init) != str.npos)
	{
		size_t start = str.find(find, init);
		size_t end = str.find(finish, start);
		
		response = str.substr(start, (end - start));
		str.erase(start, (end- start));
	}
	return (response);
}

int		FileParser::chk_simple_directive(std::string &str)
{

	size_t end = str.find(";", 0);
	if (end == str.npos)
		throw ServerExceptions("ERROR: missing ';'");
	str = str.substr(0, end);
	return (0);
}

std::string	FileParser::get_simple_directive_value(std::string &str, int &value_count)
{
	std::string	response;
	
	std::istringstream iss(str);
	std::string tmp1;
	iss >> tmp1;
	
	value_count = 0;
	while(iss >> response)
		value_count++;

	return (response);
}

std::string	FileParser::get_block_directive_value(std::string &str, int &value_count)
{
	std::string	response;
	
	std::istringstream iss(str);
	std::string tmp1, tmp2;
	iss >> tmp1 >> response;
	
	value_count = 0;
	while(iss >> tmp2)
		value_count++;
	return (response);
}

void		FileParser::parse_file_to_string(char *file, std::string &response)
{
	std::fstream	open_file;
	std::string		buff;

	open_file.open(file,  std::fstream::in);
	if (open_file.fail())
		throw ServerExceptions("Configuration file fail to read");
	while (!open_file.eof())
	{
		std::getline(open_file, buff,'\n');
		response += buff;
		response += "\n";
	}

	/*ERASE COMMENTS*/
	while (response.find("#", 0) != response.npos)
		str_substring(response, "#", 0, '\n');
}

void	FileParser::parse_events_from_config_file(std::string &servers_config_file, std::string &events)
{
	size_t		start, end, first_bracket;

	if ((start = servers_config_file.find("events")) == servers_config_file.npos)
		throw ServerExceptions("ERROR: Please define events directive into configuration file!");
	first_bracket = servers_config_file.find_first_of("{");
	end = servers_config_file.find_first_of("}", first_bracket);
	if (end < first_bracket || end > servers_config_file.find("{", first_bracket + 1))
		throw ServerExceptions("ERROR: bracket not opening or closing events block");
	events = servers_config_file.substr(start, (end + 1 - start));
	servers_config_file.erase(start, ((end + 1) - start));
}

void	FileParser::parse_worker_connections(std::string &events, int &worker_connections)
{

	if (events.find("worker_connections", 0) == events.npos)
		throw ServerExceptions("ERROR: no worker connections found");
	std::string	wp = str_substring(events, "worker_connections", 0, '\n');
	chk_simple_directive(wp);
	setup_worker_connections(wp, worker_connections);

	/*SHOULD NOT HAVE MORE DIRECTIVES*/
	size_t	start, end;
	start = events.find_first_of("{");
	start++;
	end = events.find_last_of("}");
	for (size_t i = start; i < end; i++) {
        if (events[i] != ' ' && events[i] != '\n' && events[i] != '\r' && events[i] != '\t')
			throw ServerExceptions("ERROR: events directive not valid!");
    }

}

void	FileParser::setup_worker_connections(std::string buff, int &worker_connections)
{
	if (buff.compare(0, 18, "worker_connections") == 0)
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
		if ((worker_connections = atoi(tmp.c_str())) < 1)
			throw ServerExceptions("ERROR: worker connection is empty or 0");
	}
}

void	FileParser::parse_work_processes(std::string &servers_config_file, int &max_worker_connections)
{
	std::string	wp;

	if (servers_config_file.find("worker_connections", 0) != servers_config_file.npos)
	{
		wp = str_substring(servers_config_file, "worker_connections", 0, '\n');
		chk_simple_directive(wp);
	
		if (wp.compare(0, 18, "worker_connections") == 0)
		{
			std::string	tmp;
		
			size_t i = 19;
			while (i < wp.size() && isspace(wp.at(i)))
				i++;
			while (i < wp.size() && isdigit(wp.at(i)))
			{
				tmp += wp.at(i);
				i++;
			}
			max_worker_connections = atoi(tmp.c_str());
		}
	}
}

void	FileParser::parse_server_from_config_file(std::string &servers_config_file, std::vector<std::string> &servers)
{
	size_t		start, end, first_bracket;

	while (1)
	{
		if ((start = servers_config_file.find("server ", 0)) == servers_config_file.npos)
			break ;
		first_bracket = servers_config_file.find_first_of("{", start);
		std::string tmp = servers_config_file.substr(start, ((first_bracket + 1) - start));
		if (tmp.compare("server {") != 0)
			throw ServerExceptions("ERROR: there is no bracket opening server block");
		end = servers_config_file.find("server {", first_bracket);
		end = servers_config_file.rfind('}', end);
		count_brackets(0, 1, end, first_bracket, servers_config_file);
		servers.push_back(servers_config_file.substr(start, ((end + 1) - start)));
		servers_config_file.erase(start, ((end + 1) - start));
	}
	if (servers.size() == 0)
		throw ServerExceptions("ERROR: there is no server block in file");
}

void FileParser::parseHead(char *file, std::string &_servers_config_file, std::string &_events, int &_max_worker_connections, std::vector<std::string> &_servers)
{
	parse_file_to_string(file, _servers_config_file);
	parse_events_from_config_file(_servers_config_file, _events);
	parse_worker_connections(_events, _max_worker_connections);	
	// parse_work_processes(_servers_config_file, _max_worker_connections);
	parse_server_from_config_file(_servers_config_file, _servers);
}

//parse_serverBlock_methods
void FileParser::parse_listening(std::string &server_conf_file)
{
	std::string tmp_ltn;
	bool num = false;

	while (server_conf_file.find("listen", 0) != server_conf_file.npos)
	{
		num = false;
		tmp_ltn = str_substring(server_conf_file, "listen", 0, '\n');
		chk_simple_directive(tmp_ltn);
		for (size_t i = 0; i < tmp_ltn.length(); i++)
		{
			if (isdigit(tmp_ltn.at(i)))
				num = true;
		}
		if (num == false)
			throw ServerExceptions("ERROR: host not found in default_server of the listen directive");
		else if (tmp_ltn.find("default_server") == tmp_ltn.npos)
			throw ServerExceptions("ERROR: default_server param not found");
		listen.push_back(tmp_ltn);
	}
	if (num == false)
		throw ServerExceptions("ERROR: host not found in default_server of the listen directive");
}

void	FileParser::parse_server_name_flag(std::string &server_conf_file)
{
	if (server_conf_file.find("server_name", 0) != server_conf_file.npos)
	{
		std::string tmp1 = str_substring(server_conf_file, "server_name", 0, '\n');
		chk_simple_directive(tmp1);
	
		std::istringstream iss(tmp1);
		iss >> serverName;
	
		int value_count = 0;
		while(iss >> serverName)
			value_count++;
	} 
	else if (server_conf_file.find("localhost", 0) != server_conf_file.npos)
		throw ServerExceptions("ERROR: server_name directive not found");
}

void		FileParser::parse_simple_root_directive(std::string &server_config_file)
{
	std::string	root_path;
	size_t 		end;
	size_t 		start;

	/*IS THERE A ROOT SIMPLE DIRECTIVE?*/
	end = server_config_file.find("location ", 0);
	start = server_config_file.find("root", 0);
	/*NO*/
	if (start > end)
	{
		locations["/"]._server_path = "./server_root/webserver.html";
		locations.at("/")._autoindex = false;
		return ;
	}
	/*YES*/
	else
	{
		parse_path(server_config_file, "root", root_path, SIMPLE_DIRECTIVE);
		locations["/"]._server_path = root_path;
		locations.at("/")._autoindex = false;

		/*CHECK DUPLICATED DIRECTIVE*/
		/*IF THERE ARE IDENCAL DIRECTIVES, JUST THE FIRST WILL BE CONSIDERED*/
		while (1)
		{
			end = server_config_file.find("location ", 0);
			start = server_config_file.find("root", 0);
			if (start >= end)
				break ;
			else
				std::string tmp = str_substring(server_config_file, "root", 0, '\n');
		}
	}
	/*IF DIRECTORY DO NOT EXIST EXIT PROGRAM*/
	if (access(root_path.c_str(), F_OK) != 0)
		throw ServerExceptions("ERROR: Path " + root_path + "does not exist");
}

void	FileParser::parse_buffer_size(std::string &servers_config_file)
{
	std::string buff;

	if (servers_config_file.find("proxy_buffer_size", 0) != servers_config_file.npos)
		parse_path(servers_config_file, "proxy_buffer_size", buff, SIMPLE_DIRECTIVE);
	atoi_bytes(buff, bufferSize, "proxy_file_buffer");

}

void		FileParser::parse_body_size(std::string &servers_config_file)
{
	std::string	buff;

	if (servers_config_file.find("client_max_body_size", 0) != servers_config_file.npos)
	{
		parse_path(servers_config_file, "client_max_body_size", buff, SIMPLE_DIRECTIVE);
	}
	atoi_bytes(buff, bodySize, "max_body_size");
}

void		FileParser::parse_error_page(std::string &server_config_file)
{
	/*IS THERE A ROOT SIMPLE DIRECTIVE?*/
	while(1)
	{
		std::string	error_code = "";
		std::string	error_path = "";
		std::string	str = "";
		std::string tmp = "";

		size_t end = server_config_file.find("location ", 0);
		size_t start = server_config_file.find("error_page", 0);
		if (start >= end)
			break ;
		tmp = str_substring(server_config_file, "error_page", 0, '\n');
		chk_simple_directive(tmp);
		errorPath.push_back(tmp);
	}
}

void		FileParser::parse_index_simple_directive(std::string &server_config_file)
{
	size_t	start, end;

	/*IS THERE A ROOT SIMPLE DIRECTIVE?*/
	end = server_config_file.find("location ", 0);
	start = server_config_file.find("index", 0);
	/*NO*/
	if (start > end)
		return ;
	/*YES*/
	else
	{
		end = server_config_file.find("\n", start);
		std::string index_tmp = server_config_file.substr(start, (end - start));
		chk_simple_directive(index_tmp);

		std::stringstream iss(index_tmp);
		std::string	word;

		iss >> word;
		while (iss >> word)
			simpleIndexes.push_back(word);
	
	}
	/*CHECK DUPLICATED DIRECTIVE*/
	/*IF THERE ARE IDENCAL DIRECTIVES, JUST THE FIRST WILL BE CONSIDERED*/
	while (1)
	{
		size_t end = server_config_file.find("location ", 0);
		size_t start = server_config_file.find("index", 0);
		if (start >= end)
			break ;
		else{
			std::string tmp = str_substring(server_config_file, "index", 0, '\n');
		}
	}
}

void FileParser::parse_location_block(std::string &servers_config_file, std::string &server_path, std::string &location, std::string &root_path, std::string	&request_path)
{
	// location = str_substring(servers_config_file, "location ", 0, '}');
	
		
	if (servers_config_file.find("location ", 0) != servers_config_file.npos)
	{
		size_t start = servers_config_file.find("location ", 0);
		size_t end = servers_config_file.find('}', start);
		
		location = servers_config_file.substr(start, (end - start));
		servers_config_file.erase(start, ((end + 1) - start));
	}
	
	parse_path(location, "location ", request_path, BLOCK_DIRECTIVE);
	/*ROOT INSIDE LOCATION BLOCK TAKES PRECEDENCE OVER SIMPLE ROOT DIRECTIVE*/
	if (location.find("root", 0) != location.npos)
	{
		parse_path(location, "root", root_path, SIMPLE_DIRECTIVE);
		server_path = root_path + request_path;
	}
	else
		server_path = locations.at("/")._server_path + request_path;
}

void	FileParser::parse_request_absolute_path_in_location_block(std::string &server_path, std::string &root_path, std::string &request_absolute_path)
{
	/*REWRITE THE PATH CORRECTILY IF '//' IS FIND*/
	size_t pos = server_path.find("//");
	if (pos != server_path.npos)
		server_path.replace(pos, 1, "");

	/*REWRITE REQUEST_PATH DEPENDS ON ROOT_BLOCK DIRECTIVE*/
	if (root_path.compare("") != 0)
	{
		if (root_path.find("/", 2) != root_path.npos)
		{
			size_t		pos = root_path.find("/", 2);
			std::string	tmp = root_path.substr(pos, (root_path.size() - pos));
			if (tmp.compare(request_absolute_path) != 0)
				request_absolute_path = tmp + request_absolute_path;
		}
	}
}

void FileParser::parse_autoindex_in_location_block(std::string &request_path, std::string &location)
{
	locations.at(request_path)._autoindex = false;
	if (location.find("autoindex", 0) != location.npos)
	{
		std::string auto_idx;
		
		parse_path(location, "autoindex", auto_idx, SIMPLE_DIRECTIVE);

		if (auto_idx.compare("on") == 0)
			locations.at(request_path)._autoindex = true;
		else if (auto_idx.compare("off") == 0)
			locations.at(request_path)._autoindex = false;
		else
			throw ServerExceptions("ERROR: invalid autoindex");
	}
}

void	FileParser::parse_index_in_location_block(std::string &location, std::string &request_path)
{
	/*CHECK INDEX DIRECTIVE*/
	if (location.find("index", 0) != location.npos)
	{
		std::string	idx;
		idx = str_substring(location, "index", 0, '\n');
		chk_simple_directive(idx);
		parse_index_block_directive(locations.at(request_path)._index_block, idx);
	}
}

void	FileParser::parse_index_block_directive(std::vector<std::string> &idx, std::string &str)
{
	std::istringstream iss(str);
	std::string index;

	iss >> index;
	while (iss >> index) {
		idx.push_back(index);
	}

}

void FileParser::parse_allowed_methods_in_location_block(std::string &location, std::string &request_path)
{
	/*CHECK ALLOWED_METHODS*/
	if (location.find("allowed_methods ", 0) != location.npos)
	{
		std::string	methods;
		methods = str_substring(location, "allowed_methods ", 0, '\n');
		chk_simple_directive(methods);
		parse_allowed_methods(methods, locations.at(request_path)._allowed_methods);
	}
}	

void FileParser::parse_redirects_in_location_block(std::string &location, std::string &request_path)
{
	/*CHECK REDIRECT*/
	locations.at(request_path)._redirect_url = "";
	locations.at(request_path)._redirect_code = "";
	if (location.find("return ", 0) != location.npos)
	{
		std::string	rtrn;
		rtrn = str_substring(location, "return ", 0, '\n');
		chk_simple_directive(rtrn);
		std::istringstream iss(rtrn);
		std::string tmp;
		iss >> tmp >> locations.at(request_path)._redirect_code >> locations.at(request_path)._redirect_url;
	}
}

void FileParser::parse_cgi_pass_in_location_block(std::string &location, std::string &request_path)
{
	/*CHECK CGI_PASS*/
	locations.at(request_path)._cgi_pass = "";
	if (location.find("cgi_pass", 0) != location.npos)
	{
		std::string	cgi;
		parse_path(location, "cgi_pass", cgi, SIMPLE_DIRECTIVE);
		locations.at(request_path)._cgi_pass = cgi;
	}
}

void		FileParser::parse_locations(std::string &servers_config_file)
{
	while (servers_config_file.find("location ", 0) != servers_config_file.npos)
	{
		std::string	root_path = "";
		std::string	location = "";
		std::string	request_absolute_path = "";
		std::string	request_relative_path = "";
		std::string	server_path = "";

		parse_location_block(servers_config_file, server_path, location, root_path, request_relative_path);
	
		/*REMOVE '/' IN LAST STRING POSITION REQUEST_RELATIVE_PATH*/
		if (request_relative_path.size() > 1 && request_relative_path.at(request_relative_path.size() -1) == '/')
		{
			request_relative_path.erase(request_relative_path.size() -1);
		}
		/*REMOVE '/' IN LAST STRING POSITION SERVER_PATH*/
		if (server_path.at(server_path.size() -1) == '/')
		{
			server_path.erase(server_path.size() -1);
		}

		request_absolute_path = request_relative_path;
	
		parse_request_absolute_path_in_location_block(server_path, root_path, request_absolute_path);
		/*CREATE AND INSERT LOCATION PATH INTO THE _PATH MAP*/
		if (locations.find(request_relative_path) == locations.end())
			locations[request_relative_path]._server_path = server_path;

		parse_autoindex_in_location_block(request_relative_path, location);
		parse_index_in_location_block(location, request_relative_path);
		parse_allowed_methods_in_location_block(location, request_relative_path);
		parse_redirects_in_location_block(location, request_relative_path);
		parse_cgi_pass_in_location_block(location, request_relative_path);

		if (request_absolute_path.compare(request_relative_path) != 0)
		{
			locations[request_absolute_path]._server_path = server_path;
			locations.at(request_absolute_path)._allowed_methods = locations.at(request_relative_path)._allowed_methods;
			locations.at(request_absolute_path)._autoindex = locations.at(request_relative_path)._autoindex;
			locations.at(request_absolute_path)._cgi_pass = locations.at(request_relative_path)._cgi_pass;
			locations.at(request_absolute_path)._index_block = locations.at(request_relative_path)._index_block;
			locations.at(request_absolute_path)._max_body_size = locations.at(request_relative_path)._max_body_size;
			locations.at(request_absolute_path)._path_ok = locations.at(request_relative_path)._path_ok;
			locations.at(request_absolute_path)._redirect_code = locations.at(request_relative_path)._redirect_code;
			locations.at(request_absolute_path)._redirect_url = locations.at(request_relative_path)._redirect_url;
			locations.at(request_absolute_path)._server_path = locations.at(request_relative_path)._server_path;
		}
		/*CHECK DUPLICATED DIRECTIVE*/
		/*IF THERE ARE IDENCAL DIRECTIVES, JUST THE FIRST WILL BE CONSIDERED*/
		std::string	duplicate = "location " + request_relative_path + " ";
		while (servers_config_file.find(duplicate, 0) != servers_config_file.npos)
			location = str_substring(servers_config_file, duplicate, 0, '}');
	}
}


void	FileParser::parse_allowed_methods(std::string &directive, std::vector<std::string> &allowed_methods)
{
	std::istringstream iss(directive);
	std::string method;

	iss >> method;
	while (iss >> method) {
		allowed_methods.push_back(method);
	}
}

FileParser	FileParser::parseServersBlock(std::string &server)
{
	FileParser file;

	file.parse_listening(server);
	file.parse_server_name_flag(server);
	file.parse_simple_root_directive(server);
	file.parse_buffer_size(server);
	file.parse_body_size(server);
	file.parse_error_page(server);
	file.parse_index_simple_directive(server);
	file.parse_locations(server);

	/*SHOULD NOT HAVE MORE DIRECTIVES*/
	size_t	start, end;
	start = server.find_first_of("{");
	start++;
	end = server.find_last_of("}");
	for (size_t i = start; i < end; i++) {
        if (server[i] != ' ' && server[i] != '\n' && server[i] != '\r' && server[i] != '\t')
			throw ServerExceptions("ERROR: server directive not valid!");
    }
	return (file);
}


//GETTERS
std::vector<std::string> FileParser::getListen() const
{
	return (listen);
}

std::string	FileParser::getServerName() const
{
	return (serverName);
}

std::map<std::string, directive> FileParser::getLocations() const
{
	return (locations);
}

size_t FileParser::getBufferSize() const
{
	return (bufferSize);
}

size_t FileParser::getBodySize() const
{
	return (bodySize);
}

std::vector<std::string> FileParser::getErrorPath() const
{
	return (errorPath);
}

std::vector<std::string> FileParser::getSimpleIndexes() const
{
	return (simpleIndexes);
}
