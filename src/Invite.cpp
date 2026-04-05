#include "Invite.hpp"
#include "Server.hpp"
#include "Replies.hpp"

Invite::Invite(Server& server, int fd, const std::vector<std::string>& params) : _server(server), _fd(fd), _params(params)
{

}

Invite::~Invite()
{

}

int Invite::execute()
{
	// Vérifier qu'il y a au moins 2 paramètres: target et channel
	if (_params.size() < 2)
	{
		_server.sendToClient(_fd, err_needmoreparams(getNickname(), "INVITE"));
		return -1;
	}
	_target = _params[0];
	_channel = _params[1];
	
	// Valider que le channel existe et que le sender est dedans et opérateur
	if (validateChannel() < 0)
		return -1;
	
	// Valider que le target existe et n'est pas déjà dans le channel
	if (validateTarget() < 0)
		return -1;
	
	// Exécuter l'INVITE
	executeInvite();
	
	return 0;
}

std::string Invite::getNickname()
{
	return _server.getClient(_fd).getNickname();
}

int Invite::validateChannel()
{
	// Vérifier que le channel existe
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
	// Vérifier que le sender est membre du channel
	if (!channel->isMember(_fd))
	{
		_server.sendToClient(_fd, err_notonchannel(getNickname(), _channel));
		return -1;
	}
	// Vérifier que le sender est opérateur si le channel est invite-only
	if (channel->isInviteOnly() && !channel->isOperator(_fd))
	{
		_server.sendToClient(_fd, err_chanoprivsneeded(getNickname(), _channel));
		return -1;
	}
	
	return 0;
}

int Invite::validateTarget()
{
	// Chercher l'user par son nickname
	int targetFd = _server.getClientFdByNickname(_target);
	if (targetFd < 0)
	{
		_server.sendToClient(_fd, err_nosuchnick(getNickname(), _target));
		return -1;
	}
	
	// Vérifier que le target n'est pas déjà dans le channel
	Channel& channel = _server.getChannel(_channel);
	if (channel.isMember(targetFd))
	{
		_server.sendToClient(_fd, err_useronchannel(getNickname(), _target, _channel));
		return -1;
	}
	
	return 0;
}

void Invite::executeInvite()
{
	// Récupérer le fd du target
	int targetFd = _server.getClientFdByNickname(_target);
	Channel& channel = _server.getChannel(_channel);
	
	// Ajouter le target à la liste des invités du channel
	channel.addInvited(targetFd);
	
	// Envoyer confirmation au sender
	_server.sendToClient(_fd, rpl_inviting(getNickname(), _target, _channel));
	
	// Envoyer notification au target qu'il a été invité (message INVITE en IRC)
	std::string message = ":" + _server.getClient(_fd).getPrefix() + " INVITE " + _target + " " + _channel + "\r\n";
	_server.sendToClient(targetFd, message);
}
