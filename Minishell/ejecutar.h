#ifndef EJECUTAR_H
#define EJECUTAR_H
#include <unistd.h>

void ejecutar_linea_ordenes(const char *orden);
pid_t ejecutar_orden(const char *orden, int entrada, int salida, int *pbackgr);

#endif
