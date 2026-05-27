#include <Ircserv.hpp>

int cmdPart(std::vector<Client> &clients, std::vector<Channel> &channels, Client *client, std::vector<std::string> &args)
{
	std::string command = "PART";

	if (!client->getRegistered())
	{
		sendError(clients, client->getFd(), "451", ":You have not registered");
		return 0;
	}

	if (args.size() < 1)
	{
		sendError(clients, client->getFd(), "461 " + command, ":Not enough parameters");
		return 0;
	}
	std::string channel_name = args[0];
	Channel *channel = findChannel(channels, channel_name);
	
	if (!channel)
	{
		sendError(clients, client->getFd(), "403 " + channel_name, ":No such channel");
		return 0;
	}
	
	if (!isClientInChannel(channel, client))
	{
		sendError(clients, client->getFd(), "442 " + channel->getName(), ":You're not on that channel");
		return 0;
	}
	
	const std::vector<int>& ch_clients = channel->getClients();
	for (size_t j = 0; j < ch_clients.size(); ++j)
	{
		if (ch_clients[j] == client->getFd())
		{
			std::string msg = client->getNickname() + " left " + channel_name + "\r\n";
			sendToChannel(clients, channel, client->getFd(), msg);
			channel->removeClient(client->getFd());
			break;
		}
	}
	const std::vector<int>& ch_operators = channel->getOperators();
	for (size_t j = 0; j < ch_operators.size(); ++j)
	{
		if (ch_operators[j] == client->getFd())
		{
			channel->removeOperator(client->getFd());
			break;
		}
	}
	if (channel->getOperators().empty() && !channel->getClients().empty())
	{
		channel->addOperator(channel->getClients()[0]);
		std::string msg = findClient(clients, channel->getClients()[0])->getNickname() + " is now operator\r\n";
		sendToChannel(clients, channel,client->getFd(), msg);
	}
	if (channel->getClients().empty())
		removeEmptyChannels(channels);
	return 1;
}
