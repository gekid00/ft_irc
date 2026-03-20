#include "Kick.hpp"
#include "Server.hpp"
#include "Replies.hpp"

Kick::Kick(Server& server, int fd, const std::vector<std::string>& params) : _server(server), _fd(fd), _params(params)
{

}

Kick::~Kick()
{

}

int Kick::execute()
{
	// Vérifier qu'il y a au moins 2 paramètres: channel et target
	if (_params.size() < 2)
	{
		_server.sendToClient(_fd, err_needmoreparams(getNickname(), "KICK"));
		return -1;
	}
	_channel = _params[0];
	_target = _params[1];
	// Extraire la raison si présente (optionnel)
	if (_params.size() >= 3)
	{
		_reason = _params[2];
		if (!_reason.empty() && _reason[0] == ':')
			_reason = _reason.substr(1);
	}
	else
		_reason = "";
	
	// Valider que le channel existe et que le sender est dedans
	if (validateChannel() < 0)
		return -1;
	
	// Valider que le target existe et est dans le channel
	if (validateTarget() < 0)
		return -1;
	
	// Exécuter le KICK
	executeKick();
	
	return 0;
}

std::string Kick::getNickname()
{
	return _server.getClient(_fd).getNickname();
}

int Kick::validateChannel()
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
	// Vérifier que le sender est opérateur du channel
	if (!channel->isOperator(_fd))
	{
		_server.sendToClient(_fd, err_chanoprivsneeded(getNickname(), _channel));
		return -1;
	}
	
	return 0;
}

int Kick::validateTarget()
{
	// Chercher le user par son nickname
	int targetFd = _server.getClientFdByNickname(_target);
	if (targetFd < 0)
	{
		_server.sendToClient(_fd, err_nosuchnick(getNickname(), _target));
		return -1;
	}
	
	// Vérifier que le target est dans le channel
	Channel& channel = _server.getChannel(_channel);
	if (!channel.isMember(targetFd))
	{
		_server.sendToClient(_fd, err_usernotinchannel(getNickname(), _target, _channel));
		return -1;
	}
	
	return 0;
}

void Kick::executeKick()
{
	// Récupérer le fd du target
	int targetFd = _server.getClientFdByNickname(_target);
	Channel& channel = _server.getChannel(_channel);
	
	// Retirer le target du channel
	channel.removeMember(targetFd);
	
	// Construire le message KICK au format IRC
	std::string message = ":" + getNickname() + " KICK " + _channel + " " + _target;
	if (!_reason.empty())
		message += " :" + _reason;
	message += "\r\n";
	
	// Broadcaster le KICK à tous les membres du channel (y compris le kicked)
	channel.broadcastMessage(message, -1);
	
	// Envoyer aussi au target qu'il a été kické
	_server.sendToClient(targetFd, message);
}
