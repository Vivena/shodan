// --------------------------
// > HEADER PRINCIPAL à inclure dans tous les fichiers .c

#ifndef main_h
#define main_h


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

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

#define is_bigendian() ( (*(char*)&ib) == 0 )
#define BLOCK_SIZE 1024 //blocks de 1KB = 2^10 bits = 128 = 2^7 octets
#define CACHE_MEMORY 8192 // cache de 8MB =2 ^13 blocks peut etre ajusté pour consommé 2% de la mem prise par le HDD
#define TTTFS_MAGIC_NUMBER 0x31534654
#define TTTFS_VOLUME_BLOCK_SIZE 1024




// --------------------------
// Définition des structures

typedef struct {
    char octets[BLOCK_SIZE]; // tableau contenant chaque octet
    int free; // nombre d'octets disponibles dans le block
} block;

/*
 fonctionnement du TAG
 Soit un uint32_t N qui represente le num d'un byte de ma memoire
 
 -------- -------- -------- --------
 un block fait 1024 bits c'est à dire 128=2^7 bytes donc les 7 1ers bits sont le offset O qui nous donne le num du block B contenant l'octet N
 
 N: 00000000 00000000 00000000 00000000
 0: -------- -------- -------- -0000000
 B: 00000000 00000000 00000000 0-------
 
 
 
 Notre cache fait 8192 = 2^13 blocks on retire les 13 prochains bits pour obtenir le TAG T affecté à notre adresse
 
 N: 00000000 00000000 00000000 00000000
 O: -------- -------- -------- -0000000
 B: 00000000 00000000 00000000 0-------
 T= 00000000 0------- -------- --------
 
 le TAG correspond à l'id du block contenant l'octet recherché une fois mis à la bonne position B dans le cache
dans notre cas, on n'a pas acces au O, on se contente de la structure 
 N: 00000000 00000000 00000000 00000000
 S: -------- -------- ---00000 00000000
 T: 00000000 00000000 000----- --------
 
*/


typedef struct{// correspond à une entrée du cache
    uint32_t TAG;// Voir au dessus
    block *data;
    int valide;// savoir si l'information est encore valide
}cache_block;

typedef struct{
    cache_block  *cmemory;
}cache;


typedef struct {
    int val;
} error;

typedef struct {
    int id;
    cache *cache;
} disk_id;

typedef struct {
  block block;
} TTTFS_description_block;

typedef struct {
  int size;
  int type;
  int sub_type;
  int tfs_direct[10];
  int tfs_indirect1;
  int tfs_indirect2;
  int tfs_next_free;
} TTTFS_File_Table_Entry;

#define SET(m) ((m) & 0x1FFF)
#define TAG(m) (((m)>>13))
#define MEM(m,n) ( m <<13 | n)

/* FUNCTIONS */

//util.c
uint32_t itoui(int i);
int uitoi(uint32_t i);


    


#endif
