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
    char* name = malloc(sizeof(char));

    //verification de la taille de argv
    if (argc > 2){
        fprintf(stderr, "Error : wrong number of arguments.\n");
        return -1;
    }
    
    //initialisation des variables
    e.val=0;
    b=malloc(sizeof(block));
    id = malloc(sizeof(disk_id));
    
    //lancement du disque
    if (argc == 2){
      strcpy(name,argv[1]);
    }
    else{
      strcpy(name,"disk.tfs");
    }
    if((e=start_disk(name,id)).val!=0){
      return e.val;
    }
    
    lseek(id->id,0,SEEK_SET);
    
    
    //recuperation du 1er block
    e=read_block(id,b,0);
    if(e.val!=0){
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
	int previous_partition_size = 0;
        for (i=0; i<npart; i++) {
            memcpy(&temp,b->octets+((i+2)*sizeof(uint32_t)),sizeof(uint32_t));
            tpart[i]=uitoi(temp);
            printf("\tpartition %i : %i blocks\n",i,tpart[i]);

	    // A mettre en commentaire--------------------------------------------------
	    block *partition_block = malloc(sizeof(block));
	    read_block(id,partition_block,1+previous_partition_size);
	    memcpy(&temp,partition_block->octets,sizeof(uint32_t));
	    int a;
	    a=uitoi(temp);
	    printf("\t\tVersion id : %d\n",a);
	    memcpy(&temp,(partition_block->octets) + sizeof(uint32_t),sizeof(uint32_t));
	    a=uitoi(temp);
	    printf("\t\tSize of a block (octets) : %d\n",a);
	    memcpy(&temp,(partition_block->octets) + (2*sizeof(uint32_t)),sizeof(uint32_t));
	    a=uitoi(temp);
	    printf("\t\tSize of partition : %d\n",a);
	    memcpy(&temp,(partition_block->octets) + (3*sizeof(uint32_t)),sizeof(uint32_t));
	    a=uitoi(temp);
	    printf("\t\tFirst free block : %d\n",a);
	    memcpy(&temp,(partition_block->octets) + (4*sizeof(uint32_t)),sizeof(uint32_t));
	    a=uitoi(temp);
	    printf("\t\tFile max count : %d\n",a);
	    memcpy(&temp,(partition_block->octets) + (5*sizeof(uint32_t)),sizeof(uint32_t));
	    a=uitoi(temp);
	    printf("\t\tFree file count : %d\n",a);
	    memcpy(&temp,(partition_block->octets) + (6*sizeof(uint32_t)),sizeof(uint32_t));
	    a=uitoi(temp);
	    printf("\t\tFirst free file : %d\n",a);


	    previous_partition_size += tpart[i];
	    // ------------------------------------------------------------------------
        }
    }
    
    
    
    return e.val;
}
