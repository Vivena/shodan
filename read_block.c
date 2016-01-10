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


/**
 * \brief Lit physiquement un block dans le disque.
 * \details Il s'agit de la lecture des données dans le fichier du disque avec la fonction C read.
 *          
 * \param id L'identifiant du disque.
 * \param block Le block (contenant les 1024 octets) à lire dans le disque.
 * \param num Le numéro du block à lire dans le disque.
 * \return Un error, à valeur -1 s'il y a un problème avec la fonction C read.
 */
error read_physical_block(disk_id *id,block *b,uint32_t num){
    error e;
    e.val=0;
    int i=0;
    uint32_t t,t1;
    char bi[BLOCK_SIZE];
    char buff2[BLOCK_SIZE];
    
    memset(buff2,'\0',BLOCK_SIZE);
    
    // read the size of the HDD in block sorted in the first block of the file
    lseek(id->id,BLOCK_SIZE*0,SEEK_SET);
    read(id->id,bi,BLOCK_SIZE);
    memcpy(&t,bi,sizeof(uint32_t));
    //printf("bi: %i\n",t);
    
    
    if (t>num) {// check if num is not too big
        lseek(id->id,BLOCK_SIZE*num,SEEK_SET);
        t=0;
        t1=0;
        // keep reading till we've reach the end of the block even if we find a '\0' or '\n'
            i=read(id->id,buff2,BLOCK_SIZE);
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

/**
 * \brief Lit un block dans le disque.
 * \details Il s'agit de la lecture des données dans le fichier du disque en utilisant la fonction read_physical_block. ici, on prend en compte les données mises en place dans le cache avant de lire.
 *          
 * \param id L'identifiant du disque.
 * \param block Le block (contenant les 1024 octets) à lire dans le disque.
 * \param num Le numéro du block à lire dans le disque.
 * \return Un error, à valeur -1 s'il y a un problème avec la fonction C read.
 */
error read_block(disk_id *id,block *b,uint32_t num){
    error e;
    e.val=0;
    
    /*
    pid_t pid1;
    pid_t pid2;
    int status;
    */
    //printf("read block \n");
    if(id->cache->cmemory[(SET(num))].TAG== (TAG(num))&&(TAG(num)!=0)){// j'ai deja l'info cherché en cache, je la copie dans le block
        printf("dans le cache \n");
        strcpy(b->octets ,id->cache->cmemory[(SET(num))].data->octets);
    }
    else{// je ne l'ai pas dans le cache
            //printf("pas dans le cache \n");
            if (id->cache->cmemory[(SET(num))].valide==1) { //l'info du cache doit etre mis à jour sur le HDD
               // printf("mise à jouer du HDD à l'adresse %i",num);
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
            printf("error!!!\n"); // TODO message d'erreur à modif
        }
        id->cache->cmemory[(SET(num))].valide=0;// la nouvelle info est la même que celle de l'HDD pour le moment, pas besoin de la réecrire
        id->cache->cmemory[(SET(num))].TAG=TAG(num);// mise à jour du tag
        memcpy(id->cache->cmemory[(SET(num))].data->octets, b->octets ,sizeof(block));
        
        
        
    }
    return e;
}
