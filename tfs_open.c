//
//  tfs_open.c
//  shodan
//
//  Created by Wintermute on 10/01/2016.
//
//

#include "main.h"
#include "ll.h"
#include "ll_volume.h"

#include <fcntl.h>

file_descriptor fdtable[1024];
int fdtend;

/**
 * \brief Ouvre un fichier à partir d'un chemin donné.
 * \details Créé un fichier temporaire (le file descriptor) dans la partition et le disque indiqués.
 *          
 * \param path  Chemin de la forme FILE://\a disk/\a volume/\a rep1/.../\a repn ou 
 *              FILE://HOST/\a rep1/.../\a repn
 *              avec \a disk le nom du disque, \a volume le numero du volume, et
 *              \a rep1/.../\a repn le chemin à parcourir.       
 * \param acess Flags.
 * \param permission Permission à prendre en compte (non traité).
 * \return Le numéro de block où se trouve le file_descriptor.
 */
int tfs_open(char *path,int access, int permission){
    char** splitPath;
    char** directories;
    char* disk_name;
    int i,temp, current_dir,type, partition, pemplacement, exists, fd, append, rez=-1;
    file_descriptor fd_contains;
    error e;

    block* block_partition = malloc(sizeof(block));
    block* block0 = malloc(sizeof(block));
    disk_id* id = malloc(sizeof(disk_id));
    
    // Découpage du path
    splitPath = malloc(sizeof(char*));
    type = cut_pathname(splitPath,path);
    
    // Gestion des différents cas
    switch (type) {
        case -1: // si erreur
            fprintf(stderr,"Error : argument %s incorrect.\n",path);
            return -1;
            break;
            
        case 0: // Si HOST
            rez=open(splitPath[0],access,permission);
            break;
            
        case 1:// Si un disk : au boulot !
            
            // On récupère les informations nécessaires
            disk_name = splitPath[0];
            partition = atoi(splitPath[1]);
            directories = splitPath+2;
            
            // On démarre le disk
            e = start_disk(disk_name,id);
            if (e.val != 0){
                fprintf(stderr, "Error while reading disk.\n");
                return -1;
            }
            
            // On réccupère le block de la partition
            read_block(id,block0,0);
            pemplacement = 0;
            for (i=0; i<partition; i++) {
                memcpy(&temp,(block0->octets)+((2+i)*sizeof(uint32_t)),sizeof(uint32_t));
                pemplacement+=uitoi(temp);
            }
            pemplacement++;
            read_block(id,block_partition,pemplacement);
            
            // Pour chaque répertoire
	    current_dir = 0; // numéro de l'entrée dans le File Table
	    while (directories[0]){
	      // ----------- Test de l'existence de ce répertoire
	      exists = is_in_directory(id,&current_dir,pemplacement,directories[0]);

	      // ----------- Action en fonction de l'existence

	      // Si on est au dernier repertoire
	      if (!directories[1][0]){
		printf("Preparing to create %s...\n", directories[0]);
		if (!exists){ // Si n'existe pas, on créé un nouveau fichier

		  break;
		}
		else{ // Sinon, erreur
		  printf("Opening %s...\n", directories[0]);

		  // Pour l'instant append = 0
		  append = 0;

		  fd = fdtend++;
		  fd_contains.ninode = current_dir;
		  fd_contains.pointeur = append;
		  fd_contains.ninode = access;
		  fd_contains.ninode = permission;
		  fd_contains.host = id;

		  fdtable[fd] = fd_contains;

		  return fd;
		}
	      }
	      // Si on n'est pas encore au dernier repertoire
	      else{
		printf("Searching for %s... ", directories[0]);
	
		if (!exists){
		  printf("\nError : directory %s doesn't exists.\n", directories[0]);
		  return -1;
		}
		else{
		  printf("ok\n");
		}
	      }
	      directories++;
	    }
            break;
        default:
            rez=-1;
            break;
    }
    return rez;
}
