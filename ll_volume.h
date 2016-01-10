// --------------------------
// Fonctions de l'API de manipulation du volume

error free_block(disk_id* id, uint32_t num_partition, uint32_t num_free);
error fill_block(disk_id* id, uint32_t num_partition);
error free_entry(disk_id* id, uint32_t num_partition, uint32_t entry_index);
error fill_entry(disk_id* id, uint32_t num_partition, TTTFS_File_Table_Entry* entry);
int cut_pathname(char** res, const char* path);
int is_in_directory(disk_id* id, int* current_dir, int pemplacement, char* name);
int have_next_block(disk_id* id, uint32_t num_partition,int entry_index,int num_block);
