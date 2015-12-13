//
//  read_block.c
//  shodan
//
//  Created by Wintermute on 27/11/2015.
//
//

#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "main.h"
#include "ll.h"

error read_physical_block(disk_id *id,block *b,uint32_t num){
    error e;
    int i=0;
    uint32_t t;
    char bi[sizeof(uint32_t)];
    char buff2[BLOCK_SIZE];
    
    memset(buff2,'\0',BLOCK_SIZE);
    
    // read the size of the HDD in block sorted in the first block of the file
    read(id->id,bi,sizeof(uint32_t));
    t=atoi(bi);
    
    if (t>num) {// check if num is not too big
        lseek(id->id,BLOCK_SIZE*num,SEEK_SET);
        t=0;
        
        // keep reading till we've reach the end of the block even if we find a '\0' or '\n'
        while (t!=BLOCK_SIZE) {
            i=read(id->id,buff2,BLOCK_SIZE-t);
            
            if (i<0) {
                e.val=-1;
                return e;
            }
            t+=i;
            i=0;
            strcat(b->octets,buff2);
            memset(buff2,'\0',BLOCK_SIZE);
        }
    }
    else{
        printf("attempting to read outside the hard drive\n");
        e.val=-1;
    }
    
    return e;
}


error read_block(disk_id *id,block *b,uint32_t num){
    error e;
    e.val=0;
    
    /*
    pid_t pid1;
    pid_t pid2;
    int status;
    */
    
    if(id->cache.cmemory[(SET(num))].TAG== (TAG(num)))// j'ai deja l'info cherché en cache, je la copie dans le block
        strcpy(b->octets ,id->cache.cmemory[(SET(num))].data.octets);
    
    else{// je ne l'ai pas dans le cache
        if (id->cache.cmemory[(SET(num))].valide==true) { //l'info du cache doit etre mis à jour sur le HDD
            
            // crée un processus pour gerer l'acces memoire (trop long à attendre) gestion de termination par double fork
	  /*
            if ((pid1 = fork())) {
                waitpid(pid1, &status, 0);
            } else if (!pid1) {
                if ((pid2 = fork())) {
                    exit(0);
                } else if (!pid2) {
	  */
                    write_physical_block(id, id->cache.cmemory[(SET(num))].data, num);// j'ecrit l'info du cache dans le HDD
		    /*
                } else {
                    // error
                }
            } else {
                // error
            }
	  */
        }
        if ((read_physical_block(id,b,num)).val!=0) {// je met la nouvelle info dans le cache
            printf("error!!!"); // TODO message d'erreur à modif
        }
        id->cache.cmemory[(SET(num))].valide=false;// la nouvelle info est la même que celle de l'HDD pour le moment, pas besoin de la réecrire
        id->cache.cmemory[(SET(num))].TAG=TAG(num);// mise à jour du tag
        strcpy(b->octets ,id->cache.cmemory[(SET(num))].data.octets);
        
        
    }
    return e;
}
