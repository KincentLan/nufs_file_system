// Inode manipulation routines.
//
// Feel free to use as inspiration.

// based on cs3650 starter code

#ifndef INODE_H
#define INODE_H

#include "blocks.h"

typedef struct inode {
  int refs;  // reference count
  int fid; // original file id
  int mode;  // permission & type
  int size;  // bytes
  int block_0; // single block pointer (if max file size <= 4K)
  int block_1; // single block pointer
  int rd_block; // pointer to data block storing more pointers
} inode_t;

void print_inode(inode_t *node);

inode_t *get_inode(int inum);

int alloc_inode(int mode);

int dup_inode(inode_t *node);

void free_inode(int inum);

int grow_inode(inode_t *node, int size);

int shrink_inode(inode_t *node, int size);

// int inode_get_pnum(inode_t *node, int fpn);

int needed_datablocks(int total_size, int offset);

#endif
