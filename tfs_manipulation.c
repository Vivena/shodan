//
//  tfs_manipulation.c
//  shodan
//
//  Created by Wintermute on 27/12/2015.
//
//

#include "main.h"
#include "ll.h"

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

// Supprimer un bloc de la liste des blocs libres
/*
error fill_block(disk_id* id, uint32_t num_partition, uint32_t num_fill){
  error e;
  e.val=0;
  block *partition_block = malloc(sizeof(block));
  block *fill_block = malloc(sizeof(block));
  uint32_t temp;
  int a;
  int first_free_block;
  int new_first_free_block;

  // Récupération des blocks
  read_block(id,partition_block,num_partition);
  read_block(id,fill_block,num_fill);

  // Récupration du premier block libre
  memcpy(&temp,(partition_block->octets) + (4*sizeof(uint32_t)),sizeof(uint32_t));
  first_free_block = uitoi(temp);

  // Gestion des différents cas
  if (first_free_block == uitoi(num_fill)){ // Si le bloc à occuper est le premier libre de la liste
    memcpy(&temp,(fill_block->octets) + (TTTFS_VOLUME_BLOCK_SIZE-sizeof(uint32_t)),sizeof(uint32_t));
    new_first_free_block = uitoi(temp);
    if (new_first_free_block == uitoi(num_fill)){ // Si plus de bloc libre
      new_first_free_block = 0;
    }
    a = itoui(new_first_free_block);
    memcpy((partition_block->octets) + (4*sizeof(uint32_t)),&a,sizeof(uint32_t));
  }
  a = itoui(first_free_block);
  memcpy((free_block->octets) + (TTTFS_VOLUME_BLOCK_SIZE-sizeof(uint32_t)),&a,sizeof(uint32_t));
  a = itoui(num_free);
  memcpy((partition_block->octets) + (4*sizeof(uint32_t)),&a,sizeof(uint32_t));

  // Réécriture des blocks
  write_block(id,partition_block,num_partition);
  write_block(id,free_block,num_free);
  sync_disk(id);

  return e;
}*/
