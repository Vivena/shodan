//
//  start_disk.c
//  shodan
//
//  Created by Wintermute on 27/11/2015.
//
//

#include <fcntl.h>
#include "main.h"
#include "ll.h"

/**
 * \brief Lance le disque.
 * \details Réccupère un disk_id pour pouvoir effectuer de nombreuses manipulation sur le disque.
 *          
 * \param name Nom du disque.   
 * \param id L'identifiant du disque sera stocké ici.
 * \return Un error, à valeur -1 si le disque n'existe pas, 0 sinon.
 */
error start_disk(char *name,disk_id *id){
    error e;
    e.val=0;
    int i;
    
    id->id=open(name,O_RDWR);
    if (id->id==-1) {
        e.val=-1;
        //printf("pblm here\n");
    }
    id->cache=malloc(sizeof(cache));
    id->cache->cmemory = malloc( CACHE_MEMORY*sizeof(cache_block));
    for (i=0; i<CACHE_MEMORY; i++) {
        id->cache->cmemory[i].data=malloc(sizeof(block));
        id->cache->cmemory[i].valide=0;
    }
    
    return e;
}

