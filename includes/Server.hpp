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
#include <set>
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
		int								_listenSocket;
		int								_port;
		std::string						_password;
		sockaddr_in						_serverAddr;
		std::vector<pollfd>				_pollfds;	// Tableau surveillé par poll()
		std::map<int, Client>			_clients;	// fd → Client
		std::map<std::string, Channel>	_channels;	// nom → Channel
		static volatile sig_atomic_t	_pollRunning;

		// Réseau bas niveau
		void	setupSocket(int port);
		void	addClient();
		void	readData(int fd);
		void	cleanupClient(int fd);

		// Gestion du buffer de sortie (POLLOUT)
		void	setPollOut(int fd, bool enable);	// Active/désactive POLLOUT sur un fd
		void	flushClient(int fd);				// Vide le buffer de sortie vers le socket

	public:
		Server(const std::string& port, const std::string& password);
		~Server();

		// Boucle principale
		void	pollMonitoring();

		// Parsing et dispatch des commandes
		void	handleCommand(int fd, const std::string& line);

		// Commandes d'authentification
		void	handlePass(int fd, const std::vector<std::string>& params);
		void	handleNick(int fd, const std::vector<std::string>& params);
		void	handleUser(int fd, const std::vector<std::string>& params);

		// Commandes de channel
		void	handleJoin(int fd, std::vector<std::string> params);
		void	handlePart(int fd, const std::vector<std::string>& params);
		void	handleQuit(int fd, const std::vector<std::string>& params);
		void	handlePrivmsg(int fd, const std::vector<std::string>& params);
		void	handleTopic(int fd, const std::vector<std::string>& params);
		void	handleKick(int fd, const std::vector<std::string>& params);
		void	handleInvite(int fd, const std::vector<std::string>& params);
		void	handleMode(int fd, const std::vector<std::string>& params);

		// Envoi de données (bufferisé, vidé via POLLOUT)
		void	sendToClient(int fd, const std::string& msg);

		// Accesseurs
		Client&		getClient(int fd);
		bool		hasClient(int fd);
		Channel&	getChannel(const std::string& name);
		Channel*	findChannel(const std::string& name);
		int			getClientFdByNickname(const std::string& nickname);
		void		addChannel(std::string channelName);
		void		removeChannel(const std::string& channelName);

		// Signaux
		static void	setupSignals();
		static void	signalHandler(int num);
};

#endif
