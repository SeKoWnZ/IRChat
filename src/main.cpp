
#include <Ircserv.hpp>


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

						std::cout << "line--->" << line << std::endl;

						parseCommand(line, command, args);
						
						if (command == "NICK")
						{
							if(!cmdNick(clients, client, args))
							{
								client->getBuffer().erase(0, pos + 1);
								continue;										
							}
						}
						else if (command == "USER")
						{
							if(!cmdUser(clients, client, args))
							{
								client->getBuffer().erase(0, pos + 1);
								continue;										
							}
						}

						else if (command == "PASS")
						{
							if(!cmdPass(clients, client, args, server_password))
							{
								client->getBuffer().erase(0, pos + 1);
								continue;										
							}
						}

						else if (command == "PRIVMSG")
						{
							if(!cmdPrivmsg(clients, channels, client, args))
							{
								client->getBuffer().erase(0, pos + 1);
								continue;										
							}
						}
						else if (command == "JOIN")
						{
							if(!cmdJoin(clients, channels, client, args))
							{
								client->getBuffer().erase(0, pos + 1);
								continue;										
							}
						}
						else if (command == "INVITE")
						{
							if(!cmdInvite(clients, channels, client, args))
							{
								client->getBuffer().erase(0, pos + 1);
								continue;										
							}
						}
						else if (command == "MODE")
						{
							if(!cmdMode(clients, channels, client, args))
							{
								client->getBuffer().erase(0, pos + 1);
								continue;										
							}
						}
						else if (command == "TOPIC")
						{
							if(!cmdTopic(clients, channels, client, args))
							{
								client->getBuffer().erase(0, pos + 1);
								continue;										
							}
						}

						else if (command == "KICK")
						{
							if(!cmdKick(clients, channels, client, args))
							{
								client->getBuffer().erase(0, pos + 1);
								continue;										
							}
						}
						
						else if (command == "PART")
						{
							if(!cmdPart(clients, channels, client, args))
							{
								client->getBuffer().erase(0, pos + 1);
								continue;										
							}
						}

						else if (command == "QUIT")
						{
							if(!cmdQuit(clients, channels, client, args))
							{
								client->getBuffer().erase(0, pos + 1);
								continue;									
							}
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
