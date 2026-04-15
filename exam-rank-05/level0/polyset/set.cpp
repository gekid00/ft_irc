#include "set.hpp"

set::set(searchable_bag &b) : bag(b) {}

set::~set() {}

/*
** has(): délègue au bag sous-jacent.
*/
bool	set::has(int value) const
{
	return (bag.has(value));
}

/*
** insert(int): n'insère que si la valeur n'est pas déjà présente.
** C'est la règle fondamentale d'un ensemble (pas de doublons).
*/
void	set::insert(int value)
{
	if (!has(value))
		bag.insert(value);
}

void	set::insert(int *data, int size)
{
	for (int i = 0; i < size; i++)
		insert(data[i]);
}

void	set::print() const
{
	bag.print();
}

void	set::clear()
{
	bag.clear();
}

const searchable_bag	&set::get_bag() const
{
	return (bag);
}
