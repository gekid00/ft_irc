#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

/*
** t_game: toute la state du jeu en un seul endroit
** i = ligne courante du crayon (axe vertical)
** j = colonne courante du crayon (axe horizontal)
** draw = 0 crayon levé, 1 crayon posé
*/
typedef struct s_game
{
	int		width;
	int		height;
	int		iterations;
	int		i;
	int		j;
	int		draw;
	char	**board;
}	t_game;

void	init_game(t_game *g, int width, int height, int iterations);
void	fill_board(t_game *g);
void	play(t_game *g);
void	print_board(t_game *g);
void	free_board(t_game *g);
