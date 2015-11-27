//
//  read_block.c
//  shodan
//
//  Created by Wintermute on 27/11/2015.
//
//

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "main.h"


error read_physical_block(disk_id id,block *b,uint32_t num){
    error e;
    int i=0;
    uint32_t t;
    char bi[sizeof(uint32_t)];
    char buff2[BLOCK_SIZE];
    
    memset(buff2,'\0',BLOCK_SIZE);
    
    // read the size of the HDD in block sorted in the first block of the file
    read(id.id,bi,sizeof(uint32_t));
    t=atoi(bi);
    
    if (t>num) {// check if num is not too big
        lseek(id.id,BLOCK_SIZE*num,SEEK_SET);
        t=0;
        
        // keep reading till we've reach the end of the block even if we find a '\0' or '\n'
        while (t!=BLOCK_SIZE) {
            i=read(id.id,buff2,BLOCK_SIZE-t);
            
            if (i<0) {
                e.val=-1;
                return e;
            }
            t+=i;
            i=0;
            strcat(b->octets,buff2);
            memset(buff2,'\0',BLOCK_SIZE);
        }
    }
    else{
        printf("attempting to read outside the hard drive\n");
        e.val=-1;
    }
    
    return e;
}

