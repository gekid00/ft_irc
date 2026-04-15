#pragma once
#include "bag.hpp"
#include <iostream>

/*
** tree_bag: implémentation de bag sur arbre binaire de recherche (BST).
** Fourni par l'examen – NE PAS MODIFIER.
**
** Remarque: les doublons sont ignorés (BST sans duplication).
** La structure node a des membres l (gauche) et r (droite).
*/
class tree_bag : virtual public bag
{
protected:
	struct node
	{
		node	*l;
		node	*r;
		int		value;
	};
	node	*tree;

private:
	static void		destroy_tree(node *n);
	static void		print_node(node *n);
	static node		*copy_node(node *n);

public:
	tree_bag();
	tree_bag(const tree_bag &src);
	tree_bag	&operator=(const tree_bag &src);
	virtual ~tree_bag();

	node	*extract_tree();
	void	set_tree(node *t);

	virtual void	insert(int item);
	virtual void	insert(int *array, int size);
	virtual void	print() const;
	virtual void	clear();
};
