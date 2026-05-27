#include <Ircserv.hpp>

int cmdTopic(std::vector<Client> &clients, std::vector<Channel> &channels, Client *client, std::vector<std::string> &args)
{
	std::string command = "TOPIC";
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
	
	if (isClientInChannel(channel, client))
	{
		if (args.size() == 1)
		{
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
		}
		
		else if (args.size() > 1)
		{
			if (!channel->getTopicProtected() || isOperator(channel, client))
			{
				std::string topic;
				for (size_t j = 1; j < args.size(); j++)
				{
					topic += args[j];
					if (j < args.size() - 1)
						topic += " ";
				}

				channel->setTopic(topic);

				std::string msg = ":" + client->getNickname() + "!" + client->getUsername() + "@localhost TOPIC " +
				channel->getName() + " :" + topic + "\r\n";
				sendToChannel(clients, channel, -1, msg);

				std::cout << "Nuevo topic en " << channel_name << ": " << topic << std::endl;
			}
			else
			{
				sendError(clients, client->getFd(), "482 " + channel->getName(), ":You're not channel operator");
			}
		}
	}
	else
	{
		sendError(clients, client->getFd(), "442 " + channel->getName(), ":You're not on that channel");
	}
	return 1;
}
