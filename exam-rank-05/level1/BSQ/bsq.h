#pragma once
#include <stdlib.h>
#include <stdio.h>

/*
** t_elements: la config lue sur la première ligne du fichier
**   n_lines  : nombre de lignes de la carte
**   empty    : caractère = case vide
**   obstacle : caractère = obstacle
**   full     : caractère = remplissage du carré trouvé
*/
typedef struct s_elements
{
	int		n_lines;
	char	empty;
	char	obstacle;
	char	full;
}	t_elements;

/*
** t_map: la grille 2D
*/
typedef struct s_map
{
	char	**grid;
	int		width;
	int		height;
}	t_map;

/*
** t_square: le plus grand carré trouvé
**   size = côté du carré
**   i, j = coin supérieur gauche
*/
typedef struct s_square
{
	int	size;
	int	i;
	int	j;
}	t_square;

int	execute_bsq(FILE *file);
int	convert_file_pointer(char *name);
