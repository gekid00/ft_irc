#include "Channel.hpp"
#include <sys/socket.h>

Channel::Channel() : _name(""), _topic(""), _members(), _operators(), _invited(), _key(""), _userLimit(0), _inviteOnly(false), _topicRestricted(false)
{

}

Channel::Channel(const std::string& name) : _name(name), _topic(""), _members(), _operators(), _invited(), _key(""), _userLimit(0), _inviteOnly(false), _topicRestricted(false)
{

}

Channel::~Channel()
{

}

const std::string& Channel::getName() const
{
	return _name;
}

const std::string& Channel::getTopic() const
{
	return _topic;
}

void Channel::setTopic(const std::string& topic)
{
	_topic = topic;
}

void Channel::addMember(int fd)
{
	_members.insert(fd);
}

void Channel::removeMember(int fd)
{
	_members.erase(fd);
	_operators.erase(fd);
	_invited.erase(fd);
}

bool Channel::isMember(int fd) const
{
	return _members.find(fd) != _members.end();
}

int Channel::getNumberOfMembers() const
{
	return _members.size();
}

const std::set<int>& Channel::getMembers() const
{
	return _members;
}

void Channel::addOperator(int fd)
{
	if (isMember(fd))
		_operators.insert(fd);
}

void Channel::removeOperator(int fd)
{
	_operators.erase(fd);
}

bool Channel::isOperator(int fd) const
{
	return _operators.find(fd) != _operators.end();
}

void Channel::addInvited(int fd)
{
	_invited.insert(fd);
}

void Channel::removeInvited(int fd)
{
	_invited.erase(fd);
}

bool Channel::isInvited(int fd) const
{
	return _invited.find(fd) != _invited.end();
}

void Channel::setKey(const std::string& key)
{
	_key = key;
}

const std::string& Channel::getKey() const
{
	return _key;
}

bool Channel::isKeyRestricted() const
{
	return !_key.empty();
}

void Channel::setUserLimit(int limit)
{
	_userLimit = limit;
}

int Channel::getUserLimit() const
{
	return _userLimit;
}

bool Channel::isInviteOnly() const
{
	return _inviteOnly;
}

void Channel::setInviteOnly(bool flag)
{
	_inviteOnly = flag;
}

bool Channel::isTopicRestricted() const
{
	return _topicRestricted;
}

void Channel::setTopicRestricted(bool flag)
{
	_topicRestricted = flag;
}

void Channel::broadcastMessage(const std::string& message, int senderFd) const
{
    for (std::set<int>::const_iterator it = _members.begin(); it != _members.end(); ++it)
    {
        int memberFd = *it;
        if (memberFd != senderFd)
        {
            send(memberFd, message.c_str(), message.size(), 0);
        }
    }
}
