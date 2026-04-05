/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Mode.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gekido <gekido@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/14 14:53:48 by egerin            #+#    #+#             */
/*   Updated: 2026/03/15 00:04:41 by gekido           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"
#include <sstream>

void    Server::handleMode(int fd, const std::vector<std::string>& params)
{
    std::string nick = _clients[fd].getNickname();

    if (params.empty())
    {
        sendToClient(fd, err_needmoreparams(nick, "MODE"));
        return ;
    }

    std::string targetChannel = params[0];
    if (_channels.find(targetChannel) == _channels.end())
    {
        sendToClient(fd, err_nosuchchannel(nick, targetChannel));
        return ;
    }

    Channel &channel = _channels[targetChannel];
    if (!channel.isMember(fd))
    {
        sendToClient(fd, err_notonchannel(nick, targetChannel));
        return ;
    }

    // MODE #channel -> return current channel modes (RPL 324)
    if (params.size() == 1)
    {
        std::string modes = "+";
        std::string modeArgs;

        if (channel.isInviteOnly())
            modes += "i";
        if (channel.isTopicRestricted())
            modes += "t";
        if (channel.isKeyRestricted())
        {
            modes += "k";
            modeArgs += " " + channel.getKey();
        }
        if (channel.getUserLimit() > 0)
        {
            std::ostringstream oss;
            modes += "l";
            oss << channel.getUserLimit();
            modeArgs += " " + oss.str();
        }
        if (modes == "+")
            modes = "+";

        sendToClient(fd, rpl_channelmodeis(nick, targetChannel, modes + modeArgs));
        return ;
    }

    if (!channel.isOperator(fd))
    {
        sendToClient(fd, err_chanoprivsneeded(nick, targetChannel));
        return ;
    }

    std::string whichMode = params[1];
    bool adding = true;
    size_t paramIdx = 2;
    std::string appliedModes;
    std::string appliedArgs;
    char lastSign = '\0';

    for (size_t i = 0; i < whichMode.size(); i++)
    {
        char mode = whichMode[i];

        if (mode == '+')
        {
            adding = true;
            continue ;
        }
        if (mode == '-')
        {
            adding = false;
            continue ;
        }

        char sign = adding ? '+' : '-';
        if (lastSign != sign)
        {
            appliedModes += sign;
            lastSign = sign;
        }

        switch (mode)
        {
            case 'i':
                channel.setInviteOnly(adding);
                appliedModes += "i";
                break ;
            case 't':
                channel.setTopicRestricted(adding);
                appliedModes += "t";
                break ;
            case 'k':
                if (adding)
                {
                    if (paramIdx >= params.size())
                    {
                        sendToClient(fd, err_needmoreparams(nick, "MODE"));
                        return ;
                    }
                    channel.setKey(params[paramIdx]);
                    appliedModes += "k";
                    appliedArgs += " " + params[paramIdx];
                    paramIdx++;
                }
                else
                {
                    channel.setKey("");
                    appliedModes += "k";
                }
                break ;
            case 'o':
            {
                if (paramIdx >= params.size())
                {
                    sendToClient(fd, err_needmoreparams(nick, "MODE"));
                    return ;
                }
                std::string targetNick = params[paramIdx];
                int targetFd = getClientFdByNickname(targetNick);
                paramIdx++;

                if (targetFd == -1)
                {
                    sendToClient(fd, err_nosuchnick(nick, targetNick));
                    return ;
                }
                if (!channel.isMember(targetFd))
                {
                    sendToClient(fd, err_usernotinchannel(nick, targetNick, targetChannel));
                    return ;
                }

                if (adding)
                    channel.addOperator(targetFd);
                else
                    channel.removeOperator(targetFd);
                appliedModes += "o";
                appliedArgs += " " + targetNick;
                break ;
            }
            case 'l':
                if (adding)
                {
                    if (paramIdx >= params.size())
                    {
                        sendToClient(fd, err_needmoreparams(nick, "MODE"));
                        return ;
                    }
                    int limit = std::atoi(params[paramIdx].c_str());
                    if (limit <= 0)
                    {
                        sendToClient(fd, err_needmoreparams(nick, "MODE"));
                        return ;
                    }
                    channel.setUserLimit(limit);
                    appliedModes += "l";
                    appliedArgs += " " + params[paramIdx];
                    paramIdx++;
                }
                else
                {
                    channel.setUserLimit(0);
                    appliedModes += "l";
                }
                break ;
            default:
                sendToClient(fd, err_unknownmode(nick, std::string(1, mode)));
                continue ;
        }
    }

    if (appliedModes.empty())
        return ;

    std::string msg = ":" + _clients[fd].getPrefix() + " MODE " + targetChannel + " " + appliedModes + appliedArgs + "\r\n";
    // Broadcast to all channel members, including author.
    channel.broadcastMessage(msg, -1);
}