#ifndef PRIVMSG_HPP
#define PRIVMSG_HPP

class Server;
#include "Utils.hpp"

class Privmsg
{
	private:
		Server& _server;
		int _fd;
		std::vector<std::string> _params;
		std::string _target;
		std::string _message;
		std::string getNickname();
		int validateTarget();
		int validateMessage();
		void sendToTarget();
	public:
		Privmsg(Server& server, int fd, const std::vector<std::string>& params);
		~Privmsg();
		int execute();
};

#endif