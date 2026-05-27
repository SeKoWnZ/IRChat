
#include <Ircserv.hpp>

int cmdNick(std::vector<Client> &clients, Client *client, std::vector<std::string> &args)
{
	if (!client->getPass_ok())
	{
		sendError(clients, client->getFd(), "904", ":SASL authentication failed");
		return 0;
	}

	if (args.size() < 1)
	{
		sendError(clients, client->getFd(), "461 ", "NICK:Not enough parameters");
		return 0;
	}

	std::string new_nick = args[0];
	
	if (nickExists(clients, new_nick, client->getFd()))
	{
		sendError(clients, client->getFd(), "433 " + new_nick, ":Nickname is already in use");
		return 0;
	}

	client->setNickname(new_nick);
	
	std::cout << "Nick set to: " << client->getNickname() << std::endl;

	return 1;
}