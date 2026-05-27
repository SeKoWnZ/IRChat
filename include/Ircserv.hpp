#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstdio>
#include <cerrno>

#include <Client.hpp>
#include <Channel.hpp>

bool isClientInChannel(Channel* channel, Client* client);
bool isOperator(Channel* channel, Client* client);
bool isValidLimit(const std::string& str, int& limit);
bool isInvited(Channel* channel, Client* client);
bool nickExists(std::vector<Client>& clients, const std::string& nick, int exclude_fd);
void removeEmptyChannels(std::vector<Channel>& channels);
Client* findClient(std::vector<Client>& clients, int fd);
Client* findClientByNick(std::vector<Client>& clients, const std::string& nick);
std::vector<Client>::iterator findClientIt(std::vector<Client>& clients, int fd);
Channel* findChannel(std::vector<Channel>& channels, const std::string& name);
bool sendToClient(std::vector<Client>& clients, int fd, const std::string& msg);
void sendToAll(std::vector<Client>& clients, int sender_fd, const std::string& msg);
void sendToChannel(std::vector<Client>& clients, Channel* channel, int sender, const std::string& msg);
void sendToAllChannels(std::vector<Client>& clients, std::vector<Channel>& channel_list, Client* sender, const std::string& msg);
void sendError(std::vector<Client>& clients, int fd, const std::string& code, const std::string& msg);
void	removeClientFromAllChannels(std::vector<Client>& clients, std::vector<Channel>& channels, int client);
bool isValidPort(const std::string& str, int& port);
void CreateUserList(std::vector<Client>& clients, Channel* channel, std::string& user_list);
void parseCommand(const std::string& line, std::string& command, std::vector<std::string>& args);

//CMD

int cmdNick(std::vector<Client> &clients, Client *client, std::vector<std::string> &args);
int cmdUser(std::vector<Client> &clients, Client *client, std::vector<std::string> &args);
int cmdPass(std::vector<Client> &clients, Client *client, std::vector<std::string> &args, std::string &server_password);
int cmdPrivmsg(std::vector<Client> &clients, std::vector<Channel> &channels, Client *client, std::vector<std::string> &args);
int cmdJoin(std::vector<Client> &clients, std::vector<Channel> &channels, Client *client, std::vector<std::string> &args);
int cmdInvite(std::vector<Client> &clients, std::vector<Channel> &channels, Client *client, std::vector<std::string> &args);
int cmdMode(std::vector<Client> &clients, std::vector<Channel> &channels, Client *client, std::vector<std::string> &args);
int cmdTopic(std::vector<Client> &clients, std::vector<Channel> &channels, Client *client, std::vector<std::string> &args);
int cmdKick(std::vector<Client> &clients, std::vector<Channel> &channels, Client *client, std::vector<std::string> &args);
int cmdPart(std::vector<Client> &clients, std::vector<Channel> &channels, Client *client, std::vector<std::string> &args);
int cmdQuit(std::vector<Client> &clients, std::vector<Channel> &channels, Client *client, std::vector<std::string> &args);

