//
//  tfs_mkdir.c
//  shodan
//
//  Created by Wintermute on 09/01/2016.
//
//

#include "main.h"
#include "ll.h"

int tfs_mkdir(const char *path){
  char** splitPath;
  char** directories;
  char* disk_name, current_dir;
  int i, temp, type, partition, pemplacement, first_free, exists;
  disk_id* id;
  error e;
  block* block_partition;
  block* block0;
  int offset = FILE_TABLE_BLOCK_SIZE*sizeof(uint32_t);

  // Découpage du path
  splitPath = malloc(sizeof(char*));
  type = cut_pathname(splitPath,path);
  
  switch (type) {
  case -1: // si erreur
    fprintf(stderr,"Error : argument %s incorrect.\n",path);
    return -1;
    break;

  case 0: // Si HOST
    execlp("/bin/mkdir","mkdir",&splitPath[1],NULL);
    break;

  case 1: // Si un disk
    // On récupère les informations nécessaires
    disk_name = splitPath[0];
    partition = atoi(splitPath[1]);
    directories = splitPath+2;

    // On démarre le disk
    id = malloc(sizeof(disk_id));
    e = start_disk(disk_name,id);
    if (e.val != 0){
      fprintf(stderr, "Error while reading disk.\n");
      return -1;
    }
    
    // On réccupère le block de la partition
    block0 = malloc(sizeof(block));
    read_block(id,block0,0);
    pemplacement = 0;
    for (i=0; i<partition; i++) {
      memcpy(&temp,(block0->octets)+((2+i)*sizeof(uint32_t)),sizeof(uint32_t));
      pemplacement+=uitoi(temp);
    }
    pemplacement++;
    block_partition = malloc(sizeof(block));
    read_block(id,block_partition,pemplacement);

    // Pour chaque répertoire
    current_dir = 0;
    while (directories[0]){
      // Test de l'existence de ce répertoire
      exists = 0;
      

      // Si on est au dernier repertoire (celui à créer)
      if (directories+1){
	if (!exists){ // Si n'existe pas, on créé un nouveau répertoire
	  // On récuppère le numéro de bloc qui sera occupé par le répertoire
	  read_block(id,block0,pemplacement);
	  memcpy(&temp,(block0->octets)+(4*sizeof(uint32_t)),sizeof(uint32_t));
	  first_free = uitoi(temp);
	  //printf("%d\n",first_free);
	}
	else{ // Sinon, erreur
	  fprintf(stderr, "Error : this directory already exists.\n");
	  return -1;
	}
      }
      // Si on n'est pas encore au dernier repertoire
      else{
	if (!exists){
	  fprintf(stderr, "Error : directory %s doesn't exists.\n", directories[0]);
	  return -1;
	}	
      }
      directories++;
    }
    
    break;   
          
  default:
    return -1;
    break;
    }

  return 0;
}

int main(int argc, char* argv[]){
  if (argc != 2){
    fprintf(stderr,"Error : Number of argument incorrect.\n");
    return -1;
  }
  
  char* test = "FILE://test.new/0/machin/truc/bidule";
  tfs_mkdir(test);

  return 0;
}
