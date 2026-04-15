#include "tree_bag.hpp"

tree_bag::tree_bag() : tree(nullptr) {}

tree_bag::tree_bag(const tree_bag &src) : tree(nullptr)
{
	tree = copy_node(src.tree);
}

tree_bag &tree_bag::operator=(const tree_bag &src)
{
	if (this != &src)
	{
		destroy_tree(tree);
		tree = copy_node(src.tree);
	}
	return (*this);
}

tree_bag::~tree_bag()
{
	destroy_tree(tree);
}

/* ── Helpers statiques ──────────────────────────────────────────────────── */

void	tree_bag::destroy_tree(node *n)
{
	if (!n)
		return ;
	destroy_tree(n->l);
	destroy_tree(n->r);
	std::cout << "destroying value: " << n->value << std::endl;
	delete n;
}

/* Parcours in-order (valeurs triées). Ignore la valeur 0. */
void	tree_bag::print_node(node *n)
{
	if (!n)
		return ;
	print_node(n->l);
	if (n->value != 0)
		std::cout << n->value << " ";
	print_node(n->r);
}

tree_bag::node	*tree_bag::copy_node(node *n)
{
	node	*newn;

	if (!n)
		return (nullptr);
	newn = new node;
	newn->value = n->value;
	newn->l = copy_node(n->l);
	newn->r = copy_node(n->r);
	return (newn);
}

/* ── Accesseurs ─────────────────────────────────────────────────────────── */

tree_bag::node	*tree_bag::extract_tree()
{
	node	*t = tree;
	tree = nullptr;
	return (t);
}

void	tree_bag::set_tree(node *t)
{
	tree = t;
}

/* ── Méthodes publiques ─────────────────────────────────────────────────── */

/*
** Insert BST standard – les doublons sont ignorés.
*/
void	tree_bag::insert(int item)
{
	node	**curr = &tree;

	while (*curr)
	{
		if (item == (*curr)->value)
			return ;
		else if (item < (*curr)->value)
			curr = &(*curr)->l;
		else
			curr = &(*curr)->r;
	}
	*curr = new node;
	(*curr)->value = item;
	(*curr)->l = nullptr;
	(*curr)->r = nullptr;
	std::cout << "creating node: " << item << std::endl;
}

void	tree_bag::insert(int *array, int size)
{
	for (int i = 0; i < size; i++)
		insert(array[i]);
}

void	tree_bag::print() const
{
	print_node(tree);
	std::cout << std::endl;
}

void	tree_bag::clear()
{
	destroy_tree(tree);
	tree = nullptr;
}
