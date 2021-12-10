#include <assert.h>
#include <stdlib.h>

#include "file.h"
#include "constants.h"
#include "blocks.h"

char** get_file_blocks(inode_t *file) {
    int num_dirs = needed_datablocks(file->size, 0);

    char **file_blocks = malloc(sizeof(char*) * num_dirs);
    file_blocks[0] = blocks_get_block(file->block_0);

    if (num_dirs >= 2) {
        file_blocks[1] = blocks_get_block(file->block_1);
    }

    if (num_dirs >= 3) {
        int* datablocks = blocks_get_block(file->rd_block);
        for (int i = 2; i < num_dirs - 2; i++) {
            file_blocks[i] = blocks_get_block(datablocks[i]);
        }
    }

    return file_blocks;
}

int overwrite(inode_t *file, char *str) {
    char *block = blocks_get_block(file->block_0);
    char *current = str;
    int counter = 0;

    while (*current != '\0') {
        current++;
        counter++;
    }

    counter += 1;
    
    if (counter - file->size > 0) {
        assert(grow_inode(file, counter - file->size) == 0);
    }
    else if (counter - file->size < 0) {
        assert(shrink_inode(file, file->size - counter) == 0);
    }
    
    current = str;
    counter = 0;
    int index = 0;

    int num_dirs = needed_datablocks(file->size, 0);
    char** file_blocks = get_file_blocks(file);
    while (*current != '\0') {
        char* file_block = file_blocks[index];
        file_block[counter] = *current;
        if (counter + 1 == BLOCK_SIZE) {
            index += 1;
            counter = 0;
        }
        else {
            counter++;
        }
        current++;
    }
    file_blocks[index][counter] = '\0';
    free(file_blocks);

    return 0;
}
