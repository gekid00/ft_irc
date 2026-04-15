#pragma once
#include "bag.hpp"

/*
** searchable_bag: ajoute la méthode has() à bag.
** Héritage virtuel pour éviter le problème diamant.
** Fourni par l'examen – NE PAS MODIFIER.
*/
class searchable_bag : virtual public bag
{
public:
	virtual bool	has(int) const = 0;
};
