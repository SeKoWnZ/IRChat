#include <Client.hpp>

Client::Client() : _fd(-1)
{
	_buffer = "";
	_send_buffer = "";
	_nickname = "";
	_username = "";
	_registered = false;
	_pass_ok = false;
	_to_delete = false;
}

Client::Client(int fd) : _fd(fd)
{
	_buffer = "";
	_send_buffer = "";
	_nickname = "";
	_username = "";
	_registered = false;
	_pass_ok = false;
	_to_delete = false;
}

Client::~Client(){}

int Client::getFd() const
{
	return _fd;
}

std::string Client::getNickname() const
{
	return _nickname;
}

std::string Client::getUsername() const
{
	return _username;
}

std::string Client::getBuffer() const
{
	return _buffer;
}

std::string Client::getSend_buffer() const
{
	return _send_buffer;
}

bool Client::getRegistered() const
{
	return _registered;
}

bool Client::getPass_ok() const
{
	return _pass_ok;
}

bool Client::getTo_delete() const
{
	return _to_delete;
}

void Client::setNickname(const std::string& nickname)
{
	_nickname = nickname;
}

void Client::setUsername(const std::string& username)
{
	_username = username;
}

void Client::appendToBuffer(const std::string& data)
{
	_buffer += data;
}

void Client::eraseFromBuffer(size_t n)
{
	if (n <= _buffer.size())
		_buffer.erase(0, n);
	else
		_buffer.clear();
}

void Client::appendToSend_buffer(const std::string& data)
{
	_send_buffer = data;
}

void Client::eraseFromSend_buffer(size_t n)
{
	if (n <= _send_buffer.size())
		_send_buffer.erase(0, n);
	else
		_send_buffer.clear();
}

void Client::setRegistered(bool val)
{
	_registered = val;
}

void Client::setPass_ok(bool val)
{
	_pass_ok = val;
}

void Client::setTo_delete(bool val)
{
	_to_delete = val;
}