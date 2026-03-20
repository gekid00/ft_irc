#include "Client.hpp"

Client::Client() : _fd(-1), _nickname(""), _username(""), _passwordOk(false), _registered(false), _buffer("")
{

}

Client::Client(int fd) : _fd(fd), _nickname(""), _username(""), _passwordOk(false), _registered(false), _buffer("")
{

}

Client::~Client()
{

}

int Client::setNickname(const std::string& nickname)
{
	if (nickname.empty())
		return -1;
	_nickname = nickname;
	return 0;
}

std::string Client::getNickname() const
{
	return _nickname;
}

int Client::setUsername(const std::string& username)
{
	if (username.empty())
		return -1;
	_username = username;
	return 0;
}

std::string Client::getUsername() const
{
	return _username;
}

int Client::setRealname(const std::string& realname)
{
	_realname = realname;
	return 0;
}

std::string Client::getRealname() const
{
	return _realname;
}

int Client::setPasswordOk(bool Ok)
{
	_passwordOk = Ok;
	return 0;
}

bool Client::isPasswordOk() const
{
	return _passwordOk;
}

int Client::getFd() const
{
	return _fd;
}

void Client::appendToBuffer(const std::string& data)
{
	_buffer += data;
}

std::string Client::getBuffer() const
{
	return _buffer;
}

bool Client::hasLine() const
{
	return _buffer.find("\n") != std::string::npos;
}

std::string Client::extractLine()
{
	size_t pos = _buffer.find("\n");
	if (pos == std::string::npos)
		return "";
	std::string line = _buffer.substr(0, pos);
	// Retirer le \r si présent (support \r\n et \n)
	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);
	_buffer.erase(0, pos + 1);
	return line;
}

bool Client::isRegistered() const
{
	return _registered;
}

bool Client::tryRegister()
{
	if (!_nickname.empty() && !_username.empty() && _passwordOk)
	{
		_registered = true;
		return true;
	}
	return false;
}

std::string Client::getPrefix() const
{
	return _nickname + "!" + _username + "@server";
}
