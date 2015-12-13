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

error start_disk(char *name,disk_id *id){
    error e;
    e.val=0;
    
    id->id=open(name,O_RDWR);
    if (id->id==-1) {
        e.val=-1;
    }
    
    id->cache.cmemory = malloc( CACHE_MEMORY*sizeof(cache_block));
    
    return e;
}

