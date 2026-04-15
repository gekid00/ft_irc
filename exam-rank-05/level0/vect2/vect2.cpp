#include "vect2.hpp"

/* ─── Constructeurs ─────────────────────────────────────────────────────── */

vect2::vect2() : x(0), y(0) {}

vect2::vect2(int x, int y) : x(x), y(y) {}

vect2::vect2(const vect2 &src) : x(src.x), y(src.y) {}

vect2 &vect2::operator=(const vect2 &src)
{
	if (this != &src)
	{
		x = src.x;
		y = src.y;
	}
	return (*this);
}

/* ─── Accès indexé ──────────────────────────────────────────────────────── */

int &vect2::operator[](int index)
{
	if (index == 0)
		return (x);
	return (y);
}

const int &vect2::operator[](int index) const
{
	if (index == 0)
		return (x);
	return (y);
}

/* ─── Négation unaire ───────────────────────────────────────────────────── */

vect2 vect2::operator-() const
{
	return (vect2(-x, -y));
}

/* ─── Arithmétique vecteur ──────────────────────────────────────────────── */

vect2 vect2::operator+(const vect2 &other) const
{
	return (vect2(x + other.x, y + other.y));
}

vect2 vect2::operator-(const vect2 &other) const
{
	return (vect2(x - other.x, y - other.y));
}

vect2 &vect2::operator+=(const vect2 &other)
{
	x += other.x;
	y += other.y;
	return (*this);
}

vect2 &vect2::operator-=(const vect2 &other)
{
	x -= other.x;
	y -= other.y;
	return (*this);
}

/* ─── Multiplication scalaire ───────────────────────────────────────────── */

vect2 vect2::operator*(int scalar) const
{
	return (vect2(x * scalar, y * scalar));
}

vect2 &vect2::operator*=(int scalar)
{
	x *= scalar;
	y *= scalar;
	return (*this);
}

/* non-membre : int * vect2 */
vect2 operator*(int scalar, const vect2 &v)
{
	return (v * scalar);
}

/* ─── Incrément / Décrément ─────────────────────────────────────────────── */

vect2 &vect2::operator++()		/* prefix : ++v */
{
	++x;
	++y;
	return (*this);
}

vect2 vect2::operator++(int)	/* postfix : v++ */
{
	vect2 tmp = *this;
	++(*this);
	return (tmp);
}

vect2 &vect2::operator--()		/* prefix : --v */
{
	--x;
	--y;
	return (*this);
}

vect2 vect2::operator--(int)	/* postfix : v-- */
{
	vect2 tmp = *this;
	--(*this);
	return (tmp);
}

/* ─── Comparaisons ──────────────────────────────────────────────────────── */

bool vect2::operator==(const vect2 &other) const
{
	return (x == other.x && y == other.y);
}

bool vect2::operator!=(const vect2 &other) const
{
	return (!(*this == other));
}

/* ─── Sortie ────────────────────────────────────────────────────────────── */

std::ostream &operator<<(std::ostream &out, const vect2 &v)
{
	out << "{" << v[0] << ", " << v[1] << "}";
	return (out);
}
