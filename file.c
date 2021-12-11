#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "file.h"
#include "constants.h"

char** get_file_blocks(inode_t *file) {
    int num_dirs = needed_datablocks(file->size, 0);

    char **file_blocks = malloc(sizeof(char*) * num_dirs);
    file_blocks[0] = (char*) blocks_get_block(file->block_0);

    if (num_dirs >= 2) {
        file_blocks[1] = (char*) blocks_get_block(file->block_1);
    }

    if (num_dirs >= 3) {
        int* datablocks = (int*) blocks_get_block(file->rd_block);
        for (int i = 2; i < num_dirs - 2; i++) {
            file_blocks[i] = (char*) blocks_get_block(datablocks[i]);
        }
    }

    return file_blocks;
}

int read_file(inode_t* file, char* buf, size_t s, off_t offset) {
    assert(offset < file->size);

    char** blocks = get_file_blocks(file);
    int num_dirs = needed_datablocks(file->size, 0);

    for (int i = 0; i < num_dirs; i++) {
        if (s <= 0) {
            break;
        }
        int difference = file->size - i * BLOCK_SIZE;
        int cap = (BLOCK_SIZE > difference) ? difference : BLOCK_SIZE;
        char* block = blocks[i];
        
        int pg_offset = offset - i * BLOCK_SIZE;

        if (pg_offset >= BLOCK_SIZE) {
            continue;
        }

        pg_offset = (pg_offset < 0) ? 0 : pg_offset;
        int num_chars = cap - pg_offset;
        char* start_position = block + pg_offset;

        memcpy(buf + i * BLOCK_SIZE, start_position, num_chars);
        s -= num_chars;
    }
    
    free(blocks);
    return 0;
}

int overwrite_file(inode_t *file, const char *str, size_t s, off_t offset) {
    int total_size = s + offset;
    
    if (file->size - total_size < 0) {
        if(grow_inode(file, total_size - file->size) == -1) {
            return -1;
        }
    }

    char **blocks = get_file_blocks(file);
    int num_dirs = needed_datablocks(s, 0);

    char* current = str;

    for (int i = 0; i < num_dirs; i++) {
        if (s <= 0) {
            break;
        }

        int difference = file->size - i * BLOCK_SIZE;
        int cap = (BLOCK_SIZE > difference) ? difference : BLOCK_SIZE;
        char* block = blocks[i];
        
        int pg_offset = offset - i * BLOCK_SIZE;

        if (pg_offset >= BLOCK_SIZE) {
            continue;
        }

        pg_offset = (pg_offset < 0) ? 0 : pg_offset;
        int num_chars = cap - pg_offset;
        char* start_position = block + pg_offset;

        memcpy(start_position, current, num_chars);
        current += num_chars;
        s -= num_chars;
    }
    
    free(blocks);
    return 0;
}
