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
    for (i=0; i<CACHE_MEMORY; i++) {
        if(id->cache.cmemory[i].valide==true){
            num=MEM(id->cache.cmemory[(SET(num))].TAG, i);
            write_physical_block(id, id->cache.cmemory[i].data, num);
        }
    }
    
    
    return e;
}