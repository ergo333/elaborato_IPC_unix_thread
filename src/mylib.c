/******************************************
MODULO: mylib.c
SCOPO: libreria di funzioni d'utilita`
******************************************/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "mylib.h"

void syserr (char *prog, char *msg)
{
  fprintf (stderr, "%s - error: %s\n",prog, msg);
  perror ("system error");
  exit (1);
}
