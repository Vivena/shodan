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

error write_physical_block(disk_id *id,block *b,uint32_t num){
    error e;
    e.val=0;
    
    uint32_t temp;
    
    lseek(id->id,BLOCK_SIZE*num,SEEK_SET);
    printf("writing in position %i in HDD\n",num);
    if((write(id->id,b->octets,sizeof(block)))<0){
        printf("gros pblm\n");
        e.val=-1;
    }
    memcpy(&temp,b->octets,sizeof(uint32_t));
    printf("finish printing %i\n",temp);
    return e;
}

error write_block(disk_id *id,block *b,uint32_t num){
    error e;
    e.val=0;
    
    //pid_t pid1;
    //pid_t pid2;
    //int status;
    
    uint32_t d;
    memcpy(&d,b->octets,sizeof(uint32_t));
    //printf("test %i \n",d);
    
    if(id->cache->cmemory[(SET(num))].TAG== (TAG(num))){// je regarde si l'info est deja dans le cache
        //printf("dans cache\n");
        memcpy(id->cache->cmemory[(SET(num))].data->octets,b->octets,BLOCK_SIZE* sizeof(char));// mis à jour de l'info
        id->cache->cmemory[(SET(num))].valide=1;// l'info dans le cache est nouvelle et doit etre mis dans le HDD plus tard
    }
    else{
        //printf("pas dans cache\n");

        if (id->cache->cmemory[(SET(num))].valide==1) {// l'info du cache doit etre passée au HDD
            
            // crée un processus pour gerer l'acces memoire (trop long à attendre) gestion de termination par double fork
            /*if ((pid1 = fork())) {
                waitpid(pid1, &status, 0);
            } else if (!pid1) {
                if ((pid2 = fork())) {
                    exit(0);
                } else if (!pid2) {*/
                    write_physical_block(id, id->cache->cmemory[(SET(num))].data, num);// mis à jour du HDD
               /* } else {
                    // error
                }
            } else {
                // error
            }*/
        }
        
        strcpy(id->cache->cmemory[(SET(num))].data->octets,b->octets);// mis à jour de l'info
        id->cache->cmemory[(SET(num))].valide=1;// l'info dans le cache est nouvelle et doit etre mis dans le HDD plus tard
        id->cache->cmemory[(SET(num))].TAG=TAG(num);// mise à jour du tag
        
    }
    return e;
}
