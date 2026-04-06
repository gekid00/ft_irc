#include "Server.hpp"

void Server::handlePass(int fd, const std::vector<std::string>& params)
{
	// PASS ne peut pas être réenvoyé après enregistrement
	if (_clients[fd].isRegistered())
	{
		sendToClient(fd, err_alreadyregistered(_clients[fd].getNickname()));
		return;
	}

	if (params.empty())
	{
		sendToClient(fd, err_needmoreparams(_clients[fd].getNickname(), "PASS"));
		return;
	}

	// Comparer avec le mot de passe du serveur
	if (params[0] == _password)
		_clients[fd].setPasswordOk(true);
	else
	{
		_clients[fd].setPasswordOk(false);
		sendToClient(fd, err_passwdmismatch());
	}
}
