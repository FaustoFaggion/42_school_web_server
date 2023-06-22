#include "HttpRequest.hpp"

HttpRequest::HttpRequest()
{

}

HttpRequest::~HttpRequest()
{

}

std::string		HttpRequest::getMethod() const
{
	return (_method);
}

std::string		HttpRequest::getUrl() const
{
	return (_url);
}

std::string		HttpRequest::getProtocol() const
{
	return(_protocol);
}

void		HttpRequest::request_parser(std::string request)
{
	// Find the end of the request line
   	size_t requestLineEnd = request.find("\r\n");
    // Extract the request line
    std::string requestLine = request.substr(0, requestLineEnd);
	request.erase(0, requestLineEnd);
    // Parse the request line
    std::istringstream iss(requestLine);
    iss >> _method >> _url >> _protocol;

	//PARSE IF POST REQUEST UNTIL GET PATH BEFORE ? SIGN
	size_t pos = _url.find("?");
	if (pos != _url.npos)
	{
		std::string tmp = _url.substr(0, pos);
		_url = tmp;
	}
}