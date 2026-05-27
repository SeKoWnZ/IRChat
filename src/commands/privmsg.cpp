#include <Ircserv.hpp>

int cmdPrivmsg(std::vector<Client> &clients, std::vector<Channel> &channels, Client *client, std::vector<std::string> &args)
{
	if (!client->getRegistered())
	{
		sendError(clients, client->getFd(), "451", ":You have not registered");
		return 0;
	}

	if (args.size() < 2)
	{
		sendError(clients, client->getFd(), "461 ", "USER:Not enough parameters");
		return 0;
	}

	std::string target = args[0];
	std::string msg;
	for (size_t j = 1; j < args.size(); ++j)
	{
		msg += args[j];
		if (j < args.size() - 1)
			msg += " ";
	}
	std::string full_msg = ":" + client->getNickname() + "!" + client->getUsername() +
	"@localhost PRIVMSG "  + target + " :" + msg + "\r\n";

	if (target[0] == '#')
	{
		Channel* channel = findChannel(channels, target);
		if (!channel)
		{
			sendError(clients, client->getFd(), "403 " + target, ":No such channel");
			return 0;
		}
		if (!isClientInChannel(channel, client))
		{
			sendError(clients, client->getFd(), "442 " + channel->getName(), ":You're not on that channel");
			return 0;
		}
		sendToChannel(clients, channel, client->getFd(), full_msg);
	}

	else
	{
		Client* target_client = findClientByNick(clients, target);
		if (!target_client)
		{
			sendError(clients, client->getFd(), "401 " + target, ":No such nick");
			return 0;
		}
		sendToClient(clients, target_client->getFd(), full_msg);
	}
	return 1;
}
