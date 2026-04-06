#include "Topic.hpp"
#include "Server.hpp"
#include "Replies.hpp"

Topic::Topic(Server& server, int fd, const std::vector<std::string>& params)
	: _server(server), _fd(fd), _params(params)
{
}

Topic::~Topic()
{
}

int Topic::execute()
{
	if (_params.empty())
	{
		_server.sendToClient(_fd, err_needmoreparams(getNickname(), "TOPIC"));
		return -1;
	}
	_channel = _params[0];

	if (validateChannel() < 0)
		return -1;

	// Pas de deuxième paramètre : afficher le topic actuel
	if (_params.size() == 1)
		getTopic();
	// Deuxième paramètre : changer le topic
	else
		setTopic();

	return 0;
}

std::string Topic::getNickname()
{
	return _server.getClient(_fd).getNickname();
}

int Topic::validateChannel()
{
	if (_channel.empty() || !_server.findChannel(_channel))
	{
		_server.sendToClient(_fd, err_nosuchchannel(getNickname(), _channel));
		return -1;
	}
	// Doit être membre du channel
	if (!_server.findChannel(_channel)->isMember(_fd))
	{
		_server.sendToClient(_fd, err_notonchannel(getNickname(), _channel));
		return -1;
	}
	return 0;
}

void Topic::getTopic()
{
	Channel& channel = _server.getChannel(_channel);
	std::string topic = channel.getTopic();

	if (topic.empty())
		_server.sendToClient(_fd, rpl_notopic(getNickname(), _channel));
	else
		_server.sendToClient(_fd, rpl_topic(getNickname(), _channel, topic));
}

void Topic::setTopic()
{
	// Supprimer le ':' du début si présent
	std::string newTopic = _params[1];
	if (!newTopic.empty() && newTopic[0] == ':')
		newTopic = newTopic.substr(1);

	Channel& channel = _server.getChannel(_channel);

	// Avec le mode +t : seul un opérateur peut changer le topic
	if (channel.isTopicRestricted() && !channel.isOperator(_fd))
	{
		_server.sendToClient(_fd, err_chanoprivsneeded(getNickname(), _channel));
		return;
	}

	// Appliquer le nouveau topic et notifier tout le channel
	channel.setTopic(newTopic);
	_server.sendToClient(_fd, rpl_topic(getNickname(), _channel, newTopic));
	std::string msg = ":" + _server.getClient(_fd).getPrefix() + " TOPIC " + _channel + " :" + newTopic + "\r\n";
	channel.broadcastMessage(msg, _fd, _server);
}
