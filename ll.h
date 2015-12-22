// --------------------------
// Fonctions de l'API de manipulation du disque

#ifndef Header_h
#define Header_h

error start_disk(char* name, disk_id* id);
error read_block(disk_id* id, block* b,uint32_t num);
error read_physical_block(disk_id *id,block *b,uint32_t num);
error write_block(disk_id* id, block b,uint32_t num);
error write_physical_block(disk_id *id,block b,uint32_t num);
error sync_disk(disk_id *id);
error stop_disk(disk_id id);

#endif
