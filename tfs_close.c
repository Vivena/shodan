#include "main.h"
#include "ll.h"
#include "ll_volume.h"

file_descriptor fdtable[1024];
int fdtend;

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
int tfs_close(int fd){
  int i;

  // S'il faut décaler les valeurs
  for (i = fd; i < fdtend-1; i++){
    fdtable[i] = fdtable[i+1];
  }

  // Décrémenter le nombre de File Dscriptors
  fdtend--;

  return 0;
}
