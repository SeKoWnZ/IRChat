#include <Channel.hpp>

Channel::Channel() :
	_name(""), 
	_topic(""),
	_key(""),
	_user_limit(-1),
	_invite_only(false),
	_topic_protected(false)
{}

Channel::Channel(const std::string name) :
	_name(name), 
	_topic(""),
	_key(""),
	_user_limit(-1),
	_invite_only(false),
	_topic_protected(false)
{}

Channel::~Channel(){}

Channel::Channel(const Channel& other) :
	_name(other._name),
	_topic(other._topic),
	_key(other._key),
	_user_limit(other._user_limit),
	_clients(other._clients),
	_operators(other._operators),
	_invited(other._invited),
	_invite_only(other._invite_only),
	_topic_protected(other._topic_protected)
{}

Channel& Channel::operator=(const Channel& other)
{
	if (this != &other)
	{
		const_cast<std::string&>(_name) = other._name;
		_topic = other._topic;
		_key = other._key;
		_user_limit = other._user_limit;
		_clients = other._clients;
		_operators = other._operators;
		_invited = other._invited;
		_invite_only = other._invite_only;
		_topic_protected = other._topic_protected;
	}
	return *this;
}

std::string Channel::getName() const
{
	return _name;
}

std::string Channel::getTopic() const
{
	return _topic;
}

std::string Channel::getKey() const
{
	return _key;
}

int Channel::getUserLimit() const
{
	return _user_limit;
}

const std::vector<int>& Channel::getClients() const
{
	return _clients;
}

const std::vector<int>& Channel::getOperators() const
{
	return _operators;
}

const std::vector<int>& Channel::getInvited() const
{
	return _invited;
}

bool Channel::getInviteOnly() const
{
	return _invite_only;
}

bool Channel::getTopicProtected() const
{
	return _topic_protected;
}

void Channel::setTopic(const std::string topic)
{
	_topic = topic;
}

void Channel::setKey(const std::string key)
{
	 _key = key;
}

void Channel::setUserLimit(const int user_limit)
{
	_user_limit = user_limit;
}

void Channel::addClient(const int client_fd)
{
	_clients.push_back(client_fd);
}

void Channel::removeClient(const int client_fd)
{
	_clients.erase(std::remove(_clients.begin(), _clients.end(), client_fd), _clients.end());
}

void Channel::addOperator(const int client_fd)
{
	_operators.push_back(client_fd);
}

void Channel::removeOperator(const int client_fd)
{
	_operators.erase(std::remove(_operators.begin(), _operators.end(), client_fd), _operators.end());
}

void Channel::addInvited(const int client_fd)
{
	_invited.push_back(client_fd);
}

void Channel::removeInvited(const int client_fd)
{
	_invited.erase(std::remove(_invited.begin(), _invited.end(), client_fd), _invited.end());
}

void Channel::setInviteOnly(const bool invite_only)
{
	_invite_only = invite_only;
}

void Channel::setTopicProtected(const bool topic_protected)
{
	_topic_protected = topic_protected;
}


// ░█▀█▀█ █▀▀░░▄▀█ █░█ ▀ █▀▀ █▀▀▄ ▄▀▄░
// ░░▒█░░ █▀▀░░█░█ █░█ █ █▀▀ █▐█▀ █░█░
// ░▒▄█▄░ ▀▀▀░░░▀█ ░▀░ ▀ ▀▀▀ ▀░▀▀ ░▀░░
// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
// ▒▐██▄▒▄██▌▒█▒█▒▐█▀█▒▐█▒▐█▒​▐█▀▀█▌
// ░▒█░▒█░▒█░▒█▒█▒▐█░░▒▐████▒​▐█▄▒█▌
// ▒▐█░░░░▒█▌▒▀▄▀▒▐█▄█▒▐█▒▐█▒​▐██▄█▌
