#include "Privmsg.hpp"
#include "Server.hpp"
#include "Replies.hpp"

Privmsg::Privmsg(Server& server, int fd, const std::vector<std::string>& params)
	: _server(server), _fd(fd), _params(params)
{
}

Privmsg::~Privmsg()
{
}

int Privmsg::execute()
{
	// Vérifier qu'on a une cible et un message
	if (_params.size() < 2)
	{
		_server.sendToClient(_fd, err_needmoreparams(getNickname(), "PRIVMSG"));
		return -1;
	}

	// Extraire la cible et le message
	_target  = _params[0];
	_message = _params[1];
	// Supprimer le ':' du début si présent (paramètre trailing IRC)
	if (!_message.empty() && _message[0] == ':')
		_message = _message.substr(1);

	// Valider la cible (channel ou user)
	if (validateTarget() < 0)
		return -1;

	// Valider que le message n'est pas vide
	if (validateMessage() < 0)
	{
		_server.sendToClient(_fd, err_notexttosend(getNickname()));
		return -1;
	}

	// Envoyer le message
	sendToTarget();
	return 0;
}

std::string Privmsg::getNickname()
{
	return _server.getClient(_fd).getNickname();
}

int Privmsg::validateTarget()
{
	if (_target.empty())
	{
		_server.sendToClient(_fd, err_norecipient(getNickname(), "PRIVMSG"));
		return -1;
	}

	if (_target[0] == '#' || _target[0] == '&')
	{
		// Cible = channel
		if (!isValidChannel(_target))
		{
			_server.sendToClient(_fd, err_badchanmask(getNickname(), _target));
			return -1;
		}
		Channel* channel = _server.findChannel(_target);
		if (!channel)
		{
			_server.sendToClient(_fd, err_nosuchchannel(getNickname(), _target));
			return -1;
		}
		// Doit être membre pour envoyer dans le channel
		if (!channel->isMember(_fd))
		{
			_server.sendToClient(_fd, err_notonchannel(getNickname(), _target));
			return -1;
		}
	}
	else
	{
		// Cible = utilisateur
		if (_server.getClientFdByNickname(_target) < 0)
		{
			_server.sendToClient(_fd, err_nosuchnick(getNickname(), _target));
			return -1;
		}
	}
	return 0;
}

int Privmsg::validateMessage()
{
	return _message.empty() ? -1 : 0;
}

void Privmsg::sendToTarget()
{
	std::string prefix = _server.getClient(_fd).getPrefix();

	if (_target[0] == '#' || _target[0] == '&')
	{
		// Channel : broadcaster à tous les membres sauf l'expéditeur
		Channel& channel = _server.getChannel(_target);
		std::string msg = ":" + prefix + " PRIVMSG " + _target + " :" + _message + "\r\n";
		channel.broadcastMessage(msg, _fd, _server);
	}
	else
	{
		// Utilisateur : envoi direct
		int targetFd = _server.getClientFdByNickname(_target);
		if (targetFd < 0)
		{
			_server.sendToClient(_fd, err_nosuchnick(getNickname(), _target));
			return;
		}
		std::string msg = ":" + prefix + " PRIVMSG " + _target + " :" + _message + "\r\n";
		_server.sendToClient(targetFd, msg);
	}
}
