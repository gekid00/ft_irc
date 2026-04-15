#pragma once
#include <iostream>
#include <string>
#include <sstream>

/*
** bigint: entier non-signé de précision arbitraire, stocké comme std::string.
** Ex: bigint(42) → str = "42"
**
** Opérateurs à implémenter:
**   +, +=, ++prefix, ++postfix
**   <<, >>, <<=, >>= (décalage base-10 : << ajoute des zéros, >> en supprime)
**   ==, !=, <, >, <=, >=
**   << (ostream)
*/
class bigint
{
private:
	std::string	str;

public:
	bigint();
	bigint(unsigned int num);
	bigint(const bigint &src);
	bigint	&operator=(const bigint &src);

	std::string	getStr() const;

	/* arithmétique */
	bigint	operator+(const bigint &other) const;
	bigint	&operator+=(const bigint &other);
	bigint	&operator++();
	bigint	operator++(int);

	/* décalage base-10 par entier */
	bigint	operator<<(unsigned int n) const;
	bigint	operator>>(unsigned int n) const;
	bigint	&operator<<=(unsigned int n);
	bigint	&operator>>=(unsigned int n);

	/* décalage base-10 par bigint */
	bigint	operator<<(const bigint &other) const;
	bigint	operator>>(const bigint &other) const;
	bigint	&operator<<=(const bigint &other);
	bigint	&operator>>=(const bigint &other);

	/* comparaisons */
	bool	operator==(const bigint &other) const;
	bool	operator!=(const bigint &other) const;
	bool	operator<(const bigint &other) const;
	bool	operator>(const bigint &other) const;
	bool	operator<=(const bigint &other) const;
	bool	operator>=(const bigint &other) const;
};

/* opérateur de sortie (non-membre) */
std::ostream	&operator<<(std::ostream &out, const bigint &obj);
