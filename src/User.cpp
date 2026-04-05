/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: egerin <egerin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/11 15:30:28 by egerin            #+#    #+#             */
/*   Updated: 2026/03/12 13:35:10 by egerin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void    Server::handleUser(int fd, const std::vector<std::string>& params)
{
    if (_clients[fd].isRegistered())
    {
        sendToClient(fd, err_alreadyregistered(_clients[fd].getNickname()));
        return ;
    }
    if (!_clients[fd].isPasswordOk())
    {
        sendToClient(fd, err_notregistered());
        return ;
    }
    if (params.empty() || params.size() < 4)
    {
        sendToClient(fd, err_needmoreparams(_clients[fd].getNickname(), "USER"));
        return ;
    }
    std::string username = params[0];
    std::string realname = params[3];
    if (!realname.empty() && realname[0] == ':')
        realname = realname.substr(1);
    _clients[fd].setUsername(username);
    _clients[fd].setRealname(realname);
    if (_clients[fd].tryRegister())
        sendToClient(fd, rpl_welcome(_clients[fd].getNickname(), _clients[fd].getPrefix()));
}