#include "main.h"
#include "ll.h"


int main(int argc, char* argv[]){
    if (argc >= 3){
        char* name = malloc(sizeof(char));
        strcpy(name,"disk.tfs");
        int nb = argc;
        
        // Si le nombre d'argument est pair, i.e le nom du disque est indiqué
        if (argc % 2 == 0) {
            strcpy(name,argv[argc-1]);
            nb--;
        }
        
        // Calcul du nombre de partitions
        int nb_partitions = (nb-1)/2;
        int* sizes = malloc(nb_partitions*sizeof(int));
        
        int i = 1;
        while (i < nb){
            // Verification du mode
            if (strcmp(argv[i],"-p") != 0){
                fprintf(stderr, "Error : wrong option mode.\n");
                return -1;
            }
            i++;
            // Stockage des tailles des partitions
            sizes[(i-2)/2] = atoi(argv[i]);
            i++;
        }
        
        // Récupération du disque (marche si le disque n'existe pas)
        disk_id* id = malloc(sizeof(disk_id));
        error e = start_disk(name,id);
        if (e.val == 0){
            block *block0 = malloc(sizeof(block));
            read_block(id,block0,0);
            
            // Ecriture du nombre de partitions dans le bloc 0
            uint32_t d = itoui(nb_partitions);
            printf("nbr partitions %i\n",d);
            memcpy((block0->octets)+sizeof(uint32_t),&d,sizeof(uint32_t));
            
            // Ecriture de chaque taille de chaque partition dans le bloc 0
	    int previous_partition_size = 0;
            for (i = 0; i < nb_partitions; i++){
                d = itoui(sizes[i]);
                memcpy((block0->octets)+((i+2)*sizeof(uint32_t)),&d,sizeof(uint32_t));

		// TTTFS description block
		block *partition_block = malloc(sizeof(block));
		read_block(id,partition_block,1+(i*previous_partition_size));
		uint32_t a;
		a = itoui(TTTFS_MAGIC_NUMBER); // id de la version
		memcpy(partition_block->octets,&a,sizeof(uint32_t));
		a = itoui(TTTFS_VOLUME_BLOCK_SIZE); // taille d'un block (1024 octets)
		memcpy((partition_block->octets) + sizeof(uint32_t),&a,sizeof(uint32_t));
		a = itoui(sizes[i]); // taille de la partition
		memcpy((partition_block->octets) + (2*sizeof(uint32_t)),&a,sizeof(uint32_t));
		a = itoui(1); // premier block libre : le 1
		memcpy((partition_block->octets) + (3*sizeof(uint32_t)),&a,sizeof(uint32_t));
		a = itoui(0); // le nombre de fichiers supportables ??
		memcpy((partition_block->octets) + (4*sizeof(uint32_t)),&a,sizeof(uint32_t));
		a = itoui(0); // le nombre de fichiers actuellement libres ??
		memcpy((partition_block->octets) + (5*sizeof(uint32_t)),&a,sizeof(uint32_t));
		a = itoui(0); // le numero du premier fichier libre du volume ??
		memcpy((partition_block->octets) + (6*sizeof(uint32_t)),&a,sizeof(uint32_t));
		write_block(id,partition_block,1+(i*previous_partition_size));

		previous_partition_size = sizes[i];
            }
            
            write_block(id,block0,0);
            sync_disk(id);
            printf("Partition created !\n");
        }
        else{
            fprintf(stderr, "Error while creating disk.\n");
            return -1;
        }
    }
    else{
        fprintf(stderr, "Error : wrong number of arguments.\n");
        return -1;
    }
    
    return 0;
}