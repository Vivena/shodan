//
//  sync_disk.c
//  shodan
//
//  Created by Wintermute on 22/12/2015.
//
//

#include <stdio.h>
#include "main.h"
#include "ll.h"

error sync_disk(disk_id *id){
    error e;
    e.val=0;
    int i,num;
    uint32_t temp;
    //printf("sync_disk\n");
    for (i=0; i<CACHE_MEMORY; i++) {
        if(id->cache->cmemory[i].valide==1){
            
            num=MEM(id->cache->cmemory[(SET(num))].TAG, i);
            
            memcpy(&temp,id->cache->cmemory[i].data->octets,sizeof(uint32_t));
            //printf("    cache emplacement %i: writing : %i\n",num,temp);
            
            write_physical_block(id, id->cache->cmemory[i].data, num);
            
        }
    }
    
    
    return e;
}