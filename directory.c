#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

#include "directory.h"
#include "inode.h"
#include "slist.h"

slist_t *get_directory_list(inode_t* dd);
int directory_delete_recursive(inode_t *dd);

int main_directory = -1;

void directory_init() {
    main_directory = directory_create();
}

int get_main_directory() {
    return main_directory;
}

int directory_create() {
    int dir_idx = alloc_inode();
    inode_t* dir = get_inode(dir_idx);
    dir->mode = 1;
    return dir_idx;
}

dirent_t** get_directory_blocks(inode_t *dd) {
    int num_dirs = needed_datablocks(dd->size, 0);

    dirent_t **dir_blocks = malloc(sizeof(dirent_t) * num_dirs);
    dir_blocks[0] = blocks_get_block(dd->block_0);

    if (num_dirs >= 2) {
        dir_blocks[1] = blocks_get_block(dd->block_1);
    }

    if (num_dirs >= 3) {
        int* datablocks = blocks_get_block(dd->rd_block);
        for (int i = 2; i < num_dirs - 2; i++) {
            dir_blocks[i] = blocks_get_block(datablocks[i]);
        }
    }

    return dir_blocks;
}


int directory_lookup(inode_t *dd, const char *name) {
    if (dd->mode == 0) {
        perror("Directory not found.\n");
        return -1;
    }

    int num_dirs = needed_datablocks(dd->size, 0);
    int num_entries = BLOCK_SIZE / sizeof(dirent_t);
    int total_entries = dd->size / sizeof(dirent_t);
    dirent_t** dir_blocks = get_directory_blocks(dd);

    for (int i = 0; i < num_dirs; i++) {
        dirent_t* current_dir_block = dir_blocks[i];
        for (int j = 0; j < num_entries && j < total_entries; j++) {
            dirent_t current_dir = current_dir_block[j];
            if (strcmp(current_dir.name, name) == 0) {
                return current_dir.inum;
            }
        }
        total_entries -= num_entries;
    }
    
    return -1;
}

int tree_lookup(const char *path) {
    if (strcmp(path, "/") == 0) {
        return main_directory;
    }
    char* copy = strdup(path);
    char *pch = strtok(copy, "/");
    
    inode_t* current_inode = get_inode(main_directory);
    int current_inode_idx = -1;

    while (pch != NULL) {
        current_inode_idx = directory_lookup(current_inode, pch);
        if (current_inode_idx == -1) {
            return -1;
        }
        current_inode = get_inode(current_inode_idx);
        pch = strtok (NULL, "/");
    }

    return current_inode_idx;
}

int directory_put(inode_t *dd, const char *name, int inum) {
    assert(dd->mode == 1);
    if (grow_inode(dd, sizeof(dirent_t)) == -1) {
        perror("Cannot allocate more memory.\n");
        return -1;
    }

    int num_dirs = needed_datablocks(dd->size, 0);
    
    dirent_t** dir_blocks = get_directory_blocks(dd);
    void* current_dir_block = (void*) dir_blocks[num_dirs - 1];
    int position = dd->size - sizeof(dirent_t) - (num_dirs - 1) * BLOCK_SIZE;
    dirent_t* next_dir = (dirent_t *)(current_dir_block + position);
    strcpy(next_dir->name, name);
    next_dir->inum = inum;

    return 0;
}

int directory_delete(inode_t *dd, const char *name) {
    int position = -1;
    int block = -1;
    int num_dirs = needed_datablocks(dd->size, 0);
    int num_entries = BLOCK_SIZE / sizeof(dirent_t);
    int total_entries = dd->size / sizeof(dirent_t);
    dirent_t** dir_blocks = get_directory_blocks(dd);

    for (int i = 0; i < num_dirs; i++) {
        dirent_t* current_dir_block = dir_blocks[i];
        for (int j = 0; j < num_entries && j < total_entries; j++) {
            dirent_t current_dir = current_dir_block[j];
            if (strcmp(current_dir.name, name) == 0) {
                block = i;
                position = j;
                int inode_no = current_dir.inum;
                inode_t* inode = get_inode(inode_no);
                if (inode->mode == 0) {
                    free_inode(0);
                }
                else {
                    directory_delete_recursive(inode);
                }
                break;
            }
        }
        total_entries -= num_entries;
    }

    if (position == -1) {
        perror("Directory not found.\n");
        return -1;
    }

    for (int i = position; i < num_entries - 1; i++) {
        dirent_t current_dir = dir_blocks[block][i];
        dirent_t next_dir= dir_blocks[block][i];
        current_dir.inum = next_dir.inum;
        strcpy(current_dir.name, next_dir.name);
    }

    if (block + 1 < num_dirs) {
        dirent_t last_dir = dir_blocks[block][num_entries - 1];
        dirent_t next_dir= dir_blocks[block+1][0];
        last_dir.inum = next_dir.inum;
        strcpy(last_dir.name, next_dir.name);
    }

    block += 1;

    while(block < num_dirs) {
        for (int i = 0; i < num_entries - 1; i++) {
            dirent_t current_dir = dir_blocks[block][i];
            dirent_t next_dir= dir_blocks[block][i];
            current_dir.inum = next_dir.inum;
            strcpy(current_dir.name, next_dir.name);
        }
        if (block + 1 < num_dirs) {
            dirent_t last_dir = dir_blocks[block][num_entries - 1];
            dirent_t next_dir= dir_blocks[block+1][0];
            last_dir.inum = next_dir.inum;
            strcpy(last_dir.name, next_dir.name);
        }

        block += 1;
    }

    return shrink_inode(dd, sizeof(dirent_t));
}

int directory_delete_recursive(inode_t *dd) {
    assert(dd->mode == 1);

    int num_dirs = needed_datablocks(dd->size, 0);
    int num_entries = BLOCK_SIZE / sizeof(dirent_t);
    int total_entries = dd->size / sizeof(dirent_t);
    dirent_t** dir_blocks = get_directory_blocks(dd);

    for (int i = 0; i < num_dirs; i++) {
        dirent_t* current_dir_block = dir_blocks[i];
        for (int j = 0; j < num_entries && j < total_entries; j++) {
            dirent_t current_dir = current_dir_block[j];
            int inode_no = current_dir.inum;
            inode_t* inode = get_inode(inode_no);
            if (inode->mode == 0) {
                free_inode(0);
            }
            else {
                directory_delete_recursive(inode);
            }
        }
        total_entries -= num_entries;
    }

    free_inode(dir_blocks[0][0].inum);
    
    return 0;
}

slist_t *directory_list(const char *path) {
    int dir_inode = tree_lookup(path);
    assert(dir_inode != -1);
    inode_t* dd = get_inode(dir_inode);
    return get_directory_list(dd);
}

slist_t *get_directory_list(inode_t* dd) {
    assert(dd->mode == 1);

    slist_t* list = NULL;

    int num_dirs = needed_datablocks(dd->size, 0);
    int num_entries = BLOCK_SIZE / sizeof(dirent_t);
    int total_entries = dd->size / sizeof(dirent_t);
    dirent_t** dir_blocks = get_directory_blocks(dd);

    for (int i = 0; i < num_dirs; i++) {
        dirent_t* current_dir_block = dir_blocks[i];
        for (int j = 0; j < num_entries && j < total_entries; j++) {
            dirent_t current_dir = current_dir_block[j];
            list = s_cons(current_dir.name, list);
        }
        total_entries -= num_entries;
    }
    
    return list;
}

void print_directory(inode_t *dd) {

    slist_t* list = get_directory_list(dd);
    if (list == NULL) {
        printf("0 files in directory.\n");
        return;
    }
    printf("-------------------------------\n");
    while (list != NULL) {
        printf("- %s\n", list->data);
        list = list->next;
    }
    
    printf("-------------------------------\n");
}
