// --------------------------
// > HEADER PRINCIPAL à inclure dans tous les fichiers .c

/*! \mainpage INDEX
 *
 * \section intro_sec Introduction
 * On peut voir la liste de chaque fichiers avec ses fonctions <a href="files.html">ICI</a>.
 * Plus de détails sur l'organisation, plus bas.
 *
 * \section section1 Makefile
 *
 * Positionnez vous dans le répertoire principal.
 *
 * \subsection sub1 Compiler le programme (génération des différentes commandes)
 *
 * Pour produire TOUS les éxécutables, utilisez la commande \code make \endcode
 * Si vous souhaitez les créer à part, utilisez la commande \code make nom_de_la_commande \endcode
 * \subsection sub2 Nettoyer l'arborescence
 *
 * Utilisez la commande \code make clean \endcode
 * Attention, ça n'enlèvera pas les disques créés. Il faut les enlever vous même.
 *
 * \section section2 Liste des commandes 
 * 
 * Pour les commandes suivantes, nous utilisons l'API de bas niveau (dans ll.h).
 *
 * \code tfs_create -s size [name]\endcode
 *  
 * Voir la documentation de tfs_create.c.
 *
 * \code tfs_partition -p size [-p size]... [name]\endcode
 *
 * Voir la documentation de tfs_partition.c.
 *
 * \code tfs_analyse [name]\endcode
 *
 * Voir la documentation de tfs_analyse.c (quelques affichages supplémentaires par rapport au sujet).
 *
 * --------------------------------------------------------------------------------------------
 *
 * Les commandes suivantes utilisent des fonctions diverses d'opérations de bas-niveau sur les volumes, elles sont stockées dans la bibliothèque ll_volume.h.
 *
 * \code tfs_format -p partition -mf file_count [disk] \endcode
 *
 * Voir la documentation de tfs_format.c.
 *
 *
 *
 *
 *
 *
 *
 */

#ifndef main_h
#define main_h


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>

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
#define FILE_TABLE_BLOCK_SIZE 16
#define STRING_MAX_LENGTH 1024
#define FILE_TABLE_OFFSET (FILE_TABLE_BLOCK_SIZE*sizeof(uint32_t))
#define TFS_DIRECTORIES_SIZE (sizeof(uint32_t) + 28) // un numéro + 28 caractères
#define TFS_DIRECTORIES_MAX_ENTRIES (TTTFS_VOLUME_BLOCK_SIZE / TFS_DIRECTORIES_SIZE)


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


typedef struct{
    int ninode;
    int pointeur;
    int flags;
    
} file_descriptor;

extern file_descriptor fdtable[1024];
extern int fdtend;

#define SET(m) ((m) & 0x1FFF)
#define TAG(m) (((m)>>13))
#define MEM(m,n) ( m <<13 | n)

/* FUNCTIONS */

//util.c
uint32_t itoui(int i);
int uitoi(uint32_t i);
int occ_block_size(disk_id* id, uint32_t num_partition);
void str_split(char** res, char* str, char c_delim);
    


#endif
