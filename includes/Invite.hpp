#ifndef INVITE_HPP
#define INVITE_HPP

class Server;
#include "Utils.hpp"

class Invite
{
	private:
		Server& _server;
		int _fd;
		std::vector<std::string> _params;
		std::string _target;
		std::string _channel;
		std::string getNickname();
		int validateChannel();
		int validateTarget();
		void executeInvite();
	public:
		Invite(Server& server, int fd, const std::vector<std::string>& params);
		~Invite();
		int execute();
};

#endif
