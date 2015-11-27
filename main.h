// --------------------------
// > HEADER PRINCIPAL à inclure dans tous les fichiers .c

#include <stdio.h>
#include <stdlib.h>

#define DEBUG

#ifndef  DEBUG
#define ASSERT(n)
#else
#define ASSERT(n) \
if(!(n)){ \
printf ("%s - fail ", #n);\
printf ("On %s ",__DATE__);\
printf ("At %s ",__TIME__);\
printf ("In File %s ",__FILE__);\
printf ("At Line %i ",__LINE__);\
exit(1);}
#endif

const int ib = 1;
#define is_bigendian() ( (*(char*)&ib) == 0 )
#define BLOCK_SIZE 1024




// --------------------------
// Définition des structures

typedef struct {
  char octets[BLOCK_SIZE]; // tableau contenant chaque octet
  int free; // nombre d'octets disponibles dans le block
} block;

typedef struct {
  int val;
} error;

typedef struct {
  int id;
} disk_id;
