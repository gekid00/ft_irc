#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>

// Déclaration anticipée pour éviter l'inclusion circulaire
class Server;

class Channel
{
	private:
		std::string		_name;
		std::string		_topic;
		std::set<int>	_members;		// Fds de tous les membres
		std::set<int>	_operators;		// Fds des opérateurs
		std::set<int>	_invited;		// Fds des clients invités (mode +i)
		std::string		_key;			// Mot de passe du channel (mode +k)
		int				_userLimit;		// Limite de membres, 0 = illimité (mode +l)
		bool			_inviteOnly;	// Mode +i
		bool			_topicRestricted; // Mode +t

	public:
		Channel();
		Channel(const std::string& name);
		~Channel();

		// Nom
		const std::string&	getName() const;

		// Topic
		const std::string&	getTopic() const;
		void				setTopic(const std::string& topic);

		// Membres
		void				addMember(int fd);
		void				removeMember(int fd);
		bool				isMember(int fd) const;
		int					getNumberOfMembers() const;
		const std::set<int>& getMembers() const;

		// Opérateurs
		void				addOperator(int fd);
		void				removeOperator(int fd);
		bool				isOperator(int fd) const;

		// Invités (mode +i)
		void				addInvited(int fd);
		void				removeInvited(int fd);
		bool				isInvited(int fd) const;

		// Clé (mode +k)
		void				setKey(const std::string& key);
		const std::string&	getKey() const;
		bool				isKeyRestricted() const;

		// Limite d'utilisateurs (mode +l)
		void				setUserLimit(int limit);
		int					getUserLimit() const;

		// Modes
		bool				isInviteOnly() const;
		bool				isTopicRestricted() const;
		void				setInviteOnly(bool flag);
		void				setTopicRestricted(bool flag);

		// Envoi d'un message à tous les membres sauf senderFd (-1 = envoyer à tous)
		// Passe par Server::sendToClient pour utiliser le buffer de sortie
		void				broadcastMessage(const std::string& message, int senderFd, Server& server) const;
};

#endif
