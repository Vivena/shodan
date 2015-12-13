#include "main.h"
#include "ll.h"

int main(int argc, char* argv[]){
  if (argc == 3 || argc == 4){

    // Initialisation 
    int size = atoi(argv[2]);
    char name[1024] = "disk.tfs";
    if (argc == 4){
      strncpy(name,argv[3],1024);
    }

    // Création du disque
    disk_id* id = malloc(sizeof(disk_id));
    error e = start_disk(name,id);
    if (e.val == 0){
      printf("Disk %s created !\n", name);
      block block0;
      // ECRIRE LA TAILLE SUR LES 4 PREMIERS OCTETS
      write_block(id,block0,0);
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
