//
//  write_block.c
//  shodan
//
//  Created by Wintermute on 27/11/2015.
//
//

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include "main.h"
#include "ll.h"

error write_physical_block(disk_id *id,block b,uint32_t num){
    error e;
    e.val=0;
    
    lseek(id->id,BLOCK_SIZE*num,SEEK_SET);
    
    if((write(id->id,&b,sizeof(block)))<0){
        e.val=-1;
    }
    
    return e;
}

error write_block(disk_id *id,block b,uint32_t num){
    error e;
    e.val=0;
    
    pid_t pid1;
    pid_t pid2;
    int status;
    if(id->cache.cmemory[(SET(num))].TAG== (TAG(num))){// je regarde si l'info est deja dans le cache
        strcpy(id->cache.cmemory[(SET(num))].data.octets,b.octets);// mis à jour de l'info
        id->cache.cmemory[(SET(num))].valide=false;// l'info dans le cache est nouvelle et doit etre mis dans le HDD plus tard
    }
    else{
        if (id->cache.cmemory[(SET(num))].valide==true) {// l'info du cache doit etre passée au HDD
            
            // crée un processus pour gerer l'acces memoire (trop long à attendre) gestion de termination par double fork
            if ((pid1 = fork())) {
                waitpid(pid1, &status, 0);
            } else if (!pid1) {
                if ((pid2 = fork())) {
                    exit(0);
                } else if (!pid2) {
                    write_physical_block(id, id->cache.cmemory[(SET(num))].data, num);// mis à jour du HDD
                } else {
                    // error
                }
            } else {
                // error
            }
        }
        
        strcpy(id->cache.cmemory[(SET(num))].data.octets,b.octets);// mis à jour de l'info
        id->cache.cmemory[(SET(num))].valide=false;// l'info dans le cache est nouvelle et doit etre mis dans le HDD plus tard
        id->cache.cmemory[(SET(num))].TAG=TAG(num);// mise à jour du tag
        
    }
    return e;
}
