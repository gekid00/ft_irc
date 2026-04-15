#pragma once
#include "tree_bag.hpp"
#include "searchable_bag.hpp"

/*
** searchable_tree_bag: tree_bag + has()
** Héritage multiple – diamant sur bag résolu par virtual inheritance.
**
** À IMPLÉMENTER lors de l'examen.
*/
class searchable_tree_bag : public tree_bag, public searchable_bag
{
private:
	/* Recherche BST récursive */
	bool	search(node *n, const int value) const;

public:
	searchable_tree_bag();
	searchable_tree_bag(const searchable_tree_bag &src);
	searchable_tree_bag	&operator=(const searchable_tree_bag &src);
	virtual ~searchable_tree_bag();

	virtual bool	has(int value) const;
};
