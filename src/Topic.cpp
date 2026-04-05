#include "Topic.hpp"
#include "Server.hpp"
#include "Replies.hpp"

Topic::Topic(Server& server, int fd, const std::vector<std::string>& params) : _server(server), _fd(fd), _params(params)
{

}

Topic::~Topic()
{

}

int Topic::execute()
{
	if (_params.size() < 1)
	{
		_server.sendToClient(_fd, err_needmoreparams(getNickname(), "TOPIC"));
		return -1;
	}
	_channel = _params[0];
	if (validateChannel() < 0)
		return -1;
	
	// Si pas de nouveau topic, afficher le topic actuel
	if (_params.size() == 1)
		getTopic();
	// Sinon, changer le topic
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
	if (_channel.empty())
	{
		_server.sendToClient(_fd, err_nosuchchannel(getNickname(), _channel));
		return -1;
	}
	Channel* channel = _server.findChannel(_channel);
	if (!channel)
	{
		_server.sendToClient(_fd, err_nosuchchannel(getNickname(), _channel));
		return -1;
	}
	// Vérifier que le client est membre du channel
	if (!channel->isMember(_fd))
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
	// Si pas de topic, envoyer message approprié
	if (topic.empty())
		_server.sendToClient(_fd, rpl_notopic(getNickname(), _channel));
	// Sinon envoyer le topic
	else
		_server.sendToClient(_fd, rpl_topic(getNickname(), _channel, topic));
}

void Topic::setTopic()
{
	// Extraire le nouveau topic
	std::string newTopic = _params[1];
	// Strip le ':' du début si présent
	if (!newTopic.empty() && newTopic[0] == ':')
		newTopic = newTopic.substr(1);
	Channel& channel = _server.getChannel(_channel);
	// Vérifier si le topic est restreint aux opérateurs
	if (channel.isTopicRestricted() && !channel.isOperator(_fd))
	{
		_server.sendToClient(_fd, err_chanoprivsneeded(getNickname(), _channel));
		return;
	}
	// Modifier le topic
	channel.setTopic(newTopic);
	// Envoyer confirmation au client
	_server.sendToClient(_fd, rpl_topic(getNickname(), _channel, newTopic));
	// Broadcaster le changement aux autres membres
	std::string message = ":" + _server.getClient(_fd).getPrefix() + " TOPIC " + _channel + " :" + newTopic + "\r\n";
	channel.broadcastMessage(message, _fd);
}
