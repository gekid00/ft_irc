#include "Invite.hpp"
#include "Server.hpp"
#include "Replies.hpp"

Invite::Invite(Server& server, int fd, const std::vector<std::string>& params)
	: _server(server), _fd(fd), _params(params)
{
}

Invite::~Invite()
{
}

int Invite::execute()
{
	// Au minimum : cible et channel
	if (_params.size() < 2)
	{
		_server.sendToClient(_fd, err_needmoreparams(getNickname(), "INVITE"));
		return -1;
	}
	_target  = _params[0];
	_channel = _params[1];

	if (validateChannel() < 0 || validateTarget() < 0)
		return -1;

	executeInvite();
	return 0;
}

std::string Invite::getNickname()
{
	return _server.getClient(_fd).getNickname();
}

int Invite::validateChannel()
{
	if (_channel.empty() || !_server.findChannel(_channel))
	{
		_server.sendToClient(_fd, err_nosuchchannel(getNickname(), _channel));
		return -1;
	}
	Channel* channel = _server.findChannel(_channel);

	// L'inviteur doit être membre
	if (!channel->isMember(_fd))
	{
		_server.sendToClient(_fd, err_notonchannel(getNickname(), _channel));
		return -1;
	}
	// En mode +i, seul un opérateur peut inviter
	if (channel->isInviteOnly() && !channel->isOperator(_fd))
	{
		_server.sendToClient(_fd, err_chanoprivsneeded(getNickname(), _channel));
		return -1;
	}
	return 0;
}

int Invite::validateTarget()
{
	int targetFd = _server.getClientFdByNickname(_target);
	if (targetFd < 0)
	{
		_server.sendToClient(_fd, err_nosuchnick(getNickname(), _target));
		return -1;
	}
	// Ne peut pas inviter quelqu'un déjà dans le channel
	if (_server.getChannel(_channel).isMember(targetFd))
	{
		_server.sendToClient(_fd, err_useronchannel(getNickname(), _target, _channel));
		return -1;
	}
	return 0;
}

void Invite::executeInvite()
{
	int targetFd = _server.getClientFdByNickname(_target);
	Channel& channel = _server.getChannel(_channel);

	// Ajouter la cible à la liste d'invités (permet de rejoindre en mode +i)
	channel.addInvited(targetFd);

	// Confirmer à l'inviteur
	_server.sendToClient(_fd, rpl_inviting(getNickname(), _target, _channel));

	// Notifier la cible
	std::string msg = ":" + _server.getClient(_fd).getPrefix() + " INVITE " + _target + " " + _channel + "\r\n";
	_server.sendToClient(targetFd, msg);
}
