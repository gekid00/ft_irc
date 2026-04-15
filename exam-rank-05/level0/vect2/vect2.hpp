#pragma once
#include <iostream>

class vect2
{
private:
	int	x;
	int	y;

public:
	vect2();
	vect2(int x, int y);
	vect2(const vect2 &src);
	vect2	&operator=(const vect2 &src);

	/* accès indexé : v[0]=x, v[1]=y */
	int		&operator[](int index);
	const int	&operator[](int index) const;

	/* négation unaire */
	vect2	operator-() const;

	/* arithmétique vecteur */
	vect2	operator+(const vect2 &other) const;
	vect2	operator-(const vect2 &other) const;
	vect2	&operator+=(const vect2 &other);
	vect2	&operator-=(const vect2 &other);

	/* multiplication scalaire (vect2 * int) */
	vect2	operator*(int scalar) const;
	vect2	&operator*=(int scalar);

	/* incrément / décrément */
	vect2	&operator++();
	vect2	operator++(int);
	vect2	&operator--();
	vect2	operator--(int);

	/* comparaisons */
	bool	operator==(const vect2 &other) const;
	bool	operator!=(const vect2 &other) const;
};

/* multiplication scalaire inversée : int * vect2 */
vect2		operator*(int scalar, const vect2 &v);

/* sortie : "{x, y}" */
std::ostream	&operator<<(std::ostream &out, const vect2 &v);
