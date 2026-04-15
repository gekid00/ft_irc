#include "bigint.hpp"

/* ─── Constructeurs ─────────────────────────────────────────────────────── */

bigint::bigint() : str("0") {}

bigint::bigint(unsigned int num)
{
	std::stringstream ss;
	ss << num;
	str = ss.str();
}

bigint::bigint(const bigint &src)
{
	*this = src;
}

bigint &bigint::operator=(const bigint &src)
{
	if (this != &src)
		str = src.str;
	return (*this);
}

std::string	bigint::getStr() const
{
	return (str);
}

/* ─── Addition ──────────────────────────────────────────────────────────── */

/*
** Additionne deux chaînes numériques:
**   1. On inverse les deux chaînes (LSB en premier)
**   2. On égalise les longueurs avec des '0'
**   3. On additionne chiffre par chiffre avec retenue
**   4. On inverse le résultat
*/
static std::string	reverse_str(const std::string &s)
{
	std::string	r;
	size_t		i;

	i = s.length();
	while (i > 0)
		r.push_back(s[--i]);
	return (r);
}

static std::string	add_strings(const bigint &a, const bigint &b)
{
	std::string	s1 = reverse_str(a.getStr());
	std::string	s2 = reverse_str(b.getStr());
	std::string	result;
	int			carry;
	int			d1;
	int			d2;
	int			sum;
	size_t		i;

	/* égalise les longueurs */
	while (s1.length() < s2.length())
		s1.push_back('0');
	while (s2.length() < s1.length())
		s2.push_back('0');

	carry = 0;
	i = 0;
	while (i < s1.length())
	{
		d1 = s1[i] - '0';
		d2 = s2[i] - '0';
		sum = d1 + d2 + carry;
		carry = sum / 10;
		result.push_back((sum % 10) + '0');
		i++;
	}
	if (carry)
		result.push_back(carry + '0');
	return (reverse_str(result));
}

bigint	bigint::operator+(const bigint &other) const
{
	bigint	temp;
	temp.str = add_strings(*this, other);
	return (temp);
}

bigint	&bigint::operator+=(const bigint &other)
{
	*this = *this + other;
	return (*this);
}

bigint	&bigint::operator++()
{
	*this = *this + bigint(1);
	return (*this);
}

bigint	bigint::operator++(int)
{
	bigint	temp = *this;
	*this = *this + bigint(1);
	return (temp);
}

/* ─── Décalage base-10 ─────────────────────────────────────────────────── */

/*
** << n  : multiplie par 10^n = ajoute n zéros à droite
** >> n  : divise par 10^n (tronque) = supprime n chars à droite
**         si n >= longueur, résultat = "0"
*/

bigint	bigint::operator<<(unsigned int n) const
{
	bigint	temp = *this;
	temp.str.insert(temp.str.end(), n, '0');
	return (temp);
}

bigint	bigint::operator>>(unsigned int n) const
{
	bigint	temp = *this;
	if (n >= temp.str.length())
		temp.str = "0";
	else
		temp.str.erase(temp.str.length() - n, n);
	return (temp);
}

bigint	&bigint::operator<<=(unsigned int n)
{
	*this = *this << n;
	return (*this);
}

bigint	&bigint::operator>>=(unsigned int n)
{
	*this = *this >> n;
	return (*this);
}

/* versions avec bigint: convertit d'abord en unsigned int */
static unsigned int	bigint_to_uint(const std::string &s)
{
	std::stringstream	ss(s);
	unsigned int		v;
	ss >> v;
	return (v);
}

bigint	bigint::operator<<(const bigint &other) const
{
	return (*this << bigint_to_uint(other.str));
}

bigint	bigint::operator>>(const bigint &other) const
{
	return (*this >> bigint_to_uint(other.str));
}

bigint	&bigint::operator<<=(const bigint &other)
{
	*this = *this << bigint_to_uint(other.str);
	return (*this);
}

bigint	&bigint::operator>>=(const bigint &other)
{
	*this = *this >> bigint_to_uint(other.str);
	return (*this);
}

/* ─── Comparaisons ──────────────────────────────────────────────────────── */

/*
** Pour comparer deux nombres sans signe et sans zéros de tête:
**   1. La chaîne plus longue est plus grande.
**   2. À longueur égale, la comparaison lexicographique est numérique.
*/
bool	bigint::operator==(const bigint &other) const
{
	return (str == other.str);
}

bool	bigint::operator!=(const bigint &other) const
{
	return (!(*this == other));
}

bool	bigint::operator<(const bigint &other) const
{
	if (str.length() != other.str.length())
		return (str.length() < other.str.length());
	return (str < other.str);
}

bool	bigint::operator>(const bigint &other) const
{
	return (other < *this);
}

bool	bigint::operator<=(const bigint &other) const
{
	return (!(*this > other));
}

bool	bigint::operator>=(const bigint &other) const
{
	return (!(*this < other));
}

/* ─── Sortie ────────────────────────────────────────────────────────────── */

std::ostream	&operator<<(std::ostream &out, const bigint &obj)
{
	out << obj.getStr();
	return (out);
}
