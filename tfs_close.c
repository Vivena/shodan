#include "main.h"
#include "ll.h"
#include "ll_volume.h"

/**
 * \brief Ferme un fichier préalablement ouvert avec tfs_open.
 * \details Détruit le fichier temporaire (le file descriptor) dans la partition et le disque indiqués.
 *          
 *          
 * \param disk_name Nom du disque contenant le fichier à fermer.
 * \param partition Le numéro de la partition dans lequel se trouve le fichier à fermer.
 * \param fd Le numéro de block où se trouve le file descriptor.
 * \return 0 si tout se passe bien, -1 sinon.
 */
int tfs_close(char* disk_name, int partition, int fd){
  int i, pemplacement, temp, res = 0;
    
  block* block0 = malloc(sizeof(block));
  disk_id* id = malloc(sizeof(disk_id));


  // On démarre le disk
  if (start_disk(disk_name,id).val != 0){
    fprintf(stderr, "Error while reading disk.\n");
    res = -1;
  }
  else{
    // On réccupère le block de la partition
    read_block(id,block0,0);
    pemplacement = 0;
    for (i=0; i<partition; i++) {
      memcpy(&temp,(block0->octets)+((2+i)*sizeof(uint32_t)),sizeof(uint32_t));
      pemplacement+=uitoi(temp);
    }
    pemplacement++;

    // On libère le block
    if (free_block(id,pemplacement,fd).val != 0){
      fprintf(stderr, "Error can't make the block being free.\n");
      res = -1;
    }  
  }

  free(id);
  free(block0);

  return res;
}
