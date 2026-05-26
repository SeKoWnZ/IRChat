#pragma once

#include <string>

class Client
{
	private:

		const int _fd;

		std::string _nickname;
		std::string _username;
		std::string _buffer;
		std::string _send_buffer;

		bool _registered;
		bool _pass_ok;
		bool _to_delete;

	public:

		Client();
		Client(int fd);
		Client(const Client& other);
		Client& operator=(const Client& other);
		~Client();

		int getFd() const;
		std::string getNickname() const;
		std::string getUsername() const;
		std::string getBuffer() const;
		std::string getSend_buffer() const;
		bool getRegistered() const;
		bool getPass_ok() const;
		bool getTo_delete() const;

		void setNickname(const std::string& nickname);
		void setUsername(const std::string& username);
		void appendToBuffer(const std::string& data);
		void eraseFromBuffer(size_t n);
		void appendToSend_buffer(const std::string& data);
		void eraseFromSend_buffer(size_t n);
		void setRegistered(bool val);
		void setPass_ok(bool val);
		void setTo_delete(bool val);
};