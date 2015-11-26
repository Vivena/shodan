// --------------------------
// > HEADER PRINCIPAL à inclure dans tous les fichiers .c

#include <stdio.h>
#include <stdlib.h>

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
