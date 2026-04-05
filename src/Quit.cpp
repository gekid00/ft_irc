/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Quit.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gekido <gekido@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/17 10:00:00 by gekido            #+#    #+#             */
/*   Updated: 2026/03/17 05:28:19 by gekido           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void    Server::handleQuit(int fd, const std::vector<std::string>& params)
{
    std::string message = "Goodbye";
    if (!params.empty() && !params[0].empty())
    {
        message = params[0];
        if (message[0] == ':')
            message = message.substr(1);
    }

    std::string prefix = _clients[fd].getPrefix();
    std::string quitBroadcast = ":" + prefix + " QUIT :" + message + "\r\n";

    // Broadcaster le QUIT à tous les membres des channels du client
    for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        if (it->second.isMember(fd))
            it->second.broadcastMessage(quitBroadcast, fd);
    }

    // Envoyer ERROR au client qui quitte
    sendToClient(fd, "ERROR :Closing link\r\n");

    // Fermer proprement le client
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
