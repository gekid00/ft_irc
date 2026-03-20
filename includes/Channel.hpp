#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>

class Channel
{
	private:
		std::string _name;
		std::string _topic;
		std::set<int> _members;
		std::set<int> _operators;
		std::set<int> _invited;
		std::string _key;
		int _userLimit;
		bool _inviteOnly;
		bool _topicRestricted;
	public:
		Channel();
		Channel(const std::string& name);  // Constructeur: initialise le channel avec un nom
		~Channel();                         // Destructeur

		/* Name */
		const std::string& getName() const;  // Retourne le nom du channel (ex: "#general")

		/* Topic */
		const std::string& getTopic() const;           // Retourne le topic du channel
		void setTopic(const std::string& topic);       // Définit le topic du channel

		/* Members */
		void addMember(int fd);                        // Ajoute un client au channel
		void removeMember(int fd);                     // Retire un client du channel
		bool isMember(int fd) const;                   // Vérifie si client est dans le channel
		int getNumberOfMembers() const;                // Retourne le nombre total de membres
		const std::set<int>& getMembers() const;       // Retourne l'ensemble de tous les fds membres

		/* Operators */
		void addOperator(int fd);                      // Promeut un membre en opérateur
		void removeOperator(int fd);                   // Rétrograde un opérateur en membre
		bool isOperator(int fd) const;                 // Vérifie si le client est opérateur

		/* Invited */
		void addInvited(int fd);                       // Ajoute un client à la liste d'invités (mode +i)
		void removeInvited(int fd);                    // Retire un client de la liste d'invités
		bool isInvited(int fd) const;                  // Vérifie si le client est invité

		/* Key (Password) */
		void setKey(const std::string& key);           // Définit le mot de passe (mode +k)
		const std::string& getKey() const;             // Retourne le mot de passe
		bool isKeyRestricted() const;                  // Vérifie si le channel a un mot de passe
	
		/* User Limit */
		void setUserLimit(int limit);                  // Définit la limite de membres (mode +l), 0 = illimité
		int getUserLimit() const;                      // Retourne la limite de membres

		/* Mode Flags */
		bool isInviteOnly() const;                     // Vérifie si invite-only (mode +i)
		bool isTopicRestricted() const;                // Vérifie si topic restreint (mode +t)
		void setInviteOnly(bool flag);                 // Active/désactive le mode invite-only
		void setTopicRestricted(bool flag);            // Active/désactive la restriction du topic

		/* Broadcasting */
		void broadcastMessage(const std::string& message, int senderFd) const;  // Envoie un message à tous les membres
};

#endif