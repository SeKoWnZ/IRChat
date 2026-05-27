#include <Ircserv.hpp>

int cmdQuit(std::vector<Client> &clients, std::vector<Channel> &channels, Client *client, std::vector<std::string> &args)
{
	client->setTo_delete(true);
	std::string msg;
	if (args.size() < 1)
	{
		msg = ":" + client->getNickname() + "!" + client->getUsername() +
		"@host QUIT :Quit\r\n";
	}
	else
	{
		std::string reason = "";
		for (size_t j = 0; j < args.size(); j++)
		{
			reason += args[j];
			if (j < args.size() - 1)
				reason += " ";
		}
		msg = ":" + client->getNickname() + "!" + client->getUsername() +
		"@host QUIT :" + reason + "\r\n";								
	}								
	sendToAllChannels(clients, channels, client, msg);
	removeClientFromAllChannels(clients, channels, client->getFd());

	return 1;
}
