#include "HttpRequest.hpp"

HttpRequest::HttpRequest()
{

}

HttpRequest::~HttpRequest()
{

}

std::string		HttpRequest::getMethod() const
{
	return (_request_method);
}

std::string		HttpRequest::getUrl() const
{
	return (_url);
}

std::string		HttpRequest::getProtocol() const
{
	return(_server_protocol);
}

std::string		HttpRequest::getContentType() const
{
	return (_content_type);
}

std::vector<std::string>	HttpRequest::getCgiEnvs() const
{
	return (_cgi_envs);
}

std::string	parse_line(std::string &request, std::string start, std::string end)
{
	size_t		requestLineEnd;
	size_t		requestLineStart;
	std::string requestLine;
	
	// Find the start and end of the request line
	requestLineStart = request.find(start);
	if (requestLineStart != request.npos)
	{
		requestLineEnd = request.find(end, requestLineStart);
		requestLine = request.substr(requestLineStart,(requestLineEnd - requestLineStart));
	}
	else
		requestLine = "";

	return (requestLine);
}

void	HttpRequest::add_cgi_envs(std::string request, std::string str, std::string env, size_t size, char c)
{
	std::string	requestLine;
	std::string	value;

	requestLine = parse_line(request, str, "\r\n");
	if (requestLine != "")
	{
		std::istringstream iss(requestLine);
		getline(iss, value, c);
		getline(iss, value, c);
		_cgi_envs.push_back(requestLine.replace(0, size, env));
	}
	else
	{
		value += env + "NULL";
		_cgi_envs.push_back(value);
	}
}
void	HttpRequest::cgi_envs_parser(std::string request)
{
	std::cout << "\nCGI_ENVS_PARSER FUNCTION\n\n";

	std::string		requestLine;
	std::string		key;
	std::string		value;

	std::cout << request << "\n";
	std::cout << "enviroment variables\n" << "\n";
	
	/*SET REDIRECT_STATUS ENV*/
	// setenv("REDIRECT_STATUS", "true", 1);
	_cgi_envs.push_back("REDIRECT_STATUS=TRUE");
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/* SSL HAS TO BE CHECKED INTO THE FILE PARSER*/
	// _cgi_envs.push_back("AUTH_TYPE=NULL");
	// setenv("AUTY_TYPE", "NULL", 1);
	_cgi_envs.push_back("AUTY_TYPE=NULL");
	std::cout << *(_cgi_envs.end() - 1) << "\n";
	
	/*CONTENT_LENGTH*/
	key = "Content-Length: ";
	add_cgi_envs(request, key, "CONTENT_LENGTH=", key.size(), ' ');
	std::cout << *(_cgi_envs.end() - 1) << "\n";
	
	/*CONTENT_TYPE*/
	key = "Content-Type: ";
	add_cgi_envs(request, key, "CONTENT_TYPE=", key.size(), ' ');
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*GATWAY_INTERFACE*/ //?????????????????????????????????
	_cgi_envs.push_back("GATWAY_INTERFACE=CGI/7.4");
		
	/*HTTP_ACCEPT*/
	key = "Accept: ";
	add_cgi_envs(request, key, "HTTP_ACCEPT=", key.size(), ' ');
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*HTTP_ACCEPT_ENCODING*/
	key = "Accept-Encoding: ";
	add_cgi_envs(request, key, "HTTP_ACCEPT_ENCODING=", key.size(), ' ');
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*HTTP_ACCEPT_LANGUAGE*/
	key = "Accept-Language: ";
	add_cgi_envs(request, key, "HTTP_ACCEPT_LANGUAGE=", key.size(), ' ');
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*HTTP_HOST, SERVER_NAME, SERVER_PORT*/
	requestLine = parse_line(request, "Host: ", "\r\n");
	std::cout << "requestline: " << requestLine << "\n";
	if (requestLine != "")
	{
		std::string		tmp0, tmp1;
		std::istringstream	iss0(requestLine);
		getline(iss0, tmp0, ' ');
		getline(iss0, tmp0, ' ');
		value = "HTTP_HOST=" + tmp0;
		_cgi_envs.push_back(value);
		std::cout << *(_cgi_envs.end() - 1) << "\n";
		std::istringstream	iss1(tmp0);
		getline(iss1, tmp1, ':');
		value = "SERVER_NAME=" + tmp1;
		// _cgi_envs.push_back(value);
		std::cout << *(_cgi_envs.end() - 1) << "\n";
		getline(iss1, tmp1, ':');
		value = "SERVER_PORT=" + tmp1;
		_cgi_envs.push_back(value);
		std::cout << *(_cgi_envs.end() - 1) << "\n";
	}
	else
	{
		std::cout << "requestline == vazio\n";
		_cgi_envs.push_back("HTTP_HOST=NULL");
		std::cout << *(_cgi_envs.end() - 1) << "\n";
		_cgi_envs.push_back("SERVER_NAME=NULL");
		std::cout << *(_cgi_envs.end() - 1) << "\n";
		_cgi_envs.push_back("SERVER_PORT=NULL");
		std::cout << *(_cgi_envs.end() - 1) << "\n";
	}

	/*HTTP_USER_AGENT*/
	key = "User-Agent: ";
	add_cgi_envs(request, key, "HTTP_USER_AGENT=", key.size(), ' ');
	std::cout << *(_cgi_envs.end() - 1) << "\n";

	/*PATH_INFO, QUERY_STRING*/
	size_t	end = request.find("\r\n");
	requestLine = request.substr(0, end);
	{
		std::string	method, url, protocol, tmp0, tmp1, tmp2;
		std::istringstream iss0(requestLine);

		iss0 >> method >> url >> protocol;
		size_t pos = _url.find("?");
		if (pos != _url.npos)
		{
			
			std::istringstream iss0(_url);
			getline(iss0, _url, '?');
			size_t start =_url.find("/");
			value = _url.substr(start, (_url.size() - start));
//			key = "PATH_INFO=" + value;
			_cgi_envs.push_back(key);
			std::cout << *(_cgi_envs.end() - 1) << "\n";
			getline(iss0, value, '?');
//			key = "QUERY_STRING=" + value;
			_cgi_envs.push_back(key);
			std::cout << *(_cgi_envs.end() - 1) << "\n";
		}
		else
		{
			size_t start =_url.find("/");
			value = _url.substr(start, (_url.size() - start));
			key = "PATH_INFO=" + value;
//			_cgi_envs.push_back(key);
			std::cout << *(_cgi_envs.end() - 1) << "\n";
		}
		// key = "REQUEST_METHOD";
		// // setenv(key.c_str(), method.c_str(), 1);
		// key = "SERVER_PROTOCOL";
		// setenv(key.c_str(), protocol.c_str(), 1); // HTTP or HTTPS
	}

	// requestLine = parse_line(request, "Origin: ", "\r\n");
	// if (requestLine != "")
	// {
	// 	key = "REMOTE_HOST";
	// 	std::istringstream iss(requestLine);
	// 	getline(iss, value, ' ');
	// 	getline(iss, value, ' ');
	// 	setenv(key.c_str(), value.c_str(), 1);
	// }
	// else
	// 	setenv(key.c_str(), "NULL", 1);

	// /*ERROR: No input file specified*/
	// setenv("SCRIPT_NAME", "./php-cgi/test2.php", 1);
	// setenv("SCRIPT_FILENAME", "/test2.php", 1);

	// setenv("REMOTE_ADDR", "127.0.0.1", 1);
	
	
	/*Declaration of the environment variable array*/
	// extern char** environ; 
    /*Iterate over the environment variables until a null pointer is encountered*/
    // for (int i = 0; environ[i] != NULL; i++) {
    //     std::cout << environ[i] << std::endl;
	// 	_cgi_envs.push_back(environ[i]);
    // }
}

void		HttpRequest::request_parser(std::string request)
{

	std::cout << "\nREQUEST_PARSER FUNCTION\n";

	std::string		tmp;
	std::string		requestLine;

	/*METHOD, URL, PROTOCOL*/
	size_t requestLineEnd = request.find("\r\n");
	requestLine = request.substr(0, requestLineEnd);
	std::istringstream iss0(requestLine);
	iss0 >> _request_method >> _url >> _server_protocol;

	size_t pos = _url.find("?");
	if (pos != _url.npos)
	{
		std::istringstream iss0(_url);
		getline(iss0, _url, '?');
	}

	/*CONTENT_TYPE*/
	pos = request.find("Content-Type: ");
	if (pos != request.npos)
	{
		requestLine = parse_line(request, "Content-Type: ", "\r\n");
		std::istringstream iss0(requestLine);
		getline(iss0, tmp, ' ');
		getline(iss0, tmp, ' ');
		std::istringstream iss1(tmp);
		getline(iss1, _content_type, ';');
		_content_type += "\r\n";
	}

	cgi_envs_parser(request);
}