# ifndef SERVEREXCEPTIIONS_HPP
# define SERVEREXCEPTIIONS_HPP

#include <iostream>

class ServerExceptions : public std::exception
{
	private:
		std::string msg;
    public:
		ServerExceptions(const std::string& msg) : msg(msg){}
		virtual ~ServerExceptions() throw(){}
		virtual const char* what() const throw()
		{
			return msg.c_str();
		}
};

#endif