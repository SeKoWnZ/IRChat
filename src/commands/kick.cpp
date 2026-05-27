#include <Ircserv.hpp>

int cmdKick(std::vector<Client> &clients, std::vector<Channel> &channels, Client *client, std::vector<std::string> &args)
{
	std::string command = "KICK";

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

	std::string channel_name = args[0];
	std::string target_nick = args[1];

	Channel* channel = findChannel(channels, channel_name);
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

	if (target->getFd() == client->getFd())
	{
		sendToClient(clients, client->getFd(), "400: You cannot kick yourself\r\n");
		return 0;
	}

	if (!isClientInChannel(channel, target))
	{
		sendError(clients, client->getFd(), "441 " + target->getNickname() + " " + channel->getName(), ":They aren't on that channel");
		return 0;
	}

	std::string reason = "Kicked";
	if (args.size() > 2)
	{
		reason = "";
		for (size_t j = 2; j < args.size(); j++)
		{
			reason += args[j];
			if (j < args.size() - 1)
				reason += " ";
		}
	}

	std::string msg = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost KICK " +
	channel->getName() + " " + target->getNickname() + " :" + reason + "\r\n";
	sendToChannel(clients, channel, -1, msg);
	sendToClient(clients, target->getFd(), msg);

	const std::vector<int>& ch_clients = channel->getClients();
	for (size_t j = 0; j < ch_clients.size(); j++)
	{
		if (ch_clients[j] == target->getFd())
		{
			channel->removeClient(target->getFd());
			break;
		}
	}

	const std::vector<int>& ch_ops = channel->getOperators();
	for (size_t j = 0; j < ch_ops.size(); j++)
	{
		if (ch_ops[j] == target->getFd())
		{
			channel->removeOperator(target->getFd());
			break;
		}
	}

	if (channel->getOperators().empty() && !channel->getClients().empty())
	{
		channel->addOperator(channel->getClients()[0]);

		Client* new_op = findClient(clients, channel->getClients()[0]);
		if (new_op)
		{
			std::string op_msg = new_op->getNickname() + " is now operator\r\n";
			sendToChannel(clients, channel, new_op->getFd(), op_msg);
		}
	}

	if (channel->getClients().empty())
		removeEmptyChannels(channels);
	return 1;
}
