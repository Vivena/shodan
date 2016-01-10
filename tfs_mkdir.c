//
//  tfs_mkdir.c
//  shodan
//
//  Created by Wintermute on 09/01/2016.
//
//

#include "main.h"
#include "ll.h"
#include "ll_volume.h"

/**
 * \brief Crée un nouveau répertoire au chemin indiqué.
 * \details Si le \a path contient le disque (voir détails dans les paramètres),
 *          alors on va tenter de parcourir le chemin dans la partition indiquée,
 *          puis créer le dernier nom de répertoire à l'endroit indiqué. Si l'un
 *          des répertoires à parcourir n'existe pas, une erreur est renvoyée, de
 *          même si le nom du répertoire à créer existe déjà dans le répertoire 
 *          courant.
 * \param path  Chemin de la forme FILE://\a disk/\a volume/\a rep1/.../\a repn ou 
 *              FILE://HOST/\a rep1/.../\a repn
 *              avec \a disk le nom du disque, \a volume le numero du volume, et
 *              \a rep1/.../\a repn le chemin à parcourir.
 * \return Un \e int, 0 si la fonction s'est terminée sans erreurs, -1 sinon.
 */
int tfs_mkdir(const char *path){
  char** splitPath;
  char** directories;
  char* disk_name;
  int i, temp, type, current_dir, partition, pemplacement, first_free_block, first_free_file, exists, index, index_entry;
  error e;

  block* block_partition = malloc(sizeof(block));
  block* block0 = malloc(sizeof(block));
  block* block_entry = malloc(sizeof(block));
  block* block_navigation = malloc(sizeof(block));
  disk_id* id = malloc(sizeof(disk_id));
  TTTFS_File_Table_Entry* entry_dir = malloc(sizeof(TTTFS_File_Table_Entry));

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
    execlp("/bin/mkdir","mkdir",&splitPath[1],NULL);
    break;

  case 1: // Si un disk : au boulot !
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

      // Si on est au dernier repertoire (celui à créer)
      if (!directories[1][0]){
	printf("Preparing to create %s...\n", directories[0]);
	if (!exists){ // Si n'existe pas, on créé un nouveau répertoire
	  printf("\tDirectory doesn't already exists [OK]\n");

	  // On récuppère le numéro de bloc / entrée qui sera occupé par le répertoire
	  memcpy(&temp,(block_partition->octets)+(4*sizeof(uint32_t)),sizeof(uint32_t));
	  first_free_block = uitoi(temp);
	  memcpy(&temp,(block_partition->octets)+(7*sizeof(uint32_t)),sizeof(uint32_t));
	  first_free_file = uitoi(temp);

	  // Création de l'entrée
	  //index = partition+1+(current_dir/FILE_TABLE_OFFSET);
	  //read_block(id,block_entry,index);
	  entry_dir->size = TFS_DIRECTORIES_SIZE*2;
	  entry_dir->type = 0;
	  entry_dir->sub_type = 1;
	  entry_dir->tfs_direct[0] = first_free_block;
	  for (i = 1; i < 10; i++){
	    entry_dir->tfs_direct[i] = 0;
	  }
	  entry_dir->tfs_indirect1 = 0;
	  entry_dir->tfs_indirect2 = 0;
	  printf("\tEntry initialized [OK]\n");

	  // Remplissage
	  if (fill_block(id,pemplacement).val != 0){
	    fprintf(stderr, "Error while filling block.\n");
	    return -1;
	  }
	  printf("\tA block is now ok to be filled [OK]\n");
	  if (fill_entry(id,pemplacement,entry_dir).val != 0){
	    fprintf(stderr, "Error while writing on File Table.\n");
	    return -1;
	  }
	  printf("\tAn entry is now filled [OK]\n");
	  char* buf_d = malloc(TFS_DIRECTORIES_SIZE);
	  read_block(id,block_navigation,first_free_block);
	  temp = itoui(first_free_file);
	  memcpy(buf_d,&temp,sizeof(uint32_t));
	  strncpy(&buf_d[sizeof(uint32_t)],".\0",TFS_DIRECTORIES_SIZE-sizeof(uint32_t));
	  memcpy((block_navigation->octets),buf_d,TFS_DIRECTORIES_SIZE);
	  strncpy(&buf_d[sizeof(uint32_t)],"..\0",TFS_DIRECTORIES_SIZE-sizeof(uint32_t));
	  memcpy((block_navigation->octets)+TFS_DIRECTORIES_SIZE,buf_d,TFS_DIRECTORIES_SIZE);
	  write_block(id,block_navigation,first_free_block);
	  
	  // Rajouter le nom du repertoire dans le dernier repertoire courant
	  index = pemplacement+1+(current_dir/FILE_TABLE_OFFSET);
	  read_block(id,block_entry,index);
	  index_entry = (current_dir % FILE_TABLE_OFFSET)*FILE_TABLE_BLOCK_SIZE;
	  memcpy(&temp,(block_entry->octets) + index_entry + (3*sizeof(uint32_t)),sizeof(uint32_t));
	  index = uitoi(temp);
	  printf("%d\n",index);
	  read_block(id,block_navigation,index);
	  strncpy(&buf_d[sizeof(uint32_t)],directories[0],TFS_DIRECTORIES_SIZE-sizeof(uint32_t));
	  memcpy((block_navigation->octets)+(TFS_DIRECTORIES_SIZE*2),buf_d,TFS_DIRECTORIES_SIZE);
	  write_block(id,block_navigation,first_free_block);

	  printf("\tBlock is filled [OK]\n");


	  sync_disk(id);

	  printf("\nSUCCESSFULLY CREATED !\n");
	  break;
	}
	else{ // Sinon, erreur
	  printf("Error : this directory already exists.\n");
	  return -1;
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
    return -1;
    break;
    }

  return 0;
}

/**
 * \brief Permet d'éxcecuter la commande tfs_mkdir \a path
 * \param path Voir le path de la fonction tfs_mkdir
 * \return Un \e int, 0 si la fonction s'est terminée sans erreurs, -1 sinon.
 */
int main(int argc, char* argv[]){
  if (argc != 2){
    fprintf(stderr,"Error : Number of argument incorrect.\n");
    return -1;
  }
  
  char* test = "FILE://test.new/0/machin";
  tfs_mkdir(test);

  return 0;
}
