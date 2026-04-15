#include "bsq.h"

/*
** Sans argument → lit depuis stdin.
** Avec arguments → traite chaque fichier, séparés par une ligne vide.
*/
int	main(int argc, char *argv[])
{
	int	i;

	if (argc == 1)
	{
		if (execute_bsq(stdin) == -1)
			fprintf(stderr, "map error\n");
	}
	else
	{
		i = 1;
		while (i < argc)
		{
			if (convert_file_pointer(argv[i]) == -1)
				fprintf(stderr, "map error\n");
			if (i < argc - 1)
				fprintf(stdout, "\n");
			i++;
		}
	}
	return (0);
}
