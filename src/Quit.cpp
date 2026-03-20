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
    
    // Envoyer le message de départ
    std::string quitMsg = "ERROR :Client quit\r\n";
    sendToClient(fd, quitMsg);
    
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
