//
//  write_block.c
//  shodan
//
//  Created by Wintermute on 27/11/2015.
//
//

#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "main.h"


error write_physical_block(disk_id id,block b,uint32_t num){
    error e;
    e.val=0;
    
    lseek(id.id,BLOCK_SIZE*num,SEEK_SET);
    
    if((write(id.id,&b,sizeof(block)))<0){
        e.val=-1;
    }
    
    return e;
}

error write_block(disk_id id,block b,uint32_t num){
    error e;
    e.val=0;
    
    pit_t pid1;
    pit_t pid2;
    int status;
    
    // crée un processus pour gerer l'acces memoire (trop long à attendre) gestion de termination par double fork
    if (pid1 = fork()) {
        waitpid(pid1, &status, NULL);
    } else if (!pit1) {
        if (pid2 = fork()) {
            exit(0);
        } else if (!pid2) {
            write_physical_block(id, b, num);
        } else {
            /* error */
        }
    } else {
        /* error */
    }
    
    return e;
}