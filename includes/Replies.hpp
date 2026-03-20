#ifndef REPLIES_HPP
#define REPLIES_HPP

#include <string>

/* ========== SUCCESS REPLY CODES ========== */

// 001: Registration complétée (PASS + NICK + USER)
#define RPL_WELCOME_CODE        001

// 324: Modes actuels du channel
#define RPL_CHANNELMODEIS_CODE  324

// 331: Topic demandé mais n'existe pas
#define RPL_NOTOPIC_CODE        331

// 332: Topic demandé et existe
#define RPL_TOPIC_CODE          332

// 341: Confirmation d'invitation
#define RPL_INVITING_CODE       341

// 353: Liste des membres du channel
#define RPL_NAMREPLY_CODE       353

// 366: Fin de la liste des membres
#define RPL_ENDOFNAMES_CODE     366

/* ========== ERROR CODES ========== */

// 401: Nickname/user n'existe pas
#define ERR_NOSUCHNICK_CODE         401

// 403: Channel n'existe pas
#define ERR_NOSUCHCHANNEL_CODE      403

// 411: Pas de destinataire (PRIVMSG)
#define ERR_NORECIPIENT_CODE        411

// 412: Pas de texte à envoyer (PRIVMSG)
#define ERR_NOTEXTTOSEND_CODE       412

// 431: Commande NICK sans argument
#define ERR_NONICKNAMEGIVEN_CODE    431

// 433: Nickname déjà utilisé
#define ERR_NICKNAMEINUSE_CODE      433

// 441: User pas dans le channel (KICK)
#define ERR_USERNOTINCHANNEL_CODE   441

// 442: Client pas dans le channel
#define ERR_NOTONCHANNEL_CODE       442

// 443: User déjà dans le channel (INVITE)
#define ERR_USERONCHANNEL_CODE      443

// 451: Client pas enregistré
#define ERR_NOTREGISTERED_CODE      451

// 461: Pas assez de paramètres
#define ERR_NEEDMOREPARAMS_CODE     461

// 462: Client déjà enregistré
#define ERR_ALREADYREGISTERED_CODE  462

// 464: Mauvais mot de passe
#define ERR_PASSWDMISMATCH_CODE     464

// 471: Channel plein (user limit atteint)
#define ERR_CHANNELISFULL_CODE      471

// 472: Mode inconnu
#define ERR_UNKNOWNMODE_CODE        472

// 473: Channel invite-only
#define ERR_INVITEONLYCHA_CODE      473

// 475: Mauvaise clé du channel
#define ERR_BADCHANNELKEY_CODE      475

// 482: Pas opérateur du channel
#define ERR_CHANOPRIVSNEEDED_CODE   482

/* ========== HELPER FUNCTIONS ========== */

// ---- RPL ----

inline std::string rpl_welcome(const std::string& nick) {
	return ":server 001 " + nick + " :Welcome to the IRC server\r\n";
}

inline std::string rpl_channelmodeis(const std::string& nick, const std::string& channel, const std::string& modes) {
	return ":server 324 " + nick + " " + channel + " " + modes + "\r\n";
}

inline std::string rpl_notopic(const std::string& nick, const std::string& channel) {
	return ":server 331 " + nick + " " + channel + " :No topic is set\r\n";
}

inline std::string rpl_topic(const std::string& nick, const std::string& channel, const std::string& topic) {
	return ":server 332 " + nick + " " + channel + " :" + topic + "\r\n";
}

inline std::string rpl_inviting(const std::string& nick, const std::string& invited, const std::string& channel) {
	return ":server 341 " + nick + " " + invited + " " + channel + "\r\n";
}

inline std::string rpl_namreply(const std::string& nick, const std::string& channel, const std::string& names) {
	return ":server 353 " + nick + " = " + channel + " :" + names + "\r\n";
}

inline std::string rpl_endofnames(const std::string& nick, const std::string& channel) {
	return ":server 366 " + nick + " " + channel + " :End of /NAMES list\r\n";
}

// ---- ERR ----

inline std::string err_nosuchnick(const std::string& nick, const std::string& target) {
	return ":server 401 " + nick + " " + target + " :No such nick/channel\r\n";
}

inline std::string err_nosuchchannel(const std::string& nick, const std::string& channel) {
	return ":server 403 " + nick + " " + channel + " :No such channel\r\n";
}

inline std::string err_badchanmask(const std::string& nick, const std::string& channel) {
	return ":server 403 " + nick + " " + channel + " :No such channel\r\n";
}

inline std::string err_norecipient(const std::string& nick, const std::string& command) {
	return ":server 411 " + nick + " :No recipient given (" + command + ")\r\n";
}

inline std::string err_notexttosend(const std::string& nick) {
	return ":server 412 " + nick + " :No text to send\r\n";
}

inline std::string err_nonicknamegiven(const std::string& nick) {
	return ":server 431 " + nick + " :No nickname given\r\n";
}

inline std::string err_nicknameinuse(const std::string& nick) {
	return ":server 433 * " + nick + " :Nickname is already in use\r\n";
}

inline std::string err_usernotinchannel(const std::string& nick, const std::string& target, const std::string& channel) {
	return ":server 441 " + nick + " " + target + " " + channel + " :They aren't on that channel\r\n";
}

inline std::string err_notonchannel(const std::string& nick, const std::string& channel) {
	return ":server 442 " + nick + " " + channel + " :You're not on that channel\r\n";
}

inline std::string err_useronchannel(const std::string& nick, const std::string& target, const std::string& channel) {
	return ":server 443 " + nick + " " + target + " " + channel + " :is already on channel\r\n";
}

inline std::string err_notregistered() {
	return ":server 451 * :You have not registered\r\n";
}

inline std::string err_needmoreparams(const std::string& nick, const std::string& command) {
	return ":server 461 " + nick + " " + command + " :Not enough parameters\r\n";
}

inline std::string err_alreadyregistered(const std::string& nick) {
	return ":server 462 " + nick + " :You may not reregister\r\n";
}

inline std::string err_passwdmismatch() {
	return ":server 464 * :Password incorrect\r\n";
}

inline std::string err_channelisfull(const std::string& nick, const std::string& channel) {
	return ":server 471 " + nick + " " + channel + " :Cannot join channel (+l)\r\n";
}

inline std::string err_unknownmode(const std::string& nick, const std::string& mode) {
	return ":server 472 " + nick + " " + mode + " :is unknown mode char to me\r\n";
}

inline std::string err_inviteonlycha(const std::string& nick, const std::string& channel) {
	return ":server 473 " + nick + " " + channel + " :Cannot join channel (+i)\r\n";
}

inline std::string err_badchannelkey(const std::string& nick, const std::string& channel) {
	return ":server 475 " + nick + " " + channel + " :Cannot join channel (+k)\r\n";
}

inline std::string err_chanoprivsneeded(const std::string& nick, const std::string& channel) {
	return ":server 482 " + nick + " " + channel + " :You're not channel operator\r\n";
}

#endif