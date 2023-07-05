#include "HttpResponse.hpp"

HttpResponse::HttpResponse()
{

}


HttpResponse::~HttpResponse()
{

}

void	HttpResponse::chk_indexies(t_client &client, std::string &html, std::map<std::string, directive> &locations, std::vector<std::string> indexes)
{
	std::cout << "\nCHK_INDEXIES FUNCTION\n";

	bool	flag = false;
	size_t	i = 0;
		
	while (flag == false && i < locations[client._url]._index_block.size())
	{
		html = locations[client._url]._server_path + "/" + locations[client._url]._index_block.at(i);
		if(access(html.c_str(), F_OK) == 0)
		{
			flag = true;
			locations[client._url]._path_ok = true;
			std::cout << "1 -path_ok = " << locations[client._url]._path_ok << "\n";
			std::cout << "index found block directive: " << html << "\n";
		}
		i++;
	}
	i = 0;
	if (flag == false)
	{
		while (flag == false && i < indexes.size())
		{
			html = locations[client._url]._server_path + "/" + indexes.at(i);
			if(access(html.c_str(), F_OK) == 0)
			{
				flag = true;
				locations[client._url]._path_ok = true;
				std::cout << "2 -path_ok = " << locations[client._url]._path_ok << "\n";
				std::cout << "index found simple directive: " << html << "\n";
			}
			i++;
		}
	}
	if (flag == false)
	{
		locations[client._url]._path_ok = false;
		html = locations[client._url]._server_path;
		std::cout << "3 -path_ok = " << locations[client._url]._path_ok << "\n";
		std::cout << "index not found: " << html << "\n";
	}

}

std::string	HttpResponse::looking_for_path(t_client &client, std::map<std::string, directive> &locations, std::vector<std::string> indexes)
{
	std::cout << "\nLOOKING_FOR_PATH FUNCTION\n";
	
	std::string	html = "";

	/*IF REQUEST IS A LOCATION, APPEND INDEX.HTML FILES DEFINED INTO CONFIGURATION FILE*/
	if(locations.find(client._url) != locations.end())
	{
		std::cout << "path on location map found: " << client._url << "\n";
		chk_indexies(client, html, locations, indexes);
		if (locations[client._url]._path_ok == true)
			return(html);
	}
	
	/*CHECK FOR FILE IN THE END OF THE PATH REQUEST*/
	if (client._url_file == "")
	{
		html = locations[client._url]._server_path;
		locations[client._url]._path_ok = false;
		std::cout << "path_ok = " << locations[client._url]._path_ok << "\n";
		std::cout << "path on location map not found: " << html << "\n";
		return (html);
	}

	if(locations.find(client._url_location) != locations.end())
	{
		if (client._url_file_extension == ".php")
			html = "php-cgi" + client._url_file;
		else
			html = locations[client._url_location]._server_path + client._url_file;
		std::cout << "html: " << html << "\n";
		client._url = client._url_location;
		if (access(html.c_str(), F_OK) == 0)
		{
			locations[client._url_location]._path_ok = true;
			std::cout << "path_ok = " << locations[client._url_location]._path_ok << "\n";
			std::cout << "find path on location map and file: " << client._url_location <<"\n";
		}
		else
		{
			locations[client._url_location]._path_ok = false;
			std::cout << "path_ok = " << locations[client._url_location]._path_ok << "\n";
			std::cout << "find path on location map but not file: " << client._url_location <<"\n";
		}
	}
	else
	{
		// html = locations[request_path]._server_path;
		locations[client._url_location]._path_ok = false;
		std::cout << "path_ok = " << locations[client._url_location]._path_ok << "\n";
		std::cout << "path not found on location map after extract file: " << client._url_location << "\n";
	}

	/*REWRITE THE PATH CORRECTILY IF '//' IS FIND*/
		size_t pos = html.find("//");
		if (pos != html.npos)
			html.replace(pos, 1, "");

	return (html);
}

void	HttpResponse::diretory_list(std::stringstream &buff, std::string path, std::string html)
{
	std::cout << "DIRETORY_LIST FUNCTION: " << "\n";
	
	buff << "<html>\n";
	buff << "<body>\n";
	buff << "<h1>Directory Listing</h1>\n";
	buff << "<ul>\n";
	DIR* dir;
	struct dirent* entry;
	std::cout << "html: "<< html << "\n";
	dir = opendir(html.c_str());
	if (dir != NULL)
	{
    	while ((entry = readdir(dir)) != NULL)
		{
    		std::string filename = entry->d_name;
    		 std::string link = path + "/" + filename;
    		buff << "<li><a href=\"" << link << "\">" << filename << "</a></li>\n";
    	}
    	closedir(dir);
	}
}

void	HttpResponse::buff_file(std::fstream &conf_file, std::stringstream &buff, std::string html)
{
	conf_file.open(html.c_str() , std::fstream::in);
	if (conf_file.fail())
	{
		conf_file.open("./locations/test/error.html",  std::fstream::in);
		if (conf_file.fail())
			std::cout << "Configuration file fail to read" << std::endl;
		buff << conf_file.rdbuf();
	}
	else
		buff << conf_file.rdbuf();
}

void	HttpResponse::http_response_syntax(std::string status, std::string &request, std::stringstream &buff, std::string content_type)
{
	request = status;
	request += content_type;
	request += "Connection: close\r\n";
	request += "\r\n";
	request += buff.str();
	request += "\r\n";

	std::cout << "\nRESPONSE SYNTAX\n\n";
	std::cout << request;
}

void	HttpResponse::cgi_envs_parser(t_client client, std::string html)
{
	std::cout << "\nCGI_ENVS_PARSER FUNCTION\n\n";

	std::string		env;
	
	/*SET REDIRECT_STATUS ENV*/
	_cgi_envs.push_back("REDIRECT_STATUS=200");
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/* SSL HAS TO BE CHECKED INTO THE FILE PARSER*/
	// _cgi_envs.push_back("AUTH_TYPE=NULL");
	// setenv("AUTY_TYPE", "NULL", 1);
	_cgi_envs.push_back("AUTY_TYPE=NULL");
	std::cout << *(_cgi_envs.end() - 1) << "\n";
	
	/*CONTENT_LENGTH*/
	env = "CONTENT_LENGTH=" + client._content_length;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";
	
	/*CONTENT_TYPE*/
	env = "CONTENT_TYPE=" + client._content_type;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*GATWAY_INTERFACE*/ //?????????????????????????????????
	_cgi_envs.push_back("GATEWAY_INTERFACE=CGI/7.4");
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*HTTP_ACCEPT*/
	env = "HTTP_ACCEPT=" + client._http_accept;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*HTTP_ACCEPT_ENCODING*/
	env = "HTTP_ACCEPT_ENCODING=" + client._http_accept_encoding;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*HTTP_ACCEPT_LANGUAGE*/
	env = "HTTP_ACCEPT_LANGUAGE=" + client._http_accept_language;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*HTTP_HOST*/
	env = "HTTP_HOST=" + client._http_host;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";
	
	/*SERVER_NAME*/
	env = "SERVER_NAME=" + client._server_name;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";
	
	/*SERVER_PORT*/
	env = "SERVER_PORT=" + client._server_port;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";


	/*HTTP_USER_AGENT*/
	env = "HTTP_USER_AGENT=" + client._user_agent;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*QUERY_STRING*/
	env = "QUERY_STRING=" + client._query_string;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*PATH_INFO*/
	env = "PATH_INFO=" + client._path_info;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*REQUEST_URI*/
	env = "REQUEST_URI=" + client._request_uri;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";
	
	/*REQUEST_METHOD*/
	env = "REQUEST_METHOD=" + client._method;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*SERVER_PROTOCOL*/
	env = "SERVER_PROTOCOL=" + client._protocol;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*REMOTE_HOST*/
	env = "REMOTE_HOST=" + client._remote_host;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*SCRIPT_NAME*/
	env = "SCRIPT_NAME=/usr/bin/php-cgi";
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";
	
	/*SCRIPT_FILENAME*/
	env = "SCRIPT_FILENAME=" + html;
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	env = "DOCUMENT_ROOT=/home/fausto/42SP/webserv_git";
	_cgi_envs.push_back(env);
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	_cgi_envs.push_back("REDIRECT_STATUS=true");
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*ADD ENVP TO CGI_ENVS */
	extern char** environ; 
	for (int i = 0; environ[i] != NULL; i++)
		_cgi_envs.push_back(environ[i]);

	/*PRINT CONTENT TO DEBUG*/
	std::cout << "CONTENT\n\n" << client._content << "\n\n";
}

void	HttpResponse::exec_cgi(std::string &html, t_client &client)
{
	std::cout << "\nEXEC_CGI FUNCTION" << html << "\n\n";

	int				fd[2];
	int				pid;
	char			*arg2[3];
	
	arg2[0] = (char *)"/usr/bin/php-cgi7.4";
	arg2[1] = (char *)html.c_str();
	arg2[2] = NULL;

	if (pipe(fd) == -1)
		exit(write(1, "pipe error\n", 11));
	pid = fork();
	if (pid < 0)
		exit(write(1, "fork error\n", 11));
	if (pid == 0)
	{

		cgi_envs_parser(client, html);

		/*CREATE ENVP_CGI ARRAY TO EXECVE*/
		char *envp_cgi[_cgi_envs.size() + 1];
		size_t i = 0;
		while (i < _cgi_envs.size()) {
			envp_cgi[i] = (char *)_cgi_envs.at(i).c_str();
			i++;
		}
		envp_cgi[i] = NULL;


		if (client._method == "POST")
		{


			// Open the file in output mode
			std::ofstream outputFile("_TMP_FILE");
			// Check if the file was opened successfully
			if (!outputFile) {
				std::cerr << "Error opening the file." << std::endl;
				return ;
			}
			// Write the content to the file
			outputFile << client._content;
			// Close the file
			outputFile.close();
		
			int fd1 = open("_TMP_FILE", O_RDONLY);

    		// Check if the file was opened successfully
   			 if (fd1 == -1) {
        		std::cerr << "Error opening the file." << std::endl;
        		return ;
		    }
			dup2(fd1, fd[0]);
			close(fd1);
		}
		dup2(fd[0], STDIN_FILENO);
		close(fd[0]);
		dup2(fd[1], STDOUT_FILENO);
		close(fd[1]);
		if (execve(arg2[0], arg2, envp_cgi) == -1)
		{
			write(2, strerror(errno), strlen(strerror(errno)));
			exit(1);
		}
	}
	waitpid(pid, NULL, 0);
	close(fd[1]);
	dup2(fd[0], STDIN_FILENO);
	client._response = "HTTP/1.1 200 OK\r\n";
	std::cout << "enter while\n";

	std::stringstream phpOutput;
	static char		buffer[1024];
	ssize_t bytesRead;
	while ((bytesRead = read(fd[0], buffer, sizeof(buffer))) != 0)
	{
		phpOutput.write(buffer, bytesRead);
	}
	client._response += phpOutput.str();
	std::cout << "out while\n";
	// std::cout << "request: " << request << "\n";
	close(fd[0]);

}

void	HttpResponse::response_parser(t_client &client, std::map<std::string, directive> locations)
{
	std::cout << "\nRESPONSE_PARSE FUNCTION\n";

	// _indexes = indexes;

	std::fstream			conf_file;
	std::stringstream		buff;

	std::cout << "\nRESPONSE_PARSE FUNCTION\n";

	// client._server_path = looking_for_path(client, locations, indexes);


	std::cout << "\n";
	for (std::map<std::string, directive>::iterator it = locations.begin(); it != locations.end(); it++)
	{
		std::cout << (*it).first << " : " << (*it).second._server_path << "\n";
	}
	std::cout << "\n";
	std::cout << "Method: " << client._method << std::endl;
	std::cout << "Path: " << client._url << std::endl;
	std::cout << "Protocol: " << client._protocol << std::endl;
	std::cout << "client._server_path: " << client._server_path << std::endl;
	std::cout << "\n";

	if (client._method.compare("GET") == 0)
	{
		if (locations[client._url]._autoindex == true)
		{
			std::cout << "autoindex on\n";
			if (locations[client._url]._path_ok == false)
				diretory_list(buff, client._url, client._server_path);
			else
				buff_file(conf_file, buff, client._server_path);
		}
		else
		{
			std::cout << "autoindex off\n";
			buff_file(conf_file, buff, client._server_path);
		}

		if (client._url_file_extension == ".php")
		{
			exec_cgi(client._server_path, client);
			std::cout << "cgi request:\n" << client._request << "\n\n";
		}
		else
			http_response_syntax("HTTP/1.1 200 OK\r\n", client._response, buff, client._content_type);
		conf_file.close();
	}
	else if (client._method.compare("POST") == 0)
	{
		buff_file(conf_file, buff, client._server_path);
		if (client._url_file_extension == ".php")
			exec_cgi(client._server_path, client);
		else
			http_response_syntax("HTTP/1.1 200 OK\r\n", client._response, buff, client._content_type);
		conf_file.close();
	}
	else if (client._method.compare("DELETE") == 0)
	{
		buff_file(conf_file, buff, client._server_path);
		http_response_syntax("HTTP/1.1 200 OK\r\n", client._response, buff, client._content_type);
		conf_file.close();
	}
	else
	{
		buff_file(conf_file, buff, "./locations/test/error.html");
		http_response_syntax("HTTP/1.1 404 Not Found\r\n", client._response, buff, client._content_type);
		conf_file.close();
	}
}