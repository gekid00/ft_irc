#ifndef TOPIC_HPP
#define TOPIC_HPP

class Server;
#include "Utils.hpp"

class Topic
{
	private:
		Server& _server;
		int _fd;
		std::vector<std::string> _params;
		std::string _channel;
		std::string _newTopic;
		std::string getNickname();
		int validateChannel();
		void setTopic();
		void getTopic();
	public:
		Topic(Server& server, int fd, const std::vector<std::string>& params);
		~Topic();
		int execute();
};

#endif