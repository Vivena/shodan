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

int tfs_open(char *path,int access, int permission ){
    char** splitPath;
    char** directories;
    char* disk_name;
    int a, i,temp, current_dir,type, partition, pemplacement, exists, index, index_entry, fd, inode, append, rez=-1;
    
    error e;
    
    block* block_partition = malloc(sizeof(block));
    block* block0 = malloc(sizeof(block));
    block* block_entry = malloc(sizeof(block));
    block* block_navigation = malloc(sizeof(block));
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

		  // On récuppère le numéro de bloc qui sera occupé par le fd
		  memcpy(&temp,(block_partition->octets)+(4*sizeof(uint32_t)),sizeof(uint32_t));
		  fd = uitoi(temp);
		  //memcpy(&temp,(block_partition->octets)+(7*sizeof(uint32_t)),sizeof(uint32_t));
		  //fd = uitoi(temp);

		  // Lecture du File Table (recherche du répertoire courrant)
		  index = pemplacement+1+(current_dir/FILE_TABLE_OFFSET);
		  read_block(id,block_entry,index);
		  index_entry = (current_dir % FILE_TABLE_OFFSET)*FILE_TABLE_BLOCK_SIZE;

		  // Récupération de l'inode du fichier
		  memcpy(&temp,(block_entry->octets) + index_entry + ((3+i)*sizeof(uint32_t)),sizeof(uint32_t));
		  inode = uitoi(temp);

		  // Ecriture du fd dans le File Table
		  a = itoui(fd);
		  memcpy((block_entry->octets) + index_entry + ((3+i)*sizeof(uint32_t)),&a,sizeof(uint32_t));
		  write_block(id,block_entry,index);

		  // Remplissage du fd
		  if (fill_block(id,pemplacement).val != 0){
		    fprintf(stderr, "Error while filling block.\n");
		    return -1;
		  }
		  read_block(id,block_navigation,fd);
		  temp = itoui(inode);
		  memcpy((block_navigation->octets),&temp,sizeof(uint32_t));
		  append = 0;
		  temp = itoui(append);
		  memcpy((block_navigation->octets) + sizeof(uint32_t),&temp,sizeof(uint32_t));
		  temp = itoui(access);
		  memcpy((block_navigation->octets) + (2*sizeof(uint32_t)),&temp,sizeof(uint32_t));
		  temp = itoui(permission);
		  memcpy((block_navigation->octets) + (3*sizeof(uint32_t)),&temp,sizeof(uint32_t));
		  write_block(id,block_navigation,fd);
		  sync_disk(id);

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
