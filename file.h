#ifndef FILE_H
#define FILE_H

#include "inode.h"
#include "blocks.h"

char** get_file_blocks(inode_t *file);
int read_file(inode_t* file, char* buf, size_t s, off_t offset);
int overwrite_file(inode_t *file, const char *str, size_t s, off_t offset);
#endif