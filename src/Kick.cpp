#include "Kick.hpp"
#include "Server.hpp"
#include "Replies.hpp"

Kick::Kick(Server& server, int fd, const std::vector<std::string>& params)
	: _server(server), _fd(fd), _params(params)
{
}

Kick::~Kick()
{
}

int Kick::execute()
{
	// Au minimum : channel et cible
	if (_params.size() < 2)
	{
		_server.sendToClient(_fd, err_needmoreparams(getNickname(), "KICK"));
		return -1;
	}
	_channel = _params[0];
	_target  = _params[1];

	// Raison optionnelle
	_reason = (_params.size() >= 3) ? _params[2] : "";
	if (!_reason.empty() && _reason[0] == ':')
		_reason = _reason.substr(1);

	if (validateChannel() < 0 || validateTarget() < 0)
		return -1;

	executeKick();
	return 0;
}

std::string Kick::getNickname()
{
	return _server.getClient(_fd).getNickname();
}

int Kick::validateChannel()
{
	if (_channel.empty() || !_server.findChannel(_channel))
	{
		_server.sendToClient(_fd, err_nosuchchannel(getNickname(), _channel));
		return -1;
	}
	Channel* channel = _server.findChannel(_channel);
	// Doit être membre du channel
	if (!channel->isMember(_fd))
	{
		_server.sendToClient(_fd, err_notonchannel(getNickname(), _channel));
		return -1;
	}
	// Doit être opérateur pour pouvoir kicker
	if (!channel->isOperator(_fd))
	{
		_server.sendToClient(_fd, err_chanoprivsneeded(getNickname(), _channel));
		return -1;
	}
	return 0;
}

int Kick::validateTarget()
{
	int targetFd = _server.getClientFdByNickname(_target);
	if (targetFd < 0)
	{
		_server.sendToClient(_fd, err_nosuchnick(getNickname(), _target));
		return -1;
	}
	// La cible doit être dans le channel
	if (!_server.getChannel(_channel).isMember(targetFd))
	{
		_server.sendToClient(_fd, err_usernotinchannel(getNickname(), _target, _channel));
		return -1;
	}
	return 0;
}

void Kick::executeKick()
{
	int targetFd = _server.getClientFdByNickname(_target);
	Channel& channel = _server.getChannel(_channel);

	// Retirer la cible du channel avant d'envoyer le message
	channel.removeMember(targetFd);

	// Construire le message KICK avec le préfixe complet
	std::string msg = ":" + _server.getClient(_fd).getPrefix() + " KICK " + _channel + " " + _target;
	if (!_reason.empty())
		msg += " :" + _reason;
	msg += "\r\n";

	// Notifier la cible en premier, puis les membres restants
	_server.sendToClient(targetFd, msg);
	channel.broadcastMessage(msg, -1, _server);

	// Supprimer le channel s'il est maintenant vide
	if (channel.getNumberOfMembers() == 0)
		_server.removeChannel(_channel);
}
