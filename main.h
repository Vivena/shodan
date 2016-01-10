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
 * \section section2 Liste des commandes réalisées 
 * 
 * Pour les commandes suivantes, nous utilisons l'API de bas niveau (dans ll.h). Le code source de ces fonctions est disponible dans les fichiers portant leur nom (un fichier par fonction, sauf write et read mélangés avec le physical).
 *
 * \code ./tfs_create -s size [name]\endcode
 *  
 * Le code source se trouve dans tfs_create.c.
 *
 * \code ./tfs_partition -p size [-p size]... [name]\endcode
 *
 * Le code source se trouve dans tfs_partition.c.
 *
 * \code ./tfs_analyse [name]\endcode
 *
 *  Le code source se trouve dans tfs_analyse.c (quelques affichages supplémentaires par rapport au sujet).
 *
 * --------------------------------------------------------------------------------------------
 *
 * Les commandes suivantes utilisent des fonctions diverses d'opérations de bas-niveau sur les volumes, les en-têtes  sont stockées dans la bibliothèque ll_volume.h, et le code source dans tfs_manipulation.c.
 *
 * \code ./tfs_format -p partition -mf file_count [disk] \endcode
 *
 * Le code source se trouve dans tfs_format.c.
 *
 * \code ./tfs_mkdir path \endcode
 *
 * Le code source se trouve dans tfs_mkdir.c.
 *
 *
 * \section section3 Liste des structures
 *
 * Voir la liste <a href="annotated.html">ICI</a>.
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

/**
 * \brief Un block de 1024 octets.
 * \details 
 *
 * \param octets Le tableau de caractères contentant les 1024 octets
 */
typedef struct {
    char octets[BLOCK_SIZE]; // tableau contenant chaque octet
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

/**
 * \brief Un block du cacheUne entrée du cache.
 * \details 
 *
 * \param TAG 
 * \param data Les données du cache.
 * \param valide Pour savoir si l'information est encore valide.
 */
typedef struct{
    uint32_t TAG;
    block *data;
    int valide;
}cache_block;

/**
 * \brief Le cache.
 * \details 
 *
 * \param cmemory Liste d'entrées du cache.
 */
typedef struct{
    cache_block  *cmemory;
}cache;

/**
 * \brief Erreur détaillée
 * \details 
 *
 * \param val Valeur de l'erreur.
 */
typedef struct {
    int val;
} error;

/**
 * \brief Identifiant du disque + cache
 * \details 
 *
 * \param id L'identifiant en question.
 * \param cache Pointeur vers le cache.
 */
typedef struct {
    int id;
    cache *cache;
} disk_id;

/**
 * \brief Description d'une entrée d'un File Table
 * \details 
 *
 * \param size Taille du fichier.
 * \param type Type de fichier (répertoire/fichier).
 * \param sub_type Sous-type du fichier (=0).
 * \param tfs_direct Les 10 premiers blocks direct du fichier.
 * \param tfs_indirect1 Le block indirect 1.
 * \param tfs_indirect2 Le block indirect 2.
 */
typedef struct {
  int size;
  int type;
  int sub_type;
  int tfs_direct[10];
  int tfs_indirect1;
  int tfs_indirect2;
} TTTFS_File_Table_Entry;

/**
 * \brief Information supplémentaires sur un fichier ouvert.
 * \details 
 *
 * \param ninode Inode du fichier ouvert (numéro dans File Table).
 * \param pointeur Pointeur.
 * \param flags Les flags.
 * \param host Host.
 */
typedef struct{
    int ninode;
    int pointeur;
    int flags;
    disk_id* host;
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
