#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
	private:
		int _fd;
		std::string _nickname;
		std::string _username;
		std::string _realname;
		bool _passwordOk;
		bool _registered;
		std::string _buffer;
		public:
		Client();
		Client(int fd);
		~Client();
		int setNickname(const std::string& nickname);
		std::string getNickname() const;
		int setUsername(const std::string& username);
		std::string getUsername() const;
		int setRealname(const std::string& realname);
		std::string getRealname() const;
		int setPasswordOk(bool Ok);
		bool isPasswordOk() const;
		int getFd() const;
		void appendToBuffer(const std::string& data);
		std::string getBuffer() const;
		bool hasLine() const;
		std::string extractLine();
		bool isRegistered() const;
		bool tryRegister();
		std::string getPrefix() const;
};

#endif