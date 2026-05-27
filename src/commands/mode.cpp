#include <Ircserv.hpp>

int cmdMode(std::vector<Client> &clients, std::vector<Channel> &channels, Client *client, std::vector<std::string> &args)
{
	std::string command = "MODE";

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
	std::string mode = args[1];

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

	if (mode == "+i")
	{
		channel->setInviteOnly(true);
		std::string msg = ":" + client->getNickname() + "!" + client->getUsername() +
		"@localhost MODE " + channel->getName() + " +i" + "\r\n";
		sendToChannel(clients, channel, -1, msg);
	}
	else if (mode == "-i")
	{
		channel->setInviteOnly(false);
		std::string msg = ":" + client->getNickname() + "!" + client->getUsername() +
		"@localhost MODE " + channel->getName() + " -i" + "\r\n";
		sendToChannel(clients, channel, -1, msg);								
	}

	else if (mode == "+t")
	{
		channel->setTopicProtected(true);
		std::string msg = ":" + client->getNickname() + "!" + client->getUsername() +
		"@localhost MODE " + channel->getName() + " +t" + "\r\n";
		sendToChannel(clients, channel, -1, msg);								
	}
	else if (mode == "-t")
	{
		channel->setTopicProtected(false);
		std::string msg = ":" + client->getNickname() + "!" + client->getUsername() +
		"@localhost MODE " + channel->getName() + " -t" + "\r\n";
		sendToChannel(clients, channel, -1, msg);								
	}

	else if (mode == "+k")
	{
		if (args.size() < 3)
		{
			sendError(clients, client->getFd(), "461 " + command, ":Not enough parameters");
			return 0;
		}
		channel->setKey(args[2]);
		std::string msg = ":" + client->getNickname() + "!" + client->getUsername() +
		"@localhost MODE " + channel->getName() + " +k " + channel->getKey() + "\r\n";
		sendToChannel(clients, channel, -1, msg);								
	}
	else if (mode == "-k")
	{
		channel->setKey("");
		std::string msg = ":" + client->getNickname() + "!" + client->getUsername() +
		"@localhost MODE " + channel->getName() + " -k" + "\r\n";
		sendToChannel(clients, channel, -1, msg);							
	}
	else if (mode == "+l")
	{
		if (args.size() < 3)
		{
			sendError(clients, client->getFd(), "461 " + command, ":Not enough parameters");
			return 0;
		}
		
		int limit;
		if (!isValidLimit(args[2], limit))
		{
			sendError(clients, client->getFd(), "400",": Invalid limit");
			return 0;
		}
		channel->setUserLimit(limit);
		std::stringstream ss;
		ss << limit;

		std::string msg = ":" + client->getNickname() + "!" + client->getUsername() +
		"@localhost MODE " + channel->getName() + " +l " + ss.str() +"\r\n";
		sendToChannel(clients, channel, -1, msg);							
	}
	else if (mode == "-l")
	{
		channel->setUserLimit(-1);
		std::string msg = ":" + client->getNickname() + "!" + client->getUsername() +
		"@localhost MODE " + channel->getName() + " -l" + "\r\n";
		sendToChannel(clients, channel, -1, msg);							
	}
	else if (mode == "+o" || mode == "-o")
	{
		if (args.size() < 3)
		{
			sendError(clients, client->getFd(), "461 " + command, ":Not enough parameters");
			return 0;
		}

		Client* target = findClientByNick(clients, args[2]);
		if (!target)
		{
			sendError(clients, client->getFd(), "401 " + args[2], ":No such nick");
			return 0;
		}

		if (!isClientInChannel(channel, target))
		{
			sendError(clients, client->getFd(), "441 " + target->getNickname() + " " + channel->getName(), ":They aren't on that channel");
			return 0;
		}

		if (mode == "+o" && !isOperator(channel, target))
		{
			channel->addOperator(target->getFd());
			
			std::string msg = ":" + client->getNickname() + "!" + client->getUsername() +
			"@localhost MODE " + channel->getName() + " +o " + target->getNickname() + "\r\n";
			sendToChannel(clients, channel, -1, msg);
		}
		else if (mode == "-o")
		{
			const std::vector<int>& ops = channel->getOperators();

			for (size_t j = 0; j < ops.size(); j++)
			{
				if (ops[j] == target->getFd())
				{
					channel->removeOperator(target->getFd());
					std::string msg = ":" + client->getNickname() + "!" + client->getUsername() +
					"@localhost MODE " + channel->getName() + " -o " + target->getNickname() + "\r\n";
					sendToChannel(clients, channel, -1, msg);										
					sendToChannel(clients, channel, client->getFd(), target->getNickname() + " is no longer operator\r\n");

					if (ops.empty() && !channel->getClients().empty())
					{
						channel->addOperator(channel->getClients()[0]);
						Client* new_op = findClient(clients, channel->getClients()[0]);
						if (new_op)
						{
							std::string msg = ":" + client->getNickname() + "!" + client->getUsername() +
							"@localhost MODE " + channel->getName() + " +o " + new_op->getNickname() + "\r\n";
							sendToChannel(clients, channel, -1, msg);													
							sendToChannel(clients, channel, new_op->getFd(), new_op->getNickname() + " is now operator\r\n");
						}
					}
					break;
				}
			}
		}
	}
	else
	{
		sendError(clients, client->getFd(), "472 " + mode, ":is unknown mode char to me");
	}
	return 1;
}