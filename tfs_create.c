#include <fcntl.h>
#include <unistd.h>
#include "main.h"
#include "ll.h"

int main(int argc, char* argv[]){
    int temp;
    if (argc == 3 || argc == 4){
        // Verification du mode
        if (strcmp(argv[1],"-s") != 0){
            fprintf(stderr, "Error : wrong option mode.\n");
            return -1;
        }
        
        // Initialisation
        int size = atoi(argv[2]);
        char* name = malloc(sizeof(char));
        if (argc == 4){
            strcpy(name,argv[3]);
        }
        else{
            strcpy(name,"disk.tfs");
        }
        
        // Création du disque
        disk_id* id = malloc(sizeof(disk_id));
        temp=open(name,O_CREAT,0777);
        close(temp);
        error e = start_disk(name,id);
        if (e.val == 0){
            block *block0;
            block0=malloc(sizeof(block));
            uint32_t d = itoui(size);
            printf("mise à jour de size à %i\n",d);
            memcpy(block0->octets,&d,sizeof(uint32_t));
            
            //memcpy(&d,block0->octets,sizeof(uint32_t));
            printf("apres mise à jour %i \n",d);
            write_block(id,block0,0);
            sync_disk(id);
            printf("Disk %s created ! - Size : %d \n", name, size);
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
