// --------------------------
// Fonctions de l'API de manipulation du disque

#ifndef Header_h
#define Header_h

error start_disk(char* name, disk_id* id);
error read_block(disk_id* id, block *b,uint32_t num);
error read_physical_block(disk_id *id,block *b,uint32_t num);
error write_block(disk_id* id, block *b,uint32_t num);
error write_physical_block(disk_id *id,block *b,uint32_t num);
error sync_disk(disk_id *id);
error stop_disk(disk_id id);


error free_block(disk_id* id, uint32_t num_partition, uint32_t num_free);
error fill_block(disk_id* id, uint32_t num_partition, uint32_t num_fill);
error free_entry(disk_id* id, uint32_t num_partition, uint32_t entry_index);
error put_dir_entry(disk_id* id, uint32_t num_partition, TTTFS_File_Table_Entry entry);
#endif
