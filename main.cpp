#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>


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

struct Client
{
	int fd;
	std::string buffer;

	std::string nickname;
	std::string username;

	bool registered;
	bool to_delete = false;
};

struct Channel
{
	std::string name;
	std::string topic;

	std::vector<int> clients;
	std::vector<int> operators;
};

bool nickExists(std::vector<Client>& clients, const std::string& nick, int exclude_fd)
{
	for (size_t i = 0; i < clients.size(); i++)
	{
		if (clients[i].nickname == nick && clients[i].fd != exclude_fd)
			return true;
	}
	return false;
}

void	removeClientFromAllChannels(std::vector<Client>& clients, std::vector<Channel>& channels, int client)
{
	for (size_t i = 0; i < channels.size(); i++)
	{
		std::vector<int>& ch_clients = channels[i].clients;
		for (size_t j = 0; j < ch_clients.size(); j++)
		{
			if (ch_clients[j] == client)
			{
				ch_clients.erase(ch_clients.begin() + j);
				break;
			}
		}
		std::vector<int>& ch_operators = channels[i].operators;
		for (size_t j = 0; j < ch_operators.size(); j++)
		{
			if (ch_operators[j] == client)
			{
				ch_operators.erase(ch_operators.begin() + j);
				if (ch_operators.empty() && !ch_clients.empty())
				{
					ch_operators.push_back(ch_clients[0]);
					Client *new_op = findClient(clients, ch_clients[0]);
					std::string msg = new_op->nickname + " is now operator\n";
					sendToChannel(&channels[i], new_op, msg);
				}
				break;
			}
		}
	}
}

void	removeEmptyChannels(std::vector<Channel>& channels)
{
	for (size_t i = 0; i < channels.size(); i++)
	{
		if (channels[i].clients.empty())
		{
			std::cout << "Canal eliminado: " << channels[i].name << std::endl;
			channels.erase(channels.begin() + i);
			i--;
		}
	}
}

Client* findClient(std::vector<Client>& clients, int fd)
{
	for (size_t i = 0; i < clients.size(); ++i)
	{
		if (clients[i].fd == fd)
			return &clients[i];
	}
	return NULL;
}

Client* findClientByNick(std::vector<Client>& clients, const std::string& nick)
{
	for (size_t i = 0; i < clients.size(); ++i)
	{
		if (clients[i].nickname == nick)
			return &clients[i];
	}
	return NULL;
}

std::vector<Client>::iterator findClientIt(std::vector<Client>& clients, int fd)
{
	for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); it++)
	{
		if (it->fd == fd)
			return it;
	}
	return clients.end();
}

Channel* findChannel(std::vector<Channel>& channels, const std::string& name)
{
	for (size_t i = 0; i < channels.size(); ++i)
	{
		if (channels[i].name == name)
			return &channels[i];
	}
	return NULL;
}

void sendToClient(int fd, const std::string& msg)
{
	send(fd, msg.c_str(), msg.size(), 0);
}

void sendToAll(std::vector<Client>& clients, int sender_fd, const std::string& msg)
{
	for (size_t i = 0; i < clients.size(); ++i)
	{
		if (clients[i].fd != sender_fd && clients[i].registered)
			sendToClient(clients[i].fd, msg);
	}
}

void sendToChannel(Channel* channel, Client* sender, const std::string& msg)
{
	for (size_t i = 0; i < channel->clients.size(); ++i)
	{
		if (channel->clients[i] != sender->fd)
			send(channel->clients[i], msg.c_str(), msg.size(), 0);
	}
}

bool isClientInChannel(Channel* channel, Client* client)
{
	for (size_t i = 0; i < channel->clients.size(); ++i)
	{
		if (channel->clients[i] == client->fd)
			return true;
	}
	return false;
}

bool isOperator(Channel* channel, Client* client)
{
	for (size_t i = 0; i < channel->operators.size(); i++)
	{
		if (channel->operators[i] == client->fd)
			return true;
	}
	return false;
}

int	main(int argc, char** argv)
{
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0)
		return 1;

	int opt = 1;	
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in address = {};
	address.sin_family = AF_INET;
	address.sin_port = htons(6667);
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
			if (fds[i].revents & POLLIN)
			{
				if (fds[i].fd == socket_fd)
				{
					int new_socket = accept(socket_fd, nullptr, nullptr);
					if (new_socket < 0)
					{
						perror("accept failed");
						continue;
					}
					pollfd new_pfd;
					new_pfd.fd = new_socket;
					new_pfd.events = POLLIN;
					fds.push_back(new_pfd);

					Client new_client;
					new_client.fd = new_socket;
					new_client.nickname = "";
					new_client.username = "";
					new_client.registered = false;
					clients.push_back(new_client);

					std::cout << "Nuevo cliente: " << new_socket << std::endl;
					sendToClient(new_client.fd, "Welcome to the IRC server\n");
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
							std::string msg = client->nickname + " has quit\n";
							sendToAll(clients, client->fd, msg);
							removeClientFromAllChannels(clients, channels, client->fd);
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
					
					client->buffer += std::string(buffer, bytes_read);
					
					size_t pos;
					while ((pos = client->buffer.find('\n')) != std::string::npos)
					{
						std::string line = client->buffer.substr(0, pos);
						if (!line.empty() && line[line.size() - 1] == '\r')
							line.erase(line.size() - 1);

						std::istringstream iss(line);
						std::string command;
						iss >> command;
						
						std::vector<std::string> args;
						std::string arg;

						while (iss >> arg) // SE PUEDEN USAR MARCADORES PARA SACAR STRINGS ENTEROS
							args.push_back(arg);
						
						if (command == "NICK")
						{
							if (args.size() < 1)
							{
								sendToClient(client->fd, "ERROR: No nickname given\n");
								client->buffer.erase(0, pos + 1);
								continue;
							}

							std::string new_nick = args[0];
							
							if (nickExists(clients, new_nick, client->fd))
							{
								sendToClient(client->fd, "ERROR: Nickname already in use\n");
								client->buffer.erase(0, pos + 1);
								continue;
							}

							client->nickname = new_nick;
							
							std::cout << "Nick set to: " << client->nickname << std::endl;
						}

						else if (command == "USER")
						{
							if (args.size() < 1)
							{
								sendToClient(client->fd, "ERROR: No username given\n");
								client->buffer.erase(0, pos + 1);
								continue;
							}
							client->username = args[0];

							std::cout << "User set to: " << client->username << std::endl;
						}

						else if (command == "MSG")
						{
							if (args.size() < 1)
							{
								sendToClient(client->fd, "ERROR: No message\n");
								client->buffer.erase(0, pos + 1);
								continue;
							}
							if (!client->registered)
							{
								sendToClient(client->fd, "ERROR: Client not registered\n");
								client->buffer.erase(0, pos + 1);
								continue;
							}
							std::string msg;
							for (size_t j = 0; j < args.size(); ++j)
							{
								msg += args[j];
								if (j < args.size() - 1)
									msg += " ";
							}
							std::string full_msg = client->nickname + ": " + msg + "\n";
							sendToAll(clients, client->fd, full_msg);
						}

						else if (command == "PRIVMSG")
						{
							if (args.size() < 2)
							{
								sendToClient(client->fd, "ERROR: No message\n");
								client->buffer.erase(0, pos + 1);
								continue;
							}
							if (!client->registered)
							{
								sendToClient(client->fd, "ERROR: Client not registered\n");
								client->buffer.erase(0, pos + 1);
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
							std::string full_msg = client->nickname + ": " + msg + "\n";

							if (target[0] == '#')
							{
								Channel* channel = findChannel(channels, target);
								if (!channel)
								{
									sendToClient(client->fd, "ERROR: Channel not found\n");
									client->buffer.erase(0, pos + 1);
									continue;
								}
								if (!isClientInChannel(channel, client))
								{
									sendToClient(client->fd, "ERROR: You're not in this channel\n");
									client->buffer.erase(0, pos + 1);
									continue;
								}
								sendToChannel(channel, client, full_msg);
							}
							
							else
							{
								Client* target_client = findClientByNick(clients, target);
								if (!target_client)
								{
									sendToClient(client->fd, "ERROR: Target not found\n");
									client->buffer.erase(0, pos + 1);
									continue;
								}
								sendToClient(target_client->fd, full_msg);
							}
						}

						else if (command == "JOIN")
						{
							if (args.size() < 1)
							{
								sendToClient(client->fd, "ERROR: No channel given\n");
								client->buffer.erase(0, pos + 1);
								continue;
							}
							std::string channel_name = args[0];
							Channel *channel = findChannel(channels, channel_name);

							if (!channel)
							{
								Channel new_channel;
								new_channel.name = channel_name;
								channels.push_back(new_channel);
								channel = &channels.back();
								channel->operators.push_back(client->fd);
								sendToClient(client->fd, "You are now operator of " +  channel_name + "\n");
							}
							
							if (isClientInChannel(channel, client))
							{
								sendToClient(client->fd, "you are already on that channel!!!\n");
							}
							else
							{
								channel->clients.push_back(client->fd);
								sendToChannel(channel, client, client->nickname + " joined " + channel_name + "\n");
								sendToClient(client->fd, client->nickname + " joined " + channel_name + "\n");
							}
						}

						else if (command == "INVITE")
						{
							if (args.size() < 2)
							{
								sendToClient(client->fd, "ERROR: A few args for 'INVITE'\n");
								client->buffer.erase(0, pos + 1);
								continue;
							}

							std::string target_nick = args[0];
							std::string channel_name = args[1];
							Channel *channel = findChannel(channels, channel_name);
							
							if (!channel)
							{
								sendToClient(client->fd, "ERROR: Channel not found\n");
								client->buffer.erase(0, pos + 1);
								continue;
							}

							if (!isClientInChannel(channel, client))
							{
								sendToClient(client->fd, "ERROR: You're not in this channel\n");
								client->buffer.erase(0, pos + 1);
								continue;
							}

							if (!isOperator(channel, client))
							{
								sendToClient(client->fd, "ERROR: You're not operator\n");
								client->buffer.erase(0, pos + 1);
								continue;			
							}

							Client* target = findClientByNick(clients, target_nick);

							if (!target)
							{
								sendToClient(client->fd, "ERROR: User not found\n");
								client->buffer.erase(0, pos + 1);
								continue;
							}
							
							if (!isClientInChannel(channel, target))
							{
								std::string msg = "You have been invited to " + channel_name + "\n";
								sendToClient(target->fd, msg);
								sendToClient(client->fd, "Invitation sent\n");

								std::cout << client->nickname << " invited " << target_nick << " to " << channel_name << std::endl;
							}
							else
								sendToClient(client->fd, "ERROR: User already on channel\n");
						}

						else if (command == "MODE")
						{
							if (args.size() < 2)
							{
								sendToClient(client->fd, "ERROR: A few args for 'MODE'\n");
								client->buffer.erase(0, pos + 1);
								continue;
							}

							std::string channel_name = args[0];
							std::string mode = args[1];
							Channel *channel = findChannel(channels, channel_name);
							
							if (!channel)
							{
								sendToClient(client->fd, "ERROR: Channel not found\n");
								client->buffer.erase(0, pos + 1);
								continue;
							}

							if (!isOperator(channel, client))
							{
								sendToClient(client->fd, "ERROR: You're not operator\n");
								client->buffer.erase(0, pos + 1);
								continue;			
							}

							if (args.size() < 3)
							{
								sendToClient(client->fd, "ERROR: A few args for 'MODE'\n");
								client->buffer.erase(0, pos + 1);
								continue;
							}

							std::string target_nick = args[2];
							Client* target = findClientByNick(clients, target_nick);

							if (!target)
							{
								sendToClient(client->fd, "ERROR: User not found\n");
								client->buffer.erase(0, pos + 1);
								continue;
							}

							if (mode == "+o" && !isOperator(channel, target))
							{
								channel->operators.push_back(target->fd);
								
								std::string msg = target->nickname + " is now operator\n";
								sendToChannel(channel,client, msg);
							}
							else if (mode == "-o")
							{
								std::vector<int>& ops = channel->operators;

								for (size_t j = 0; j < ops.size(); j++)
								{
									if (ops[j] == target->fd)
									{
										std::string msg = target_nick + " is no longer operator\n";
										sendToChannel(channel, client, msg);
										ops.erase(ops.begin() + j);
										if (channel->operators.empty() && !channel->clients.empty())
										{
											channel->operators.push_back(channel->clients[0]);
											std::string msg = findClient(clients, channel->clients[0])->nickname + " is now operator\n";
											sendToChannel(channel,client, msg);
										}
										break;
									}
								}
							}
						}

						else if (command == "TOPIC")
						{
							if (args.size() < 1)
							{
								sendToClient(client->fd, "ERROR: A few args for 'TOPIC'\n");
								client->buffer.erase(0, pos + 1);
								continue;
							}
							std::string channel_name = args[0];
							Channel *channel = findChannel(channels, channel_name);
							
							if (!channel)
							{
								sendToClient(client->fd, "ERROR: Channel not found\n");
								client->buffer.erase(0, pos + 1);
								continue;
							}

							if (args.size() == 1)
							{
								std::cout << "Topic: " << channel->topic << std::endl;
								sendToClient(client->fd, "Topic: " + channel->topic + "\n");
							}
							
							else if (args.size() > 1 && isClientInChannel(channel, client))
							{
								if (isOperator(channel, client))
								{
									std::string topic;
									for (size_t j = 1; j < args.size(); j++)
									{
										topic += args[j];
										if (j < args.size() - 1)
											topic += " ";
									}

									if (!topic.empty() && topic[0] == ':')
										topic.erase(0, 1);

									channel->topic = topic;


									std::string msg = client->nickname + " changed topic to: " + topic + '\n';
									sendToChannel(channel, client, msg);

									std::cout << "Nuevo topic en " << channel_name << ": " << topic << std::endl;
								}
								else
								{
									sendToClient(client->fd, "ERROR: You are not op to change the topic\n");
								}
							}
							else
							{
								sendToClient(client->fd, "ERROR: You are not in " + channel->name + " channel\n");
							}
						}
						
						else if (command == "PART")
						{
							if (args.size() < 1)
							{
								sendToClient(client->fd, "ERROR: A few args for 'PART'\n");
								client->buffer.erase(0, pos + 1);
								continue;
							}
							std::string channel_name = args[0];
							Channel *channel = findChannel(channels, channel_name);
							
							if (!channel)
							{
								sendToClient(client->fd, "ERROR: Channel not found\n");
								client->buffer.erase(0, pos + 1);
								continue;
							}
							
							if (!isClientInChannel(channel, client))
							{
								sendToClient(client->fd, "ERROR: You are not in " + channel->name + " channel\n");
								client->buffer.erase(0, pos + 1);
								continue;
							}
							
							std::vector<int>& ch_clients = channel->clients;
							for (size_t j = 0; j < ch_clients.size(); ++j)
							{
								if (ch_clients[j] == client->fd)
								{
									std::string msg = client->nickname + " left " + channel_name + '\n';
									sendToChannel(channel, client, msg);
									ch_clients.erase(ch_clients.begin() + j);
									break;
								}
							}
							std::vector<int>& ch_operators = channel->operators;
							for (size_t j = 0; j < ch_operators.size(); ++j)
							{
								if (ch_operators[j] == client->fd)
								{
									ch_operators.erase(ch_operators.begin() + j);
									break;
								}
							}
							if (channel->operators.empty() && !channel->clients.empty())
							{
								channel->operators.push_back(channel->clients[0]);
								std::string msg = findClient(clients, channel->clients[0])->nickname + " is now operator\n";
								sendToChannel(channel,client, msg);
							}
							if (channel->clients.empty())
								removeEmptyChannels(channels);
						}

						else if (command == "QUIT")
						{

							client->to_delete = true;
							
							std::string msg = client->nickname + " has quit\n";
							sendToAll(clients, client->fd, msg);
							removeClientFromAllChannels(clients, channels, client->fd);

							break;
						}
						
						if (!client->registered && !client->nickname.empty() && !client->username.empty())
						{
							client->registered = true;
							std::cout << "Cliente registrado: " << client->nickname << std::endl;
						}

						client->buffer.erase(0, pos + 1);
					}

					if (client->to_delete)
					{
						int fd = client->fd;

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
				}
			}
		}
	}
	return 0;
}
