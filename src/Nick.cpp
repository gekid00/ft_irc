/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Nick.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: egerin <egerin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/11 14:06:00 by egerin            #+#    #+#             */
/*   Updated: 2026/03/12 13:30:46 by egerin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void    Server::handleNick(int fd, const std::vector<std::string>& params)
{
    if (params.empty())
    {
        sendToClient(fd, err_nonicknamegiven(_clients[fd].getNickname()));
        return ;
    }
    if (!_clients[fd].isPasswordOk())
    {
        sendToClient(fd, err_notregistered());
        return ;
    }

    std::string nickname = params[0];
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); it++)
    {
        if (it->first == fd)
            continue;
        if (it->second.getNickname() == nickname)
        {
            sendToClient(fd, err_nicknameinuse(nickname));
            return ;
        }
    }
    // Si déjà enregistré, broadcaster le changement de nick aux channels
    if (_clients[fd].isRegistered())
    {
        std::string oldPrefix = _clients[fd].getPrefix();
        _clients[fd].setNickname(nickname);
        std::string nickMsg = ":" + oldPrefix + " NICK " + nickname + "\r\n";
        // Envoyer à soi-même
        sendToClient(fd, nickMsg);
        // Broadcaster aux membres des channels communs (une seule fois par fd)
        std::set<int> notified;
        notified.insert(fd);
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
        return ;
    }
    _clients[fd].setNickname(nickname);
    if (_clients[fd].tryRegister())
        sendToClient(fd, rpl_welcome(nickname));
}