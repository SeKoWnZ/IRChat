#include <Ircserv.hpp>

int cmdPass(std::vector<Client> &clients, Client *client, std::vector<std::string> &args, std::string &server_password)
{
	if (args.size() < 1)
	{
		sendError(clients, client->getFd(), "461 ", "PASS:Not enough parameters");
		return 0;
	}

	if (client->getRegistered())
	{
		sendError(clients, client->getFd(), "462", ":You may not reregister");
		return 0;
	}

	if (client->getPass_ok())
	{
		sendError(clients, client->getFd(), "907", ":You have already authenticated using SASL");
		return 0;
	}							

	if (args[0] != server_password)
	{
		sendError(clients, client->getFd(), "464", ":Password incorrect");
		return 0;
	}

	client->setPass_ok(true);
	return 1;
}