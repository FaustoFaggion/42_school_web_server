#include "WebServ.hpp"

WebServ::WebServ()
{

}

WebServ::~WebServ()
{

}

ListenerSocket	WebServ::getListener() const
{
	return(listener);
}