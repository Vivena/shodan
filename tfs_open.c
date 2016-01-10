//
//  tfs_open.c
//  shodan
//
//  Created by Wintermute on 10/01/2016.
//
//

#include "main.h"
#include "ll.h"
#include "ll_volume.h"

#include <fcntl.h>

int tfs_open(char *path,int  access,  int  permission ){
    char** splitPath;
    char** directories;
    char* disk_name, current_dir;
    //char dir_name[TFS_DIRECTORIES_SIZE];
    int i,temp, type, partition, pemplacement, first_free_block, first_free_file, exists, index, index_entry, file_size,fd,rez=-1;
    
    error e;
    
    block* block_partition = malloc(sizeof(block));
    block* block0 = malloc(sizeof(block));
    block* block_entry = malloc(sizeof(block));
    block* block_navigation = malloc(sizeof(block));
    disk_id* id = malloc(sizeof(disk_id));
    //TTTFS_File_Table_Entry* entry_dir = malloc(sizeof(TTTFS_File_Table_Entry));
    
    // Découpage du path
    splitPath = malloc(sizeof(char*));
    type = cut_pathname(splitPath,path);
    
    // Gestion des différents cas
    switch (type) {
        case -1: // si erreur
            fprintf(stderr,"Error : argument %s incorrect.\n",path);
            return -1;
            break;
            
        case 0: // Si HOST
            rez=open(splitPath[0],access,permission);
            break;
            
        case 1:// Si un disk : au boulot !
            
            // On récupère les informations nécessaires
            disk_name = splitPath[0];
            partition = atoi(splitPath[1]);
            directories = splitPath+2;
            
            // On démarre le disk
            e = start_disk(disk_name,id);
            if (e.val != 0){
                fprintf(stderr, "Error while reading disk.\n");
                return -1;
            }
            
            // On réccupère le block de la partition
            read_block(id,block0,0);
            pemplacement = 0;
            for (i=0; i<partition; i++) {
                memcpy(&temp,(block0->octets)+((2+i)*sizeof(uint32_t)),sizeof(uint32_t));
                pemplacement+=uitoi(temp);
            }
            pemplacement++;
            read_block(id,block_partition,pemplacement);
            
            
            
            
            break;
        default:
            rez=-1;
            break;
    }
    return rez;
}