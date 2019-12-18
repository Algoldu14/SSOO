#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "internas.h"
#include "entrada_minishell.h" 
#include "ejecutar.h"



static void manejar_sigchild(int signo){  //Se envía esta señal cuando uno de estos procesos termina, es decir, elimina al zombi
	int estado;
	waitpid(-1,&estado,WNOHANG); //Espera la finalización del proceso y guarda su estado de finalización. WNOHANG hace que el proceso no se bloquee
}


int main(int argc, char *argv[]){
	char buf[BUFSIZ]; //Almacena la dirección del puntero donde se guarda la entrada del prompt
	char *cadena;
	char *orden;
	struct sigaction sa; //Construimos una estructura encapsulada para el manejo de señales  

   

	memset(&sa,0,sizeof(sa)); //Permite modificar el comportamiento de la señal
	sa.sa_handler = manejar_sigchild; //Recogemos los procesos muertos
	sa.sa_flags = SA_NOCLDSTOP | SA_RESTART; //La primera flag hace que no se devuelva nada cuando el hijo zombie muere. la segunda controla lo que ocurre cuando se ejecutan ciertas acciones(open, read, write) para que el manejador de la señal vuelva a la normalidad

 	sigaction(SIGCHLD, &sa, NULL);//Se usa para cambiar la acción que lleva a cabo un proceso cuando este recibe una determinada señal 



	while (1){ 

		imprimir_prompt(); //Muestra el prompt
		leer_linea_ordenes(buf);//Lee la orden introducida por el usuario
		cadena= strdup(buf); //Duplica la orden introducida para posteriormente dividirla

		while ((orden = strsep(&cadena, ";")) != NULL) {  //Extrae un elemento de la cadena, ; es el delimitador que separa las dos funciones, con el while se podrán dividir las veces que queramos 


			if(strcmp(orden,"exit")!=0){ //Compara si el buf es exit, si es 0 ejecuta el exit y sale

				if(es_ord_interna(orden)!=0){ //Comprobamos si es orden interna

					ejecutar_ord_interna(orden); //Ejecutamos la oden interna 

		}		else{

					ejecutar_linea_ordenes(orden); //Se ejecuta la orden externa

				}

		}	else{

				exit(1);	

			}

		}

	}

   exit(EXIT_SUCCESS); 
   return 0; 

}
