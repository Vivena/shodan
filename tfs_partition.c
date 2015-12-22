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
            block *block0 = malloc(sizeof(block0));
            read_block(id,block0,0);
            
            // Ecriture du nombre de partitions dans le bloc 0
            uint32_t d = itoui(nb_partitions);
            printf("nbr partitions %i\n",d);
            memcpy((block0->octets)+sizeof(uint32_t),&d,sizeof(uint32_t));
            
            // Ecriture de chaque taille de chaque partition dans le bloc 0
            for (i = 0; i < nb_partitions; i++){
                d = itoui(sizes[i]);
                memcpy((block0->octets)+((i+1)*sizeof(uint32_t)),&d,sizeof(uint32_t));
            }
            
            printf("%d\n", block0->octets[8]);
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
