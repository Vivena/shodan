#include <math.h>

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
        
        // Récupération du disque
        disk_id* id = malloc(sizeof(disk_id));
        error e = start_disk(name,id);
        if (e.val == 0){
            block *block0 = malloc(sizeof(block));
            read_block(id,block0,0);
            
            // Ecriture du nombre de partitions dans le bloc 0
            uint32_t d = itoui(nb_partitions);
            memcpy((block0->octets)+sizeof(uint32_t),&d,sizeof(uint32_t));
            
            // Ecriture de chaque taille de chaque partition dans le bloc 0
            int previous_partition_size = 0;
            for (i = 0; i < nb_partitions; i++){
                d = itoui(sizes[i]);
                memcpy((block0->octets)+((i+2)*sizeof(uint32_t)),&d,sizeof(uint32_t));
                
                // TTTFS description block
                block *partition_block = malloc(sizeof(block));
                read_block(id,partition_block,1+previous_partition_size);
                uint32_t a;
                a = itoui(TTTFS_MAGIC_NUMBER); // id de la version
                memcpy(partition_block->octets,&a,sizeof(uint32_t));
                a = itoui(TTTFS_VOLUME_BLOCK_SIZE); // taille d'un block (1024 octets)
                memcpy((partition_block->octets) + sizeof(uint32_t),&a,sizeof(uint32_t));
                a = itoui(sizes[i]); // taille de la partition
                memcpy((partition_block->octets) + (2*sizeof(uint32_t)),&a,sizeof(uint32_t));
                int first = 2 + (int)(0.1*sizes[i]/100); // Premier block libre = 0.1% de taille de partition
                int nb_fic = sizes[i]-first-1;
                a = itoui(nb_fic);//nombre de blocks libres
                memcpy((partition_block->octets) + (3*sizeof(uint32_t)),&a,sizeof(uint32_t));
                a = itoui(first); // premier block libre
                memcpy((partition_block->octets) + (4*sizeof(uint32_t)),&a,sizeof(uint32_t));
                // le nombre de fichiers supportables
		float formule = ((0.1*sizes[i]/100)+1)*FILE_TABLE_BLOCK_SIZE;
                if (nb_fic < formule) {// verification du nombre de fichier max
                    a = itoui(nb_fic);
                }
                else{
                    a = itoui((int)formule);
                }
                memcpy((partition_block->octets) + (5*sizeof(uint32_t)),&a,sizeof(uint32_t));
                // le nombre de fichiers actuellement libres
                if (nb_fic<(((0.1*sizes[i]/100)+1)*16)) {// verification du nombre de fichier max
                    a = itoui(nb_fic);
                }
                else{
                    a = itoui((int)formule);
                }
                memcpy((partition_block->octets) + (6*sizeof(uint32_t)),&a,sizeof(uint32_t));
                a = itoui(0); // le numero du premier fichier libre du volume
                memcpy((partition_block->octets) + (7*sizeof(uint32_t)),&a,sizeof(uint32_t));

                // Next free file
                int j, k = 0, next_entry, offset_table = 0;
		block *file_table_block = malloc(sizeof(block));
		read_block(id,file_table_block,2+previous_partition_size);
                for (j = first; j < sizes[i]; j++){
                    if (j == sizes[i]-1){
                        a = itoui(j);
			next_entry = itoui(k+(offset_table*FILE_TABLE_BLOCK_SIZE*sizeof(uint32_t)));
                    }
                    else{
                        a = itoui(j+1);
			next_entry = itoui(k+1+(offset_table*FILE_TABLE_BLOCK_SIZE*sizeof(uint32_t)));
                    }
		    
		    // Mise à jour de chaque entrée de file table
		    if (k == ((FILE_TABLE_BLOCK_SIZE*sizeof(uint32_t))-1)){ // Changement de block de file tab
		      k = 0;
		      offset_table++;
		      read_block(id,file_table_block,2+previous_partition_size+offset_table);
		    }
		    memcpy((file_table_block->octets) + (k*FILE_TABLE_BLOCK_SIZE),&next_entry,sizeof(uint32_t));
		    write_block(id,file_table_block,2+previous_partition_size+offset_table);

		    // Mise à jour de chaque bloc dans la partition
                    block *partition_sub_block = malloc(sizeof(block));
                    read_block(id,partition_sub_block,j);
                    memcpy((partition_sub_block->octets) + (TTTFS_VOLUME_BLOCK_SIZE-sizeof(uint32_t)),&a,sizeof(uint32_t));
		    write_block(id,partition_sub_block,j);

		    k++;
                }
                
                write_block(id,partition_block,1+previous_partition_size);
                previous_partition_size += sizes[i];
            }
            
            write_block(id,block0,0);
            sync_disk(id);
            printf("Partition created !\n");
        }
        else{
            fprintf(stderr, "Error while reading disk.\n");
            return -1;
        }
    }
    else{
        fprintf(stderr, "Error : wrong number of arguments.\n");
        return -1;
    }
    
    return 0;
}
