#ifndef JOIN_HPP
#define JOIN_HPP

class Server;
#include "Utils.hpp"

class Join
{
	private:
		Server& _server;
		int _fd;
		std::vector<std::string> _params;
		std::vector<std::string> _channels_names;
		std::vector<std::string> _keys;
		int parseChannels();
		int parseKeys();
		int validateChannelName(const std::string& name);
		int validateAccess(const std::string& name, const std::string& key);
		void addClientToChannel(const std::string& name);
		void sendJoinResponse(const std::string& name);
		void broadcastJoin(const std::string& channel_name, const std::string& message);
		std::string getNickname();
	public:
		Join(Server& server, int fd, const std::vector<std::string>& params);
		~Join();
		int execute();
};

#endif