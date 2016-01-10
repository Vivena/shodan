//
//  tfs_manipulation.c
//  shodan
//
//  Created by Wintermute on 27/12/2015.
//
//

#include "main.h"
#include "ll.h"
#include "ll_volume.h"

#define MAGIC_NUMBER 0
#define FREE_BLOCK_COUNT 4
#define FIRST_FREE_BLOCK 5
#define VOLUME_MAX_FILE_COUNT 6

#define FILE_SIZE 0
#define DIRECT 3
#define INDIRECT1 13
#define INDIRECT2 14

#define NB_DIRECT(m) m-9
#define NB_IDIR1 (BLOCK_SIZE/sizeof(uint32_t))*BLOCK_SIZE



/**
 * \brief Met un block dans la liste des block libres
 * \details Modifie la valeur du first_free_block dans la description de la partition (devient le numéro de block de celui à libérer) et la valeur du next du block à libérer est le first_free_block qui se trouvait dans la partition.
 *          
 *          
 * \param id L'identifiant du disque.
 * \param num_partition Numero du block où se trouve le début de la partition.
 * \param num_free Numero du block à libérer
 * \return 0 si tout se passe bien, -1 sinon.
 */
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


/**
 * \brief Supprime un bloc de la liste des blocs libres
 * \details Modifie la valeur du first_free_block dans la description de la partition (devient le next de first_free_block).
 *          
 *          
 * \param id L'identifiant du disque.
 * \param num_partition Numero du block où se trouve le début de la partition.
 * \return 0 si tout se passe bien, -1 sinon.
 */
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

/**
 * \brief Met une entrée de répertoire dans la liste des entrées libres
 * \details Modifie la valeur du first_free_file dans la description de la partition (devient le next de l'entrée) et le next de l'entrée est le first_free_file qui se trouvait dans la partition.
 *          
 *          
 * \param id L'identifiant du disque.
 * \param num_partition Numero du block où se trouve le début de la partition.
 * \param entry_index Numero de l'entrée dans le File Table.
 * \return 0 si tout se passe bien, -1 sinon.
 */
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

/**
 * \brief Supprime une entrée des entrées libres
 * \details Modifie la valeur du first_free_file dans la description de la partition (devient le next de first_free_file), puis ajoute l'entrée au File Table.
 *          
 *          
 * \param id L'identifiant du disque.
 * \param num_partition Numero du block où se trouve le début de la partition.
 * \param entry Entrée à ajouter dans le File Table.
 * \return 0 si tout se passe bien, -1 sinon.
 */
error fill_entry(disk_id* id, uint32_t num_partition, TTTFS_File_Table_Entry * entry){
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
        next = 0;
    }
    a = itoui(next);
    memcpy((partition_block->octets) + (7*sizeof(uint32_t)),&a,sizeof(uint32_t));
    
    // Remplissage
    i = 0;
    a = itoui(entry->size);
    memcpy((file_table_block->octets) + entry_position + (i*sizeof(uint32_t)),&a,sizeof(uint32_t));
    a = itoui(entry->type); i++;
    memcpy((file_table_block->octets) + entry_position + (i*sizeof(uint32_t)),&a,sizeof(uint32_t));
    a = itoui(entry->sub_type); i++;
    memcpy((file_table_block->octets) + entry_position + (i*sizeof(uint32_t)),&a,sizeof(uint32_t));
    for (j = 0; j < 10; j++){
        a = itoui(entry->tfs_direct[j]); i++;
        memcpy((file_table_block->octets) + entry_position + (i*sizeof(uint32_t)),&a,sizeof(uint32_t));
    }
    a = itoui(entry->tfs_indirect1); i++;
    memcpy((file_table_block->octets) + entry_position + (i*sizeof(uint32_t)),&a,sizeof(uint32_t));
    a = itoui(entry->tfs_indirect2); i++;
    memcpy((file_table_block->octets) + entry_position + (i*sizeof(uint32_t)),&a,sizeof(uint32_t));
    a = itoui(next); i++;
    memcpy((file_table_block->octets) + entry_position + (i*sizeof(uint32_t)),&a,sizeof(uint32_t));
    
    // Réécriture des blocks
    write_block(id,partition_block,num_partition);
    write_block(id,file_table_block,num_block_entry);
    sync_disk(id);
    
    return e;
}


/**
 * \brief Ajoute un block à la liste des block d'un fichier
 * \details Regarde l'emplacement du premier lien libre du fichier dont le numero d'inode est entry_index et le fait pointer sur le premier block libre de la partition, puis met à jour le numero du premier block libre.
 *          
 *          
 * \param id L'identifiant du disque.
 * \param num_partition Numero du block où se trouve le début de la partition.
 * \param adresse 
 * \return 0 si tout se passe bien, -1 sinon.
 */
error add_block_to_file(disk_id* id, uint32_t num_partition,int entry_index,int *adresse){
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
    else if(NB_DIRECT(nbtoadd) <=NB_IDIR1){
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
        memcpy(&temp,idir1->octets+(sizeof(uint32_t)*(NB_DIRECT(nbtoadd))),sizeof(uint32_t));
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
        memcpy(idir1->octets+(sizeof(uint32_t)*(NB_DIRECT(NB_DIRECT(nbtoadd)))),&temp,sizeof(uint32_t));
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
        memcpy(&temp,(idir2->octets) + (( (NB_DIRECT(nbtoadd)-(NB_IDIR1))/(TTTFS_VOLUME_BLOCK_SIZE/sizeof(uint32_t)))*sizeof(uint32_t)),sizeof(uint32_t));//a verifier
        idir1_block=uitoi(temp);
        
        //pas de block indirect1 alloué
        if (idir1_block==0) {
            idir1_block=ffb;
            temp=itoui(idir1_block);
            if((e=fill_block(id, num_partition)).val!=0){
                return e;
            }
            memcpy( (idir2->octets) + ((NB_DIRECT(nbtoadd)-(NB_IDIR1))/((TTTFS_VOLUME_BLOCK_SIZE/sizeof(uint32_t)))*sizeof(uint32_t)),&temp,sizeof(uint32_t));
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
        memcpy(&temp,(idir1->octets)+((sizeof(uint32_t)*((NB_DIRECT(nbtoadd)-(NB_IDIR1))%(NB_IDIR1)))),sizeof(uint32_t));
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
        memcpy(idir1->octets+(sizeof(uint32_t)*((NB_DIRECT(nbtoadd))%(TTTFS_VOLUME_BLOCK_SIZE/sizeof(uint32_t)))),&temp,sizeof(uint32_t));
    }
    
    adresse[0]=ffb;
    
    free(idir2);
    free(idir1);
    free(partition_block);
    free(file_entry_block);
    
    return e;
}

/**
 * \brief Supprime le dernier block occupé d'un fichier
 * \details Va au dernier lien occupé du fichier dont le numero d'inode est entry_index, regarde le nombre d'octet occupé dans ce block puis supprime le block et met à jour la taille du fichier
 *          
 *          
 * \param id L'identifiant du disque.
 * \param num_partition Numero du block où se trouve le début de la partition.
 * \param entry_index Numero de l'entrée dans le File Table.
 * \return 0 si tout se passe bien, -1 sinon.
 */
error free_block_from_file(disk_id* id, uint32_t num_partition,int entry_index){
    error e;
    e.val=0;
    uint32_t temp;
    int t,nbitdel,fsize,offset,num_block_entry,nbofblock,idir1_block,idir2_block;
    
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
        temp=itoui(0);
        memcpy((file_entry_block->octets) + ((DIRECT+nbofblock)*sizeof(uint32_t)),&temp,sizeof(uint32_t));
        temp=itoui(fsize);
        memcpy((file_entry_block->octets) + (FILE_SIZE*sizeof(uint32_t)),&temp,sizeof(uint32_t));
        
    }
    //plus de 10 blocks est moins de 266 blocks
    else if(nbofblock-10 <=(BLOCK_SIZE/sizeof(uint32_t))*BLOCK_SIZE){
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
        
        //block indirect1 alloué
        read_block(id,idir1,idir1_block);
        //recuperation du dernier block occupé
        memcpy(&temp,idir1->octets+(sizeof(uint32_t)*(nbofblock-10)),sizeof(uint32_t));
        t=uitoi(temp);
        //si le dernier block occupé n'est pas attribué on essail de regarder le block precedent
        if(t==0){
            if ((sizeof(uint32_t)*(nbofblock-10))-1==0) {
                e.val=-1;
                return e;
            }
            memcpy(&temp,(idir1->octets) + (((sizeof(uint32_t)*(nbofblock-10))-1)*sizeof(uint32_t)),sizeof(uint32_t));
            nbofblock--;
            t=uitoi(temp);
        }
        
        
        //calcul de la taille de la file apres
        nbitdel=occ_block_size(id,idir1_block);
        fsize-=nbitdel;
        
       
        //supression du dernier block alloué de indirect1
        free_block(id,num_partition, t);
        memcpy((file_entry_block->octets) + ((DIRECT+nbofblock)*sizeof(uint32_t)),&temp,sizeof(uint32_t));
        nbofblock--;
        //suppression de infirect1 si besoin
        temp=itoui(0);
        if(nbofblock<=10){
            free_block(id,num_partition, idir1_block);
            memcpy((file_entry_block->octets) + (INDIRECT1*sizeof(uint32_t)),&temp,sizeof(uint32_t));
        }
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
        memcpy(&temp,(idir2->octets) + ((NB_DIRECT(nbofblock)-NB_IDIR1)/(NB_IDIR1))*sizeof(uint32_t),sizeof(uint32_t));//a verifier
        idir1_block=uitoi(temp);
        //pas de block indirect1 alloué
        if (idir1_block==0) {
            fprintf(stderr,"Error : indirect 1 not allocated.\n");
            e.val=-1;
            return e;
        }
        //block indirect 1 alloué
        read_block(id,idir1,idir1_block);
        
        
        if (idir1_block==0) {
            memcpy(&temp,(file_entry_block->octets) + ((DIRECT+nbofblock-1)*sizeof(uint32_t)),sizeof(uint32_t));
            nbofblock--;
            idir1_block=uitoi(temp);
        }
        
        //recuperation du dernier block occupé
        memcpy(&temp,idir1->octets+(sizeof(uint32_t)*(nbofblock-10)),sizeof(uint32_t));
        t=uitoi(temp);
        //si le dernier block occupé n'est pas attribué on essail de regarder le block precedent
        if(t==0){
            if ((sizeof(uint32_t)*((NB_DIRECT(nbofblock)-NB_IDIR1)/(NB_IDIR1)))-1==0) {
                e.val=-1;
                return e;
            }
            memcpy(&temp,(idir1->octets) + (((sizeof(uint32_t)*((NB_DIRECT(nbofblock)-NB_IDIR1)/(NB_IDIR1)))-1)*sizeof(uint32_t)),sizeof(uint32_t));
            nbofblock--;
            t=uitoi(temp);
        }
        
        
        //calcul de la taille de la file apres
        nbitdel=occ_block_size(id,idir1_block);
        fsize-=nbitdel;
        
        
        //supression du dernier block alloué de indirect1
        free_block(id,num_partition, t);
        memcpy((file_entry_block->octets) + ((DIRECT+nbofblock)*sizeof(uint32_t)),&temp,sizeof(uint32_t));
        nbofblock--;
        
        //suppression si besoin de indirect 1
	int oldnb = 0;
        if(oldnb-(NB_DIRECT(nbofblock)-NB_IDIR1)/(NB_IDIR1)==1){
            free_block(id,num_partition, idir1_block);
            memcpy((idir2->octets) + ((NB_DIRECT(nbofblock)-NB_IDIR1)/(NB_IDIR1))*sizeof(uint32_t),&temp,sizeof(uint32_t));
        }
        
        //supression si besoin de indirect 2
        if ((NB_DIRECT(nbofblock)-NB_IDIR1)/(NB_IDIR1)==0) {
            free_block(id,num_partition, idir2_block);
            memcpy((file_entry_block->octets) + (INDIRECT2*sizeof(uint32_t)),&temp,sizeof(uint32_t));
        }
    
    }
    
    
    free(idir2);
    free(idir1);
    free(partition_block);
    free(file_entry_block);
    
    return e;
}

/**
 * \brief Supprime tous les blocks d'un fichier.
 * \details 
 *          
 *          
 * \param id L'identifiant du disque.
 * \param num_partition Numero du block où se trouve le début de la partition.
 * \param entry_index Numero de l'entrée dans le File Table.
 * \return 0 si tout se passe bien, -1 sinon.
 */
error wipe_file(disk_id* id, uint32_t num_partition,int entry_index){
    error e;
    e.val=0;
    uint32_t temp;
    int t,fsize,offset,num_block_entry;
    
    block *partition_block = malloc(sizeof(block));
    block *file_entry_block = malloc(sizeof(block));
    
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
    while (fsize>0) {
        free_block_from_file(id,num_partition,entry_index);
        fsize=uitoi(temp);
    }
    
    
    free(partition_block);
    free(file_entry_block);
    return e;
}

/**
 * \brief Renvois le block suivant d'un fichier
 * \details Recois le numero d'un block num_block d'un fichier, regarde si num_block+1 est initialisé. Si il l'est, renvois le lien sur ce block, sinon renvois -1
 *          
 *          
 * \param id L'identifiant du disque.
 * \param num_partition Numero du block où se trouve le début de la partition.
 * \param entry_index Numero de l'entrée dans le File Table.
 * \param entry_index Numero du block.
 * \return 0 si tout se passe bien, -1 sinon.
 */
int have_next_block(disk_id* id, uint32_t num_partition,int entry_index,int num_block){
    uint32_t temp;
    int t=1,offset,num_block_entry,idir1_block=0,idir2_block=0;
    
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
        
        return -1;
    }
    
    //verification de file_entry
    memcpy(&temp,(partition_block->octets) + (VOLUME_MAX_FILE_COUNT*sizeof(uint32_t)),sizeof(uint32_t));
    t=uitoi(temp);
    if (t<entry_index) {
        fprintf(stderr, "Error : File_entry too big./n");
        
        return -1;
    }
    
    //recuperation du block contenant la file_entry
    offset = (int)(FILE_TABLE_BLOCK_SIZE*sizeof(uint32_t));
    num_block_entry = num_partition+1+(entry_index / offset);
    read_block(id,file_entry_block,num_block_entry);
    num_block++;
    
    // suivant dans direct
    if (num_block>=0 && num_block<10) {
        memcpy(&temp,(file_entry_block->octets) + ((DIRECT+num_block)*sizeof(uint32_t)),sizeof(uint32_t));
        t=uitoi(temp);
        //n'a pas de next
        if(t==0){
            free(idir2);
            free(idir1);
            free(partition_block);
            free(file_entry_block);
            return -1;
        }
    }
    
    //suivant dans indirect 1
    else if(num_block<NB_IDIR1){
        memcpy(&temp,(file_entry_block->octets) + (INDIRECT1*sizeof(uint32_t)),sizeof(uint32_t));
        idir1_block=uitoi(temp);
        //n'a pas de next et necessite la crea de indire 1
        if(idir1_block==0){
            free(idir2);
            free(idir1);
            free(partition_block);
            free(file_entry_block);
            return -1;
        }
        
        //block indirect1 alloué
        read_block(id,idir1,idir1_block);
        memcpy(&temp,idir1->octets+(sizeof(uint32_t)*(NB_DIRECT(num_block))),sizeof(uint32_t));
        
        t=uitoi(temp);
        if (t==0) {
            free(idir2);
            free(idir1);
            free(partition_block);
            free(file_entry_block);
            return -1;
        }
        free(idir2);
        free(idir1);
        free(partition_block);
        free(file_entry_block);
        return t;
    }
    
    //suivant dans indirect 2
    else{
        //recuperation du block indirect2
        memcpy(&temp,(file_entry_block->octets) + (INDIRECT2*sizeof(uint32_t)),sizeof(uint32_t));
        idir2_block=uitoi(temp);
        
        //pas de block indirect2 alloué
        if (idir2_block==0) {
            free(idir2);
            free(idir1);
            free(partition_block);
            free(file_entry_block);
            return -1;
        }
        
        //block indirect2 alloué
        read_block(id,idir2,idir2_block);
        
        //recuperation du block indirect1
        memcpy(&temp,(idir2->octets) + ((NB_DIRECT(num_block)-NB_IDIR1)/(NB_IDIR1))*sizeof(uint32_t),sizeof(uint32_t));//a verifier
        idir1_block=uitoi(temp);
        //pas de block indirect1 alloué
        if (idir1_block==0) {
            free(idir2);
            free(idir1);
            free(partition_block);
            free(file_entry_block);
            return -1;
        }
        //block indirect 1 alloué
        read_block(id,idir1,idir1_block);
        memcpy(&temp,idir1->octets+(sizeof(uint32_t)*((NB_DIRECT(num_block)-NB_IDIR1)%(NB_IDIR1))),sizeof(uint32_t));
        
        t=uitoi(temp);
        if (t==0) {
            free(idir2);
            free(idir1);
            free(partition_block);
            free(file_entry_block);
            return -1;
        }
        free(idir2);
        free(idir1);
        free(partition_block);
        free(file_entry_block);
        return t;
        
    }
    
    
    free(idir2);
    free(idir1);
    free(partition_block);
    free(file_entry_block);
    
    return -1;
}



/*get_ffd(disk_id* id, uint32_t num_partition){
    
    //recuperation du block 0
    read_block(id,partition_block,num_partition);
    
    //verification du block 0
    memcpy(&temp,(partition_block->octets) + (MAGIC_NUMBER*sizeof(uint32_t)),sizeof(uint32_t));
    t=uitoi(temp);
    if (t != TTTFS_MAGIC_NUMBER) {
        fprintf(stderr, "Error : Partition is not using the same version as the programme.\n");
        return -2;
    }
    
    
}*/


/**
 * \brief Découpe un path
 * \details Sépare un pathname dans un tableau avec toutes les infos
 *          necessaires (disque, volume (si pas HOST), repertoires...
 *                 
 * \param res Tableau de string à remplir
 * \param path Chemin à évaluer
 * \return 0 si tout se passe bien, -1 sinon.
 */
int cut_pathname(char** res, const char* path){
  int e = 1;
  char file[8];
  char path_copy[STRING_MAX_LENGTH];
  int offset = 7;
  strncpy(path_copy,path,STRING_MAX_LENGTH);

  // Teste si on a bien "FILE://" au début du path
  memcpy(file,path_copy,offset);
  file[offset] = '\0';
  if (strcmp(file,"FILE://") != 0){
    fprintf(stderr,"Error pathname syntax, waiting for FILE://");
    return -1;
  }

  // Récupération du disk, du volume et du reste sous forme d'un tableau
  str_split(res, &path_copy[offset],'/');

  // Si c'est un HOST
  if (strcmp(res[0],"HOST") == 0){
    e = 0;
  }
  else{
    // Si c'est un disque, on vérifie s'il existe bien
    if (access(res[0],F_OK) == -1){
      fprintf(stderr,"Error pathname, disk %s doesn't exists.\n", res[0]);
      return -1;
    }
  }

  return e;
}

/**
 * \brief Teste si un répertoire existe
 * \details Teste si le nom d'un fichier/répertoire apparait dans le dossier courant indiqué, et donne le numéro d'entrée du fichier s'il existe.
 *                 
 * \param id L'identifiant du disque.
 * \param current_dir Addresse du répertoire courant.
 * \param pemplacement Numero du block où se trouve le début de la partition.
 * \param name Nom du répertoire don't il faut tester l'existence.
 * \return Vrai si le répertoire existe, faux sinon.
 */
int is_in_directory(disk_id* id, int* current_dir, int pemplacement, char* name){
  int exists = 0, index, index_entry, i, j, temp;
  char dir_name[TFS_DIRECTORIES_SIZE];

  block* block_entry = malloc(sizeof(block));
  block* block_navigation = malloc(sizeof(block));


  // Lecture du File Table (recherche du répertoire courrant)
  index = pemplacement+1+((*current_dir)/FILE_TABLE_OFFSET);
  read_block(id,block_entry,index);
  index_entry = ((*current_dir) % FILE_TABLE_OFFSET)*FILE_TABLE_BLOCK_SIZE;

  // Recherche du repertoire dans l'un des blocs
  for (i = 0; i < 13; i++){
    memcpy(&temp,(block_entry->octets) + index_entry + ((3+i)*sizeof(uint32_t)),sizeof(uint32_t));
    index = uitoi(temp);
    if (index == 0) break; // Plus rien à visiter
    if (i < 10){ // Si bloc direct
      read_block(id,block_navigation,index);
      for (j = 0; j < TFS_DIRECTORIES_MAX_ENTRIES;j++){
	memcpy(dir_name,(block_navigation->octets) + (j*TFS_DIRECTORIES_SIZE) + sizeof(uint32_t),TFS_DIRECTORIES_SIZE-sizeof(uint32_t));
	if (strcmp(dir_name,name) == 0){
	  memcpy(&temp,(block_navigation->octets) + (j*TFS_DIRECTORIES_SIZE),sizeof(uint32_t));
	  (*current_dir) = uitoi(temp);
	  exists = 1;
	  i = 13; 
	  j = TFS_DIRECTORIES_MAX_ENTRIES;
	  break;
	}
	if (!dir_name[0]){ // Si nom vide, alors plus rien à tester
	  i = 13; 
	  j = TFS_DIRECTORIES_MAX_ENTRIES;
	  break;
	}
      }
    }
    else if (i == 11){ // Si bloc indirect1

    }
    else{ // Si bloc indirect2

    }	
  }

  free(block_entry);
  free(block_navigation);

  return exists;
}


