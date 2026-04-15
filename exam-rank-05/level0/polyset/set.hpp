#pragma once
#include "searchable_bag.hpp"

/*
** set: wrapper autour d'un searchable_bag qui garantit l'unicité.
**
** Composition (pas héritage) : on stocke une référence vers un searchable_bag.
** Conséquence : pas de constructeur par défaut ni de copie (référence = non rebindable).
**
** À IMPLÉMENTER lors de l'examen.
*/
class set
{
private:
	searchable_bag	&bag;

public:
	set() = delete;
	set(const set &) = delete;
	set	&operator=(const set &) = delete;

	set(searchable_bag &b);
	~set();

	bool	has(int value) const;
	void	insert(int value);
	void	insert(int *data, int size);
	void	print() const;
	void	clear();
	const searchable_bag	&get_bag() const;
};
