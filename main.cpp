#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>

struct Client
{
	int fd;
	std::string buffer;
};

Client* findClient(std::vector<Client>& clients, int fd)
{
	for (size_t i = 0; i < clients.size(); ++i)
	{
		if (clients[i].fd = fd)
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
					clients.push_back(new_client);

					std::cout << "Nuevo cliente: " << new_socket << std::endl;
				}
				else
				{
					char buffer[1024];
					ssize_t bytes_read = recv(fds[i].fd, buffer, sizeof(buffer), 0);
					if (bytes_read <= 0)
					{
						close(fds[i].fd);
						std::cout << "Cliente desconectado: " << fds[i].fd << std::endl;
						fds.erase(fds.begin() + i);
						std::vector<Client>::iterator it = findClientIt(clients, fds[i].fd);
						if (it != clients.end())
							clients.erase(it);
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

						while (iss >> arg)
							args.push_back(arg);

						std::cout << "Comando: " << command << std::endl;
						for (size_t h = 0; h < args.size(); h++)
							std::cout << "ARG[" << h << "]: " << args[h] <<std::endl;

						client->buffer.erase(0, pos + 1);
					}
				}
			}
		}
	}
	return 0;
}
