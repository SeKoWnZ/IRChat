#pragma once

#include <string>
#include <vector>
#include <algorithm>

class Channel
{
	private:

		const std::string _name;
		std::string _topic;
		std::string _key;

		int _user_limit;

		std::vector<int> _clients;
		std::vector<int> _operators;
		std::vector<int> _invited;

		bool _invite_only;
		bool _topic_protected;

	public:

		Channel();
		Channel(const std::string name);
		Channel(const Channel& other);
		Channel& operator=(const Channel& other);
		~Channel();

		std::string getName() const;
		std::string getTopic() const;
		std::string getKey() const;

		int getUserLimit() const;

		const std::vector<int>& getClients() const;
		const std::vector<int>& getOperators() const;
		const std::vector<int>& getInvited() const;

		bool getInviteOnly() const;
		bool getTopicProtected() const;
	
		void setTopic(const std::string topic);
		void setKey(const std::string key);
		void setUserLimit(const int user_limit);

		void addClient(const int fd);
		void removeClient(const int fd);

		void addOperator(const int fd);
		void removeOperator(const int fd);

		void addInvited(const int fd);
		void removeInvited(const int fd);

		void setInviteOnly(const bool invite_only);
		void setTopicProtected(const bool topic_protected);

};


// ▄▀─────────────▀▄
// █▄█──█▀█─█▀█─▄█▄█
// ─▀██▄▀▄▀─▀▄▀▄██▀
// ░░░▄██▀███▀███▄
// ░▐▀█▀██▄▄▄██▀█▀▌


