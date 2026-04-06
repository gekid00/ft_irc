#include "Client.hpp"

// Constructeur par défaut (utilisé par std::map lors d'une insertion implicite)
Client::Client() : _fd(-1), _nickname(""), _username(""), _realname(""),
	_passwordOk(false), _registered(false), _buffer(""), _outBuffer("")
{
}

// Constructeur normal : associe le client à son descripteur de fichier
Client::Client(int fd) : _fd(fd), _nickname(""), _username(""), _realname(""),
	_passwordOk(false), _registered(false), _buffer(""), _outBuffer("")
{
}

Client::~Client()
{
}

/* ===== IDENTITE ===== */

int Client::setNickname(const std::string& nickname)
{
	if (nickname.empty())
		return -1;
	_nickname = nickname;
	return 0;
}

std::string Client::getNickname() const { return _nickname; }

int Client::setUsername(const std::string& username)
{
	if (username.empty())
		return -1;
	_username = username;
	return 0;
}

std::string Client::getUsername() const { return _username; }

int Client::setRealname(const std::string& realname)
{
	_realname = realname;
	return 0;
}

std::string Client::getRealname() const { return _realname; }

int Client::getFd() const { return _fd; }

// Préfixe IRC standard au format nick!user@host
std::string Client::getPrefix() const
{
	return _nickname + "!" + _username + "@server";
}

/* ===== AUTHENTIFICATION ===== */

int Client::setPasswordOk(bool ok)
{
	_passwordOk = ok;
	return 0;
}

bool Client::isPasswordOk() const { return _passwordOk; }
bool Client::isRegistered() const { return _registered; }

// Enregistre le client si PASS + NICK + USER sont tous fournis
bool Client::tryRegister()
{
	if (!_nickname.empty() && !_username.empty() && _passwordOk)
	{
		_registered = true;
		return true;
	}
	return false;
}

/* ===== BUFFER D'ENTREE ===== */

// Ajoute les données reçues à la fin du buffer
void Client::appendToBuffer(const std::string& data)
{
	_buffer += data;
}

std::string Client::getBuffer() const { return _buffer; }

// Vérifie si une ligne complète (\n) est disponible dans le buffer
bool Client::hasLine() const
{
	return _buffer.find("\n") != std::string::npos;
}

// Extrait la première ligne du buffer et la retourne sans \r\n
std::string Client::extractLine()
{
	size_t pos = _buffer.find("\n");
	if (pos == std::string::npos)
		return "";
	std::string line = _buffer.substr(0, pos);
	// Supprimer le \r si présent (support \r\n et \n)
	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);
	_buffer.erase(0, pos + 1);
	return line;
}

/* ===== BUFFER DE SORTIE (POLLOUT) ===== */

// Ajoute un message au buffer de sortie en attente d'envoi
void Client::appendToOutBuffer(const std::string& data)
{
	_outBuffer += data;
}

const std::string& Client::getOutBuffer() const { return _outBuffer; }

// Supprime les n premiers octets du buffer après un send() réussi
void Client::shiftOutBuffer(size_t n)
{
	_outBuffer.erase(0, n);
}

// Indique si des données sont encore en attente d'envoi
bool Client::hasOutBuffer() const
{
	return !_outBuffer.empty();
}
