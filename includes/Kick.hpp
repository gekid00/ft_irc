#ifndef KICK_HPP
#define KICK_HPP

class Server;
#include "Utils.hpp"

class Kick
{
	private:
		Server& _server;
		int _fd;
		std::vector<std::string> _params;
		std::string _channel;
		std::string _target;
		std::string _reason;
		std::string getNickname();
		int validateChannel();
		int validateTarget();
		void executeKick();
	public:
		Kick(Server& server, int fd, const std::vector<std::string>& params);
		~Kick();
		int execute();
};

#endif
