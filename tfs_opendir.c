//
//  opendir.c
//  shodan
//
//  Created by Wintermute on 10/01/2016.
//
//

#include <sys/types.h>
#include <dirent.h>

#include "main.h"
#include "ll.h"
#include "ll_volume.h"

#define FILE_SIZE 0
#define DIRENT_SIZE 32
#define N_DIRENT(m) m/DIRENT_SIZE

DIR * tfs_opendir(char *path,disk_id* id, uint32_t num_partition){
    DIR*  dir;
    uint32_t temp;
    int i,j,fd,dir_inode,dir_size,bl,type,offset,num_block_entry;
    struct dirent **dt;
    char **splitPath;
    char buff[64];
    
    block *fd_block;
    block *file_entry_block;
    block *dir_block;
    
    dir = (DIR *)malloc(sizeof(DIR));
    
    // Découpage du path
    splitPath = malloc(sizeof(char*));
    type = cut_pathname(splitPath,path);
    
    switch (type) {
        case -1: // si erreur
            fprintf(stderr,"Error : argument %s incorrect.\n",path);
            return dir;
            break;
            
        case 0: // Si HOST
            dir=NULL;
            if((dir = opendir(splitPath[0])) == NULL) {
                fprintf(stderr,"cannot open directory.");
                return NULL;
            }
            break;
            
        case 1: // Si un disk : au boulot !
            
            fd=tfs_open(path,0,0);
            if (fd==-1) {
                tfs_close(fd);
                return NULL;
            }
            
            
            
            //recuperation du n° d'inode du fichier ouvert
            fd_block=malloc(sizeof(block));
            read_block(id,fd_block,fd);
            memcpy(&temp,fd_block,sizeof(uint32_t));
            dir_inode=uitoi(temp);
            
            //recuperation de l'inode
            file_entry_block=malloc(sizeof(block));
            offset = (int)(FILE_TABLE_BLOCK_SIZE*sizeof(uint32_t));
            num_block_entry = num_partition+1+(dir_inode / offset);
            read_block(id,file_entry_block,num_block_entry);
            
            //recuperation de la taille du dossier
            memcpy(&temp,(file_entry_block->octets) + (FILE_SIZE*sizeof(uint32_t)),sizeof(uint32_t));
            dir_size=uitoi(temp);
            
            dir->__dd_size=N_DIRENT(dir_size);
            dir->__dd_len=dir_size;
            dir->__dd_loc=0;
            dir->__dd_fd=fd;
            dir->__dd_buf=malloc(dir->__dd_len*sizeof(char));
            dt=malloc(dir->__dd_len*sizeof( struct dirent*));
            j=0;
            //ouverture du 1er block
            dir_block=malloc(sizeof(block));
            read_block(id,file_entry_block,3);
            
            //recuperation des dirent
            for (i=0; i<dir->__dd_size; i++) {
                if (i%32==0 && i!=0) {
                    if ((bl=have_next_block(id,num_partition,dir_inode,j))<0) {
                        break;
                    }
                    read_block(id,file_entry_block,bl);
                }
                
                memcpy(buff,file_entry_block+((i%32)*DIRENT_SIZE),DIRENT_SIZE*sizeof(char));
                
                memcpy(dir->__dd_buf+i*32,buff,32*(sizeof(char)));
                
            }
            
            
            break;
            
        default:
            return NULL;
            break;
    }

    
    return dir;
}