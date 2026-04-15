#include "life.h"

/*
** Alloue la grille (tableau 2D de chars) et initialise à ' ' (mort).
** Toutes les cellules = espace = dead.
*/
void	init_game(t_game *g, int width, int height, int iterations)
{
	int	row;
	int	col;

	g->width = width;
	g->height = height;
	g->iterations = iterations;
	g->i = 0;
	g->j = 0;
	g->draw = 0;
	g->board = (char **)calloc(height, sizeof(char *));
	row = 0;
	while (row < height)
	{
		g->board[row] = (char *)malloc((width + 1) * sizeof(char));
		col = 0;
		while (col < width)
		{
			g->board[row][col] = ' ';
			col++;
		}
		g->board[row][width] = '\0';
		row++;
	}
}

/*
** Lecture depuis stdin:
**   w/s/a/d = déplacer le crayon (bloqué aux bords)
**   x       = toggle mode dessin
** Après chaque commande, si draw=1, on marque la cellule courante 'O'.
*/
void	fill_board(t_game *g)
{
	char	c;

	while (read(0, &c, 1) > 0)
	{
		if (c == 'w' && g->i > 0)
			g->i--;
		else if (c == 's' && g->i < g->height - 1)
			g->i++;
		else if (c == 'a' && g->j > 0)
			g->j--;
		else if (c == 'd' && g->j < g->width - 1)
			g->j++;
		else if (c == 'x')
			g->draw = !g->draw;
		if (g->draw)
			g->board[g->i][g->j] = 'O';
	}
}

/*
** Compte les voisins vivants (8 directions) d'une cellule.
** Les cellules hors-grille comptent comme mortes.
*/
static int	count_neighbors(t_game *g, int row, int col)
{
	int	count;
	int	dr;
	int	dc;
	int	nr;
	int	nc;

	count = 0;
	dr = -1;
	while (dr <= 1)
	{
		dc = -1;
		while (dc <= 1)
		{
			if (!(dr == 0 && dc == 0))
			{
				nr = row + dr;
				nc = col + dc;
				if (nr >= 0 && nr < g->height && nc >= 0 && nc < g->width)
					if (g->board[nr][nc] == 'O')
						count++;
			}
			dc++;
		}
		dr++;
	}
	return (count);
}

/*
** Une génération de Conway :
**   - vivant + 2 ou 3 voisins → survit
**   - mort   + exactement 3 voisins → naît
**   - sinon → mort
** On alloue une nouvelle grille, on calcule, on remplace l'ancienne.
*/
void	play(t_game *g)
{
	char	**next;
	int		iter;
	int		row;
	int		col;
	int		n;

	iter = 0;
	while (iter < g->iterations)
	{
		next = (char **)malloc(g->height * sizeof(char *));
		row = 0;
		while (row < g->height)
		{
			next[row] = (char *)malloc((g->width + 1) * sizeof(char));
			col = 0;
			while (col < g->width)
			{
				n = count_neighbors(g, row, col);
				if (g->board[row][col] == 'O')
					next[row][col] = (n == 2 || n == 3) ? 'O' : ' ';
				else
					next[row][col] = (n == 3) ? 'O' : ' ';
				col++;
			}
			next[row][col] = '\0';
			row++;
		}
		row = 0;
		while (row < g->height)
		{
			free(g->board[row]);
			row++;
		}
		free(g->board);
		g->board = next;
		iter++;
	}
}

/*
** Affiche la grille ligne par ligne avec putchar.
*/
void	print_board(t_game *g)
{
	int	row;
	int	col;

	row = 0;
	while (row < g->height)
	{
		col = 0;
		while (col < g->width)
		{
			putchar(g->board[row][col]);
			col++;
		}
		putchar('\n');
		row++;
	}
}

/*
** Libère la grille.
*/
void	free_board(t_game *g)
{
	int	row;

	row = 0;
	while (row < g->height)
	{
		free(g->board[row]);
		row++;
	}
	free(g->board);
}

/*
** Usage: ./life width height iterations
** Fonctions autorisées: atoi, read, putchar, malloc, calloc, realloc, free
*/
int	main(int argc, char **argv)
{
	int		width;
	int		height;
	int		iterations;
	t_game	g;

	if (argc != 4)
		return (1);
	width = atoi(argv[1]);
	height = atoi(argv[2]);
	iterations = atoi(argv[3]);
	if (width <= 0 || height <= 0 || iterations < 0)
		return (1);
	init_game(&g, width, height, iterations);
	fill_board(&g);
	play(&g);
	print_board(&g);
	free_board(&g);
	return (0);
}
