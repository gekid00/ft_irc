#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>

/* ========== STRING UTILITIES ========== */

// Nettoyer les \r\n de la fin d'une ligne
inline std::string cleanLine(const std::string& line)
{
	std::string cleaned = line;
	while (!cleaned.empty() && (cleaned[cleaned.size() - 1] == '\r' || cleaned[cleaned.size() - 1] == '\n'))
		cleaned.erase(cleaned.size() - 1);
	return cleaned;
}

// Convertir une string en UPPERCASE
inline std::string toUpperCase(const std::string& str)
{
	std::string result = str;
	for (size_t i = 0; i < result.size(); ++i)
	{
		if (result[i] >= 'a' && result[i] <= 'z')
			result[i] = result[i] - 32;  // Convertir en majuscule
	}
	return result;
}

// Vérifier si c'est un channel valide (commence par # ou &)
inline bool isValidChannel(const std::string& name)
{
	return !name.empty() && (name[0] == '#' || name[0] == '&');
}

/* ========== PARSING UTILITIES ========== */

// Split une string sur un délimiteur
inline std::vector<std::string> split(const std::string& str, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	
	for (size_t i = 0; i < str.size(); ++i)
	{
		if (str[i] == delimiter)
		{
			if (!token.empty())
				tokens.push_back(token);
			token.clear();
		}
		else
		{
			token += str[i];
		}
	}
	if (!token.empty())
		tokens.push_back(token);
	
	return tokens;
}

// Split IRC command line (tokenizes on spaces with trailing support)
// Supports IRC trailing parameter: "command param1 :trailing message with spaces"
inline std::vector<std::string> split(const std::string& line)
{
	std::vector<std::string> tokens;
	std::string token;
	bool inTrailing = false;
	std::string cleaned = cleanLine(line);

	for (size_t i = 0; i < cleaned.size(); ++i)
	{
		char c = cleaned[i];
		
		// IRC trailing parameter starts with ':'
		if (c == ':' && token.empty() && !inTrailing)
		{
			inTrailing = true;
			token += c;
			continue;
		}
		
		// In trailing mode, capture everything (including spaces)
		if (inTrailing)
		{
			token += c;
			continue;
		}
		
		// Normal mode: split on spaces
		if (c == ' ')
		{
			if (!token.empty())
			{
				tokens.push_back(token);
				token.clear();
			}
		}
		else
		{
			token += c;
		}
	}
	
	if (!token.empty())
		tokens.push_back(token);
	
	return tokens;
}

#endif
