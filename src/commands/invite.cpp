#include <Ircserv.hpp>

int cmdInvite(std::vector<Client> &clients, std::vector<Channel> &channels, Client *client, std::vector<std::string> &args)
{
	std::string command = "INVITE";
	if (!client->getRegistered())
	{
		sendError(clients, client->getFd(), "451", ":You have not registered");
		return 0;
	}
								
	if (args.size() < 2)
	{
		sendError(clients, client->getFd(), "461 " + command, ":Not enough parameters");
		return 0;
	}

	std::string target_nick = args[0];
	std::string channel_name = args[1];
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

	if (!isOperator(channel, client))
	{
		sendError(clients, client->getFd(), "482 " + channel->getName(), ":You're not channel operator");
		return 0;			
	}

	Client* target = findClientByNick(clients, target_nick);

	if (!target)
	{
		sendError(clients, client->getFd(), "401 " + target_nick, ":No such nick");
		return 0;
	}
	
	if (isClientInChannel(channel, target))
	{
		sendError(clients, client->getFd(), "443 " + target->getNickname() + " " + channel->getName(), ":is already on channel");
		return 0;
	}

	if (!isInvited(channel, target))
		channel->addInvited(target->getFd());

	sendToClient(clients, target->getFd(), "You have been invited to " + channel_name + "\r\n");
	sendToClient(clients, client->getFd(), "Invitation sent\r\n");

	return 1;
}

