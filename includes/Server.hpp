/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gekido <gekido@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 14:24:14 by egerin            #+#    #+#             */
/*   Updated: 2026/03/17 05:28:31 by gekido           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <iomanip>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <exception>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <poll.h>
#include <signal.h>
#include <map>
#include "Client.hpp"
#include "Command.hpp"
#include "Replies.hpp"
#include "Channel.hpp"
#include "Utils.hpp"
#include "Join.hpp"
#include "Privmsg.hpp"
#include "Topic.hpp"
#include "Kick.hpp"
#include "Invite.hpp"

class Server
{
    private:
    /* data */
        int                             _listenSocket;
        int                             _port;
        std::vector<pollfd>             _pollfds;
        std::map<int, Client>           _clients;
        std::map<std::string, Channel>  _channels;
        std::string                     _password;
        sockaddr_in                     _serverAddr;
        static volatile sig_atomic_t    _pollRunning;
        
    /* functions */
        void                setupSocket(int port);
        void                addClient();
        void                readData(int fd);
        void                cleanupClient(int fd);
    public:
        Server(const std::string& port, const std::string& password);
        ~Server();
        void                addChannel(std::string channelName);
        void                pollMonitoring();
        void                handleCommand(int fd, const std::string& line);
        void                handlePass(int fd, const std::vector<std::string>& params);
        void                handleNick(int fd, const std::vector<std::string>& params);
        void                handleJoin(int fd, std::vector<std::string> params);
        void                handleUser(int fd, const std::vector<std::string>& params);
        void                handlePrivmsg(int fd, const std::vector<std::string>& params);
        void                handleTopic(int fd, const std::vector<std::string>& params);
        void                handleKick(int fd, const std::vector<std::string>& params);
        void                handleInvite(int fd, const std::vector<std::string>& params);
        void                handleMode(int fd, const std::vector<std::string>& params);
        void                sendToClient(int fd, const std::string& msg);
        Client&             getClient(int fd);
        bool                hasClient(int fd);
        Channel&            getChannel(const std::string& name);
        Channel*            findChannel(const std::string& name);
        int                 getClientFdByNickname(const std::string& nickname);
        void                handleQuit(int fd, const std::vector<std::string>& params);
        static void         setupSignals();
        static void         signalHandler(int num);
};


#endif