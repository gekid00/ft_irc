#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
	private:
		int			_fd;
		std::string	_nickname;
		std::string	_username;
		std::string	_realname;
		bool		_passwordOk;
		bool		_registered;
		std::string	_buffer;		// Buffer d'entrée : accumule les données reçues par recv()
		std::string	_outBuffer;		// Buffer de sortie : données en attente d'envoi (POLLOUT)

	public:
		Client();
		Client(int fd);
		~Client();

		// Identité du client
		int			setNickname(const std::string& nickname);
		std::string	getNickname() const;
		int			setUsername(const std::string& username);
		std::string	getUsername() const;
		int			setRealname(const std::string& realname);
		std::string	getRealname() const;

		// Authentification et enregistrement
		int			setPasswordOk(bool ok);
		bool		isPasswordOk() const;
		bool		isRegistered() const;
		bool		tryRegister();

		// Descripteur de fichier
		int			getFd() const;

		// Préfixe IRC : nick!user@host
		std::string	getPrefix() const;

		// Buffer d'entrée (réception)
		void		appendToBuffer(const std::string& data);
		std::string	getBuffer() const;
		bool		hasLine() const;
		std::string	extractLine();

		// Buffer de sortie (envoi différé via POLLOUT)
		void				appendToOutBuffer(const std::string& data);
		const std::string&	getOutBuffer() const;
		void				shiftOutBuffer(size_t n);
		bool				hasOutBuffer() const;
};

#endif
