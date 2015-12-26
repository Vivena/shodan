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
    e.val=0;
    int i=0;
    uint32_t t,t1;
    char bi[sizeof(uint32_t)];
    char buff2[BLOCK_SIZE];
    
    memset(buff2,'\0',BLOCK_SIZE);
    
    // read the size of the HDD in block sorted in the first block of the file
    read(id->id,bi,sizeof(uint32_t));
    memcpy(&t,bi,sizeof(uint32_t));
    printf("bi: %i\n",t);
    if (t>num) {// check if num is not too big
        lseek(id->id,BLOCK_SIZE*num,SEEK_SET);
        t=0;
        t1=0;
        // keep reading till we've reach the end of the block even if we find a '\0' or '\n'
            i=read(id->id,buff2,BLOCK_SIZE-t);
            //printf("read %i octets\n",i);
            if (i<0) {
                e.val=-1;
                return e;
            }
            memcpy(&t1,buff2,sizeof(uint32_t));
            //printf("buffer: %i\n",t1);
            memcpy(b->octets,buff2,sizeof(buff2));
            memcpy(&t1,b->octets,sizeof(uint32_t));
            //printf("block: %i\n",t1);
            memset(buff2,'\0',BLOCK_SIZE);
        
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
    printf("read block \n");
    if(id->cache->cmemory[(SET(num))].TAG== (TAG(num))&&(TAG(num)!=0)){// j'ai deja l'info cherché en cache, je la copie dans le block
        printf("dans le cache \n");
        strcpy(b->octets ,id->cache->cmemory[(SET(num))].data->octets);
    }
    else{// je ne l'ai pas dans le cache
            printf("pas dans le cache \n");
            if (id->cache->cmemory[(SET(num))].valide==1) { //l'info du cache doit etre mis à jour sur le HDD
                printf("mise à jouer du HDD à l'adresse %i",num);
            // crée un processus pour gerer l'acces memoire (trop long à attendre) gestion de termination par double fork
	  /*
            if ((pid1 = fork())) {
                waitpid(pid1, &status, 0);
            } else if (!pid1) {
                if ((pid2 = fork())) {
                    exit(0);
                } else if (!pid2) {
	  */
                    write_physical_block(id, id->cache->cmemory[(SET(num))].data, num);// j'ecrit l'info du cache dans le HDD
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
        id->cache->cmemory[(SET(num))].valide=0;// la nouvelle info est la même que celle de l'HDD pour le moment, pas besoin de la réecrire
        id->cache->cmemory[(SET(num))].TAG=TAG(num);// mise à jour du tag
        memcpy(id->cache->cmemory[(SET(num))].data->octets, b->octets ,sizeof(block));
        
        printf("test du TAG %i %i %i %i \n",0xFF00FF00 ,TAG(0xFF00FF00),SET(0xFF00FF00),MEM(TAG(0xFF00FF00),SET(0xFF00FF00)));
        
        
    }
    return e;
}
