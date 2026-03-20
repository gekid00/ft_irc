#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <string>
#include <vector>
#include "Utils.hpp"

struct Message
{
	int fd;
	std::string text;
};

struct CommandResponse
{
	std::vector<Message> messages;
	void addMessage(int fd, const std::string& text)
	{
		Message msg;
		msg.fd = fd;
		msg.text = text;
		messages.push_back(msg);
	}
};

CommandResponse handleCommand(int fd, const std::string& line);

#endif