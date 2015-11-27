//
//  start_disk.c
//  shodan
//
//  Created by Wintermute on 27/11/2015.
//
//

#include <fcntl.h>
#include "main.h"


error start_disk(char *name,disk_id *id){
    error e;
    e.val=0;
    id->id=open(name,O_RDWR);
    if (id->id==-1) {
        e.val=-1;
    }
    
    return e;
}

