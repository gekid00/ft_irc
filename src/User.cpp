#include "Server.hpp"

void Server::handleUser(int fd, const std::vector<std::string>& params)
{
	// USER ne peut pas être réenvoyé après enregistrement
	if (_clients[fd].isRegistered())
	{
		sendToClient(fd, err_alreadyregistered(_clients[fd].getNickname()));
		return;
	}

	// PASS doit être validé avant USER
	if (!_clients[fd].isPasswordOk())
	{
		sendToClient(fd, err_notregistered());
		return;
	}

	// Format : USER <username> <mode> <unused> :<realname>
	if (params.size() < 4)
	{
		sendToClient(fd, err_needmoreparams(_clients[fd].getNickname(), "USER"));
		return;
	}

	std::string username = params[0];
	std::string realname = params[3];
	// Supprimer le ':' du paramètre trailing si présent
	if (!realname.empty() && realname[0] == ':')
		realname = realname.substr(1);

	_clients[fd].setUsername(username);
	_clients[fd].setRealname(realname);

	// Tenter de compléter l'enregistrement (nécessite aussi NICK)
	if (_clients[fd].tryRegister())
		sendToClient(fd, rpl_welcome(_clients[fd].getNickname(), _clients[fd].getPrefix()));
}
