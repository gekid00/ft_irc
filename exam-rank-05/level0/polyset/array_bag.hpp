#pragma once
#include "bag.hpp"

/*
** array_bag: implémentation de bag sur tableau dynamique.
** Fourni par l'examen – NE PAS MODIFIER.
*/
class array_bag : virtual public bag
{
protected:
	int	*data;
	int	size;

public:
	array_bag();
	array_bag(const array_bag &src);
	array_bag	&operator=(const array_bag &src);
	virtual ~array_bag();

	virtual void	insert(int item);
	virtual void	insert(int *items, int count);
	virtual void	print() const;
	virtual void	clear();
};
