//
//  tfs_format.c
//  shodan
//
//  Created by Wintermute on 27/12/2015.
//
//


#include <math.h>

#include "main.h"
#include "ll.h"
#include "ll_volume.h"

int main(int argc, char* argv[]){
    uint32_t temp;
    int a,b,i,nb,npart,mf,first,pemplacement=0;
    char* name;
    disk_id* id;
    error e;
    block *block0;
    // block* block_navigation = malloc(sizeof(block));
    //block *block_file_table;
    TTTFS_File_Table_Entry* entry_root = malloc(sizeof(TTTFS_File_Table_Entry));
    
    if (argc < 5){
        fprintf(stderr, "Error : wrong number of arguments.\n");
        return -1;
    }
    
    name = malloc(sizeof(char));
    strcpy(name,"disk.tfs");
    nb = argc;
    
    // Si le nombre d'argument est pair, i.e le nom du disque est indiqué
    if (argc % 2 == 0) {
        strcpy(name,argv[argc-1]);
        nb--;
    }
    //recuperation du numero de la partition
    if (strcmp(argv[1],"-p") != 0){
        fprintf(stderr, "Error : wrong option mode \"-p\" .\n");
        return -1;
    }
    npart=atoi(argv[2]);
    //recuperation du nombre de file demandé
    if (strcmp(argv[3],"-mf") != 0){
        fprintf(stderr, "Error : wrong option mode \"-mf\".\n");
        return -1;
    }
    mf=atoi(argv[4]);
    
    //démarage du disque
    id = malloc(sizeof(disk_id));
    e = start_disk(name,id);
    if (e.val != 0){
        fprintf(stderr, "Error while reading disk.\n");
        return -1;
    }
    
    //recuperation du block 0
    block0 = malloc(sizeof(block));
    read_block(id,block0,0);
    
    //verification de l'existance de la partition
    memcpy(&temp,(block0->octets)+(sizeof(uint32_t)),sizeof(uint32_t));
    if ((a=uitoi(temp))< npart) {
        fprintf(stderr, "Non-existing partition number .\n");
        return -1;
    }
    
    //recuperation du decalage de la partition
    for (i=0; i<npart; i++) {
        memcpy(&temp,(block0->octets)+((2+i)*sizeof(uint32_t)),sizeof(uint32_t));
        pemplacement+=uitoi(temp);
    }
    pemplacement++;
    //printf("decalage pour part: %i \n",pemplacement);
    
    //mise à jour de l'entête de la partition
    memcpy(&temp,(block0->octets) + ((npart+2)*sizeof(uint32_t)),sizeof(uint32_t));
    a=uitoi(temp);
    //printf("a:%i\n",a);
    
    first = 2+(mf/FILE_TABLE_BLOCK_SIZE);
    if (a-first<0) {// file count trop grand
        fprintf(stderr, "Error file_count too big .\n");
        return -1;
    }
    
    else if(a-first<mf){//file count trop grand mais tiens dans la memoire
        fprintf(stderr, "file_count of %i too big for this partition .\n",mf);
        mf=a-first;
        printf("changing file_count to %i\n",mf);
    }
    
    //printf("cas n : \n\t mf/ftb:%i \n\t mf:%i \n\t first:%i \n\t a:%i\n",mf/FILE_TABLE_BLOCK_SIZE+1,mf,first,a);
    read_block(id,block0,pemplacement);
    //first = 2+(mf/FILE_TABLE_BLOCK_SIZE);
    a-=first;
    //printf("a : %i\n",a);
    
    temp = itoui(a);//nombre de blocks libres
    memcpy((block0->octets) + (3*sizeof(uint32_t)),&temp,sizeof(uint32_t));
    temp = itoui(first); // premier block libre
    memcpy((block0->octets) + (4*sizeof(uint32_t)),&temp,sizeof(uint32_t));
    // le nombre de fichiers supportables
    if (a<mf) {// verification du nombre de fichier max
        temp = itoui(a);
        
    }
    else{
        temp = itoui(mf);
        //printf("temp: %i\n",temp);
    }
    memcpy((block0->octets) + (5*sizeof(uint32_t)),&temp,sizeof(uint32_t));
    // le nombre de fichiers actuellement libres
    if (a<mf) {// verification du nombre de fichier max
        temp = itoui(a);
    }
    else{
        temp = itoui(mf);
    }
    memcpy((block0->octets) + (6*sizeof(uint32_t)),&temp,sizeof(uint32_t));
    temp = itoui(0); // le numero du premier fichier libre du volume
    memcpy((block0->octets) + (7*sizeof(uint32_t)),&temp,sizeof(uint32_t));
    // Next free file
    
    int j;
    block *partition_sub_block = malloc(sizeof(block));
    for (j = first; j < a; j++){
        if (j == a-1){
            a = itoui(j);
        }
        else{
            a = itoui(j+1);
        }
        read_block(id,partition_sub_block,j);
        memcpy((partition_sub_block->octets) + (TTTFS_VOLUME_BLOCK_SIZE-sizeof(uint32_t)),&a,sizeof(uint32_t));
        write_block(id,partition_sub_block,j);
    }
    write_block(id,block0,pemplacement);
    
    for (i = 0; i < (mf/FILE_TABLE_BLOCK_SIZE); i++){
        if (i == (mf/FILE_TABLE_BLOCK_SIZE)-1){
            a = itoui(i);
        }
        else{
            a = itoui(i+1);
        }
        // Pour chaque bloc...
        read_block(id,partition_sub_block,pemplacement+1+i);
        for (j = 0; j < FILE_TABLE_BLOCK_SIZE; j++){
            memcpy((partition_sub_block->octets) + ((j+1)*sizeof(uint32_t))-sizeof(uint32_t),&a,sizeof(uint32_t));
        }
        write_block(id,partition_sub_block,pemplacement+1+i);
    }
    
    
    // ---------------- Construction du répertoire racine
    
    // On récuppère le numéro de bloc qui sera occupé par la racine
    read_block(id,block0,pemplacement);
    memcpy(&temp,(block0->octets)+(4*sizeof(uint32_t)),sizeof(uint32_t));
    first = uitoi(temp);
    
    // Création de l'entrée
    //block_file_table = malloc(sizeof(block));
    //read_block(id,block_file_table,pemplacement+1);
    entry_root->size = TFS_DIRECTORIES_SIZE*2;
    entry_root->type = 0;
    entry_root->sub_type = 1;
    entry_root->tfs_direct[0] = first;
    for (i = 1; i < 10; i++){
        entry_root->tfs_direct[i] = 0;
    }
    entry_root->tfs_indirect1 = 0;
    entry_root->tfs_indirect2 = 0;
    
    // Remplissage
    if (fill_block(id,pemplacement).val != 0){
        fprintf(stderr, "Error while filling block.\n");
        return -1;
    }
    if (fill_entry(id,pemplacement,entry_root).val != 0){
        fprintf(stderr, "Error while writing on File Table.\n");
        return -1;
    }
    //write_block(id,block_file_table,pemplacement+1);
    char* buf_d = malloc(sizeof(uint32_t)+28);
    read_block(id,block0,first);
    temp = itoui(0);
    memcpy(buf_d,&temp,sizeof(uint32_t));
    strncpy(&buf_d[sizeof(uint32_t)],".\0",28);
    memcpy((block0->octets),buf_d,sizeof(uint32_t)+28);
    strncpy(&buf_d[sizeof(uint32_t)],"..\0",28);
    memcpy((block0->octets)+(sizeof(uint32_t)+28),buf_d,sizeof(uint32_t)+28);
    write_block(id,block0,first);
    
    sync_disk(id);
    printf("Partition %i formated !\n",npart);
    return 0;
}
