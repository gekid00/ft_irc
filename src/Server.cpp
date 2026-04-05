/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gekido <gekido@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 13:20:56 by egerin            #+#    #+#             */
/*   Updated: 2026/03/17 05:28:30 by gekido           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Join.hpp"
#include <errno.h>

volatile sig_atomic_t Server::_pollRunning = 1;

void Server::setupSignals()
{
    struct sigaction sa;
    sa.sa_handler = Server::signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
	signal(SIGPIPE, SIG_IGN);
}

void Server::signalHandler(int num)
{
    (void)num;
    std::cout << "\n" << "server shutting down" << std::endl;
    _pollRunning = 0;
}

Server::Server(const std::string& port, const std::string& password) : _password(password)
{
    _port = std::atoi(port.c_str());
    if (_port <= 1024 || _port >= 65535)
        throw std::invalid_argument("port is not between 1024 and 65535");
    setupSocket(_port);
    
}

Server::~Server()
{
    for (size_t i = 0; i < _pollfds.size(); i++)
        close(_pollfds[i].fd);
    _clients.clear();
    _channels.clear();
}

/*
setupSocket: creation de la socket d'ecoute, le socket est un fd donc close en cas d'erreur
etape de la creation :
-fonction socket avec les parametres pour ipv4 et TCP (AF_INET, SOCK_STREAM)
-regler les options avec setsockopt, le flag SO_REUSEADDR nous permettra de, si il y a un arret,
 relancer le serveur sans attendre la liberation du port
-utiliser fcntl avec le flag O_NONBLOCK pour que le socket soit non-bloquant pour qu'accept() ne bloque jamais
-init la struct sockaddr_in avec ipv4, le port donne via htons, et le flag INADDR_ANY pour n'importe quelle adresse ip
-bind pour asocier le socket au port choisi
-listen pour que le socket puisse attendre les connexions entrantes
-ajout du socket au tableau pollfds avec le flag POLLIN
*/
void    Server::setupSocket(int port)
{
    _listenSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (_listenSocket < 0)
    {
        close(_listenSocket);
        throw std::invalid_argument("unable to create socket");
    }

    int opt = 1;
    if (setsockopt(_listenSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {    
        close(_listenSocket);
        throw std::invalid_argument("unable to set socket options");
    }

    if (fcntl(_listenSocket, F_SETFL, O_NONBLOCK) < 0)
    {
        close(_listenSocket);
        throw std::invalid_argument("unable to protect the socket with fcntl");
    }

    std::memset(&_serverAddr, 0, sizeof(_serverAddr));
    _serverAddr.sin_family = AF_INET;
    _serverAddr.sin_port = htons(port);
    _serverAddr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(_listenSocket, (sockaddr*)&_serverAddr, sizeof(_serverAddr)) < 0)
    {
        close(_listenSocket);
        throw std::invalid_argument("unable to bind the socket");
    }

    if (listen(_listenSocket, SOMAXCONN) < 0)
    {
        close(_listenSocket);
        throw std::invalid_argument("unable to listen");
    }
    pollfd listenPollFd;
    listenPollFd.fd = _listenSocket;
    listenPollFd.events = POLLIN;
    listenPollFd.revents = 0;
    _pollfds.push_back(listenPollFd);
}

/*
pollMonitoring: boucle principale servant au monitoring des sockets
etapes:
-utiliser la fonction poll, poll est un multiplexeur, il permet a un seul thread de tout surveiller
-si poll retourne moins de zero et que ce n'est pas du a un signal, le programme s'arrete
-boucler sur le tableau de pollfd, si c'est la socket d'ecoute et que l'event est POLLIN, on cree
 un nouveau fd client, si c'est une socket client et que l'event est POLLIN on recv, bufferise et parse,
 si l'event est POLLHUP ou POLLERR on cleanup

*/
void    Server::pollMonitoring()
{
    while (_pollRunning)
    {
        int pollReturn = poll(&_pollfds[0], _pollfds.size(), 60000);

        if (pollReturn < 0)
        {
            if (errno == EINTR)
                continue ;
            std::cerr << "error: poll: stopping the program" << std::endl;
            break;
        }
        
        for (size_t i = 0; i < _pollfds.size(); i++)
        {
            if (_pollfds[i].fd == _listenSocket && _pollfds[i].revents & POLLIN)
                addClient();
            else if (_pollfds[i].fd != _listenSocket && _pollfds[i].revents & POLLIN)
            {
                int fd = _pollfds[i].fd;
                size_t prevSize = _pollfds.size();
                readData(fd);
                // Si readData a supprimé ce fd de _pollfds, ajuster l'index
                if (_pollfds.size() < prevSize)
                    i--;
            }
            else if (_pollfds[i].fd != _listenSocket && _pollfds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
            {
                cleanupClient(_pollfds[i].fd);
                _pollfds.erase(_pollfds.begin() + i);  // ✅ ENLEVER de _pollfds!
                i--;  // Ajuster l'index après erase()
            }
        }
    }
}

/*
fonction d'ajout de nouveaux clients au serveur
-accepter la nouvelle connexion via la fonction accept()
-fcntl pour le non bloquant comme sur le socket d'ecoute
-ajout du pollfd pour le nouveau client au vecteur et du client en lui meme a la map
*/
void    Server::addClient()
{
    sockaddr_in clientAddr;
    socklen_t len = sizeof(clientAddr);
    int clientFd = accept(_listenSocket, (sockaddr*)&clientAddr, &len);

    if (clientFd < 0)
    {
        // ✅ Juste log l'erreur et continue
        std::cerr << "accept error: " << strerror(errno) << std::endl;
        return;  // Return, continue la boucle principale
    }

    if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0)
    {
        close(clientFd);
        std::cerr << "fcntl error on client fd: " << strerror(errno) << std::endl;
        return;
    }

    pollfd clientPfd;
    clientPfd.fd = clientFd;
    clientPfd.events = POLLIN;
    clientPfd.revents = 0;
    _pollfds.push_back(clientPfd);

    _clients[clientFd] = Client(clientFd);
}

/*
lire la data recue via recv
-appel a recv a chaque fois que la boucle pollMonitoring itere dessus
-si il y a de la data, l'ajouter au fur et a mesure au buffer du client
-parser les commandes en extractant les lignes jusqu'a \r\n
on met tout dans un buffer car TCP est un flux continu, un appel a recv peut donner un message partiel comme tout le message
*/
void    Server::readData(int fd)
{
    const size_t kMaxClientBuffer = 8192;
    char buf[512];

    ssize_t bytes = recv(fd, buf, sizeof(buf), 0);
    if (bytes <= 0)
    {
        cleanupClient(fd);
        for (size_t i = 0; i < _pollfds.size(); ++i)
        {
            if (_pollfds[i].fd == fd)
            {
                _pollfds.erase(_pollfds.begin() + i);
                break;
            }
        }
        return;
    }
    else if (bytes > 0)
    {
        _clients[fd].appendToBuffer(std::string(buf, bytes));

        // Protection contre les flux sans fin de ligne qui feraient grossir le buffer indéfiniment.
        if (_clients[fd].getBuffer().size() > kMaxClientBuffer)
        {
            cleanupClient(fd);
            for (size_t i = 0; i < _pollfds.size(); ++i)
            {
                if (_pollfds[i].fd == fd)
                {
                    _pollfds.erase(_pollfds.begin() + i);
                    break;
                }
            }
            return;
        }

        while (hasClient(fd) && _clients[fd].hasLine())
            handleCommand(fd, _clients[fd].extractLine());
    }
}

void    Server::cleanupClient(int fd)
{
    // Retirer de tous les channels et supprimer les channels vides
    std::vector<std::string> toRemove;
    for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        it->second.removeMember(fd);
        if (it->second.getNumberOfMembers() == 0)
            toRemove.push_back(it->first);
    }
    for (size_t i = 0; i < toRemove.size(); i++)
        _channels.erase(toRemove[i]);

    close(fd);
    _clients.erase(fd);
}

// Tous les helpers (cleanLine, toUpperCase, isValidChannel, split) sont maintenant dans Utils.hpp

void Server::handleCommand(int fd, const std::string& line)
{
    // ✅ Pas de CommandResponse, on envoie direct
    
    std::vector<std::string> tokens = split(line);
    if (tokens.empty())
        return;
    std::string command = toUpperCase(tokens[0]);
    std::vector<std::string> params(tokens.begin() + 1, tokens.end());
    
    if (command == "PASS")
        handlePass(fd, params);
    else if (command == "NICK")
        handleNick(fd, params);
    else if (command == "USER")
    {
        handleUser(fd, params);
    }
    else
    {
        if (!_clients[fd].isRegistered())
        {
            sendToClient(fd, err_notregistered());  // ✅ ENVOIE IMMÉDIATEMENT!
            return;
        }
        if (command == "JOIN")
            handleJoin(fd, params);
        else if (command == "PRIVMSG")
            handlePrivmsg(fd, params);
        else if (command == "TOPIC")
            handleTopic(fd, params);
        else if (command == "KICK")
            handleKick(fd, params);
        else if (command == "INVITE")
            handleInvite(fd, params);
        else if (command == "MODE")
            handleMode(fd, params);
        else if (command == "PART")
            handlePart(fd, params);
        else if (command == "QUIT")
            handleQuit(fd, params);
    }
}

void    Server::sendToClient(int fd, const std::string& msg)
{
    send(fd, msg.c_str(), msg.size(), 0);
}

Client& Server::getClient(int fd)
{
    return _clients[fd];
}

bool Server::hasClient(int fd)
{
    return _clients.find(fd) != _clients.end();
}

Channel& Server::getChannel(const std::string& name)
{
    if (_channels.find(name) == _channels.end())
        _channels[name] = Channel(name);  // Crée avec le nom correct
    return _channels[name];
}

Channel* Server::findChannel(const std::string& name)
{
    if (_channels.find(name) == _channels.end())
        return NULL;
    return &_channels[name];
}

int Server::getClientFdByNickname(const std::string& nickname)
{
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second.getNickname() == nickname)
            return it->first;
    }
    return -1;
}

void Server::handleJoin(int fd, std::vector<std::string> params)
{
    Join joinHandler(*this, fd, params);
    joinHandler.execute();
}

void Server::handlePrivmsg(int fd, const std::vector<std::string>& params)
{
    Privmsg privmsgHandler(*this, fd, params);
    privmsgHandler.execute();
}

void Server::handleTopic(int fd, const std::vector<std::string>& params)
{
    Topic topicHandler(*this, fd, params);
    topicHandler.execute();
}

void Server::handleKick(int fd, const std::vector<std::string>& params)
{
    Kick kickHandler(*this, fd, params);
    kickHandler.execute();
}

void Server::handleInvite(int fd, const std::vector<std::string>& params)
{
    Invite inviteHandler(*this, fd, params);
    inviteHandler.execute();
}

void    Server::addChannel(std::string channelName)
{
    _channels.insert(std::make_pair(channelName, Channel(channelName)));
}

// CommandResponse handleCommand(int fd, const std::string& line)
// {
//     CommandResponse response;
    
//     // 1. Parser la ligne en tokens
//     std::vector<std::string> tokens = split(line);
    
//     if (tokens.empty())
//         return response;  // Ligne vide
    
//     // 2. Extraire command et params - NORMALISER EN UPPERCASE
//     std::string command = toUpperCase(tokens[0]);
//     std::vector<std::string> params(tokens.begin() + 1, tokens.end());

//     // ========== AUTHENTICATION ==========
//     if (command == "PASS")
//     {
//         if (params.size() < 1)
//             response.addMessage(fd, err_needmoreparams("PASS"));
//         else
//             response.addMessage(fd, rpl_welcome("*"));  // Utiliser code IRC
//     }
    
//     else if (command == "NICK")
//     {
//         if (params.size() < 1)
//             response.addMessage(fd, err_needmoreparams("NICK"));
//         else if (params[0].empty())
//             response.addMessage(fd, err_needmoreparams("NICK"));
//         else
//             response.addMessage(fd, ":server 200 * :Nickname set\r\n");
//     }
    
//     else if (command == "USER")
//     {
//         if (params.size() < 3)
//             response.addMessage(fd, err_needmoreparams("USER"));
//         else
//             response.addMessage(fd, rpl_welcome(params[0]));
//     }
    
//     // ========== CHANNEL COMMANDS ==========
//     else if (command == "JOIN")
//     {
//         if (params.size() < 1)
//             response.addMessage(fd, err_needmoreparams("JOIN"));
//         else if (!isValidChannel(params[0]))
//             response.addMessage(fd, err_nosuchchannel(params[0]));
//         else
//             response.addMessage(fd, ":server 200 * :Joined channel\r\n");
//     }
    
//     else if (command == "PRIVMSG")
//     {
//         if (params.size() < 2)
//             response.addMessage(fd, err_needmoreparams("PRIVMSG"));
//         else
//         {
//             std::string target = params[0];
//             // Si c'est un channel, vérifier qu'il est valide
//             if (target[0] == '#' || target[0] == '&')
//             {
//                 if (!isValidChannel(target))
//                     response.addMessage(fd, err_nosuchchannel(target));
//                 else
//                     response.addMessage(fd, ":server 200 * :Message sent\r\n");
//             }
//             else
//             {
//                 // C'est un user - accepter pour l'instant
//                 response.addMessage(fd, ":server 200 * :Message sent\r\n");
//             }
//         }
//     }
    
//     else if (command == "TOPIC")
//     {
//         if (params.size() < 1)
//             response.addMessage(fd, err_needmoreparams("TOPIC"));
//         else if (!isValidChannel(params[0]))
//             response.addMessage(fd, err_nosuchchannel(params[0]));
//         else
//             response.addMessage(fd, ":server 200 * :Topic changed\r\n");
//     }
    
//     // ========== OPERATOR COMMANDS ==========
//     else if (command == "MODE")
//     {
//         if (params.size() < 2)
//             response.addMessage(fd, err_needmoreparams("MODE"));
//         else
//         {
//             std::string target = params[0];
//             std::string modes = params[1];
            
//             // Valider format des modes : +/- suivi de lettres
//             if (modes.size() < 2 || (modes[0] != '+' && modes[0] != '-'))
//                 response.addMessage(fd, err_unknownmode(modes));
//             else
//                 response.addMessage(fd, ":server 200 * :Mode changed\r\n");
//         }
//     }
    
//     else if (command == "KICK")
//     {
//         if (params.size() < 2)
//             response.addMessage(fd, err_needmoreparams("KICK"));
//         else if (!isValidChannel(params[0]))
//             response.addMessage(fd, err_nosuchchannel(params[0]));
//         else
//             response.addMessage(fd, ":server 200 * :User kicked\r\n");
//     }
    
//     else if (command == "INVITE")
//     {
//         if (params.size() < 2)
//             response.addMessage(fd, err_needmoreparams("INVITE"));
//         else if (!isValidChannel(params[1]))
//             response.addMessage(fd, err_nosuchchannel(params[1]));
//         else
//             response.addMessage(fd, ":server 200 * :User invited\r\n");
//     }
    
//     // Commandes inconnues : aucune réponse (comportement IRC standard)

//     return response;
// }
