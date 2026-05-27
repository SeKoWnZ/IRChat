#include <Ircserv.hpp>

int cmdJoin(std::vector<Client> &clients, std::vector<Channel> &channels, Client *client, std::vector<std::string> &args)
{
	std::string command = "JOIN";
	if (!client->getRegistered())
	{
		sendError(clients, client->getFd(), "451", ":You have not registered");
		return 0;;
	}

	if (args.size() < 1)
	{
		sendError(clients, client->getFd(), "461 " + command, ":Not enough parameters");
		return 0;;
	}

	std::string channel_name = args[0];
	if (channel_name.empty() || channel_name[0] != '#')
	{
		sendError(clients, client->getFd(), "403", "<" + channel_name +"> :No such channel");
		return 0;;
	}

	Channel *channel = findChannel(channels, channel_name);

	if (!channel)
	{
		Channel new_channel(channel_name);
		channels.push_back(new_channel);
		channel = &channels.back();
		channel->addClient(client->getFd());
		channel->addOperator(client->getFd());

		std::string join_msg =
		":" + client->getNickname() +
		"!" + client->getUsername() +
		"@localhost JOIN " +
		channel_name + "\r\n";
		sendToChannel(clients, channel, -1, join_msg);
		sendToClient(clients, client->getFd(), "You are now operator of " +  channel_name + "\r\n");
		
		if (channel->getTopic().empty())
		{
			sendToClient(clients, client->getFd(), ":ircserv 331 " + client->getNickname() +
			" " + channel->getName() + " :No topic is set\r\n");
		}
		else
		{
			sendToClient(clients, client->getFd(), ":ircserv 332 " + client->getNickname() +
			" " + channel->getName() + " :" + channel->getTopic() + "\r\n");
		}

		std::string user_list;
		CreateUserList(clients, channel, user_list);

		sendToClient(clients, client->getFd(), ":ircserv 353 " + client->getNickname() +
		" = " + channel->getName() + " :" + user_list + "\r\n");
		sendToClient(clients, client->getFd(), ":ircserv 366 " + client->getNickname() + " " +
		channel->getName() + " :End of /NAMES list.\r\n");
	}
	else if (isClientInChannel(channel, client))
		sendToClient(clients, client->getFd(), "you are already on that channel!!!\r\n");
	else if (!channel->getKey().empty() && (args.size() < 2 || args[1] != channel->getKey()))
		sendError(clients, client->getFd(), "475 " + channel->getName(), ":Cannot join channel (+k)");
	else if (channel->getInviteOnly() && !isInvited(channel, client))
		sendError(clients, client->getFd(), "473 " + channel->getName(), ":Cannot join channel (+i)");
	else if (channel->getUserLimit() != -1 && channel->getClients().size() >= (size_t)channel->getUserLimit())
		sendError(clients, client->getFd(), "471 " + channel->getName(), ":Cannot join channel (+l)");
	else
	{
		channel->addClient(client->getFd());
		std::string join_msg =
		":" + client->getNickname() +
		"!" + client->getUsername() +
		"@localhost JOIN " +
		channel_name + "\r\n";

		sendToChannel(clients, channel, -1, join_msg);
		std::string user_list;
		CreateUserList(clients, channel, user_list);
		
		sendToClient(clients, client->getFd(), ":ircserv 353 " + client->getNickname() +
		" = " + channel->getName() + " :" + user_list + "\r\n");
		sendToClient(clients, client->getFd(), ":ircserv 366 " + client->getNickname() + " " +
		channel->getName() + " :End of /NAMES list.\r\n");
		
		if (channel->getInviteOnly())
		{
			const std::vector<int>& invited = channel->getInvited();
			for (size_t j = 0; j < invited.size(); j++)
			{
				if (invited[j] == client->getFd())
				{
					channel->removeInvited(client->getFd());
					break;
				}
			}
		}
	}
	return 1;
}