#include "bsq.h"

/*
** Libère un tableau de chaînes terminé par NULL.
*/
static void	free_map(char **grid)
{
	int	i;

	i = 0;
	if (!grid)
		return ;
	while (grid[i])
		free(grid[i++]);
	free(grid);
}

/*
** Lit la première ligne: "{n_lines}{empty}{obstacle}{full}"
** Ex: "5.ox" → n=5, empty='.', obstacle='o', full='x'
** fscanf lit l'entier puis les 3 chars directement (pas de skip espace).
*/
static int	load_elements(FILE *file, t_elements *e)
{
	int	ret;

	ret = fscanf(file, "%d%c%c%c",
			&e->n_lines, &e->empty, &e->obstacle, &e->full);
	if (ret != 4)
		return (-1);
	if (e->n_lines <= 0)
		return (-1);
	if (e->empty == e->obstacle || e->empty == e->full
		|| e->obstacle == e->full)
		return (-1);
	if (e->empty < 32 || e->empty > 126)
		return (-1);
	if (e->obstacle < 32 || e->obstacle > 126)
		return (-1);
	if (e->full < 32 || e->full > 126)
		return (-1);
	return (0);
}

/*
** Vérifie que toutes les cellules de la grille
** ne contiennent que empty ou obstacle.
*/
static int	check_cells(char **grid, char empty, char obstacle)
{
	int	i;
	int	j;

	i = 0;
	while (grid[i])
	{
		j = 0;
		while (grid[i][j])
		{
			if (grid[i][j] != empty && grid[i][j] != obstacle)
				return (-1);
			j++;
		}
		i++;
	}
	return (0);
}

/*
** Lit les n_lines lignes de carte depuis le fichier.
** Chaque ligne doit se terminer par '\n'.
** Toutes les lignes doivent avoir la même largeur.
** La première getline() consomme le '\n' de fin de ligne d'en-tête.
*/
static int	load_map(FILE *file, t_map *map, t_elements *e)
{
	char	*line;
	size_t	len;
	int		read;
	int		i;

	map->height = e->n_lines;
	map->grid = (char **)malloc((map->height + 1) * sizeof(char *));
	if (!map->grid)
		return (-1);
	map->grid[map->height] = NULL;
	line = NULL;
	len = 0;
	/* consomme le '\n' de fin de la ligne d'en-tête */
	if (getline(&line, &len, file) == -1)
	{
		free(line);
		free_map(map->grid);
		return (-1);
	}
	i = 0;
	while (i < map->height)
	{
		read = (int)getline(&line, &len, file);
		if (read == -1)
		{
			free(line);
			free_map(map->grid);
			return (-1);
		}
		/* la ligne doit se terminer par '\n' */
		if (line[read - 1] != '\n')
		{
			free(line);
			free_map(map->grid);
			return (-1);
		}
		read--;
		/* toutes les lignes doivent avoir la même largeur */
		if (i == 0)
			map->width = read;
		else if (map->width != read)
		{
			free(line);
			free_map(map->grid);
			return (-1);
		}
		map->grid[i] = (char *)malloc((read + 1) * sizeof(char));
		if (!map->grid[i])
		{
			free(line);
			free_map(map->grid);
			return (-1);
		}
		map->grid[i][read] = '\0';
		while (read-- > 0)
			map->grid[i][read] = line[read];
		i++;
	}
	free(line);
	if (check_cells(map->grid, e->empty, e->obstacle) == -1)
	{
		free_map(map->grid);
		return (-1);
	}
	return (0);
}

static int	find_min(int a, int b, int c)
{
	int	min;

	min = a;
	if (b < min)
		min = b;
	if (c < min)
		min = c;
	return (min);
}

/*
** Algorithme DP pour trouver le plus grand carré sans obstacle.
**
** dp[i][j] = côté du plus grand carré avec coin bas-droit en (i,j)
**   - obstacle → 0
**   - bord gauche ou bord supérieur → 1 (si pas obstacle)
**   - sinon → min(dp[i-1][j], dp[i][j-1], dp[i-1][j-1]) + 1
**
** On itère top→bottom, left→right et on ne met à jour le max
** que quand dp > max existant : ça garantit "le plus haut, puis le plus gauche".
*/
static void	find_big_square(t_map *map, t_square *sq, t_elements *e)
{
	int	(*dp)[map->width];
	int	i;
	int	j;

	dp = (int (*)[map->width])calloc(map->height * map->width, sizeof(int));
	i = 0;
	while (i < map->height)
	{
		j = 0;
		while (j < map->width)
		{
			if (map->grid[i][j] == e->obstacle)
				dp[i][j] = 0;
			else if (i == 0 || j == 0)
				dp[i][j] = 1;
			else
				dp[i][j] = find_min(dp[i - 1][j],
						dp[i][j - 1], dp[i - 1][j - 1]) + 1;
			if (dp[i][j] > sq->size)
			{
				sq->size = dp[i][j];
				sq->i = i - dp[i][j] + 1;
				sq->j = j - dp[i][j] + 1;
			}
			j++;
		}
		i++;
	}
	free(dp);
}

/*
** Remplace les cellules du carré par e->full puis affiche la grille.
*/
static void	print_filled(t_map *map, t_square *sq, t_elements *e)
{
	int	i;
	int	j;

	i = sq->i;
	while (i < sq->i + sq->size)
	{
		j = sq->j;
		while (j < sq->j + sq->size)
		{
			map->grid[i][j] = e->full;
			j++;
		}
		i++;
	}
	i = 0;
	while (i < map->height)
	{
		fputs(map->grid[i], stdout);
		fputc('\n', stdout);
		i++;
	}
}

/*
** Point d'entrée pour traiter un FILE*.
*/
int	execute_bsq(FILE *file)
{
	t_elements	e;
	t_map		map;
	t_square	sq;

	if (load_elements(file, &e) == -1)
		return (-1);
	if (load_map(file, &map, &e) == -1)
		return (-1);
	sq.size = 0;
	sq.i = 0;
	sq.j = 0;
	find_big_square(&map, &sq, &e);
	print_filled(&map, &sq, &e);
	free_map(map.grid);
	return (0);
}

/*
** Ouvre un fichier par son nom et appelle execute_bsq.
*/
int	convert_file_pointer(char *name)
{
	FILE	*file;
	int		ret;

	file = fopen(name, "r");
	if (!file)
		return (-1);
	ret = execute_bsq(file);
	fclose(file);
	return (ret);
}
