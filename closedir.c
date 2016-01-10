//
//  closedir.c
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


int tfs_closedir(DIR *dirp,disk_id* id){
    int rez;
    if (id==NULL) {
        rez=closedir(dirp);
        return rez;
    }
    
    else{
        free(dirp->__dd_buf);
        free(dirp);
    }
    return 0;
}