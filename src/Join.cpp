#include "Join.hpp"
#include "Server.hpp"
#include "Replies.hpp"

std::string Join::getNickname()
{
    return _server.getClient(_fd).getNickname();
}

Join::Join(Server& server, int fd, const std::vector<std::string>& params) : _server(server), _fd(fd), _params(params)
{

}

Join::~Join()
{

}

int Join::execute()
{
	if (_params.empty())
	{
		_server.sendToClient(_fd, err_needmoreparams(getNickname(), "JOIN"));
		return -1;
	}
	if (parseChannels() < 0)
	{
		_server.sendToClient(_fd, err_needmoreparams(getNickname(), "JOIN"));
		return -1;
	}
	parseKeys();

	for (size_t i = 0; i < _channels_names.size(); i++)
	{
		const std::string& channel_name = _channels_names[i];
		std::string key = "";
		if (i < _keys.size())
			key = _keys[i];
		if (validateChannelName(channel_name) < 0)
		{
			_server.sendToClient(_fd, err_badchanmask(getNickname(), channel_name));
			continue;
		}
		
		if (validateAccess(channel_name, key) < 0)
			continue;
		
		addClientToChannel(channel_name);
		std::string joinMsg = ":" + _server.getClient(_fd).getPrefix() + " JOIN " + channel_name + "\r\n";
		_server.sendToClient(_fd, joinMsg);
		sendJoinResponse(channel_name);
		broadcastJoin(channel_name, joinMsg);
	}
	return 0;
}

int Join::parseChannels()
{
	if (_params.empty())
		return -1;
	std::string channels_str = _params[0];
	_channels_names = split(channels_str, ',');
	if (_channels_names.empty())
		return -1;
	return 0;
}

int Join::parseKeys()
{
    if (_params.size() > 1)
    {
        std::string keys_str = _params[1];
        _keys = split(keys_str, ',');
    }
    return 0;
}

int Join::validateChannelName(const std::string& name)
{
	if (name.empty() || !isValidChannel(name))
		return -1;
	return 0;
}

int Join::validateAccess(const std::string& name, const std::string& key)
{
	Channel& channel = _server.getChannel(name);
	if (channel.isInviteOnly() && !channel.isInvited(_fd))
	{
		_server.sendToClient(_fd, err_inviteonlycha(getNickname(), name));
		return -1;
	}
	if (channel.isKeyRestricted() && channel.getKey() != key)
	{
		_server.sendToClient(_fd, err_badchannelkey(getNickname(), name));
		return -1;
	}
	if (channel.getUserLimit() > 0 && channel.getNumberOfMembers() >= channel.getUserLimit())
	{
		_server.sendToClient(_fd, err_channelisfull(getNickname(), name));
		return -1;
	}
	return 0;
}

void Join::addClientToChannel(const std::string& name)
{
	Channel& channel = _server.getChannel(name);
	if (!channel.isMember(_fd))
		channel.addMember(_fd);
	if (channel.getNumberOfMembers() == 1)
		channel.addOperator(_fd);
}

void Join::sendJoinResponse(const std::string& name)
{
	Channel& channel = _server.getChannel(name);
	std::string names_list;
	const std::set<int>& members = channel.getMembers();
	
	for (std::set<int>::const_iterator it = members.begin(); it != members.end(); ++it)
	{
		int memberFd = *it;
		if (!_server.hasClient(memberFd))
			continue;
		if (!names_list.empty())
			names_list += " ";
		if (channel.isOperator(memberFd))
			names_list += "@";
		names_list += _server.getClient(memberFd).getNickname();
	}
	
	_server.sendToClient(_fd, rpl_namreply(getNickname(), name, names_list));
	_server.sendToClient(_fd, rpl_endofnames(getNickname(), name));
}

void Join::broadcastJoin(const std::string& channel_name, const std::string& message)
{
	Channel& channel = _server.getChannel(channel_name);
	const std::set<int>& members = channel.getMembers();
	for (std::set<int>::const_iterator it = members.begin(); it != members.end(); ++it)
	{
		int memberFd = *it;
		if (memberFd != _fd && _server.hasClient(memberFd))
			_server.sendToClient(memberFd, message);
	}
}