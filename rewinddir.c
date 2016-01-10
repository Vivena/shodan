//
//  rewindir.c
//  shodan
//
//  Created by Wintermute on 10/01/2016.
//
//

#include <stdio.h>

#include <sys/types.h>
#include <dirent.h>

#include "main.h"
#include "ll.h"
#include "ll_volume.h"


void tfs_rewinddir(DIR *dirp,disk_id* id){
    if (id==NULL) {
        rewinddir(dirp);
        return;
    }
    else{
        dirp->__dd_loc=0;
        return;
    }
}