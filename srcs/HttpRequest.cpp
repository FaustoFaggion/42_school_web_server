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

std::string		HttpRequest::getServer_name() const
{
	return(_server_name);
}

std::string	parse_line(std::string &request, std::string start)
{
	size_t		requestLineEnd;
	size_t		requestLineStart;
	std::string requestLine;
	
	// Find the start and end of the request line
	requestLineStart = request.find(start);
	requestLineEnd = request.find("\r\n");
	// Extract the request line
	requestLine = request.substr(requestLineStart,(requestLineEnd - requestLineStart));
	requestLineEnd += 2;
	std::cout << "request_line function: " << requestLine << "\n\n\n";
	/*CLEAN THE EXTRACT LINE FROM REQUEST*/
	request.erase(requestLineStart, requestLineEnd);
	
	return (requestLine);
}

void		HttpRequest::request_parser(std::string request)
{

	std::cout << "REQUEST_PARSER FUNCTION\n";

	std::string		tmp;
	std::string		requestLine;
	
	/*METHOD, URL, PROTOCOL*/
	requestLine = parse_line(request, "GET");
	{
		std::istringstream iss0(requestLine);
		iss0 >> _request_method >> _url >> _server_protocol;
	}

	/*_URL, _PATH_INFO, _QUERY_STRING*/
	size_t pos = _url.find("?");
	if (pos != _url.npos)
	{
		std::istringstream iss0(_url);
		getline(iss0, _url, '?');
		getline(iss0, tmp, '?');
		std::istringstream iss1(tmp);
		getline(iss1, _path_info, '=');
		getline(iss1, _query_string, '=');
	}

	/*_SERVER_NAME, _SERVER_PORT*/
	requestLine = parse_line(request, "Host:");
	{
		std::istringstream iss0(requestLine);
		getline(iss0, tmp, ' ');
		getline(iss0, tmp, ' ');
		std::istringstream iss1(tmp);
		getline(iss1, _server_name, ':');
		getline(iss1, _server_port, ':');
	}

	std::cout << "_url:  " << _url << "\n";
	std::cout << "_server_name:  " << _server_name << "\n";
	std::cout << "_server_port:  " << _server_port << "\n";
	std::cout << "_path_info: " << _path_info << "\n";
	std::cout << "_query_string: " << _query_string << "\n";
}