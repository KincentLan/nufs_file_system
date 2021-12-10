// Inode manipulation routines.
//
// Feel free to use as inspiration.

// based on cs3650 starter code

#include <stdio.h>
#include <assert.h>

#include "inode.h"
#include "bitmap.h"
#include "constants.h"


void print_inode(inode_t *node) {
    printf("Inode: %d\nReferences: %d, Type: %d, Size: %d\nBlocks: (%d, %d, %d)\n", node->fid, node->refs, node->mode,
        node->size, node->block_0, node->block_1, node->rd_block);
}

inode_t *get_inode(int inum) {
    assert(inum < DATABLOCK_COUNT);
    int amountPerBlock = BLOCK_SIZE / sizeof(inode_t);
    int index = inum / amountPerBlock + 1;
    int currentInode = inum % amountPerBlock;
    void *inode_block = blocks_get_block(index);
    void *inode = inode_block + currentInode * sizeof(inode_t);
    return (inode_t *) inode;
}

int alloc_inode(int mode) {
  void *bbm = get_inode_bitmap();
  for (int i = 0; i < DATABLOCK_COUNT; ++i) {
    if (!bitmap_get(bbm, i)) {
      bitmap_put(bbm, i, 1);
      printf("+ alloc_inode_block() -> %d\n", i);
      inode_t* new_inode = get_inode(i);
      new_inode->refs = 1;
      new_inode->fid = i;
      new_inode->mode = mode;
      new_inode->size = 0;
      new_inode->block_0 = alloc_block();
      new_inode->block_1 = -1;
      new_inode->rd_block = -1;
      return i;
    }
  }
  return -1;
}

int dup_inode(inode_t *node) {
  void *bbm = get_inode_bitmap();
  for (int i = 1; i < DATABLOCK_COUNT; ++i) {
    if (!bitmap_get(bbm, i)) {
      node->refs += 1;
      bitmap_put(bbm, i, 1);
      printf("+ alloc_inode_block() -> %d\n", i);
      inode_t* new_inode = get_inode(i);
      new_inode->refs = node->refs;
      new_inode->fid = node->fid;
      new_inode->mode = node->mode;
      new_inode->size = node->size;
      new_inode->block_0 = node->block_0;
      new_inode->block_1 = node->block_1;
      new_inode->rd_block = node->rd_block;
      return i;
    }
  }
  return -1;
}

void free_inode(int inum) {
  printf("+ free_inode_block(%d)\n", inum);
  inode_t *node = get_inode(inum);
  int current_fid = node->fid;
  if (node->refs > 1) {
    int next_fid = -1;
    for (int i = 0; i < DATABLOCK_COUNT; i++) {
      inode_t* current_node = get_inode(i);
      if (current_node->fid == current_fid) {
        current_node->refs -= 1;
        if (inum == current_fid && next_fid == -1 && i != inum) {
          next_fid = i;
          current_node->fid = next_fid;
        }

        if (inum == current_fid) {
          current_node->fid = next_fid;
        }
      }
    }
  }
  void *bbm = get_inode_bitmap();
  bitmap_put(bbm, inum, 0);
}

int grow_inode(inode_t *node, int size) {
    assert(size > 0);

    int total_size = node->size + size;
    if (node->block_0 == -1) {
        node->block_0 = alloc_block();
    }
    if (total_size > BLOCK_SIZE && node->block_1 == -1) {
        node->block_1 = alloc_block();
    }
    if (total_size > 2 * BLOCK_SIZE && node->rd_block == -1) {
        node->rd_block = alloc_block();
        int *block_array = blocks_get_block(node->rd_block);
        int num_datablocks = needed_datablocks(total_size, 2 * BLOCK_SIZE);
        if (num_datablocks > 255) {
            perror("Not enough memory to store file.\n");
            return -1;
        }
        for (int i = 0; i < num_datablocks; i++) {
            block_array[i] = alloc_block();
        }
    }
    node->size = total_size;
    return 0;
}

int shrink_inode(inode_t *node, int size) {
    assert(size > 0);
    assert(node->block_0 != -1);
    
    int total_size = node->size - size;
    if (node->size > 2 * BLOCK_SIZE && node->rd_block != -1) {
        int num_datablocks = needed_datablocks(node->size, 2 * BLOCK_SIZE);
        int goal_datablocks = needed_datablocks(total_size, 2 * BLOCK_SIZE);
        int *block_array = blocks_get_block(node->rd_block);
        for (int i = num_datablocks - 1; i >= goal_datablocks; i--) {
            free_block(block_array[i]);
            block_array[i] = 0;
        }
        if (goal_datablocks == 0) {
            free_block(node->rd_block);
            node->rd_block = -1;
        }
    }

    if (total_size <= BLOCK_SIZE && node->block_1 != -1) {
        free_block(node->block_1);
        node->block_1 = -1;
    }

    node->size = total_size;
}

int needed_datablocks(int total_size, int offset) {
  int remainder = total_size - offset;

  if (remainder <= 0) {
    return 0;
  }

  if (remainder % BLOCK_SIZE != 0) {
    return remainder / BLOCK_SIZE + 1;
  }
  return remainder / BLOCK_SIZE;
}
