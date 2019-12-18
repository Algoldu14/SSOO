#include <stdlib.h>

#include "libmemoria.h"


void free_argumentos(char **args)
{
   int i = 0;

   while(args[i])
      free(args[i++]);
   free(args);
}

void free_ordenes_pipes(char **ordenes, int **fds, int n_ordenes)
{
   int i = 0;

   for (i = 0; i < n_ordenes; i++)
   {
      free(ordenes[i]);
      if (i < (n_ordenes - 1))
         free(fds[i]);
   }

   free(ordenes);
   free(fds);
}
