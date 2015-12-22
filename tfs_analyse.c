//
//  tfs_analyse.c
//  shodan
//
//  Created by Wintermute on 22/12/2015.
//
//

#include <sys/types.h>
#include <unistd.h>
#include "main.h"
#include "ll.h"

int main(int argc, char* argv[]){
    disk_id* id;
    error e;
    block *b;
    uint32_t temp;
    int i,size,npart;
    int *tpart;
    
    //verification de la taille de argv
    if (argc !=2){
        fprintf(stderr, "Error : wrong number of arguments.\n");
        return -1;
    }
    
    //initialisation des variables
    e.val=0;
    b=malloc(sizeof(block));
    id = malloc(sizeof(disk_id));
    
    //lancement du disque
    if((e=start_disk(argv[1],id)).val!=0){
        return e.val;
    }
    
    lseek(id->id,0,SEEK_SET);
    
    //recuperation du 1er block
    if((e=read_block(id,b,0)).val!=0){
        return e.val;
    }
    
    //recuperation de la taille du disque dur et du nombre de partition
    memcpy(&temp,b->octets,sizeof(uint32_t));
    size=uitoi(temp);
    printf("Size of the HDD: %i\n",size);
    
    memcpy(&temp,b->octets+(sizeof(uint32_t)),sizeof(uint32_t));
    npart=uitoi(temp);
    printf("%i partitions: \n", npart);
    
    if (npart!=0) {
        //creation du tableau des tailles de partition
        tpart=malloc(sizeof(int)*npart);
        for (i=0; i<npart; i++) {
            memcpy(&temp,b->octets+((i+2)*sizeof(uint32_t)),sizeof(uint32_t));
            tpart[i]=uitoi(temp);
            printf("    partition %i : %i blocks\n",i,tpart[i]);
        }
    }
    
    
    
    return e.val;
}