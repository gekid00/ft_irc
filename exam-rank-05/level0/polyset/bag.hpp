#pragma once

/*
** bag: interface abstraite (multiset).
** Fourni par l'examen – NE PAS MODIFIER.
*/
class bag
{
public:
	virtual void	insert(int) = 0;
	virtual void	insert(int *, int) = 0;
	virtual void	print() const = 0;
	virtual void	clear() = 0;
};
