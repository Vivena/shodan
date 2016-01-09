//
//  tfs_mkdir.c
//  shodan
//
//  Created by Wintermute on 09/01/2016.
//
//

#include "main.h"
#include "ll.h"


int main(int argc, char* argv[]){
    char **splitPath=NULL;
    int type;
    if (argc != 2){
        fprintf(stderr,"Error : Number of argument incorrect.\n");
        return -1;
    }
    else{
        type = cut_pathname(splitPath, argv[1]);
        switch (type) {
            case -1:
                fprintf(stderr,"Error : argument incorrect.\n");
                return -1;
                break;
            case 0:
                execlp("/bin/mkdir","mkdir",splitPath,NULL);
                break;
            case 1:
                
                break;
                
            default:
                return -1;
                break;
        }
        
    }

    return 0;
}