//
//  tfs_manipulation.c
//  shodan
//
//  Created by Wintermute on 27/12/2015.
//
//

#include "main.h"
#include "ll.h"

#define MAGIC_NUMBER 0
#define FREE_BLOCK_COUNT 4
#define FIRST_FREE_BLOCK 5
#define VOLUME_MAX_FILE_COUNT 6

#define FILE_SIZE 0
#define DIRECT 3
#define INDIRECT1 13
#define INDIRECT2 14



// Met un block dans la liste des block libres
error free_block(disk_id* id, uint32_t num_partition, uint32_t num_free){
    error e;
    e.val=0;
    block *partition_block = malloc(sizeof(block));
    block *free_block = malloc(sizeof(block));
    uint32_t temp;
    int a;
    int first_free_block;
    
    // Récupération des blocks
    read_block(id,partition_block,num_partition);
    read_block(id,free_block,num_free);
    
    // Récupration du premier block libre
    memcpy(&temp,(partition_block->octets) + (4*sizeof(uint32_t)),sizeof(uint32_t));
    first_free_block = uitoi(temp);
    
    // Modification du suivant du block libre + premier block libre de partition
    a = itoui(first_free_block);
    memcpy((free_block->octets) + (TTTFS_VOLUME_BLOCK_SIZE-sizeof(uint32_t)),&a,sizeof(uint32_t));
    a = itoui(num_free);
    memcpy((partition_block->octets) + (4*sizeof(uint32_t)),&a,sizeof(uint32_t));
    
    // Réécriture des blocks
    write_block(id,partition_block,num_partition);
    write_block(id,free_block,num_free);
    sync_disk(id);
    
    return e;
}

// Supprimer un bloc de la liste des blocs libres = occuper un block de la partition
error fill_block(disk_id* id, uint32_t num_partition){
    error e;
    e.val=0;
    block *partition_block = malloc(sizeof(block));
    block *fill_block = malloc(sizeof(block));
    uint32_t temp;
    int a, first_free_block, next;
    
    // Récupration des blocks
    read_block(id,partition_block,num_partition);
    memcpy(&temp,(partition_block->octets) + (4*sizeof(uint32_t)),sizeof(uint32_t));
    first_free_block = uitoi(temp);
    if (first_free_block == 0){
        e.val=1;
        fprintf(stderr, "Error : no free blocks.\n");
        return e;
    }
    read_block(id,fill_block,first_free_block);
    
    // Changement du premier block libre
    memcpy(&temp,(fill_block->octets) + (TTTFS_VOLUME_BLOCK_SIZE-sizeof(uint32_t)),sizeof(uint32_t));
    next = uitoi(temp);
    if (next == first_free_block){ // Si plus de bloc libre
        next = 0;
    }
    a = itoui(next);
    memcpy((partition_block->octets) + (4*sizeof(uint32_t)),&a,sizeof(uint32_t));
    temp=itoui(0);
    memcpy((fill_block->octets) + (TTTFS_VOLUME_BLOCK_SIZE-sizeof(uint32_t)),&temp,sizeof(uint32_t));
    // Réécriture des blocks
    write_block(id,partition_block,num_partition);
    write_block(id,fill_block,first_free_block);
    sync_disk(id);
    
    return e;
}

// mettre une entrée de répertoire dans la liste des entrées libres = rendre une entrée libre
error free_entry(disk_id* id, uint32_t num_partition, uint32_t entry_index){
    error e;
    e.val=0;
    block *partition_block = malloc(sizeof(block));
    block *file_table_block = malloc(sizeof(block));
    int first_free_entry;
    int offset = (int)(FILE_TABLE_BLOCK_SIZE*sizeof(uint32_t));
    uint32_t temp;
    int a;
    int num_block_entry = num_partition+1+(entry_index / offset);
    
    // Récupération des blocks
    read_block(id,partition_block,num_partition);
    read_block(id,file_table_block,num_block_entry);
    
    // Récupration du premier block libre
    memcpy(&temp,(partition_block->octets) + (7*sizeof(uint32_t)),sizeof(uint32_t));
    first_free_entry = uitoi(temp);
    
    // Modification du suivant de la nouvelle entrée libre + premier file libre de partition
    a = itoui(first_free_entry);
    memcpy((file_table_block->octets) + (((entry_index+1) % offset)*FILE_TABLE_BLOCK_SIZE) - sizeof(uint32_t),&a,sizeof(uint32_t));
    a = itoui(entry_index);
    memcpy((partition_block->octets) + (7*sizeof(uint32_t)),&a,sizeof(uint32_t));
    
    // Réécriture des blocks
    write_block(id,partition_block,num_partition);
    write_block(id,file_table_block,num_block_entry);
    sync_disk(id);
    
    return e;
}

// supprimer une entrée des entrées libres = occuper une entrée
error fill_entry(disk_id* id, uint32_t num_partition, TTTFS_File_Table_Entry entry){
    error e;
    e.val=0;
    block *partition_block = malloc(sizeof(block));
    block *file_table_block = malloc(sizeof(block));
    int offset = (int)(FILE_TABLE_BLOCK_SIZE*sizeof(uint32_t));
    uint32_t temp;
    int first_free_entry, a, next, num_block_entry, entry_position, i, j;
    
    // Récupration des blocks
    read_block(id,partition_block,num_partition);
    memcpy(&temp,(partition_block->octets) + (7*sizeof(uint32_t)),sizeof(uint32_t));
    first_free_entry = uitoi(temp);
    
    num_block_entry = num_partition+1+(first_free_entry / offset);
    read_block(id,file_table_block,num_block_entry);
    
    // Changement de la première entrée libre
    entry_position = (first_free_entry % offset)*FILE_TABLE_BLOCK_SIZE;
    memcpy(&temp,(file_table_block->octets) + (((first_free_entry+1) % offset)*FILE_TABLE_BLOCK_SIZE) - sizeof(uint32_t), sizeof(uint32_t));
    next = uitoi(temp);
    if (next == first_free_entry){ // Si plus de bloc libre
        next = -1;
    }
    a = itoui(next);
    memcpy((partition_block->octets) + (7*sizeof(uint32_t)),&a,sizeof(uint32_t));
    
    // Remplissage
    i = 0;
    a = itoui(entry.size);
    memcpy((file_table_block->octets) + entry_position + (i*sizeof(uint32_t)),&a,sizeof(uint32_t));
    a = itoui(entry.type); i++;
    memcpy((file_table_block->octets) + entry_position + (i*sizeof(uint32_t)),&a,sizeof(uint32_t));
    a = itoui(entry.sub_type); i++;
    memcpy((file_table_block->octets) + entry_position + (i*sizeof(uint32_t)),&a,sizeof(uint32_t));
    for (j = 0; j < 10; j++){
        a = itoui(entry.tfs_direct[j]); i++;
        memcpy((file_table_block->octets) + entry_position + (i*sizeof(uint32_t)),&a,sizeof(uint32_t));
    }
    a = itoui(entry.tfs_indirect1); i++;
    memcpy((file_table_block->octets) + entry_position + (i*sizeof(uint32_t)),&a,sizeof(uint32_t));
    a = itoui(entry.tfs_indirect2); i++;
    memcpy((file_table_block->octets) + entry_position + (i*sizeof(uint32_t)),&a,sizeof(uint32_t));
    a = itoui(entry.tfs_next_free); i++;
    memcpy((file_table_block->octets) + entry_position + (i*sizeof(uint32_t)),&a,sizeof(uint32_t));
    
    // Réécriture des blocks
    write_block(id,partition_block,num_partition);
    write_block(id,file_table_block,num_block_entry);
    sync_disk(id);
    
    return e;
}



error add_block_to_file(disk_id* id, uint32_t num_partition,int entry_index){
    error e;
    e.val=0;
    uint32_t temp;
    int t,ffb,fsize,offset,num_block_entry,nbtoadd,idir1_block,idir2_block;
    
    block *partition_block = malloc(sizeof(block));
    block *file_entry_block = malloc(sizeof(block));
    block *idir1 = malloc(sizeof(block));
    block *idir2 = malloc(sizeof(block));
    
    //recuperation du block 0
    read_block(id,partition_block,num_partition);
    
    //verification du block 0
    memcpy(&temp,(partition_block->octets) + (MAGIC_NUMBER*sizeof(uint32_t)),sizeof(uint32_t));
    t=uitoi(temp);
    if (t != TTTFS_MAGIC_NUMBER) {
        fprintf(stderr, "Error : Partition is not using the same version as the programme.\n");
        e.val = -1;
        return e;
    }
    
    //verification du nombre de blocks libres et recuperation du num du 1er block libre 
    memcpy(&temp,(partition_block->octets) + (FREE_BLOCK_COUNT*sizeof(uint32_t)),sizeof(uint32_t));
    t=uitoi(temp);
    if (t==0) {
        fprintf(stderr, "Error : Partition full./n");
        e.val=-1;
        return e;
    }
    else{
        memcpy(&temp,(partition_block->octets) + (FREE_BLOCK_COUNT*sizeof(uint32_t)),sizeof(uint32_t));
        ffb=uitoi(temp);
    }
    
    //verification de file_entry
    memcpy(&temp,(partition_block->octets) + (VOLUME_MAX_FILE_COUNT*sizeof(uint32_t)),sizeof(uint32_t));
    t=uitoi(temp);
    if (t<entry_index) {
        fprintf(stderr, "Error : File_entry too big./n");
        e.val=-1;
        return e;
    }
    
    //recuperation du block contenant la file_entry
    offset = (int)(FILE_TABLE_BLOCK_SIZE*sizeof(uint32_t));
    num_block_entry = num_partition+1+(entry_index / offset);
    read_block(id,file_entry_block,num_block_entry);
    
    //recuperation de la taille du fichier
    memcpy(&temp,(file_entry_block->octets) + (FILE_SIZE*sizeof(uint32_t)),sizeof(uint32_t));
    fsize=uitoi(temp);
    
    //verification de l'emplacement pour le lien du block à ajouter
    if (fsize==0) {
        nbtoadd=0;
    }
    else{
        if (fsize%BLOCK_SIZE==0) {
            nbtoadd=(fsize/BLOCK_SIZE);
        }
        else{
            nbtoadd=(fsize/BLOCK_SIZE)+1;
        }
    }
    
    
    //dans direct
    if (nbtoadd<10) {
        memcpy(&temp,(file_entry_block->octets) + ((DIRECT+nbtoadd)*sizeof(uint32_t)),sizeof(uint32_t));
        t=uitoi(temp);
        
        if (t!=0) {
            fprintf(stderr,"Error : Already have an empty block at the end of the file.");
            e.val=-1;
            return e;
        }
        
        if((e=fill_block(id, num_partition)).val!=0){
            return e;
        }
        
        temp=itoui(ffb);
        memcpy(file_entry_block->octets+(sizeof(uint32_t)*(nbtoadd+DIRECT)),&temp,sizeof(uint32_t));
        write_block(id,file_entry_block,num_partition);
    }
    
    //dans indirect1
    else if(nbtoadd-10 >=(BLOCK_SIZE/sizeof(uint32_t))*BLOCK_SIZE){
        //recuperation du block indirecte1
        memcpy(&temp,(file_entry_block->octets) + (INDIRECT1*sizeof(uint32_t)),sizeof(uint32_t));
        idir1_block=uitoi(temp);
        
        //pas de block indirect1 alloué
        if (idir1_block==0) {
            idir1_block=ffb;
            temp=itoui(idir1_block);
            if((e=fill_block(id, num_partition)).val!=0){
                return e;
            }
            memcpy(file_entry_block->octets+(INDIRECT1*sizeof(uint32_t)),&temp,sizeof(uint32_t));
            write_block(id,file_entry_block,num_partition);
            
            //verification du nombre de block libres restant
            memcpy(&temp,(partition_block->octets) + (FREE_BLOCK_COUNT*sizeof(uint32_t)),sizeof(uint32_t));
            t=uitoi(temp);
            if (t==0) {
                fprintf(stderr, "Error : Partition full./n");
                e.val=-1;
                return e;
            }
            
            else{
                memcpy(&temp,(partition_block->octets) + (FREE_BLOCK_COUNT*sizeof(uint32_t)),sizeof(uint32_t));
                ffb=uitoi(temp);
            }
        }
        
        //block indirect1 alloué
        read_block(id,idir1,idir1_block);
        memcpy(&temp,idir1->octets+(sizeof(uint32_t)*(nbtoadd-10)),sizeof(uint32_t));
        t=uitoi(temp);
        if (t!=0) {
            fprintf(stderr,"Error : Already have an empty block at the end of the file.");
            e.val=-1;
            return e;
        }
        if((e=fill_block(id, num_partition)).val!=0){
            return e;
        }
        temp=itoui(ffb);
        memcpy(idir1->octets+(sizeof(uint32_t)*(nbtoadd-10)),&temp,sizeof(uint32_t));
    }

    //dans indirect2
    else{
        //recuperation du block indirect2
        memcpy(&temp,(file_entry_block->octets) + (INDIRECT2*sizeof(uint32_t)),sizeof(uint32_t));
        idir2_block=uitoi(temp);
        
        //pas de block indirect2 alloué
        if (idir2_block==0) {
            idir2_block=ffb;
            temp=itoui(idir2_block);
            if((e=fill_block(id, num_partition)).val!=0){
                return e;
            }
            memcpy(file_entry_block->octets+(INDIRECT2)*sizeof(uint32_t),&temp,sizeof(uint32_t));
            write_block(id,file_entry_block,num_partition);
            
            //verification du nombre de block libres restant
            memcpy(&temp,(partition_block->octets) + (FREE_BLOCK_COUNT*sizeof(uint32_t)),sizeof(uint32_t));
            t=uitoi(temp);
            if (t==0) {
                fprintf(stderr, "Error : Partition full./n");
                e.val=-1;
                return e;
            }
            
            else{
                memcpy(&temp,(partition_block->octets) + (FREE_BLOCK_COUNT*sizeof(uint32_t)),sizeof(uint32_t));
                ffb=uitoi(temp);
            }
        }
        
        //block indirect2 alloué
        read_block(id,idir2,idir2_block);
        
        //recuperation du block indirect1
        memcpy(&temp,(idir2->octets) + ((((nbtoadd-10-(TTTFS_VOLUME_BLOCK_SIZE/sizeof(uint32_t))*TTTFS_VOLUME_BLOCK_SIZE))/(TTTFS_VOLUME_BLOCK_SIZE/sizeof(uint32_t)))*sizeof(uint32_t)),sizeof(uint32_t));//a verifier
        idir1_block=uitoi(temp);
        
        //pas de block indirect1 alloué
        if (idir1_block==0) {
            idir1_block=ffb;
            temp=itoui(idir1_block);
            if((e=fill_block(id, num_partition)).val!=0){
                return e;
            }
            memcpy((idir2->octets) + (((nbtoadd-10-(TTTFS_VOLUME_BLOCK_SIZE/sizeof(uint32_t))*TTTFS_VOLUME_BLOCK_SIZE))/(TTTFS_VOLUME_BLOCK_SIZE/sizeof(uint32_t)))*sizeof(uint32_t),&temp,sizeof(uint32_t));
            write_block(id,idir2,num_partition);
            
            //verification du nombre de block libres restant
            memcpy(&temp,(partition_block->octets) + (FREE_BLOCK_COUNT*sizeof(uint32_t)),sizeof(uint32_t));
            t=uitoi(temp);
            if (t==0) {
                fprintf(stderr, "Error : Partition full./n");
                e.val=-1;
                return e;
            }
            
            else{
                memcpy(&temp,(partition_block->octets) + (FREE_BLOCK_COUNT*sizeof(uint32_t)),sizeof(uint32_t));
                ffb=uitoi(temp);
            }
        }
        
        //block indirect1 alloué
        read_block(id,idir1,idir1_block);
        memcpy(&temp,idir1->octets+(sizeof(uint32_t)*(((nbtoadd-10-(TTTFS_VOLUME_BLOCK_SIZE/sizeof(uint32_t))*TTTFS_VOLUME_BLOCK_SIZE))%(TTTFS_VOLUME_BLOCK_SIZE/sizeof(uint32_t)))),sizeof(uint32_t));
        t=uitoi(temp);
        if (t!=0) {
            fprintf(stderr,"Error : Already have an empty block at the end of the file.");
            e.val=-1;
            return e;
        }
        if((e=fill_block(id, num_partition)).val!=0){
            return e;
        }
        temp=itoui(ffb);
        memcpy(idir1->octets+(sizeof(uint32_t)*((nbtoadd-10)%(TTTFS_VOLUME_BLOCK_SIZE/sizeof(uint32_t)))),&temp,sizeof(uint32_t));
    }
    
    free(idir2);
    free(idir1);
    free(partition_block);
    free(file_entry_block);
    
    return e;
}



error free_block_from_file(disk_id* id, uint32_t num_partition,int entry_index){
    error e;
    e.val=0;
    uint32_t temp;
    int t,nbitdel,fsize,offset,num_block_entry,nbofblock,idir1_block,idir2_block;
    
    block *partition_block = malloc(sizeof(block));
    block *file_entry_block = malloc(sizeof(block));
    block *idir1 = malloc(sizeof(block));
    block *idir2 = malloc(sizeof(block));
    block *file_block = malloc(sizeof(block));
    
    //recuperation du block 0
    read_block(id,partition_block,num_partition);
    
    //verification du block 0
    memcpy(&temp,(partition_block->octets) + (MAGIC_NUMBER*sizeof(uint32_t)),sizeof(uint32_t));
    t=uitoi(temp);
    if (t != TTTFS_MAGIC_NUMBER) {
        fprintf(stderr, "Error : Partition is not using the same version as the programme.\n");
        e.val = -1;
        return e;
    }
    
    //verification de file_entry
    memcpy(&temp,(partition_block->octets) + (VOLUME_MAX_FILE_COUNT*sizeof(uint32_t)),sizeof(uint32_t));
    t=uitoi(temp);
    if (t<entry_index) {
        fprintf(stderr, "Error : File_entry too big./n");
        e.val=-1;
        return e;
    }
    
    //recuperation du block contenant la file_entry
    offset = (int)(FILE_TABLE_BLOCK_SIZE*sizeof(uint32_t));
    num_block_entry = num_partition+1+(entry_index / offset);
    read_block(id,file_entry_block,num_block_entry);
    
    //recuperation de la taille du fichier
    memcpy(&temp,(file_entry_block->octets) + (FILE_SIZE*sizeof(uint32_t)),sizeof(uint32_t));
    fsize=uitoi(temp);
    
    //recuperation de la taille en block
    if (fsize==0) {
        return e;
    }
    else{
        if (fsize%BLOCK_SIZE==0) {
            nbofblock=(fsize/BLOCK_SIZE)-1;
        }
        else{
            nbofblock=(fsize/BLOCK_SIZE);
        }
    }
    
    //au plus 10 blocks
    if (nbofblock<10) {
        memcpy(&temp,(file_entry_block->octets) + ((DIRECT+nbofblock)*sizeof(uint32_t)),sizeof(uint32_t));
        t=uitoi(temp);
        if(t==0){
            memcpy(&temp,(file_entry_block->octets) + ((DIRECT+nbofblock-1)*sizeof(uint32_t)),sizeof(uint32_t));
            nbofblock--;
            t=uitoi(temp);
        }
        
        nbitdel=occ_block_size(id,t);
        fsize-=nbitdel;
        
        
        free_block(id,num_partition, t);
        memcpy((file_entry_block->octets) + ((DIRECT+nbofblock)*sizeof(uint32_t)),0,sizeof(uint32_t));
        temp=itoui(fsize);
        memcpy((file_entry_block->octets) + (FILE_SIZE*sizeof(uint32_t)),&temp,sizeof(uint32_t));
        
    }
    //plus de 10 blocks est moins de 266 blocks
    else if(nbofblock-10 >=(BLOCK_SIZE/sizeof(uint32_t))*BLOCK_SIZE){
        memcpy(&temp,(file_entry_block->octets) + (INDIRECT1*sizeof(uint32_t)),sizeof(uint32_t));
        idir1_block=uitoi(temp);
        
        //pas de block indirect1 alloué
        if (idir1_block==0) {
            fprintf(stderr,"Error : Block indirect 1 not allocated");
            e.val=-1;
            return e;
           /* memcpy(&temp,(file_entry_block->octets) + ((DIRECT+nbofblock-1)*sizeof(uint32_t)),sizeof(uint32_t));
            nbofblock--;
            idir1_block=uitoi(temp);*/
        }
        
        nbitdel=occ_block_size(id,idir1_block);
        fsize-=nbitdel;
        
        nbofblock--;
        //supression de infirect1 si besoin
        if(nbofblock<=10){
            free_block(id,num_partition, idir1_block);
            memcpy((file_entry_block->octets) + (INDIRECT1*sizeof(uint32_t)),0,sizeof(uint32_t));
        }
        free_block(id,num_partition, idir1_block);
        memcpy((file_entry_block->octets) + ((DIRECT+nbofblock)*sizeof(uint32_t)),0,sizeof(uint32_t));
        temp=itoui(fsize);
        memcpy((file_entry_block->octets) + (FILE_SIZE*sizeof(uint32_t)),&temp,sizeof(uint32_t));
        
    }
    //plus de 266 blocks
    else{
        //recuperation du block indirect2
        memcpy(&temp,(file_entry_block->octets) + (INDIRECT2*sizeof(uint32_t)),sizeof(uint32_t));
        idir2_block=uitoi(temp);
        
        //pas de block indirect2 alloué
        if (idir2_block==0) {
            fprintf(stderr,"Error : Block indirect 2 not allocated");
            e.val=-1;
            return e;
            //on part du principe que il n'y a pas d'erreur
        }
        
        //block indirect2 alloué
        read_block(id,idir2,idir2_block);
        
        //recuperation du block indirect1
        memcpy(&temp,(idir2->octets) + ((((nbofblock-10-(TTTFS_VOLUME_BLOCK_SIZE/sizeof(uint32_t))*TTTFS_VOLUME_BLOCK_SIZE))/(TTTFS_VOLUME_BLOCK_SIZE/sizeof(uint32_t)))*sizeof(uint32_t)),sizeof(uint32_t));//a verifier
        idir1_block=uitoi(temp);
        //pas de block indirect1 alloué
        if (idir1_block==0) {
            memcpy(&temp,(file_entry_block->octets) + ((DIRECT+nbofblock-1)*sizeof(uint32_t)),sizeof(uint32_t));
            nbofblock--;
            idir1_block=uitoi(temp);
        }
    
    }
    
    
    free(idir2);
    free(idir1);
    free(partition_block);
    free(file_entry_block);
    
    return e;
}


