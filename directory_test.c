#include <stdio.h>
#include <sys/stat.h>

#include "bitmap.h"
#include "blocks.h"
#include "inode.h"
#include "constants.h"
#include "directory.h"
#define TEST_NAME "block_test.img"

int main(int argc, char **argv){
    // main
    blocks_init(TEST_NAME);
    erase_memory();
    blocks_init(TEST_NAME);
    directory_init();

    int main_idx = get_main_directory();
    inode_t* inode_main = get_inode(main_idx);
    print_inode(inode_main);
    printf("\n\n");

    printf("print_directory of main:\n");
    print_directory(inode_main);

    printf("\n\n");

    printf("Creating directory 'side':\n");
    int side_idx = directory_create(S_IFDIR);
    inode_t* inode_side = get_inode(side_idx);
    print_inode(inode_side);
    printf("\n\n");

    directory_put(inode_main, "side", side_idx);
    
    printf("print_directory of main after putting side:\n");
    print_directory(inode_main);

    printf("\n\n");
   
    printf("Directory look up side directory: %d\n", directory_lookup(inode_main, "side"));
    printf("Tree look up empty directory: %d\n", tree_lookup("/"));
    printf("Tree look up side directory: %d\n", tree_lookup("/side"));

    // /main/side/last
    printf("Creating directory 'last':\n");

    int last_idx = directory_create(S_IFDIR);
    inode_t* inode_last = get_inode(last_idx);
    printf("Adding last to main directory\n");
    directory_put(inode_side, "last", last_idx);

    printf("print_directory of main after putting last:\n");
    print_directory(inode_main);

    printf("\n\n");

    printf("print_directory of side after putting last:\n");
    print_directory(inode_side);

    printf("\n\n");

    printf("Tree look up /side/last directory: %d\n", tree_lookup("/side/last"));

    printf("\n\n");

    printf("Looking up directory 'side' and 'last': %d, %d\n", 
        directory_lookup(inode_main, "side"),
        directory_lookup(inode_side, "last"));

    // /main/last
    printf("Deleting 'side':\n");
    directory_delete(inode_main, "side");
    print_directory(inode_main);
}