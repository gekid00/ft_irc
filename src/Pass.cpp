/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Pass.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: egerin <egerin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/11 13:36:13 by egerin            #+#    #+#             */
/*   Updated: 2026/03/12 13:33:22 by egerin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void    Server::handlePass(int fd, const std::vector<std::string>& params)
{
    if (_clients[fd].isRegistered())
    {
        sendToClient(fd, err_alreadyregistered(_clients[fd].getNickname()));
        return ;
    }

    if (params.empty())
    {
        sendToClient(fd, err_needmoreparams(_clients[fd].getNickname(), "PASS"));
        return ;
    }
    else if (params[0] == _password)
        _clients[fd].setPasswordOk(true);
    else
    {
        _clients[fd].setPasswordOk(false);
        sendToClient(fd, err_passwdmismatch());
    }
}