// TE QUIEROOOOOOOOOOO
// Y YO A TUUUUU MUXOS MUXOTEEEEEEEEEEEEEEEEEEEEEEESSSSSSSSS, TA QUEAO CLARO?

// WHILE(1)
// 	TE QUIERO MUCHO;
	
// ░░░░░░░░░░░░░░░░▄▓▄░░░
// ░░░░▄█▄░░░░░░░░▄▓▓▓▄░░
// ░░▄█████▄░░░░░▄▓▓▓▓▓▄░
// ░▀██┼█┼██▀░░░▄▓▓▓▓▓▓▓▄
// ▄▄███████▄▄▄▄▄▄▄▄█▄▄▄▄ 

// bonica
// te loveu muxo yo ma   

#include <Ircserv.hpp>


bool isClientInChannel(Channel* channel, Client* client)
{
	for (size_t i = 0; i < channel->getClients().size(); ++i)
	{
		if (channel->getClients()[i] == client->getFd())
			return true;
		}
	return false;
}

bool isOperator(Channel* channel, Client* client)
{
	for (size_t i = 0; i < channel->getOperators().size(); i++)
	{
		if (channel->getOperators()[i] == client->getFd())
		return true;
	}
	return false;
}

bool isValidLimit(const std::string& str, int& limit)
{
	if (str.empty())
		return false;

	for (size_t i = 0; i < str.size(); i++)
	{
			if (!isdigit(str[i]))
				return false;
	}

	std::istringstream iss(str);
	iss >> limit;

	if (limit <= 0)
		return false;

	return true;
}

bool isInvited(Channel* channel, Client* client)
{
		for (size_t i = 0; i < channel->getInvited().size(); i++)
		{
			if (channel->getInvited()[i] == client->getFd())
				return true;
		}
		return false;
}

bool nickExists(std::vector<Client>& clients, const std::string& nick, int exclude_fd)
{
	for (size_t i = 0; i < clients.size(); i++)
	{
		if (clients[i].getNickname() == nick && clients[i].getFd() != exclude_fd)
			return true;
	}
	return false;
}

void	removeEmptyChannels(std::vector<Channel>& channels)
{
	for (size_t i = 0; i < channels.size(); i++)
	{
		if (channels[i].getClients().empty())
		{
			std::cout << "Canal eliminado: " << channels[i].getName() << std::endl;
			channels.erase(channels.begin() + i);
			i--;
		}
	}
}

Client* findClient(std::vector<Client>& clients, int fd)
{
	for (size_t i = 0; i < clients.size(); ++i)
	{
		if (clients[i].getFd() == fd)
			return &clients[i];
	}
	return NULL;
}

Client* findClientByNick(std::vector<Client>& clients, const std::string& nick)
{
	for (size_t i = 0; i < clients.size(); ++i)
	{
		if (clients[i].getNickname() == nick)
			return &clients[i];
	}
	return NULL;
}

std::vector<Client>::iterator findClientIt(std::vector<Client>& clients, int fd)
{
	for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); it++)
	{
		if (it->getFd() == fd)
			return it;
	}
	return clients.end();
}

Channel* findChannel(std::vector<Channel>& channels, const std::string& name)
{
	for (size_t i = 0; i < channels.size(); ++i)
	{
		if (channels[i].getName() == name)
		return &channels[i];
	}
	return NULL;
}

bool sendToClient(std::vector<Client>& clients, int fd, const std::string& msg)
{
	Client* client = findClient(clients, fd);
	if (!client)
		return false;

	// Si ya hay datos pendientes, añadir a la cola
	if (!client->getSend_buffer().empty())
	{
		client->getSend_buffer() += msg;
		return true;
	}

	ssize_t sent = send(fd, msg.c_str(), msg.size(), MSG_DONTWAIT);

	if (sent < 0)
	{
		// No se pudo enviar, guardar todo en buffer
		client->getSend_buffer() = msg;
		return true;
	}

	if (sent == 0)
		return false;

	if (sent < (ssize_t)msg.size())
	{
		// Se envió parcialmente, guardar el resto
		client->getSend_buffer() = msg.substr(sent);
	}

	return true;
}

void sendToAll(std::vector<Client>& clients, int sender_fd, const std::string& msg)
{
	for (size_t i = 0; i < clients.size(); ++i)
	{
		if (clients[i].getFd() != sender_fd && clients[i].getRegistered())
		sendToClient(clients, clients[i].getFd(), msg);
	}
}

void sendToChannel(std::vector<Client>& clients, Channel* channel, int sender, const std::string& msg)
{
	for (size_t i = 0; i < channel->getClients().size(); ++i)
	{
		if (channel->getClients()[i] != sender)
			sendToClient(clients, channel->getClients()[i], msg);
	}
}

void sendToAllChannels(std::vector<Client>& clients, std::vector<Channel>& channel_list, Client* sender, const std::string& msg)
{
	for (size_t i = 0; i < channel_list.size(); i++)
	{
		if (isClientInChannel(&channel_list[i], sender))
		{
			sendToChannel(clients, &channel_list[i], sender->getFd(), msg);
		}
	}
}

void sendError(std::vector<Client>& clients, int fd, const std::string& code, const std::string& msg)
{
	sendToClient(clients, fd, code + " " + msg + "\r\n");	
}

void	removeClientFromAllChannels(std::vector<Client>& clients, std::vector<Channel>& channels, int client)
{
	for (size_t i = 0; i < channels.size(); i++)
	{
		const std::vector<int>& ch_clients = channels[i].getClients();
		for (size_t j = 0; j < ch_clients.size(); j++)
		{
			if (ch_clients[j] == client)
			{
				channels[i].removeClient(client);
				break;
			}
		}
		const std::vector<int>& ch_operators = channels[i].getOperators();
		for (size_t j = 0; j < ch_operators.size(); j++)
		{
			if (ch_operators[j] == client)
			{
				channels[i].removeOperator(client);
				if (channels[i].getOperators().empty() && !channels[i].getClients().empty())
				{
					channels[i].addOperator(channels[i].getClients()[0]);
					Client *new_op = findClient(clients, channels[i].getClients()[0]);
					if (new_op)
					{
						std::string msg = new_op->getNickname() + " is now operator\n";
						sendToChannel(clients, &channels[i], client, msg);
					}
				}
				break;
			}
		}
	}
}

bool isValidPort(const std::string& str, int& port)
{
    if (str.empty())
        return false;

    for (size_t i = 0; i < str.size(); i++)
    {
        if (!isdigit(str[i]))
            return false;
    }

    std::istringstream iss(str);
    iss >> port;

    if (port < 1024 || port > 65535)
        return false;

    return true;
}

void CreateUserList(std::vector<Client>& clients, Channel* channel, std::string& user_list)
{


	for (size_t j = 0; j < channel->getClients().size(); j++)
	{
		Client* ch_client = findClient(clients, channel->getClients()[j]);

		if (ch_client)
		{
			if (isOperator(channel, ch_client))
				user_list += "@";

			user_list += ch_client->getNickname();

			if (j < channel->getClients().size() - 1)
				user_list += " ";
		}
	}
}

void parseCommand(const std::string& line, std::string& command, std::vector<std::string>& args)
{
	args.clear();

	std::istringstream iss(line);

	iss >> command;

	for (size_t i = 0; i < command.size(); i++)
		command[i] = toupper(command[i]);

	std::string token;

	while (iss >> token)
	{
		if (token[0] == ':')
		{
			token.erase(0, 1);

			std::string trailing;

			std::getline(iss, trailing);

			token += trailing;

			args.push_back(token);

			break;
		}

		args.push_back(token);
	}
}

int	main(int argc, char** argv)
{
	signal(SIGPIPE, SIG_IGN);

	if (argc != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>\n";
		return 1;
	}

	int port;

	if (!isValidPort(argv[1], port))
	{
		std::cerr << "Invalid port. Use number between 1024 and 65535\n";
		return 1;
	}

	std::string server_password = argv[2];

	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0)
		return 1;
	fcntl(socket_fd, F_SETFL, O_NONBLOCK);
	int opt = 1;	
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	
	struct sockaddr_in address = {};
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = INADDR_ANY;
	
	if (bind(socket_fd, (struct sockaddr *)&address, sizeof(address)))
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	listen(socket_fd, 10);

	std::vector<pollfd> fds;
	std::vector<Client> clients;
	std::vector<Channel> channels;
	
	pollfd pfd;
	pfd.fd = socket_fd;
	pfd.events = POLLIN;
	fds.push_back(pfd);
	
	while (true)
	{

		int poll_count = poll(fds.data(), fds.size(), -1);
		if (poll_count < 0)
		{
			perror("poll failed");
			exit(EXIT_FAILURE);
		}

		for (size_t i = 0; i < fds.size(); ++i)
		{
			if (fds[i].revents & (POLL_HUP | POLL_ERR | POLLNVAL))
			{
				int fd_to_remove = fds[i].fd;

				Client* client = findClient(clients, fd_to_remove);
				if (client)
				{
					std::string msg = client->getNickname() + " has disconnected\r\n";
					sendToAll(clients, client->getFd(), msg);
					removeClientFromAllChannels(clients, channels, client->getFd());
				}
				close(fd_to_remove);

				std::vector<Client>::iterator it = findClientIt(clients, fd_to_remove);

				if (it != clients.end())
					clients.erase(it);
				
				fds.erase(fds.begin() + i);

				removeEmptyChannels(channels);
				--i;
				continue;
			}
			if (fds[i].revents & POLLIN)
			{
				if (fds[i].fd == socket_fd)
				{
					int new_socket = accept(socket_fd, NULL, NULL);
					if (new_socket < 0)
					{
						perror("accept failed");
						continue;
					}
					fcntl(new_socket, F_SETFL, O_NONBLOCK);
					pollfd new_pfd;
					new_pfd.fd = new_socket;
					new_pfd.events = POLLIN;
					fds.push_back(new_pfd);

					Client new_client(new_socket);
					clients.push_back(new_client);

					std::cout << "Nuevo cliente: " << new_socket << std::endl;
					sendToClient(clients, new_client.getFd(), "Welcome to the IRC server\r\n");
				}
				else
				{
					char buffer[1024];
					ssize_t bytes_read = recv(fds[i].fd, buffer, sizeof(buffer), 0);
					if (bytes_read <= 0)
					{
						int fd_to_remove = fds[i].fd;
						Client* client = findClient(clients, fd_to_remove);
						if (client)
						{
							std::string msg = client->getNickname() + " has quit\r\n";
							sendToAll(clients, client->getFd(), msg);
							removeClientFromAllChannels(clients, channels, client->getFd());
						}
						close(fd_to_remove);
						std::cout << "Cliente desconectado: " << fd_to_remove << std::endl;
						std::vector<Client>::iterator it = findClientIt(clients, fd_to_remove);
						if (it != clients.end())
							clients.erase(it);
						fds.erase(fds.begin() + i);
						removeEmptyChannels(channels);
						--i;
						continue;
					}
					
					Client* client = findClient(clients, fds[i].fd);
					if (!client)
						continue;
					
					client->getBuffer() += std::string(buffer, bytes_read);
					
					size_t pos;
					while ((pos = client->getBuffer().find('\n')) != std::string::npos)
					{
						std::string line = client->getBuffer().substr(0, pos);
						if (!line.empty() && line[line.size() - 1] == '\r')
							line.erase(line.size() - 1);
							
						std::string command;
						std::vector<std::string> args;

						parseCommand(line, command, args);
						
						if (command == "NICK")
						{
							if (!client->getPass_ok())
							{
								sendToClient(clients, client->getFd(), "ERROR: Password required\r\n");
								client->getBuffer().erase(0, pos + 1);
								continue;								
							}

							if (args.size() < 1)
							{
								sendError(clients, client->getFd(), "461 " + command, ":Not enough parameters");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}

							std::string new_nick = args[0];
							
							if (nickExists(clients, new_nick, client->getFd()))
							{
								sendError(clients, client->getFd(), "433 " + new_nick, ":Nickname is already in use");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}

							client->setNickname(new_nick);
							
							std::cout << "Nick set to: " << client->getNickname() << std::endl;
						}

						else if (command == "USER")
						{
							if (!client->getPass_ok())
							{
								sendToClient(clients, client->getFd(), "ERROR: Password required\r\n");
								client->getBuffer().erase(0, pos + 1);
								continue;								
							}

							if (args.size() < 1)
							{
							sendError(clients, client->getFd(), "461 " + command, ":Not enough parameters");
							}

							client->setUsername(args[0]);

							std::cout << "User set to: " << client->getUsername() << std::endl;
						}

						else if (command == "PASS")
						{
							if (args.size() < 1)
							{
								sendError(clients, client->getFd(), "461 " + command, ":Not enough parameters");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}

							if (client->getRegistered())
							{
								sendError(clients, client->getFd(), "462", ":You may not reregister");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}

							if (args[0] != server_password)
							{
								sendError(clients, client->getFd(), "464", ":Password incorrect");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}

							client->setPass_ok(true);
						}

						else if (command == "PRIVMSG")
						{
							if (!client->getRegistered())
							{
								sendError(clients, client->getFd(), "451", ":You have not registered");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}

							if (args.size() < 2)
							{
								sendError(clients, client->getFd(), "461 " + command, ":Not enough parameters");
								client->getBuffer().erase(0, pos + 1);
								continue;
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
									client->getBuffer().erase(0, pos + 1);
									continue;
								}
								if (!isClientInChannel(channel, client))
								{
									sendError(clients, client->getFd(), "442 " + channel->getName(), ":You're not on that channel");
									client->getBuffer().erase(0, pos + 1);
									continue;
								}
								std::cout << "MENSAJE ENVIADO: [" << full_msg << "]" << std::endl;
								sendToChannel(clients, channel, client->getFd(), full_msg);
							}
							
							else
							{
								Client* target_client = findClientByNick(clients, target);
								if (!target_client)
								{
									sendError(clients, client->getFd(), "401 " + target, ":No such nick");
									client->getBuffer().erase(0, pos + 1);
									continue;
								}
								sendToClient(clients, target_client->getFd(), full_msg);
							}
						}

						else if (command == "JOIN")
						{
							if (!client->getRegistered())
							{
								sendError(clients, client->getFd(), "451", ":You have not registered");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}

							if (args.size() < 1)
							{
								sendError(clients, client->getFd(), "461 " + command, ":Not enough parameters");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}

							std::string channel_name = args[0];
							if (channel_name.empty() || channel_name[0] != '#')
							{
								sendToClient(clients, client->getFd(), "ERROR: Invalid channel format\r\n");
								client->getBuffer().erase(0, pos + 1);
								continue;
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
						}

						else if (command == "INVITE")
						{
							if (!client->getRegistered())
							{
								sendError(clients, client->getFd(), "451", ":You have not registered");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}
														
							if (args.size() < 2)
							{
								sendError(clients, client->getFd(), "461 " + command, ":Not enough parameters");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}

							std::string target_nick = args[0];
							std::string channel_name = args[1];
							Channel *channel = findChannel(channels, channel_name);
							
							if (!channel)
							{
								sendError(clients, client->getFd(), "403 " + channel_name, ":No such channel");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}

							if (!isClientInChannel(channel, client))
							{
								sendError(clients, client->getFd(), "442 " + channel->getName(), ":You're not on that channel");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}

							if (!isOperator(channel, client))
							{
								sendError(clients, client->getFd(), "482 " + channel->getName(), ":You're not channel operator");
								client->getBuffer().erase(0, pos + 1);
								continue;			
							}

							Client* target = findClientByNick(clients, target_nick);

							if (!target)
							{
								sendError(clients, client->getFd(), "401 " + target_nick, ":No such nick");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}
							
							if (isClientInChannel(channel, target))
							{
								sendError(clients, client->getFd(), "443 " + target->getNickname() + " " + channel->getName(), ":is already on channel");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}

							if (!isInvited(channel, target))
								channel->addInvited(target->getFd());

							sendToClient(clients, target->getFd(), "You have been invited to " + channel_name + "\r\n");
							sendToClient(clients, client->getFd(), "Invitation sent\r\n");
						}

						else if (command == "MODE")
						{
							if (!client->getRegistered())
							{
								sendError(clients, client->getFd(), "451", ":You have not registered");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}

							if (args.size() < 2)
							{
								sendError(clients, client->getFd(), "461 " + command, ":Not enough parameters");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}

							std::string channel_name = args[0];
							std::string mode = args[1];

							Channel* channel = findChannel(channels, channel_name);
							if (!channel)
							{
								sendError(clients, client->getFd(), "403 " + channel_name, ":No such channel");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}

							if (!isClientInChannel(channel, client))
							{
								sendError(clients, client->getFd(), "442 " + channel->getName(), ":You're not on that channel");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}

							if (!isOperator(channel, client))
							{
								sendError(clients, client->getFd(), "482 " + channel->getName(), ":You're not channel operator");
								client->getBuffer().erase(0, pos + 1);
								continue;
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
									client->getBuffer().erase(0, pos + 1);
									continue;
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
									client->getBuffer().erase(0, pos + 1);
									continue;
								}
								
								int limit;
								if (!isValidLimit(args[2], limit))
								{
									sendToClient(clients, client->getFd(), "ERROR: Invalid limit\r\n");
									client->getBuffer().erase(0, pos + 1);
									continue;
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
									client->getBuffer().erase(0, pos + 1);
									continue;
								}

								Client* target = findClientByNick(clients, args[2]);
								if (!target)
								{
									sendError(clients, client->getFd(), "401 " + args[2], ":No such nick");
									client->getBuffer().erase(0, pos + 1);
									continue;
								}

								if (!isClientInChannel(channel, target))
								{
									sendError(clients, client->getFd(), "441 " + target->getNickname() + " " + channel->getName(), ":They aren't on that channel");
									client->getBuffer().erase(0, pos + 1);
									continue;
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
						}
						else if (command == "TOPIC")
						{
							if (!client->getRegistered())
							{
								sendError(clients, client->getFd(), "451", ":You have not registered");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}

							if (args.size() < 1)
							{
								sendError(clients, client->getFd(), "461 " + command, ":Not enough parameters");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}
							std::string channel_name = args[0];
							Channel *channel = findChannel(channels, channel_name);
							
							if (!channel)
							{
								sendError(clients, client->getFd(), "403 " + channel_name, ":No such channel");
								client->getBuffer().erase(0, pos + 1);
								continue;
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
										sendToChannel(clients, channel, client->getFd(), msg);

										std::cout << "Nuevo topic en " << channel_name << ": " << topic << std::endl;
									}
									else
									{
										sendToClient(clients, client->getFd(), "ERROR: You are not op to change the topic\r\n");
									}
								}
							}
							else
							{
								sendError(clients, client->getFd(), "442 " + channel->getName(), ":You're not on that channel");
							}
						}

						else if (command == "KICK")
						{
							if (!client->getRegistered())
							{
								sendError(clients, client->getFd(), "451", ":You have not registered");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}

							if (args.size() < 2)
							{
								sendError(clients, client->getFd(), "461 " + command, ":Not enough parameters");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}

							std::string channel_name = args[0];
							std::string target_nick = args[1];

							Channel* channel = findChannel(channels, channel_name);
							if (!channel)
							{
								sendError(clients, client->getFd(), "403 " + channel_name, ":No such channel");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}

							if (!isClientInChannel(channel, client))
							{
								sendError(clients, client->getFd(), "442 " + channel->getName(), ":You're not on that channel");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}

							if (!isOperator(channel, client))
							{
								sendError(clients, client->getFd(), "482 " + channel->getName(), ":You're not channel operator");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}

							Client* target = findClientByNick(clients, target_nick);
							if (!target)
							{
								sendError(clients, client->getFd(), "401 " + target_nick, ":No such nick");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}

							if (target->getFd() == client->getFd())
							{
								sendToClient(clients, client->getFd(), "ERROR: You cannot kick yourself\r\n");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}

							if (!isClientInChannel(channel, target))
							{
								sendError(clients, client->getFd(), "441 " + target->getNickname() + " " + channel->getName(), ":They aren't on that channel");
								client->getBuffer().erase(0, pos + 1);
								continue;
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
						}
						
						else if (command == "PART")
						{
							if (!client->getRegistered())
							{
								sendError(clients, client->getFd(), "451", ":You have not registered");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}

							if (args.size() < 1)
							{
								sendError(clients, client->getFd(), "461 " + command, ":Not enough parameters");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}
							std::string channel_name = args[0];
							Channel *channel = findChannel(channels, channel_name);
							
							if (!channel)
							{
								sendError(clients, client->getFd(), "403 " + channel_name, ":No such channel");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}
							
							if (!isClientInChannel(channel, client))
							{
								sendError(clients, client->getFd(), "442 " + channel->getName(), ":You're not on that channel");
								client->getBuffer().erase(0, pos + 1);
								continue;
							}
							
							const std::vector<int>& ch_clients = channel->getClients();
							for (size_t j = 0; j < ch_clients.size(); ++j)
							{
								if (ch_clients[j] == client->getFd())
								{
									std::string msg = client->getNickname() + " left " + channel_name + "\r\n";
									sendToChannel(clients, channel, client->getFd(), msg);
									channel->removeClient(client->getFd());
									break;
								}
							}
							const std::vector<int>& ch_operators = channel->getOperators();
							for (size_t j = 0; j < ch_operators.size(); ++j)
							{
								if (ch_operators[j] == client->getFd())
								{
									channel->removeOperator(client->getFd());
									break;
								}
							}
							if (channel->getOperators().empty() && !channel->getClients().empty())
							{
								channel->addOperator(channel->getClients()[0]);
								std::string msg = findClient(clients, channel->getClients()[0])->getNickname() + " is now operator\r\n";
								sendToChannel(clients, channel,client->getFd(), msg);
							}
							if (channel->getClients().empty())
								removeEmptyChannels(channels);
						}

						else if (command == "QUIT")
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

							break;
						}
						
						if (!client->getRegistered() 
							&& !client->getNickname().empty() 
							&& !client->getUsername().empty() 
							&& client->getPass_ok())
						{
							client->setRegistered(true);
							sendToClient(clients, client->getFd(), ":ircserv 001 " + client->getNickname() + " :Welcome to the IRC server\r\n");
							sendToClient(clients, client->getFd(), ":ircserv 002 " + client->getNickname() + " :Your host is ircserv\r\n");
							sendToClient(clients, client->getFd(), ":ircserv 003 " + client->getNickname() + " :This server was created today\r\n");
							sendToClient(clients, client->getFd(), ":ircserv 004 " + client->getNickname() + " ircserv 1.0 o o\r\n");
						}

						client->getBuffer().erase(0, pos + 1);
					}

					if (client->getTo_delete())
					{
						int fd = client->getFd();

						close(fd);
						
						std::vector<Client>::iterator it = findClientIt(clients, fd);
						if (it != clients.end())
						clients.erase(it);

						for (size_t j = 0; j < fds.size(); j++)
						{
							if (fds[j].fd == fd)
							{
								fds.erase(fds.begin() + j);
								break;
							}
						}
						removeEmptyChannels(channels);
						i--;
						continue;
					}

					// Flush send buffer
					if (client && !client->getSend_buffer().empty())
					{
						ssize_t sent = send(client->getFd(), client->getSend_buffer().c_str(), 
											client->getSend_buffer().size(), MSG_DONTWAIT);
						if (sent > 0)
						{
							client->getSend_buffer().erase(0, sent);
						}
					}
				}
			}
		}
	}
	return 0;
}
