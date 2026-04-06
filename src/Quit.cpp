#include "Server.hpp"

void Server::handleQuit(int fd, const std::vector<std::string>& params)
{
	// Extraire le message de départ (paramètre optionnel)
	std::string message = "Goodbye";
	if (!params.empty() && !params[0].empty())
	{
		message = params[0];
		if (message[0] == ':')
			message = message.substr(1);
	}

	std::string prefix = _clients[fd].getPrefix();
	std::string quitBroadcast = ":" + prefix + " QUIT :" + message + "\r\n";

	// Notifier tous les membres des channels du client
	for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		if (it->second.isMember(fd))
			it->second.broadcastMessage(quitBroadcast, fd, *this);
	}

	// Envoyer ERROR au client qui quitte
	sendToClient(fd, "ERROR :Closing link\r\n");

	// Supprimer le client (flushClient est appelé dans cleanupClient avant close())
	cleanupClient(fd);
	for (size_t i = 0; i < _pollfds.size(); ++i)
	{
		if (_pollfds[i].fd == fd)
		{
			_pollfds.erase(_pollfds.begin() + i);
			break;
		}
	}
}
