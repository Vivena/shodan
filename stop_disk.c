#include "main.h"
#include "ll.h"

/**
 * \brief Arrête le disque.
 * \details Libère la mémoire necessaire disponible dans le disk_id.
 *           
 * \param id L'identifiant du disque.
 * \return Un error à valeur 0.
 */
error stop_disk(disk_id* id){
  error e;
  e.val = 0;
    int i=0;
    for (i=0; i<CACHE_MEMORY; i++) {
        free(id->cache->cmemory[i].data);
    }
  
  free(id->cache->cmemory);
  free(id->cache);
  free(id);

  return e;
}
