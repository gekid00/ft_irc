#include "Server.hpp"

void Server::handlePart(int fd, const std::vector<std::string>& params)
{
	if (params.empty())
	{
		sendToClient(fd, err_needmoreparams(_clients[fd].getNickname(), "PART"));
		return;
	}

	std::string nick   = _clients[fd].getNickname();
	std::string prefix = _clients[fd].getPrefix();

	// Extraire la raison optionnelle
	std::string reason = "";
	if (params.size() >= 2)
	{
		reason = params[1];
		if (!reason.empty() && reason[0] == ':')
			reason = reason.substr(1);
	}

	// Parser les channels séparés par des virgules
	std::vector<std::string> channels = split(params[0], ',');

	for (size_t i = 0; i < channels.size(); i++)
	{
		const std::string& channelName = channels[i];

		// Vérifier que le channel existe
		Channel* channel = findChannel(channelName);
		if (!channel)
		{
			sendToClient(fd, err_nosuchchannel(nick, channelName));
			continue;
		}

		// Vérifier que le client est membre
		if (!channel->isMember(fd))
		{
			sendToClient(fd, err_notonchannel(nick, channelName));
			continue;
		}

		// Construire et broadcaster le message PART à tous les membres (y compris celui qui part)
		std::string partMsg = ":" + prefix + " PART " + channelName;
		if (!reason.empty())
			partMsg += " :" + reason;
		partMsg += "\r\n";
		channel->broadcastMessage(partMsg, -1, *this);

		// Retirer le client du channel
		channel->removeMember(fd);

		// Supprimer le channel s'il est vide
		if (channel->getNumberOfMembers() == 0)
			_channels.erase(channelName);
	}
}
