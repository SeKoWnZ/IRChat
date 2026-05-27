#include <Ircserv.hpp>

bool isClientInChannel(Channel* channel, Client* client)
{
	for (size_t i = 0; i < channel->getClients().size(); ++i)
	{
		if (channel->getClients()[i] == client->getFd())
			return true;
		}
	return false;
}

bool isOperator(Channel* channel, Client* client)
{
	for (size_t i = 0; i < channel->getOperators().size(); i++)
	{
		if (channel->getOperators()[i] == client->getFd())
		return true;
	}
	return false;
}

bool isValidLimit(const std::string& str, int& limit)
{
	if (str.empty())
		return false;

	for (size_t i = 0; i < str.size(); i++)
	{
			if (!isdigit(str[i]))
				return false;
	}

	std::istringstream iss(str);
	iss >> limit;

	if (limit <= 0)
		return false;

	return true;
}

bool isInvited(Channel* channel, Client* client)
{
		for (size_t i = 0; i < channel->getInvited().size(); i++)
		{
			if (channel->getInvited()[i] == client->getFd())
				return true;
		}
		return false;
}

bool nickExists(std::vector<Client>& clients, const std::string& nick, int exclude_fd)
{
	for (size_t i = 0; i < clients.size(); i++)
	{
		if (clients[i].getNickname() == nick && clients[i].getFd() != exclude_fd)
			return true;
	}
	return false;
}

void	removeEmptyChannels(std::vector<Channel>& channels)
{
	for (size_t i = 0; i < channels.size(); i++)
	{
		if (channels[i].getClients().empty())
		{
			channels.erase(channels.begin() + i);
			i--;
		}
	}
}

Client* findClient(std::vector<Client>& clients, int fd)
{
	for (size_t i = 0; i < clients.size(); ++i)
	{
		if (clients[i].getFd() == fd)
			return &clients[i];
	}
	return NULL;
}

Client* findClientByNick(std::vector<Client>& clients, const std::string& nick)
{
	for (size_t i = 0; i < clients.size(); ++i)
	{
		if (clients[i].getNickname() == nick)
			return &clients[i];
	}
	return NULL;
}

std::vector<Client>::iterator findClientIt(std::vector<Client>& clients, int fd)
{
	for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); it++)
	{
		if (it->getFd() == fd)
			return it;
	}
	return clients.end();
}

Channel* findChannel(std::vector<Channel>& channels, const std::string& name)
{
	for (size_t i = 0; i < channels.size(); ++i)
	{
		if (channels[i].getName() == name)
		return &channels[i];
	}
	return NULL;
}

bool sendToClient(std::vector<Client>& clients, int fd, const std::string& msg)
{
	Client* client = findClient(clients, fd);
	if (!client)
		return false;

	if (!client->getSend_buffer().empty())
	{
		client->getSend_buffer() += msg;
		return true;
	}

	ssize_t sent = send(fd, msg.c_str(), msg.size(), MSG_DONTWAIT);

	if (sent < 0)
	{
		client->getSend_buffer() = msg;
		return true;
	}

	if (sent == 0)
		return false;

	if (sent < (ssize_t)msg.size())
	{
		client->getSend_buffer() = msg.substr(sent);
	}

	return true;
}

void sendToAll(std::vector<Client>& clients, int sender_fd, const std::string& msg)
{
	for (size_t i = 0; i < clients.size(); ++i)
	{
		if (clients[i].getFd() != sender_fd && clients[i].getRegistered())
		sendToClient(clients, clients[i].getFd(), msg);
	}
}

void sendToChannel(std::vector<Client>& clients, Channel* channel, int sender, const std::string& msg)
{
	for (size_t i = 0; i < channel->getClients().size(); ++i)
	{
		if (channel->getClients()[i] != sender)
			sendToClient(clients, channel->getClients()[i], msg);
	}
}

void sendToAllChannels(std::vector<Client>& clients, std::vector<Channel>& channel_list, Client* sender, const std::string& msg)
{
	for (size_t i = 0; i < channel_list.size(); i++)
	{
		if (isClientInChannel(&channel_list[i], sender))
		{
			sendToChannel(clients, &channel_list[i], sender->getFd(), msg);
		}
	}
}

void sendError(std::vector<Client>& clients, int fd, const std::string& code, const std::string& msg)
{
	sendToClient(clients, fd, code + " " + msg + "\r\n");	
}

void	removeClientFromAllChannels(std::vector<Client>& clients, std::vector<Channel>& channels, int client)
{
	for (size_t i = 0; i < channels.size(); i++)
	{
		const std::vector<int>& ch_clients = channels[i].getClients();
		for (size_t j = 0; j < ch_clients.size(); j++)
		{
			if (ch_clients[j] == client)
			{
				channels[i].removeClient(client);
				break;
			}
		}
		const std::vector<int>& ch_operators = channels[i].getOperators();
		for (size_t j = 0; j < ch_operators.size(); j++)
		{
			if (ch_operators[j] == client)
			{
				channels[i].removeOperator(client);
				if (channels[i].getOperators().empty() && !channels[i].getClients().empty())
				{
					channels[i].addOperator(channels[i].getClients()[0]);
					Client *new_op = findClient(clients, channels[i].getClients()[0]);
					if (new_op)
					{
						std::string msg = new_op->getNickname() + " is now operator\n";
						sendToChannel(clients, &channels[i], client, msg);
					}
				}
				break;
			}
		}
	}
}

bool isValidPort(const std::string& str, int& port)
{
    if (str.empty())
        return false;

    for (size_t i = 0; i < str.size(); i++)
    {
        if (!isdigit(str[i]))
            return false;
    }

    std::istringstream iss(str);
    iss >> port;

    if (port < 1024 || port > 65535)
        return false;

    return true;
}

void CreateUserList(std::vector<Client>& clients, Channel* channel, std::string& user_list)
{


	for (size_t j = 0; j < channel->getClients().size(); j++)
	{
		Client* ch_client = findClient(clients, channel->getClients()[j]);

		if (ch_client)
		{
			if (isOperator(channel, ch_client))
				user_list += "@";

			user_list += ch_client->getNickname();

			if (j < channel->getClients().size() - 1)
				user_list += " ";
		}
	}
}

void parseCommand(const std::string& line, std::string& command, std::vector<std::string>& args)
{
	args.clear();

	std::istringstream iss(line);

	iss >> command;

	for (size_t i = 0; i < command.size(); i++)
		command[i] = toupper(command[i]);

	std::string token;

	while (iss >> token)
	{
		if (token[0] == ':')
		{
			token.erase(0, 1);

			std::string trailing;

			std::getline(iss, trailing);

			token += trailing;

			args.push_back(token);

			break;
		}

		args.push_back(token);
	}
}
