#include <Ircserv.hpp>

int cmdUser(std::vector<Client> &clients, Client *client, std::vector<std::string> &args)
{
	if (!client->getPass_ok())
	{
		sendError(clients, client->getFd(), "904", ":SASL authentication failed");
		return 0;							
	}

	if (args.size() < 1)
	{
		sendError(clients, client->getFd(), "461 ", "USER :Not enough parameters");
		return 0;
	}

	client->setUsername(args[0]);

	std::cout << "User set to: " << client->getUsername() << std::endl;
	return 1;
}