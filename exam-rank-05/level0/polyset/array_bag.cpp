#include "array_bag.hpp"
#include <iostream>

array_bag::array_bag() : data(nullptr), size(0) {}

array_bag::array_bag(const array_bag &src) : data(nullptr), size(0)
{
	*this = src;
}

array_bag &array_bag::operator=(const array_bag &src)
{
	if (this != &src)
	{
		delete[] data;
		size = src.size;
		data = new int[size];
		for (int i = 0; i < size; i++)
			data[i] = src.data[i];
	}
	return (*this);
}

array_bag::~array_bag()
{
	delete[] data;
}

void	array_bag::insert(int item)
{
	int	*newdata = new int[size + 1];
	for (int i = 0; i < size; i++)
		newdata[i] = data[i];
	newdata[size] = item;
	delete[] data;
	data = newdata;
	size++;
}

void	array_bag::insert(int *items, int count)
{
	int	*newdata = new int[size + count];
	for (int i = 0; i < size; i++)
		newdata[i] = data[i];
	for (int i = 0; i < count; i++)
		newdata[size + i] = items[i];
	delete[] data;
	data = newdata;
	size += count;
}

void	array_bag::print() const
{
	for (int i = 0; i < size; i++)
		std::cout << data[i] << " ";
	std::cout << std::endl;
}

void	array_bag::clear()
{
	delete[] data;
	data = nullptr;
	size = 0;
}
