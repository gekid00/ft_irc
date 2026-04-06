#include "Server.hpp"

void Server::handleNick(int fd, const std::vector<std::string>& params)
{
	// Vérifier qu'un nickname est fourni
	if (params.empty())
	{
		sendToClient(fd, err_nonicknamegiven(_clients[fd].getNickname()));
		return;
	}

	// PASS doit être envoyé avant NICK
	if (!_clients[fd].isPasswordOk())
	{
		sendToClient(fd, err_notregistered());
		return;
	}

	std::string nickname = params[0];

	// Vérifier l'unicité du nickname sur le serveur
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->first != fd && it->second.getNickname() == nickname)
		{
			sendToClient(fd, err_nicknameinuse(nickname));
			return;
		}
	}

	// Si le client est déjà enregistré : changement de nick en cours de session
	if (_clients[fd].isRegistered())
	{
		std::string oldPrefix = _clients[fd].getPrefix();
		_clients[fd].setNickname(nickname);
		std::string nickMsg = ":" + oldPrefix + " NICK " + nickname + "\r\n";

		// Notifier soi-même et tous les membres des channels communs (une seule fois par fd)
		std::set<int> notified;
		notified.insert(fd);
		sendToClient(fd, nickMsg);

		for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
		{
			if (!it->second.isMember(fd))
				continue;
			const std::set<int>& members = it->second.getMembers();
			for (std::set<int>::const_iterator m = members.begin(); m != members.end(); ++m)
			{
				if (notified.find(*m) == notified.end())
				{
					sendToClient(*m, nickMsg);
					notified.insert(*m);
				}
			}
		}
		return;
	}

	// Première fois (enregistrement) : stocker le nick et tenter de compléter l'enregistrement
	_clients[fd].setNickname(nickname);
	if (_clients[fd].tryRegister())
		sendToClient(fd, rpl_welcome(nickname, _clients[fd].getPrefix()));
}
