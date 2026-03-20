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
    _clients[fd].setNickname(nickname);
    if (_clients[fd].tryRegister())
        sendToClient(fd, rpl_welcome(nickname));
    
}