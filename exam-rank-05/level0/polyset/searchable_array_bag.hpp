#pragma once
#include "array_bag.hpp"
#include "searchable_bag.hpp"

/*
** searchable_array_bag: array_bag + has()
** Héritage multiple – le diamant sur bag est résolu par virtual inheritance.
**
** À IMPLÉMENTER lors de l'examen.
*/
class searchable_array_bag : public array_bag, public searchable_bag
{
public:
	searchable_array_bag();
	searchable_array_bag(const searchable_array_bag &src);
	searchable_array_bag	&operator=(const searchable_array_bag &src);
	virtual ~searchable_array_bag();

	virtual bool	has(int value) const;
};
