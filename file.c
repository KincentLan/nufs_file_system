#include <assert.h>
#include "file.h"
#include "constants.h"

const int BLOCK_SIZE = 4096; // = 4K

int overwrite(inode_t *file, char *str) {
    char *block = blocks_get_block(file->block_0);
    char *current = str;

    while (*current != '\0') {
        current++;
    }

    current += 1;
    
    file->size = 0;
    grow_inode(file, current);
    int *block_array;
    if (file->rd_block != -1) {
        block_array = blocks_get_block(file->rd_block);
    }
    current = str;

    int counter = 0;
    int curr_block = 0;

    while (*current != '\0') {
        block[counter] = *current;
        if (counter + 1 % BLOCK_SIZE == 0) {
            if (curr_block == 0) {
                block = blocks_get_block(file->block_1);
            }
            if (curr_block >= 1) {
                int data_block = block_array[curr_block - 1];
                assert(data_block != 0);
                block = blocks_get_block(data_block);
            }
            curr_block++;
        }
        counter++;
        current++;
    }
    block[counter] = '\0';
    return 0;
}
