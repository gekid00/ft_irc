#include "Channel.hpp"
#include "Server.hpp"

// Constructeur par défaut (requis par std::map)
Channel::Channel() : _name(""), _topic(""), _key(""), _userLimit(0),
	_inviteOnly(false), _topicRestricted(false)
{
}

// Constructeur principal : initialise le channel avec son nom
Channel::Channel(const std::string& name) : _name(name), _topic(""), _key(""),
	_userLimit(0), _inviteOnly(false), _topicRestricted(false)
{
}

Channel::~Channel()
{
}

/* ===== NOM ===== */

const std::string& Channel::getName() const { return _name; }

/* ===== TOPIC ===== */

const std::string& Channel::getTopic() const { return _topic; }

void Channel::setTopic(const std::string& topic) { _topic = topic; }

/* ===== MEMBRES ===== */

void Channel::addMember(int fd) { _members.insert(fd); }

// Retire le membre et ses privilèges (opérateur, invitation)
void Channel::removeMember(int fd)
{
	_members.erase(fd);
	_operators.erase(fd);
	_invited.erase(fd);
}

bool Channel::isMember(int fd) const { return _members.find(fd) != _members.end(); }

int Channel::getNumberOfMembers() const { return _members.size(); }

const std::set<int>& Channel::getMembers() const { return _members; }

/* ===== OPERATEURS ===== */

// Un client ne peut être opérateur que s'il est déjà membre
void Channel::addOperator(int fd)
{
	if (isMember(fd))
		_operators.insert(fd);
}

void Channel::removeOperator(int fd) { _operators.erase(fd); }

bool Channel::isOperator(int fd) const { return _operators.find(fd) != _operators.end(); }

/* ===== INVITES (mode +i) ===== */

void Channel::addInvited(int fd) { _invited.insert(fd); }

void Channel::removeInvited(int fd) { _invited.erase(fd); }

bool Channel::isInvited(int fd) const { return _invited.find(fd) != _invited.end(); }

/* ===== CLE (mode +k) ===== */

void Channel::setKey(const std::string& key) { _key = key; }

const std::string& Channel::getKey() const { return _key; }

bool Channel::isKeyRestricted() const { return !_key.empty(); }

/* ===== LIMITE D'UTILISATEURS (mode +l) ===== */

void Channel::setUserLimit(int limit) { _userLimit = limit; }

int Channel::getUserLimit() const { return _userLimit; }

/* ===== MODES ===== */

bool Channel::isInviteOnly() const { return _inviteOnly; }
void Channel::setInviteOnly(bool flag) { _inviteOnly = flag; }

bool Channel::isTopicRestricted() const { return _topicRestricted; }
void Channel::setTopicRestricted(bool flag) { _topicRestricted = flag; }

/* ===== BROADCAST ===== */
/*
Envoie un message à tous les membres du channel sauf senderFd.
Passe par Server::sendToClient() pour utiliser le buffer de sortie (POLLOUT),
ce qui garantit que le serveur ne bloque pas si un client est suspendu (^Z).
*/
void Channel::broadcastMessage(const std::string& message, int senderFd, Server& server) const
{
	for (std::set<int>::const_iterator it = _members.begin(); it != _members.end(); ++it)
	{
		if (*it != senderFd)
			server.sendToClient(*it, message);
	}
}
