#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include "parser.h"
#include "ejecutar.h"
#include "libmemoria.h"
#include "redirecciones.h"

int **crear_pipes(int n_ordenes){ //Crea tantas tuberías como ordenes-1 haya
   int **pipes = NULL;
   int i;

   pipes = (int **)malloc(sizeof(int *) * (n_ordenes - 1)); //Se asigna memoria para las tuberias
   for(i = 0; i < (n_ordenes - 1); i++){
      pipes[i]=(int *)malloc(sizeof(int) * 2); //Se asigna memoria para cada uno de los pipes
      
      pipe(pipes[i]); //Se crea la tubería
   }

    return (pipes);
}


pid_t ejecutar_orden(const char *orden, int entrada , int salida , int *pbackgr){
  char **args;
  pid_t pid; //Pid del padre
  int indice_ent = -1, indice_sal = -1; /* por defecto, no hay < ni > */
  
   if ((args = parser_orden(orden, &indice_ent, &indice_sal, pbackgr)) == NULL) //Si no se introduce orden (<ENTER>)  
   {
	
      return(-1);
   }

   /* Si la línea de órdenes posee tuberías o redirecciones, podrá incluir */
   /* aquí, en otras fases de la practica, el código para su tratamiento.  */

   if(indice_ent!=-1){ //Si hay redirección de entrada que llame a la función redirec_entrada
      redirec_entrada(args,indice_ent,&entrada);
   }
   
   if(indice_sal!=-1){ //Si hay redirección de salida que llame a la función redirec_salida
      redirec_salida(args,indice_sal,&salida);
   }

   
   pid=fork(); //Se crea un proceso hijo

   if(pid==-1){
   	perror("Error al llamar a la funcion fork");
   	exit(-1); //Si es el hijo
   }else if(pid == 0){
//Tiene que comprobar que la entrada se hace desde fichero, si no es asi, se cambia para que sea así
//Crea una copia del archivo entrada en STDIN_FILENO, y después cierra entrada
//Los datos de entrada toman descriptor 0 (STDIN_FILENO) y los de salida descriptor 1 STDOUT_FILENO
      if(entrada!=0){
         dup2(entrada,STDIN_FILENO); //Cogemos el archivo de entrada y lo duplicamos, STDIN_FILENO equivale a 1
         close(entrada);

      }
	if(salida!=1){ //Comprueba si la salida se hace en el fichero en vez de por pantalla, si no es así, lo cambia para que sea así
         dup2(salida,STDOUT_FILENO); //Igual que el anterior, pero en este caso STDIN_FILENO equivale a 0
         close(salida);

      }
   	if(execvp(args[0], args) == -1){   //En caso de que el execvp funcione ejecuta la orden y sale de este programa
   	    perror("Error al tratar de ejecutar");
   	    exit(-1);
   	}
   }
	if(entrada!=0){ //El padre ha de cerrar la entrada y salida en el caso de ser distintos de 0 y 1 respectivamente
         close(entrada);
   }
   	if (salida!=1){
         close(salida);
   }
      
   


   free_argumentos(args); //Liberación de array dinámico
   return(pid);	
}
 
void ejecutar_linea_ordenes(const char *orden){
   char **ordenes;
   int n_ordenes; //Número de órdenes
   pid_t *pids = NULL; //Asigna el número de órdenes
   int **pipes;
   int entrada,salida,i;
   int backgr;
   int estado;
   int opciones=0; //Puede modificar el comportamiento de la espera del waitpid

   ordenes=parser_pipes(orden,&n_ordenes); //Convierte la cadena asociada a una orden compuesta
   pipes=crear_pipes(n_ordenes); //Crea tantas tuberías como número de ordenes menos uno hayan pasado (si hay n órdenes, hay n-1 órdenes intermedias)
   pids=(int *)malloc(sizeof(int) * n_ordenes); //Asignamos memoria a pids igual al numero de ordenes

   for(i=0;i<n_ordenes;i++){
      if(i==0){ //Primera tubería

         if(n_ordenes>1){ //Si hay tuberías
            salida=pipes[0][1];
         }
	  else{ //Si no hay tuberías
            salida = STDOUT_FILENO;
         }
	 pids[0] = ejecutar_orden(ordenes[0], 0, salida, &backgr);  //Ejecución de la primera orden exista o no tubería

      } else if( (i == n_ordenes-1) && (n_ordenes>1) ){ //Última tubería, siempre que haya más de una orden
	 entrada=pipes[n_ordenes-2][0];
         salida=STDOUT_FILENO;
	 pids[i]=ejecutar_orden(ordenes[i], entrada, salida, &backgr); //Ejecución de la ultima orden exista o no tubería

      } else {  //Tuberías intermedias
	 salida=pipes[i][1];
         entrada=pipes[i-1][0];
	 pids[i]=ejecutar_orden(ordenes[i], entrada, salida, &backgr); //Ejecutamos la orden intermedia exista o no tubería
      }

   }

   if((backgr==0) && (pids[n_ordenes-1] > 0) ){ //Si no hay background y es la última orden
       waitpid(pids[n_ordenes-1], &estado, opciones); //El proceso padre espera finalización del proceso hijo
     }
   
   //Liberacion de la memoria dinamica usada
   free_ordenes_pipes(ordenes,pipes,n_ordenes);
   free(pids);

}   
