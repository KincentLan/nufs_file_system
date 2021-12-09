#include <stdio.h>

#include "bitmap.h"
#include "blocks.h"
#include "inode.h"
#include "constants.h"
#define TEST_NAME "block_test.img"

int main(int argc, char **argv) {
  blocks_init(TEST_NAME);
  
  printf("Bitmaps at the beginning:\n");
  bitmap_print(get_blocks_bitmap(), 256);
  printf("--------------------------------------------\n");
  bitmap_print(get_inode_bitmap(), DATABLOCK_COUNT);
  printf("\n\n");

  int inode_idx_0 = alloc_inode();
  inode_t* inode_0 = get_inode(inode_idx_0);
  grow_inode(inode_0, 4097);
  print_inode(inode_0);

  printf("\n\n");

  int inode_idx_1 = alloc_inode();
  inode_t* inode_1 = get_inode(inode_idx_1);
  grow_inode(inode_1, 256);
  print_inode(inode_1);

  printf("\n\n");

  int inode_idx_2 = dup_inode(inode_1);
  inode_t* inode_dup = get_inode(inode_idx_2);
  printf("Duplicated:\n");
  print_inode(inode_dup);

  printf("\n\n");

  int inode_idx_3 = alloc_inode();
  inode_t* inode_3 = get_inode(inode_idx_3);
  grow_inode(inode_3, 170);
  print_inode(inode_3);
  shrink_inode(inode_3, 163);
  print_inode(inode_3);

  printf("\n\n");

  print_inode(inode_1);

  printf("\n\n");

  free_inode(inode_idx_1);
  printf("Duplicated:\n");
  print_inode(inode_dup);

  printf("\n\n");

  printf("Bitmaps after allocation:\n");
  bitmap_print(get_blocks_bitmap(), 256);
  printf("--------------------------------------------\n");
  bitmap_print(get_inode_bitmap(), DATABLOCK_COUNT);
  printf("\n\n");

  erase_memory();
  blocks_free();

  return 0;
}
