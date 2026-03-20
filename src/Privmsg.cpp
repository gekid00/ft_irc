#include "Privmsg.hpp"
#include "Server.hpp"
#include "Replies.hpp"

Privmsg::Privmsg(Server& server, int fd, const std::vector<std::string>& params) : _server(server), _fd(fd), _params(params)
{

}

Privmsg::~Privmsg()
{

}

int Privmsg::execute()
{
	// Vérifier qu'on a au moins 2 paramètres (cible et message)
	if (_params.size() < 2)
	{
		_server.sendToClient(_fd, err_needmoreparams(getNickname(), "PRIVMSG"));
		return -1;
	}

	// Extraire la cible et le message
	_target = _params[0];
	_message = _params[1];
	// Enlever le ':' du début du message si présent
	if (!_message.empty() && _message[0] == ':')
		_message = _message.substr(1);

	// Valider que la cible existe et est accessible
	if (validateTarget() < 0)
		return -1;

	// Valider que le message n'est pas vide
	if (validateMessage() < 0)
	{
		_server.sendToClient(_fd, err_notexttosend(getNickname()));
		return -1;
	}

	// Envoyer le message à la cible
	sendToTarget();
	return 0;
}

std::string Privmsg::getNickname()
{
	return _server.getClient(_fd).getNickname();
}

int Privmsg::validateTarget()
{
	// Si la cible est vide, erreur
	if (_target.empty())
	{
		_server.sendToClient(_fd, err_norecipient(getNickname(), "PRIVMSG"));
		return -1;
	}

	// Vérifier si c'est un channel (commence par # ou &)
	if (_target[0] == '#' || _target[0] == '&')
	{
		// Format du channel invalide
		if (!isValidChannel(_target))
		{
			_server.sendToClient(_fd, err_badchanmask(getNickname(), _target));
			return -1;
		}
		// Vérifier que le channel existe sans le créer implicitement
		Channel* channel = _server.findChannel(_target);
		if (!channel)
		{
			_server.sendToClient(_fd, err_nosuchchannel(getNickname(), _target));
			return -1;
		}
		// Vérifier que le client est membre du channel
		if (!channel->isMember(_fd))
		{
			_server.sendToClient(_fd, err_notonchannel(getNickname(), _target));
			return -1;
		}
	}
	else
	{
		// Vérifier que le user cible existe
		int targetFd = _server.getClientFdByNickname(_target);
		if (targetFd < 0)
		{
			_server.sendToClient(_fd, err_nosuchnick(getNickname(), _target));
			return -1;
		}
	}
	return 0;
}

int Privmsg::validateMessage()
{
	// Vérifier que le message n'est pas vide
	if (_message.empty())
		return -1;
	return 0;
}

void Privmsg::sendToTarget()
{
	// Si c'est un channel, broadcaster à tous les membres sauf l'envoyeur
	if (_target[0] == '#' || _target[0] == '&')
	{
		Channel& channel = _server.getChannel(_target);
		// Construire le message au format IRC
		std::string fullMessage = ":" + getNickname() + " PRIVMSG " + _target + " :" + _message + "\r\n";
		// Envoyer à tous les membres sauf l'envoyeur
		channel.broadcastMessage(fullMessage, _fd);
	}
	else
	{
		// C'est un user, trouver le fd du user par son nickname
		int targetFd = _server.getClientFdByNickname(_target);
		if (targetFd < 0)
		{
			_server.sendToClient(_fd, err_nosuchnick(getNickname(), _target));
			return;
		}
		// Construire le message au format IRC
		std::string fullMessage = ":" + getNickname() + " PRIVMSG " + _target + " :" + _message + "\r\n";
		// Envoyer au user cible
		_server.sendToClient(targetFd, fullMessage);
	}
}