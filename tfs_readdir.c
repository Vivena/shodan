//
//  tfs_readdir.c
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


#define DIRENT_SIZE 32

struct dirent *tfs_readdir(DIR *dirp,disk_id* id){
    uint32_t temp;
    char buf[DIRENT_SIZE];
    struct dirent *rez;
    
    rez=malloc(sizeof(struct dirent *));
    if (id==NULL) {
        return readdir(dirp);
    }
    else{
        memcpy(buf,dirp+(dirp->__dd_loc),32*sizeof(char));
        dirp->__dd_loc+=32;
        memcpy(&temp,buf,sizeof(uint32_t));
        rez->d_ino=uitoi(temp);
        memcpy(rez->d_name,buf+sizeof(uint32_t),32-sizeof(uint32_t));
        
    }
    return rez;
}