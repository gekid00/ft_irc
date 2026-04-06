#include "Server.hpp"
#include "Join.hpp"
#include <errno.h>

volatile sig_atomic_t Server::_pollRunning = 1;

/* ===== SIGNAUX ===== */

void Server::setupSignals()
{
	// Intercepter SIGINT et SIGTERM pour arrêter proprement la boucle
	struct sigaction sa;
	sa.sa_handler = Server::signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	// Ignorer SIGPIPE : un send() sur un socket fermé retourne -1 plutôt que de tuer le processus
	signal(SIGPIPE, SIG_IGN);
}

void Server::signalHandler(int num)
{
	(void)num;
	std::cout << "\nserver shutting down" << std::endl;
	_pollRunning = 0;
}

/* ===== CONSTRUCTEUR / DESTRUCTEUR ===== */

Server::Server(const std::string& port, const std::string& password) : _password(password)
{
	_port = std::atoi(port.c_str());
	if (_port <= 1024 || _port > 65535)
		throw std::invalid_argument("port is not between 1024 and 65535");
	setupSocket(_port);
}

Server::~Server()
{
	// Fermer tous les descripteurs de fichiers ouverts
	for (size_t i = 0; i < _pollfds.size(); i++)
		close(_pollfds[i].fd);
	_clients.clear();
	_channels.clear();
}

/* ===== CREATION DU SOCKET D'ECOUTE ===== */
/*
Étapes :
1. Créer le socket TCP IPv4
2. SO_REUSEADDR : permet de relancer le serveur sans attendre la libération du port
3. O_NONBLOCK : accept() ne bloque jamais, même sans connexion entrante
4. Lier le socket à INADDR_ANY (toutes interfaces) sur le port donné
5. Mettre en écoute, puis ajouter à _pollfds avec POLLIN
*/
void Server::setupSocket(int port)
{
	// ETAPE 1: Créer le socket
	_listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenSocket < 0)
		throw std::invalid_argument("unable to create socket");

	// ETAPE 2: Réutiliser le port immédiatement après un arrêt
	int opt = 1;
	if (setsockopt(_listenSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		close(_listenSocket);
		throw std::invalid_argument("unable to set socket options");
	}

	// ETAPE 3: Mode non-bloquant
	if (fcntl(_listenSocket, F_SETFL, O_NONBLOCK) < 0)
	{
		close(_listenSocket);
		throw std::invalid_argument("unable to set non-blocking mode");
	}

	// ETAPE 4: Configurer l'adresse et lier le socket
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

	// ETAPE 5: Ajouter à la liste de surveillance poll()
	pollfd listenPollFd;
	listenPollFd.fd = _listenSocket;
	listenPollFd.events = POLLIN;
	listenPollFd.revents = 0;
	_pollfds.push_back(listenPollFd);
}

/* ===== BOUCLE PRINCIPALE ===== */
/*
poll() surveille tous les fds en même temps sans bloquer :
- POLLIN sur _listenSocket  → nouvelle connexion entrante
- POLLIN sur client fd      → données à lire et traiter
- POLLOUT sur client fd     → buffer de sortie à vider
- POLLHUP/POLLERR/POLLNVAL  → déconnexion ou erreur
*/
void Server::pollMonitoring()
{
	while (_pollRunning)
	{
		int pollReturn = poll(&_pollfds[0], _pollfds.size(), 60000);

		// Interruption par signal : reprendre la boucle
		if (pollReturn < 0)
		{
			if (errno == EINTR)
				continue;
			std::cerr << "error: poll failed" << std::endl;
			break;
		}

		for (size_t i = 0; i < _pollfds.size(); i++)
		{
			int fd = _pollfds[i].fd;
			int revents = _pollfds[i].revents;

			// Socket d'écoute : nouvelle connexion
			if (fd == _listenSocket)
			{
				if (revents & POLLIN)
					addClient();
				continue;
			}

			// Déconnexion ou erreur : nettoyer le client
			if (revents & (POLLHUP | POLLERR | POLLNVAL))
			{
				cleanupClient(fd);
				_pollfds.erase(_pollfds.begin() + i);
				i--;
				continue;
			}

			// Données à lire : traiter les commandes IRC
			if (revents & POLLIN)
			{
				size_t prevSize = _pollfds.size();
				readData(fd);
				// Si readData a supprimé ce fd (déconnexion), ajuster l'index
				if (_pollfds.size() < prevSize)
				{
					i--;
					continue;
				}
			}

			// Buffer de sortie non vide : vider vers le socket
			if (hasClient(fd) && (revents & POLLOUT))
				flushClient(fd);
		}
	}
}

/* ===== ACCEPTER UN NOUVEAU CLIENT ===== */

void Server::addClient()
{
	// Accepter la connexion entrante (variable locale pour ne pas écraser _serverAddr)
	sockaddr_in clientAddr;
	socklen_t len = sizeof(clientAddr);
	int clientFd = accept(_listenSocket, (sockaddr*)&clientAddr, &len);
	if (clientFd < 0)
	{
		std::cerr << "accept error: " << strerror(errno) << std::endl;
		return;
	}

	// Mode non-bloquant sur le socket client
	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0)
	{
		close(clientFd);
		std::cerr << "fcntl error: " << strerror(errno) << std::endl;
		return;
	}

	// Ajouter à poll() et créer l'entrée client
	pollfd clientPfd;
	clientPfd.fd = clientFd;
	clientPfd.events = POLLIN;
	clientPfd.revents = 0;
	_pollfds.push_back(clientPfd);
	_clients[clientFd] = Client(clientFd);
}

/* ===== LIRE ET TRAITER LES DONNEES D'UN CLIENT ===== */
/*
TCP est un flux continu : un recv() peut recevoir un message partiel ou plusieurs d'un coup.
On accumule tout dans un buffer par client, puis on extrait les lignes complètes (\n).
*/
void Server::readData(int fd)
{
	const size_t kMaxClientBuffer = 8192;
	char buf[512];

	ssize_t bytes = recv(fd, buf, sizeof(buf), 0);

	// Déconnexion propre (0) ou erreur : supprimer le client
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

	// Ajouter les données reçues au buffer du client
	_clients[fd].appendToBuffer(std::string(buf, bytes));

	// Protection : buffer trop grand = client malveillant, on déconnecte
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

	// Traiter toutes les lignes complètes disponibles dans le buffer
	// hasClient() vérifie que le client n'a pas été supprimé par une commande précédente (ex: QUIT)
	while (hasClient(fd) && _clients[fd].hasLine())
		handleCommand(fd, _clients[fd].extractLine());
}

/* ===== NETTOYAGE D'UN CLIENT ===== */

void Server::cleanupClient(int fd)
{
	if (!hasClient(fd))
		return;

	// Vider le buffer de sortie avant de fermer (best-effort : envoi immédiat)
	if (_clients[fd].hasOutBuffer())
	{
		const std::string& buf = _clients[fd].getOutBuffer();
		send(fd, buf.c_str(), buf.size(), 0);
	}

	// Retirer le client de tous ses channels et supprimer les channels vides
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

/* ===== BUFFER DE SORTIE (POLLOUT) ===== */

// Active ou désactive POLLOUT sur le fd dans _pollfds
void Server::setPollOut(int fd, bool enable)
{
	for (size_t i = 0; i < _pollfds.size(); i++)
	{
		if (_pollfds[i].fd == fd)
		{
			if (enable)
				_pollfds[i].events |= POLLOUT;
			else
				_pollfds[i].events &= ~POLLOUT;
			return;
		}
	}
}

// Vide le buffer de sortie du client vers son socket (appelé sur POLLOUT)
void Server::flushClient(int fd)
{
	if (!hasClient(fd))
		return;
	Client& client = _clients[fd];
	if (!client.hasOutBuffer())
	{
		setPollOut(fd, false);
		return;
	}
	// Tenter d'envoyer tout le buffer (send() peut envoyer moins que demandé)
	ssize_t sent = send(fd, client.getOutBuffer().c_str(), client.getOutBuffer().size(), 0);
	if (sent > 0)
		client.shiftOutBuffer(static_cast<size_t>(sent));
	// Si le buffer est vidé, désactiver POLLOUT pour ne pas surcharger poll()
	if (!client.hasOutBuffer())
		setPollOut(fd, false);
}

/* ===== ENVOI VERS UN CLIENT ===== */
/*
Au lieu d'appeler send() directement, on met le message dans le buffer de sortie
et on active POLLOUT. poll() signalera quand le socket est prêt à écrire.
Cela permet de ne jamais bloquer, même si le client est suspendu (^Z).
*/
void Server::sendToClient(int fd, const std::string& msg)
{
	if (!hasClient(fd))
		return;
	_clients[fd].appendToOutBuffer(msg);
	setPollOut(fd, true);
}

/* ===== DISPATCH DES COMMANDES IRC ===== */

void Server::handleCommand(int fd, const std::string& line)
{
	// Parser la ligne en tokens (supporte le paramètre trailing ':')
	std::vector<std::string> tokens = split(line);
	if (tokens.empty())
		return;

	std::string command = toUpperCase(tokens[0]);
	std::vector<std::string> params(tokens.begin() + 1, tokens.end());

	// PASS, NICK et USER sont autorisés avant l'enregistrement
	if (command == "PASS")
		handlePass(fd, params);
	else if (command == "NICK")
		handleNick(fd, params);
	else if (command == "USER")
		handleUser(fd, params);
	else
	{
		// Toutes les autres commandes nécessitent d'être enregistré
		if (!_clients[fd].isRegistered())
		{
			sendToClient(fd, err_notregistered());
			return;
		}
		if (command == "JOIN")
			handleJoin(fd, params);
		else if (command == "PART")
			handlePart(fd, params);
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
		else if (command == "QUIT")
			handleQuit(fd, params);
	}
}

/* ===== DELEGATEURS VERS LES CLASSES COMMANDES ===== */

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

/* ===== ACCESSEURS ===== */

Client& Server::getClient(int fd)
{
	return _clients[fd];
}

bool Server::hasClient(int fd)
{
	return _clients.find(fd) != _clients.end();
}

// Retourne le channel (le crée s'il n'existe pas encore)
Channel& Server::getChannel(const std::string& name)
{
	if (_channels.find(name) == _channels.end())
		_channels[name] = Channel(name);
	return _channels[name];
}

// Retourne un pointeur vers le channel, ou NULL s'il n'existe pas
Channel* Server::findChannel(const std::string& name)
{
	if (_channels.find(name) == _channels.end())
		return NULL;
	return &_channels[name];
}

// Recherche un client par son nickname, retourne son fd ou -1
int Server::getClientFdByNickname(const std::string& nickname)
{
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second.getNickname() == nickname)
			return it->first;
	}
	return -1;
}

void Server::addChannel(std::string channelName)
{
	_channels.insert(std::make_pair(channelName, Channel(channelName)));
}

void Server::removeChannel(const std::string& channelName)
{
	_channels.erase(channelName);
}
